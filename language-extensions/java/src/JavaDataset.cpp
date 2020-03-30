//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JavaDataset.cpp
//
// Purpose:
//  Handles loading and retrieving data from the dataset class
//
//*********************************************************************
#include "JavaDataset.h"
#include "JavaSqlTypeHelper.h"
#include "JniHelper.h"
#include "JniTypeHelper.h"
#include "Logger.h"
#include <assert.h>
#include <tuple>

using namespace std;

// Map of Dataset function names and signatures for adding a column to the dataset
//
const unordered_map<SQLSMALLINT, tuple<string, string>> x_addColumnFuncInfo =
{
	{static_cast<SQLSMALLINT>(SQL_C_BIT), make_tuple("addBooleanColumn", "(I[Z[Z)V")},
	{static_cast<SQLSMALLINT>(SQL_C_SLONG), make_tuple("addIntColumn", "(I[I[Z)V")},
	{static_cast<SQLSMALLINT>(SQL_C_DOUBLE), make_tuple("addDoubleColumn", "(I[D[Z)V")},
	{static_cast<SQLSMALLINT>(SQL_C_FLOAT), make_tuple("addFloatColumn", "(I[F[Z)V")},
	{static_cast<SQLSMALLINT>(SQL_C_SSHORT), make_tuple("addShortColumn", "(I[S[Z)V")},
	{static_cast<SQLSMALLINT>(SQL_C_UTINYINT), make_tuple("addShortColumn", "(I[S[Z)V")},
	{static_cast<SQLSMALLINT>(SQL_C_SBIGINT), make_tuple("addLongColumn", "(I[J[Z)V")},
	{static_cast<SQLSMALLINT>(SQL_C_CHAR),
	 make_tuple("addStringColumn", "(I[Ljava/lang/String;)V")},
	{static_cast<SQLSMALLINT>(SQL_C_WCHAR),
	 make_tuple("addStringColumn",  "(I[Ljava/lang/String;)V")},
	{static_cast<SQLSMALLINT>(SQL_C_BINARY), make_tuple("addBinaryColumn", "(I[[B)V")},
	{static_cast<SQLSMALLINT>(SQL_C_GUID),
	 make_tuple("addStringColumn",  "(I[Ljava/lang/String;)V")},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_DATE),
	 make_tuple("addDateColumn",  "(I[Ljava/sql/Date;)V")},
	{static_cast<SQLSMALLINT>(SQL_C_NUMERIC),
	 make_tuple("addNumericColumn",  "(I[Ljava/math/BigDecimal;)V")},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_TIMESTAMP),
	 make_tuple("addTimestampColumn",  "(I[Ljava/sql/Timestamp;)V")},
};

// Map of Dataset function names  and signatures for getting a column from the dataset
//
const unordered_map<SQLSMALLINT, tuple<string, string>> x_getColumnFuncInfo =
{
	{static_cast<SQLSMALLINT>(SQL_C_BIT), make_tuple("getBooleanColumn", "(I)[Z")},
	{static_cast<SQLSMALLINT>(SQL_C_SLONG), make_tuple("getIntColumn", "(I)[I")},
	{static_cast<SQLSMALLINT>(SQL_C_DOUBLE), make_tuple("getDoubleColumn", "(I)[D")},
	{static_cast<SQLSMALLINT>(SQL_C_FLOAT), make_tuple("getFloatColumn", "(I)[F")},
	{static_cast<SQLSMALLINT>(SQL_C_SSHORT), make_tuple("getShortColumn", "(I)[S")},
	{static_cast<SQLSMALLINT>(SQL_C_UTINYINT), make_tuple("getShortColumn", "(I)[S")},
	{static_cast<SQLSMALLINT>(SQL_C_SBIGINT), make_tuple("getLongColumn", "(I)[J")},
	{static_cast<SQLSMALLINT>(SQL_C_CHAR), make_tuple("getStringColumn", "(I)[Ljava/lang/String;")},
	{static_cast<SQLSMALLINT>(SQL_C_WCHAR),
	 make_tuple("getStringColumn",  "(I)[Ljava/lang/String;")},
	{static_cast<SQLSMALLINT>(SQL_C_BINARY), make_tuple("getBinaryColumn", "(I)[[B")},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_DATE), make_tuple("getDateColumn", "(I)[Ljava/sql/Date;")},
	{static_cast<SQLSMALLINT>(SQL_C_NUMERIC),
	 make_tuple("getNumericColumn", "(I)[Ljava/math/BigDecimal;")},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_TIMESTAMP),
	 make_tuple("getTimestampColumn", "(I)[Ljava/sql/Timestamp;")},
};

// Function signature map typedef
//
typedef unordered_map<SQLSMALLINT, tuple<string, string>> ColumnFnSignatureMap;

// Map of function pointers for adding a column
//
unordered_map<SQLSMALLINT, JavaDataset::fnAddColumn> JavaDataset::m_fnAddColumnMap =
{
	{static_cast<SQLSMALLINT>(SQL_C_BIT),
	 static_cast<fnAddColumn>(&JavaDataset::AddColumnInternal<jbooleanArray, jboolean, SQLCHAR>)},
	{static_cast<SQLSMALLINT>(SQL_C_SLONG),
	 static_cast<fnAddColumn>(&JavaDataset::AddColumnInternal<jintArray, jint, SQLINTEGER>)},
	{static_cast<SQLSMALLINT>(SQL_C_DOUBLE),
	 static_cast<fnAddColumn>(&JavaDataset::AddColumnInternal<jdoubleArray, jdouble, SQLDOUBLE>)},
	{static_cast<SQLSMALLINT>(SQL_C_FLOAT),
	 static_cast<fnAddColumn>(&JavaDataset::AddColumnInternal<jfloatArray, jfloat, SQLREAL>)},
	{static_cast<SQLSMALLINT>(SQL_C_SSHORT),
	 static_cast<fnAddColumn>(&JavaDataset::AddColumnInternal<jshortArray, jshort, SQLSMALLINT>)},
	{static_cast<SQLSMALLINT>(SQL_C_UTINYINT),
	 static_cast<fnAddColumn>(&JavaDataset::AddColumnInternal<jshortArray, jshort, SQLCHAR>)},
	{static_cast<SQLSMALLINT>(SQL_C_SBIGINT),
	 static_cast<fnAddColumn>(&JavaDataset::AddColumnInternal<jlongArray, jlong, SQLBIGINT>)},
	{static_cast<SQLSMALLINT>(SQL_C_CHAR),
	 static_cast<fnAddColumn>(&JavaDataset::AddStringColumnInternal<true>)},
	{static_cast<SQLSMALLINT>(SQL_C_WCHAR),
	 static_cast<fnAddColumn>(&JavaDataset::AddStringColumnInternal<false>)},
	{static_cast<SQLSMALLINT>(SQL_C_BINARY),
	 static_cast<fnAddColumn>(&JavaDataset::AddBinaryColumnInternal)},
	{static_cast<SQLSMALLINT>(SQL_C_GUID),
	 static_cast<fnAddColumn>(&JavaDataset::AddGuidColumnInternal)},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_DATE),
	 static_cast<fnAddColumn>(&JavaDataset::AddDateColumnInternal)},
	{static_cast<SQLSMALLINT>(SQL_C_NUMERIC),
	 static_cast<fnAddColumn>(&JavaDataset::AddNumericColumnInternal)},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_TIMESTAMP),
	 static_cast<fnAddColumn>(&JavaDataset::AddTimestampColumnInternal)},
};

// Map of function pointers for getting a column
//
unordered_map<SQLSMALLINT, JavaDataset::fnGetColumn> JavaDataset::m_fnGetColumnMap =
{
	{static_cast<SQLSMALLINT>(SQL_C_BIT),
	 static_cast<fnGetColumn>(&JavaDataset::GetColumnInternal<jbooleanArray, jboolean, SQLCHAR>)},
	{static_cast<SQLSMALLINT>(SQL_C_SLONG),
	 static_cast<fnGetColumn>(&JavaDataset::GetColumnInternal<jintArray, jint, SQLINTEGER>)},
	{static_cast<SQLSMALLINT>(SQL_C_DOUBLE),
	 static_cast<fnGetColumn>(&JavaDataset::GetColumnInternal<jdoubleArray, jdouble, SQLDOUBLE>)},
	{static_cast<SQLSMALLINT>(SQL_C_FLOAT),
	 static_cast<fnGetColumn>(&JavaDataset::GetColumnInternal<jfloatArray, jfloat, SQLREAL>)},
	{static_cast<SQLSMALLINT>(SQL_C_SSHORT),
	 static_cast<fnGetColumn>(&JavaDataset::GetColumnInternal<jshortArray, jshort, SQLSMALLINT>)},
	{static_cast<SQLSMALLINT>(SQL_C_UTINYINT),
	 static_cast<fnGetColumn>(&JavaDataset::GetColumnInternal<jshortArray, jshort, SQLCHAR>)},
	{static_cast<SQLSMALLINT>(SQL_C_SBIGINT),
	 static_cast<fnGetColumn>(&JavaDataset::GetColumnInternal<jlongArray, jlong, SQLBIGINT>)},
	{static_cast<SQLSMALLINT>(SQL_C_CHAR),
	 static_cast<fnGetColumn>(&JavaDataset::GetStringColumnInternal<true>)},
	{static_cast<SQLSMALLINT>(SQL_C_WCHAR),
	 static_cast<fnGetColumn>(&JavaDataset::GetStringColumnInternal<false>)},
	{static_cast<SQLSMALLINT>(SQL_C_BINARY),
	 static_cast<fnGetColumn>(&JavaDataset::GetBinaryColumnInternal)},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_DATE),
	 static_cast<fnGetColumn>(&JavaDataset::GetDateColumnInternal)},
	{static_cast<SQLSMALLINT>(SQL_C_NUMERIC),
	 static_cast<fnGetColumn>(&JavaDataset::GetNumericColumnInternal)},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_TIMESTAMP),
	 static_cast<fnGetColumn>(&JavaDataset::GetTimestampColumnInternal)},
};

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::CreateJniNullMap
//
// Description:
//	Creates a boolean array in JNI representing a column null map and returns a global reference.
//
jbooleanArray JavaDataset::CreateJniNullMap(jsize numRows, const SQLINTEGER *values)
{
	LOG("JavaDataset::CreateJniNullMap");
	jbooleanArray jNullMapArray = nullptr;

	// Create the JNI boolean array
	//
	jNullMapArray = JniTypeHelper::CreateJniArray<jbooleanArray>(m_env, numRows);
	jboolean *jData = JniTypeHelper::GetJniArrayElems<jbooleanArray, jboolean>(m_env,
																			   jNullMapArray);

	JniHelper::ThrowOnJavaException(m_env);

	// Convert data and populate java array
	//
	for (SQLULEN i = 0; i < numRows; ++i)
	{
		jData[i] = (values[i] == SQL_NULL_DATA) ? JNI_TRUE : JNI_FALSE;
	}

	JniTypeHelper::ReleaseJniArrayElems<jbooleanArray, jboolean>(m_env,
																 jNullMapArray,
																 jData);

	return jNullMapArray;
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::CreateSqlNullMap
//
// Description:
//	Creates a null map to send to SQL based on the column null map.
//
SQLINTEGER* JavaDataset::CreateSqlNullMap(SQLULEN numRows, jbooleanArray jArray)
{
	LOG("JavaDataset::CreateSqlNullMap");

	SQLINTEGER *nullMap = nullptr;

	if (jArray != nullptr)
	{
		jboolean *jData = JniTypeHelper::GetJniArrayElems<jbooleanArray, jboolean>(m_env, jArray);
		JniHelper::ThrowOnJavaException(m_env);

		nullMap = new SQLINTEGER[numRows];

		// Convert data and populate java array
		//
		for (SQLULEN i = 0; i < numRows; ++i)
		{
			nullMap[i] = (jData[i] == JNI_TRUE) ? SQL_NULL_DATA : 0;
		}

		JniTypeHelper::ReleaseJniArrayElems<jbooleanArray, jboolean>(m_env,
																	 jArray,
																	 jData);
	}

	return nullMap;
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::CalculateMaxForVarLengthOutputData
//
// Description:
//	Iterate over all the variable length values in the java array to calculate their combined
//	length and find the longest one.
//
template<typename jType, bool isUTF8>
void JavaDataset::CalculateMaxForVarLengthOutputData(
	jobjectArray		jArray,
	unsigned long long	&totalSizeInBytes,
	SQLULEN				&dataTypeSizeInBytes)
{
	LOG("JavaDataset::CalculateMaxForVarLengthOutputData");

	totalSizeInBytes = 0;
	dataTypeSizeInBytes = 0;

	// The size of the longest element
	//
	jsize maxSizeInBytes = 0;

	jsize numRows = m_env->GetArrayLength(jArray);

	// Iterate over all the java elements to calculate their combined length
	// and find the longest one
	//
	for (jsize i = 0; i < numRows; ++i)
	{
		jType jElem =
			reinterpret_cast<jType>(m_env->GetObjectArrayElement(jArray, i));
		if (jElem != nullptr)
		{
			// Get the size of the element
			//
			jsize elemSizeInBytes = JniTypeHelper::GetSizeInBytes<jType, isUTF8>(m_env, jElem);
			totalSizeInBytes += elemSizeInBytes;

			if (maxSizeInBytes < elemSizeInBytes)
			{
				maxSizeInBytes = elemSizeInBytes;
			}

			m_env->DeleteLocalRef(jElem);
		}
	}

	dataTypeSizeInBytes = maxSizeInBytes;
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::FindAddColumnMethod
//
// Description:
//	Finds the Java Dataset Add<type>Column function corresponding to the column type.
//
jmethodID JavaDataset::FindAddColumnMethod(SQLSMALLINT colType)
{
	LOG("JavaDataset::FindAddColumnMethod");

	// Find the function name for this type
	//
	ColumnFnSignatureMap::const_iterator it = x_addColumnFuncInfo.find(colType);

	if (it == x_addColumnFuncInfo.end())
	{
		throw runtime_error("Invalid column type encounted");
	}

	return FindDatasetMethod(get<0>(it->second), get<1>(it->second));
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::FindGetColumnMethod
//
// Description:
//	Finds the Java Dataset Get<type>Column function corresponding to the column type.
//
jmethodID JavaDataset::FindGetColumnMethod(SQLSMALLINT colType)
{
	LOG("JavaDataset::FindGetColumnMethod");

	// Find the function name for this type
	//
	ColumnFnSignatureMap::const_iterator it = x_getColumnFuncInfo.find(colType);

	if (it == x_getColumnFuncInfo.end())
	{
		throw runtime_error("Invalid column type encounted");
	}

	return FindDatasetMethod(get<0>(it->second), get<1>(it->second));
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::FindDatasetMethod
//
// Description:
//	Finds the Java Dataset method from the function name and signature.
//
jmethodID JavaDataset::FindDatasetMethod(const string &funcName, const string &funcSignature)
{
	LOG("JavaDataset::FindDatasetMethod");

	return JniHelper::FindMethod(m_env, m_class, funcName, funcSignature);
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::Init
//
// Description:
//	Initializes a empty dataset object from the class name provided
//
void JavaDataset::Init(JNIEnv *env, const string &className)
{
	LOG("JavaDataset::Init");

	if (env != nullptr)
	{
		m_env = env;

		// Max number of local references for this function are 2:
		// 1 for the dataset class
		// 1 for the object created
		//
		AutoJniLocalFrame jFrame(m_env, 2);

		jclass classLocalRef = m_env->FindClass(className.c_str());

		if (classLocalRef != nullptr)
		{
			m_class = static_cast<jclass>(m_env->NewGlobalRef(classLocalRef));

			if (m_class != nullptr)
			{
				jmethodID method = FindDatasetMethod("<init>", "()V");

				if (method != nullptr)
				{
					// Create the Dataset object
					//
					jobject data = m_env->NewObject(m_class, method);

					if (data != nullptr)
					{
						// Create a global reference on the new Dataset object
						//
						m_object = m_env->NewGlobalRef(data);

						if (m_object == nullptr)
						{
							throw runtime_error(
									"Could not create global reference for dataset object");
						}
					}
					else
					{
						JniHelper::ThrowOnJavaException(m_env, "Could not create dataset object");
						throw runtime_error("Could not create dataset object");
					}
				}
				else
				{
					JniHelper::ThrowOnJavaException(m_env, "Could not find dataset constructor");
					throw runtime_error("Could not find dataset constructor");
				}
			}
			else
			{
				throw runtime_error("Could not create global reference for dataset class");
			}
		}
		else
		{
			JniHelper::ThrowOnJavaException(m_env, "Could not find dataset class");
			throw runtime_error("Could not find dataset class");
		}
	}
	else
	{
		throw runtime_error("Invalid JNI enviroment");
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::Init
//
// Description:
//	Initializes a dataset object from a current Java object
//
void JavaDataset::Init(JNIEnv *env, jobject obj)
{
	LOG("JavaDataset::Init");

	if (env != nullptr)
	{
		m_env = env;

		// Max number of local references for this function are 2:
		// 1 for the dataset class
		// 1 for the object created
		//
		AutoJniLocalFrame jFrame(m_env, 2);

		jclass classLocalRef = m_env->GetObjectClass(obj);

		if (classLocalRef != nullptr)
		{
			m_class = static_cast<jclass>(m_env->NewGlobalRef(classLocalRef));

			if (m_class != nullptr)
			{
				// Create a global reference on the new Dataset object
				//
				m_object = m_env->NewGlobalRef(obj);

				if (m_object == nullptr)
				{
					throw runtime_error("Could not create global reference for dataset object");
				}
			}
			else
			{
				throw runtime_error("Could not create global reference for dataset class");
			}
		}
		else
		{
			throw runtime_error("Could not find dataset class");
		}
	}
	else
	{
		throw runtime_error("Could not create global reference for dataset class");
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::AddColumnMetadata
//
// Description:
//	Calls addColumnMetadata on the Dataset
//
void JavaDataset::AddColumnMetadata(
	jint			colId,
	const string	&colName,
	SQLSMALLINT		colType,
	SQLULEN			colSize,
	SQLSMALLINT		decimalDigits)
{
	LOG("JavaDataset::AddColumnMetadata");

	// Find the addColumnMetadata function
	// The signature for addColumnMetadata(int columnId, String columnName, int columnType, int precision, int scale)
	//
	jmethodID method = FindDatasetMethod("addColumnMetadata", "(ILjava/lang/String;III)V");

	jint javaSqlType = 0;

	if (JavaSqlTypeHelper::GetJavaSqlType(colType, javaSqlType))
	{
		const jstring name = m_env->NewStringUTF(colName.c_str());

		m_env->CallVoidMethod(m_object, method, colId, name, javaSqlType, colSize, decimalDigits);

		JniHelper::ThrowOnJavaException(m_env);
	}
	else
	{
		throw runtime_error("Unsupported column type encountered for column ID " +
							to_string(colId));
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::GetColumnType
//
// Description:
//	Calls getColumnType on the Dataset
//
void JavaDataset::GetColumnType(jint colId, SQLSMALLINT &colType)
{
	LOG("JavaDataset::GetColumnType");

	colType = 0;

	// Find the getColumnType method
	//
	jmethodID method = FindDatasetMethod("getColumnType", "(I)I");

	jint type = m_env->CallIntMethod(m_object, method, colId);

	JniHelper::ThrowOnJavaException(m_env);

	// Convert the java.sql.Type to ODBC C
	//
	if (!JavaSqlTypeHelper::GetOdbcType(type, colType))
	{
		throw runtime_error("Unsupported java.sql.Type encountered for column ID " +
							to_string(colId));
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::GetColumnPrecision
//
// Description:
//	Calls getColumnPrecision on the Dataset
//
void JavaDataset::GetColumnPrecision(jint colId, SQLULEN &colPrec)
{
	LOG("JavaDataset::GetColumnPrecision");

	colPrec = 0;

	// Find the getColumnPrecision method
	//
	jmethodID method = FindDatasetMethod("getColumnPrecision", "(I)I");

	jint precision = m_env->CallIntMethod(m_object, method, colId);
	JniHelper::ThrowOnJavaException(m_env);

	colPrec = precision;
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::GetColumnScale
//
// Description:
//	Calls getColumnScale on the Dataset
//
void JavaDataset::GetColumnScale(jint colId, SQLSMALLINT &colScale)
{
	LOG("JavaDataset::GetColumnScale");

	colScale = 0;

	// Find the getColumnPrecision method
	//
	jmethodID method = FindDatasetMethod("getColumnScale", "(I)I");

	jint scale = m_env->CallIntMethod(m_object, method, colId);
	JniHelper::ThrowOnJavaException(m_env);

	colScale = static_cast<SQLSMALLINT>(scale);
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::GetColumnInternal
//
// Description:
//	Internal template function to get the column data.
//
template<typename jArrayType, typename jType, typename cType>
void JavaDataset::GetColumnInternal(
	jint		colId,
	SQLSMALLINT	colType,
	SQLULEN		&numRows,
	SQLULEN		&dataSizeInBytes,
	SQLSMALLINT	&decimalDigits,
	SQLPOINTER	*data,
	SQLINTEGER	**nullMap)
{
	LOG("JavaDataset::GetColumnInternal");

	numRows = 0;
	dataSizeInBytes = 0;
	decimalDigits = 0;
	*data = nullptr;
	*nullMap = nullptr;

	// Get the column function for this column type
	//
	jmethodID method = FindGetColumnMethod(colType);

	jobject arr = m_env->CallObjectMethod(m_object, method, colId);

	JniHelper::ThrowOnJavaException(m_env);

	// If the result is not empty, then fill the output data
	//
	if (arr != nullptr)
	{
		jArrayType jArray = static_cast<jArrayType>(arr);

		jType *outputColData = JniTypeHelper::GetJniArrayElems<jArrayType, jType>(m_env,
																				  jArray);

		if (outputColData != nullptr)
		{
			// Copy the data from Java to the output data set
			//
			*data = JniTypeHelper::CopyOutputData<jType, jArrayType, cType>(
				m_env,
				outputColData,
				jArray,
				numRows,
				dataSizeInBytes);

			JniTypeHelper::ReleaseJniArrayElems<jArrayType, jType>(m_env,
																   jArray,
																   outputColData);
		}
		else
		{
			throw runtime_error("Could not get elements for output column ID " + to_string(colId));
		}
	}

	// If the operation was successful, and there are output rows then create the null map.
	//
	if (numRows > 0)
	{
		SQLULEN numNullMapRows = 0;

		GetColumnNullMap(colId, numNullMapRows, nullMap);

		if (numNullMapRows != 0 && numNullMapRows != numRows)
		{
			throw runtime_error(
					"Number of rows in null map does not equal number of rows in column data");
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::GetStringColumnInternal
//
// Description:
//	Internal function to get the string column data.
//
template<bool isUTF8>
void JavaDataset::GetStringColumnInternal(
	jint		colId,
	SQLSMALLINT	colType,
	SQLULEN		&numRows,
	SQLULEN		&dataSizeInBytes,
	SQLSMALLINT	&decimalDigits,
	SQLPOINTER	*data,
	SQLINTEGER	**nullMap)
{
	LOG("JavaDataset::GetStringColumnInternal");

	numRows = 0;
	dataSizeInBytes = 0;
	decimalDigits = 0;
	*data = nullptr;
	*nullMap = nullptr;

	// Get the column function for this column type
	//
	jmethodID method = FindGetColumnMethod(colType);

	jobject jArray = m_env->CallObjectMethod(m_object, method, colId);

	JniHelper::ThrowOnJavaException(m_env);

	if (jArray != nullptr)
	{
		jobjectArray stringArray = static_cast<jobjectArray>(jArray);

		// Get the array length
		//
		jsize jNumRows = m_env->GetArrayLength(stringArray);
		numRows = static_cast<SQLULEN>(jNumRows);

		// The required size of the buffer to return back to exthost.
		//
		unsigned long long totalSizeInBytes = 0;

		CalculateMaxForVarLengthOutputData<jstring, isUTF8>(stringArray,
															totalSizeInBytes,
															dataSizeInBytes);

		// Declare a new buffer to hold the return data
		//
		*data = new char[totalSizeInBytes];
		*nullMap = new SQLINTEGER[numRows];

		JniTypeHelper::CopyStringOutputData<isUTF8>(m_env,
													stringArray,
													jNumRows,
													totalSizeInBytes,
													static_cast<char*>(*data),
													*nullMap);
	}

	dataSizeInBytes = max(dataSizeInBytes, 2ul);
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::GetBinaryColumnInternal
//
// Description:
//	Internal function to get the binary column data.
//
void JavaDataset::GetBinaryColumnInternal(
	jint		colId,
	SQLSMALLINT	colType,
	SQLULEN		&numRows,
	SQLULEN		&dataSizeInBytes,
	SQLSMALLINT	&decimalDigits,
	SQLPOINTER	*data,
	SQLINTEGER	**nullMap)
{
	LOG("JavaDataset::GetBinaryColumnInternal");

	numRows = 0;
	dataSizeInBytes = 0;
	decimalDigits = 0;
	*data = nullptr;
	*nullMap = nullptr;

	// Get the column function for this column type
	//
	jmethodID method = FindGetColumnMethod(colType);

	jobject jArray = m_env->CallObjectMethod(m_object, method, colId);

	JniHelper::ThrowOnJavaException(m_env);

	if (jArray != nullptr)
	{
		jobjectArray byteArray = static_cast<jobjectArray>(jArray);

		// Get the array length
		//
		jsize jNumRows = m_env->GetArrayLength(byteArray);
		numRows = static_cast<SQLULEN>(jNumRows);

		// The required size of the buffer to return back to exthost.
		//
		unsigned long long totalSizeInBytes = 0;

		CalculateMaxForVarLengthOutputData<jbyteArray>(byteArray,
													   totalSizeInBytes,
													   dataSizeInBytes);

		// Declare a new buffer to hold the return data
		//
		*data = new jbyte[totalSizeInBytes];
		*nullMap = new SQLINTEGER[numRows];

		JniTypeHelper::CopyBinaryOutputData(m_env,
											byteArray,
											jNumRows,
											totalSizeInBytes,
											static_cast<jbyte*>(*data),
											*nullMap);
	}

	dataSizeInBytes = max(dataSizeInBytes, 2ul);
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::GetDateColumnInternal
//
// Description:
//	Internal function to get the date column data.
//
void JavaDataset::GetDateColumnInternal(
	jint		colId,
	SQLSMALLINT	colType,
	SQLULEN		&numRows,
	SQLULEN		&dataSizeInBytes,
	SQLSMALLINT	&decimalDigits,
	SQLPOINTER	*data,
	SQLINTEGER	**nullMap)
{
	LOG("JavaDataset::GetDateColumnInternal");

	numRows = 0;
	dataSizeInBytes = sizeof(SQL_DATE_STRUCT);
	decimalDigits = 0;
	*data = nullptr;
	*nullMap = nullptr;

	// Get the column function for this column type
	//
	jmethodID method = FindGetColumnMethod(colType);

	jobject jArray = m_env->CallObjectMethod(m_object, method, colId);
	JniHelper::ThrowOnJavaException(m_env);

	if (jArray != nullptr)
	{
		jobjectArray dateArray = static_cast<jobjectArray>(jArray);

		// Get the array length
		//
		jsize jNumRows = m_env->GetArrayLength(dateArray);
		numRows = static_cast<SQLULEN>(jNumRows);

		// Declare a new buffer to hold the return data
		//
		*data = new SQL_DATE_STRUCT[numRows];
		*nullMap = new SQLINTEGER[numRows];

		JniTypeHelper::CopyDateOutputData(m_env,
										  dateArray,
										  jNumRows,
										  static_cast<SQL_DATE_STRUCT*>(*data),
										  *nullMap);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::GetNumericColumnInternal
//
// Description:
//	Internal function to get the numeric column data.
//
void JavaDataset::GetNumericColumnInternal(
	jint		colId,
	SQLSMALLINT	colType,
	SQLULEN		&numRows,
	SQLULEN		&dataSizeInBytes,
	SQLSMALLINT	&decimalDigits,
	SQLPOINTER	*data,
	SQLINTEGER	**nullMap)
{
	LOG("JavaDataset::GetNumericColumnInternal");

	numRows = 0;
	*data = nullptr;
	*nullMap = nullptr;

	// Set the precision and scale
	//
	GetColumnPrecision(colId, dataSizeInBytes);
	GetColumnScale(colId, decimalDigits);

	// Get the column function for this column type
	//
	jmethodID method = FindGetColumnMethod(colType);

	jobject jArray = m_env->CallObjectMethod(m_object, method, colId);
	JniHelper::ThrowOnJavaException(m_env);

	if (jArray != nullptr)
	{
		jobjectArray bigDecimalArray = static_cast<jobjectArray>(jArray);

		// Get the array length
		//
		jsize jNumRows = m_env->GetArrayLength(bigDecimalArray);
		numRows = static_cast<SQLULEN>(jNumRows);

		// Declare a new buffer to hold the return data
		//
		*data = new SQL_NUMERIC_STRUCT[numRows];
		*nullMap = new SQLINTEGER[numRows];

		JniTypeHelper::CopyNumericOutputData(m_env,
											 bigDecimalArray,
											 jNumRows,
											 static_cast<SQLCHAR>(dataSizeInBytes),
											 decimalDigits,
											 colId,
											 static_cast<SQL_NUMERIC_STRUCT*>(*data),
											 *nullMap);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::GetTimestampColumnInternal
//
// Description:
//	Internal function to get the timestamp column data.
//
void JavaDataset::GetTimestampColumnInternal(
	jint		colId,
	SQLSMALLINT	colType,
	SQLULEN		&numRows,
	SQLULEN		&dataSizeInBytes,
	SQLSMALLINT	&decimalDigits,
	SQLPOINTER	*data,
	SQLINTEGER	**nullMap)
{
	LOG("JavaDataset::GetTimestampColumnInternal");

	numRows = 0;
	dataSizeInBytes = sizeof(SQL_TIMESTAMP_STRUCT);
	*data = nullptr;
	*nullMap = nullptr;

	// Set the scale
	//
	GetColumnScale(colId, decimalDigits);

	// Get the column function for this column type
	//
	jmethodID method = FindGetColumnMethod(colType);

	jobject jArray = m_env->CallObjectMethod(m_object, method, colId);
	JniHelper::ThrowOnJavaException(m_env);

	if (jArray != nullptr)
	{
		jobjectArray timestampArray = static_cast<jobjectArray>(jArray);

		// Get the array length
		//
		jsize jNumRows = m_env->GetArrayLength(timestampArray);
		numRows = static_cast<SQLULEN>(jNumRows);

		// Declare a new buffer to hold the return data
		//
		*data = new SQL_TIMESTAMP_STRUCT[numRows];
		*nullMap = new SQLINTEGER[numRows];

		JniTypeHelper::CopyTimestampOutputData(m_env,
											   timestampArray,
											   jNumRows,
											   static_cast<SQL_TIMESTAMP_STRUCT*>(*data),
											   *nullMap);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::GetColumnNullMap
//
// Description:
//	Calls getColumnNullMap on the Dataset and creates a SQL null map
//
void JavaDataset::GetColumnNullMap(jint colId, SQLULEN &numRows, SQLINTEGER **nullMap)
{
	LOG("JavaDataset::GetColumnNullMap");

	numRows = 0;
	*nullMap = nullptr;

	jmethodID method = FindDatasetMethod("getColumnNullMap", "(I)[Z");

	jbooleanArray jArray =
		static_cast<jbooleanArray>(m_env->CallObjectMethod(m_object, method, colId));

	JniHelper::ThrowOnJavaException(m_env);

	// No exception occurred and there is a null map, then create the null map to send to SQL
	//
	if (jArray != nullptr)
	{
		numRows = static_cast<SQLULEN>(m_env->GetArrayLength(jArray));

		*nullMap = CreateSqlNullMap(numRows, jArray);

		if (*nullMap == nullptr)
		{
			throw runtime_error("Failed to create null map for column ID " + to_string(colId));
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::AddColumns
//
// Description:
//	Adds metadata and data for each column supplied.
//
void JavaDataset::AddColumns(
	SQLUSMALLINT		numCols,
	SQLULEN				numRows,
	vector<SQLSMALLINT>	&colTypes,
	vector<string>		&colNames,
	vector<SQLULEN>		&colSizes,
	vector<SQLSMALLINT>	&decimalDigits,
	vector<SQLSMALLINT>	&nullableCols,
	SQLPOINTER			*data,
	SQLINTEGER			**nullMap)
{
	LOG("JavaDataset::AddColumns");

	jint numColumns = static_cast<jint>(numCols);

	// For each column there will be at max be 4 local references:
	// a. 1 for column name
	// b. 1 for the jArray storing the data
	// c. 1 for class
	// d. 1 for the object allocated for each row
	// e. 1 for a possible intermediate object to assist with the conversion
	// (d) and (e) are local references that are created per row, and should be deleted
	// after the row is populated. This max local reference will ensure this is enforced
	//
	jint maxLocalReferences = 5;
	for (jint i = 0; i < numColumns; ++i)
	{
		// To limit the number of local references, use one local frame
		// per column.
		//
		AutoJniLocalFrame jFrame(m_env, maxLocalReferences);

		const SQLINTEGER *colNullMap = nullptr;
		SQLPOINTER colData = nullptr;

		if (nullMap != nullptr)
		{
			colNullMap = nullMap[i];
		}

		if (data != nullptr)
		{
			colData = data[i];
		}

		AddColumnMetadata(i, colNames[i], colTypes[i], colSizes[i], decimalDigits[i]);

		AddColumn(i,
				  colTypes[i],
				  colSizes[i],
				  decimalDigits[i],
				  numRows,
				  colData,
				  colNullMap);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::GetColumns
//
// Description:
//	Gets metadata and data for all columns in the Dataset.
//
void JavaDataset::GetColumns(
	SQLUSMALLINT		&numCols,
	SQLULEN				&numRows,
	vector<SQLSMALLINT> &colTypes,
	vector<SQLULEN>		&colSizes,
	vector<SQLSMALLINT> &colDecimalDigits,
	vector<SQLSMALLINT> &nullableCols,
	vector<SQLPOINTER>  &data,
	vector<SQLINTEGER*> &nullMaps)
{
	LOG("JavaDataset::GetColumns");

	numCols = 0;
	numRows = 0;
	colTypes.clear();
	colSizes.clear();
	colDecimalDigits.clear();
	nullableCols.clear();
	data.clear();
	nullMaps.clear();

	jmethodID method = FindDatasetMethod("getColumnCount", "()I");

	jint numColumns = m_env->CallIntMethod(m_object, method);
	JniHelper::ThrowOnJavaException(m_env);

	// For each column there will be at max be 4 local references,
	// a. 1 for the jArray retrieving data from
	// b. 1 for the object allocated for each row
	//    This local reference for the row should be deleted after retrieving
	//    data from the row and this max local reference will ensure this is enforced
	// c. 2 for intermediate objects to assist with the conversion.
	//    For some data types, we need jni's help to perform type conversion.
	//    So allow 2 additional references (for the jclass, and the jobject it returns)
	//
	jint maxLocalReferences = 4;

	numCols = static_cast<SQLUSMALLINT>(numColumns);

	for (jint i = 0; i < numColumns; ++i)
	{
		AutoJniLocalFrame jFrame(m_env, maxLocalReferences);

		SQLPOINTER colData = nullptr;
		SQLINTEGER *colNullMap = nullptr;
		SQLULEN colSize = 0;
		SQLSMALLINT colType = 0;
		SQLSMALLINT colScale = 0;

		GetColumn(i, colType, colSize, colScale, numRows, &colData, &colNullMap);

		// Add the column to the results
		//
		colTypes.push_back(colType);
		colSizes.push_back(colSize);
		colDecimalDigits.push_back(colScale);

		data.push_back(colData);
		nullMaps.push_back(colNullMap);
		nullableCols.push_back(colNullMap != nullptr ? SQL_NULLABLE : SQL_NO_NULLS);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::AddColumnInternal
//
// Description:
//	Internal template function to add the column data.
//
template<typename jArrayType, typename jType, typename cType>
void JavaDataset::AddColumnInternal(
	jint				colId,
	SQLSMALLINT			colType,
	SQLULEN				colSize,
	SQLSMALLINT			decimalDigits,
	jsize				numRows,
	SQLPOINTER			data,
	const SQLINTEGER	*nullMap)
{
	LOG("JavaDataset::AddColumnInternal");

	jArrayType jArray = nullptr;
	jType *jData = nullptr;
	jbooleanArray jNullMapArray = nullptr;

	jArray = JniTypeHelper::CreateJniArray<jArrayType>(m_env, numRows);
	JniHelper::ThrowOnJavaException(m_env);

	jData = JniTypeHelper::GetJniArrayElems<jArrayType, jType>(m_env, jArray);
	if (jData != nullptr)
	{
		// Copy the data into the java array and release
		//
		JniTypeHelper::CopyInputData<jType, cType>(m_env,
												   numRows,
												   data,
												   nullMap,
												   jData);

		JniTypeHelper::ReleaseJniArrayElems<jArrayType, jType>(m_env, jArray, jData);

		// Create a corresponding null map for the current column
		//
		if (nullMap != nullptr)
		{
			jNullMapArray = CreateJniNullMap(numRows, nullMap);
			assert(jNullMapArray != nullptr);
		}

		jmethodID method = FindAddColumnMethod(colType);

		m_env->CallVoidMethod(m_object, method, colId, jArray, jNullMapArray);
		JniHelper::ThrowOnJavaException(m_env);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::AddStringColumnInternal
//
// Description:
//	Internal template function to add the string column data.
//
template<bool isUTF8>
void JavaDataset::AddStringColumnInternal(
	jint				colId,
	SQLSMALLINT			colType,
	SQLULEN				colSize,
	SQLSMALLINT			decimalDigits,
	jsize				numRows,
	SQLPOINTER			data,
	const SQLINTEGER	*nullMap)
{
	LOG("JavaDataset::AddStringColumnInternal");

	jclass stringClass = m_env->FindClass("java/lang/String");
	jobjectArray jArray = m_env->NewObjectArray(numRows, stringClass, nullptr);
	JniHelper::ThrowOnJavaException(m_env);

	JniTypeHelper::CopyStringInputData<isUTF8>(m_env,
											   numRows,
											   data,
											   nullMap,
											   jArray);

	jmethodID method = FindAddColumnMethod(colType);

	m_env->CallVoidMethod(m_object, method, colId, jArray);
	JniHelper::ThrowOnJavaException(m_env);
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::AddGuidColumnInternal
//
// Description:
//	Internal function to add the guid column data.
//
void JavaDataset::AddGuidColumnInternal(
	jint				colId,
	SQLSMALLINT			colType,
	SQLULEN				colSize,
	SQLSMALLINT			decimalDigits,
	jsize				numRows,
	SQLPOINTER			data,
	const SQLINTEGER	*nullMap)
{
	LOG("JavaDataset::AddGuidColumnInternal");

	jclass stringClass = m_env->FindClass("java/lang/String");
	jobjectArray jArray = m_env->NewObjectArray(numRows, stringClass, nullptr);
	JniHelper::ThrowOnJavaException(m_env);

	JniTypeHelper::CopyGuidInputData(m_env,
									 numRows,
									 data,
									 nullMap,
									 jArray);

	jmethodID method = FindAddColumnMethod(colType);

	m_env->CallVoidMethod(m_object, method, colId, jArray);
	JniHelper::ThrowOnJavaException(m_env);
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::AddBinaryColumnInternal
//
// Description:
//	Internal function to add the binary column data.
//
void JavaDataset::AddBinaryColumnInternal(
	jint				colId,
	SQLSMALLINT			colType,
	SQLULEN				colSize,
	SQLSMALLINT			decimalDigits,
	jsize				numRows,
	SQLPOINTER			data,
	const SQLINTEGER	*nullMap)
{
	LOG("JavaDataset::AddBinaryColumnInternal");

	jclass byteArrayClass = m_env->FindClass("[B");
	jobjectArray jArray = m_env->NewObjectArray(numRows, byteArrayClass, nullptr);
	JniHelper::ThrowOnJavaException(m_env);

	JniTypeHelper::CopyBinaryInputData(m_env,
									   numRows,
									   data,
									   nullMap,
									   jArray);

	jmethodID method = FindAddColumnMethod(colType);

	m_env->CallVoidMethod(m_object, method, colId, jArray);
	JniHelper::ThrowOnJavaException(m_env);
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::AddDateColumnInternal
//
// Description:
//	Internal function to add the date column data.
//
void JavaDataset::AddDateColumnInternal(
	jint				colId,
	SQLSMALLINT			colType,
	SQLULEN				colSize,
	SQLSMALLINT			decimalDigits,
	jsize				numRows,
	SQLPOINTER			data,
	const SQLINTEGER	*nullMap)
{
	LOG("JavaDataset::AddDateColumnInternal");

	jclass dateClass = m_env->FindClass("java/sql/Date");

	jobjectArray jArray = m_env->NewObjectArray(numRows, dateClass, nullptr);
	JniHelper::ThrowOnJavaException(m_env);

	JniTypeHelper::CopyDateInputData(m_env,
									 dateClass,
									 numRows,
									 data,
									 nullMap,
									 jArray);

	jmethodID method = FindAddColumnMethod(colType);

	m_env->CallVoidMethod(m_object, method, colId, jArray);
	JniHelper::ThrowOnJavaException(m_env);
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::AddNumericColumnInternal
//
// Description:
//	Internal function to add the numeric column data.
//
void JavaDataset::AddNumericColumnInternal(
	jint				colId,
	SQLSMALLINT			colType,
	SQLULEN				colSize,
	SQLSMALLINT			decimalDigits,
	jsize				numRows,
	SQLPOINTER			data,
	const SQLINTEGER	*nullMap)
{
	LOG("JavaDataset::AddNumericColumnInternal");

	jclass bigDecimalClass = m_env->FindClass("java/math/BigDecimal");

	jobjectArray jArray = m_env->NewObjectArray(numRows, bigDecimalClass, nullptr);
	JniHelper::ThrowOnJavaException(m_env);

	JniTypeHelper::CopyNumericInputData(m_env,
										bigDecimalClass,
										numRows,
										data,
										nullMap,
										decimalDigits,
										jArray);

	jmethodID method = FindAddColumnMethod(colType);

	m_env->CallVoidMethod(m_object, method, colId, jArray);
	JniHelper::ThrowOnJavaException(m_env);
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::AddTimestampColumnInternal
//
// Description:
//	Internal function to add the timestamp column data.
//
void JavaDataset::AddTimestampColumnInternal(
	jint				colId,
	SQLSMALLINT			colType,
	SQLULEN				colSize,
	SQLSMALLINT			decimalDigits,
	jsize				numRows,
	SQLPOINTER			data,
	const SQLINTEGER	*nullMap)
{
	LOG("JavaDataset::AddTimestampColumnInternal");

	jclass timestampClass = m_env->FindClass("java/sql/Timestamp");

	jobjectArray jArray = m_env->NewObjectArray(numRows, timestampClass, nullptr);
	JniHelper::ThrowOnJavaException(m_env);

	JniTypeHelper::CopyTimestampInputData(m_env,
										  timestampClass,
										  numRows,
										  data,
										  nullMap,
										  jArray);

	jmethodID method = FindAddColumnMethod(colType);

	m_env->CallVoidMethod(m_object, method, colId, jArray);
	JniHelper::ThrowOnJavaException(m_env);
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::AddColumn
//
// Description:
//	Adds the column to the Dataset.
//
void JavaDataset::AddColumn(
	jint				colId,
	SQLSMALLINT			colType,
	SQLULEN				colSize,
	SQLSMALLINT			decimalDigits,
	SQLULEN				numRows,
	SQLPOINTER const	data,
	SQLINTEGER const	*nullMap)
{
	LOG("JavaDataset::AddColumn");

	AddColumnFnMap::const_iterator it = m_fnAddColumnMap.find(colType);

	if (it == m_fnAddColumnMap.end())
	{
		throw runtime_error("Unsupported column type encountered");
	}

	(this->*it->second)(
		static_cast<jint>(colId),
		colType,
		colSize,
		decimalDigits,
		static_cast<jsize>(numRows),
		data,
		nullMap);
}

//--------------------------------------------------------------------------------------------------
// Name: JavaDataset::GetColumn
//
// Description:
//	Gets a column from the Dataset.
//
void JavaDataset::GetColumn(
	jint		colId,
	SQLSMALLINT	&colType,
	SQLULEN		&colSize,
	SQLSMALLINT	&colDecimalDigits,
	SQLULEN		&numRows,
	SQLPOINTER	*data,
	SQLINTEGER	**nullMap)
{
	LOG("JavaDataset::GetColumn");

	colType = 0;
	colSize = 0;
	colDecimalDigits = 0;
	numRows = 0;
	*data = nullptr;
	*nullMap = nullptr;

	GetColumnType(colId, colType);

	GetColumnFnMap::const_iterator it = m_fnGetColumnMap.find(colType);

	if (it == m_fnGetColumnMap.end())
	{
		throw runtime_error("Unsupported column type encountered");
	}

	(this->*it->second)(
		static_cast<jint>(colId),
		colType,
		numRows,
		colSize,
		colDecimalDigits,
		data,
		nullMap);
}
