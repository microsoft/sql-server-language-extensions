//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: JavaArgContainer.h
// @Owner: brnieb
//
// Purpose:
// Java extension input/output parameter wrappers, along with container
// for consolidating them.
//
//*********************************************************************
#ifdef _WIN64
#include <windows.h>
#endif
#include <assert.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <jni.h>
#include <sstream>
#ifndef _WIN64
#include <string.h>
// These sal include headers must follow the standard c++ headers, or there
// will be compilation issues. This is because headers like iostream/algorithm use
// variables like __in which are the same as a SAL annotation causing redefinition issues.
//
#include <sal_def.h>
#include <xplat_sal.h>
#endif
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include "Logger.h"
#include "JniHelper.h"
#include "JniTypeHelper.h"
#include "JavaArgContainer.h"

using namespace std;

typedef unordered_map<string, unique_ptr<JavaArg>> JavaArgMap;

//---------------------------------------------------------------------
// Name: JavaArg::JavaArg
//
// Description:
//	Constructor for the argument.
//
// Note:
//  Constructor may throw exceptions besides OOM.
//
JavaArg::JavaArg(
	_In_ SQLUSMALLINT id,
	_In_ SQLSMALLINT  type,
	_In_ SQLULEN	  size,
	_In_ SQLSMALLINT  decimalDigits,
	_In_ SQLPOINTER	  value,
	_In_ SQLINTEGER	  strLen_or_Ind,
	_In_ SQLSMALLINT  inputOutputType)
	: m_id(id),
	m_type(type),
	m_size(size),
	m_decimalDigits(decimalDigits),
	m_value(nullptr),
	m_strLenOrInd(strLen_or_Ind),
	m_inputOutputType(inputOutputType)
{
	// If the value is not null then copy it
	//
	if (value != nullptr)
	{
		DeepCopyValue(type, strLen_or_Ind, value);
	}
}

//---------------------------------------------------------------------
// Name: JavaArg::~JavaArg
//
// Description:
//	Destructor for the argument
//
JavaArg::~JavaArg()
{
	Cleanup();
}

//---------------------------------------------------------------------
// Name: JavaArg::DeepCopyValue
//
// Description:
//  Deep copies the input parameter, the memory supplied is not
//	guaranteed to live passed InitParam extension call.
//
void JavaArg::DeepCopyValue(
	_In_ SQLSMALLINT	  type,
	_In_ SQLINTEGER		  strLen_or_Ind,
	_In_ const SQLPOINTER value
	)
{
	switch (type)
	{
	case SQL_C_SLONG:
	{
		int *pValue = new int;
		*pValue = *(reinterpret_cast<const int*>(value));
		m_value = static_cast<SQLPOINTER>(pValue);

		break;
	}
	case SQL_C_BIT:
	{
		bool *pValue = new bool;
		*pValue = *(reinterpret_cast<const bool*>(value));
		m_value = static_cast<SQLPOINTER>(pValue);

		break;
	}
	case SQL_C_DOUBLE:
	{
		double *pValue = new double;
		*pValue = *(reinterpret_cast<const double*>(value));
		m_value = static_cast<SQLPOINTER>(pValue);

		break;
	}
	case SQL_C_FLOAT:
	{
		float *pValue = new float;
		*pValue = *(reinterpret_cast<const float*>(value));
		m_value = static_cast<SQLPOINTER>(pValue);

		break;
	}
	case SQL_C_SBIGINT:
	{
		long long *pValue = new long long;
		*pValue = *(reinterpret_cast<const long long*>(value));
		m_value = static_cast<SQLPOINTER>(pValue);

		break;
	}
	case SQL_C_UTINYINT:
	{
		unsigned char *pValue = new unsigned char;
		*pValue = *(reinterpret_cast<const unsigned char*>(value));
		m_value = static_cast<SQLPOINTER>(pValue);

		break;
	}
	case SQL_C_SSHORT:
	{
		short *pValue = new short;
		*pValue = *(reinterpret_cast<const short*>(value));
		m_value = static_cast<SQLPOINTER>(pValue);

		break;
	}
	case SQL_C_CHAR:
	case SQL_C_WCHAR:
	case SQL_C_BINARY:
	{
		char *pValue = new char[strLen_or_Ind];

		memcpy(pValue, value, strLen_or_Ind);
		m_value = static_cast<SQLPOINTER>(pValue);

		break;
	}
	case SQL_C_GUID:
	{
		SQLGUID *pValue = new SQLGUID;
		memcpy(pValue, value, sizeof(SQLGUID));
		m_value = static_cast<SQLPOINTER>(pValue);

		break;
	}
	case SQL_C_TYPE_DATE:
	{
		SQL_DATE_STRUCT *pValue = new SQL_DATE_STRUCT;
		memcpy(pValue, value, sizeof(SQL_DATE_STRUCT));
		m_value = static_cast<SQLPOINTER>(pValue);

		break;
	}
	case SQL_C_NUMERIC:
	{
		SQL_NUMERIC_STRUCT *pValue = new SQL_NUMERIC_STRUCT;
		memcpy(pValue, value, sizeof(SQL_NUMERIC_STRUCT));
		m_value = static_cast<SQLPOINTER>(pValue);

		break;
	}
	case SQL_C_TYPE_TIMESTAMP:
	{
		SQL_TIMESTAMP_STRUCT *pValue = new SQL_TIMESTAMP_STRUCT;
		memcpy(pValue, value, sizeof(SQL_TIMESTAMP_STRUCT));
		m_value = static_cast<SQLPOINTER>(pValue);

		break;
	}
	default:
		throw invalid_argument("Unsupported input parameter type");
	}
}

//---------------------------------------------------------------------
// Name: Cleanup
//
// Description:
//  Frees the memory used to store the argument
//
void JavaArg::Cleanup()
{
	string msg = "Cleaning up input arg #" + to_string(m_id);
	LOG(msg);

	if (m_value != nullptr)
	{
		switch (m_type)
		{
		case SQL_C_SLONG:
		{
			int *pValue = reinterpret_cast<int*>(m_value);
			delete pValue;

			break;
		}
		case SQL_C_BIT:
		{
			bool *pValue = reinterpret_cast<bool*>(m_value);
			delete pValue;

			break;
		}
		case SQL_C_DOUBLE:
		{
			double *pValue = reinterpret_cast<double*>(m_value);
			delete pValue;

			break;
		}
		case SQL_C_FLOAT:
		{
			float *pValue = reinterpret_cast<float*>(m_value);
			delete pValue;

			break;
		}
		case SQL_C_SBIGINT:
		{
			long long *pValue = reinterpret_cast<long long*>(m_value);
			delete pValue;

			break;
		}
		break;
		case SQL_C_UTINYINT:
		{
			unsigned char *pValue = reinterpret_cast<unsigned char*>(m_value);
			delete pValue;

			break;
		}
		case SQL_C_SSHORT:
		{
			short *pValue = reinterpret_cast<short*>(m_value);
			delete pValue;

			break;
		}
		case SQL_C_WCHAR:
		case SQL_C_CHAR:
		case SQL_C_BINARY:
		{
			char *pValue = reinterpret_cast<char*>(m_value);
			delete[] pValue;

			break;
		}
		case SQL_C_GUID:
		{
			SQLGUID *pValue = reinterpret_cast<SQLGUID*>(m_value);
			delete pValue;

			break;
		}
		case SQL_C_TYPE_DATE:
		{
			SQL_DATE_STRUCT *pValue = reinterpret_cast<SQL_DATE_STRUCT*>(m_value);
			delete pValue;

			break;
		}
		case SQL_C_NUMERIC:
		{
			SQL_NUMERIC_STRUCT *pValue = reinterpret_cast<SQL_NUMERIC_STRUCT*>(m_value);
			delete pValue;

			break;
		}
		case SQL_C_TYPE_TIMESTAMP:
		{
			SQL_TIMESTAMP_STRUCT *pValue = reinterpret_cast<SQL_TIMESTAMP_STRUCT*>(m_value);
			delete pValue;

			break;
		}
		default:
			break;
		}
	}

	m_value = nullptr;
}

//---------------------------------------------------------------------
// Name: JavaArgContainer::~JavaArgContainer
//
// Description:
//	Destructor for the argument container
//
JavaArgContainer::~JavaArgContainer()
{
	m_argMap.clear();
}

//---------------------------------------------------------------------
// Name: JavaArgContainer::Init
//
// Description:
//	Initialize the container
//
void JavaArgContainer::Init(_In_ const SQLUSMALLINT numOfArgs)
{
	m_argNames.resize(numOfArgs);
}

//---------------------------------------------------------------------
// Name: JavaArgContainer::AddArg
//
// Description:
//	Add an argument to the container
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN JavaArgContainer::AddArg(
	_In_ SQLUSMALLINT	  id,
	_In_ const SQLCHAR	  *paramName,
	_In_ SQLSMALLINT	  paramNameLength,
	_In_ SQLSMALLINT	  type,
	_In_ SQLULEN		  size,
	_In_ SQLSMALLINT	  decimalDigits,
	_In_ const SQLPOINTER value,
	_In_ SQLINTEGER		  strLen_or_Ind,
	_In_ SQLSMALLINT	  inputOutputType)
{
	SQLRETURN retVal = SQL_SUCCESS;

	try
	{
		if (id < m_argNames.size() && id >= 0)
		{
			// Parameters are named @param, but @ is invalid in Java naming,
			// so remove it.
			//
			string name(reinterpret_cast<const char*>((paramName + 1)));

			m_argNames[id] = name;

			JavaArgMap::iterator it = m_argMap.find(name);

			// Confirm the parameter with this name does not already exist
			//
			if (it == m_argMap.end())
			{
				unique_ptr<JavaArg> arg(new JavaArg(id,
													type,
													size,
													decimalDigits,
													value,
													strLen_or_Ind,
													inputOutputType));

				m_argMap.insert(pair<string, unique_ptr<JavaArg>>(name, move(arg)));
			}
			else
			{
				LOG_ERROR("Duplicate parameter supplied for " + m_argNames[id]);

				retVal = SQL_ERROR;
			}
		}
	}
	catch (const std::exception& e)
	{
		LOG_EXCEPTION(e);
		retVal = SQL_ERROR;
	}

	return retVal;
}

//---------------------------------------------------------------------
// Name: JavaArgContainer::GetArg
//
// Description:
//	Get the argument from the name
//
// Returns:
//	Pointer to the Java argument, or null if not found.
//
JavaArg* JavaArgContainer::GetArg(_In_ const std::string &name)
{
	JavaArg *arg = nullptr;

	JavaArgMap::iterator it = m_argMap.find(name);

	if (it != m_argMap.end())
	{
		arg = it->second.get();
	}

	return arg;
}

//---------------------------------------------------------------------
// Name: JavaArgContainer::GetArg
//
// Description:
//	Get the argument from the id
//
// Returns:
//	Pointer to the Java argument, or null if not found.
//
JavaArg* JavaArgContainer::GetArg(_In_ const SQLUSMALLINT id)
{
	JavaArg *arg = nullptr;

	if (id < m_argNames.size())
	{
		arg = GetArg(m_argNames[id]);
	}

	return arg;
}

//---------------------------------------------------------------------
// Name: JavaArgContainer::CreateArgMap
//
// Description:
//	Creates a HashMap in Java and populates with all the arguments
//
// Returns:
//	Global reference to the LinkedHashMap containing all the arguments
//
jobject JavaArgContainer::CreateArgMap(_In_ JNIEnv *env)
{
	SQLUSMALLINT count = GetCount();

	// Auto cleanup any local references
	// 1 reference for LinkedHashMap
	// 1 refernce for hash map created
	//
	AutoJniLocalFrame jFrame(env, 2);

	jclass mapClass = env->FindClass("java/util/LinkedHashMap");

	// Create a hash map to store the parameters
	//
	jmethodID init = JniHelper::FindMethod(env, mapClass, "<init>", "(I)V");
	jobject hashMap = env->NewObject(mapClass, init, count);

	// Check for exceptions
	//
	JniHelper::ThrowOnJavaException(env);

	jmethodID putMethod = JniHelper::FindMethod(env,
												mapClass,
												"put",
												"(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

	for (SQLUSMALLINT i = 0; i < count; ++i)
	{
		// Auto cleanup any local references for each parameter
		// 1 for the parameter name
		//
		// Up to 2 references may be created for each parameter value depending on type
		// 1 for the class
		// 1 for the object created
		//
		// For SQL_C_NUMERIC type, we create another 3 references for intermediate objects
		//
		AutoJniLocalFrame jFrameParam(env, 6);

		const JavaArg *arg = GetArg(i);
		const string &paramName = GetParamName(i);

		// Get the parameter name
		//
		const jstring paramStr = env->NewStringUTF(paramName.c_str());
		JniHelper::ThrowOnJavaException(env);

		jobject jObj = nullptr;

		if (arg->GetStrLenOrInd() != SQL_NULL_DATA)
		{
			// Create the Object
			//
			jObj = CreateJavaArgObject(env, arg);
		}

		// Put the value in the hash map
		//
		env->CallObjectMethod(hashMap, putMethod, paramStr, jObj);

		JniHelper::ThrowOnJavaException(env);
	}

	return env->NewGlobalRef(hashMap);
}

//---------------------------------------------------------------------
// Name: JavaArgContainer::CreateJavaArgObject
//
// Description:
//	Creates a Java object for the argument provided
//
// Returns:
//	Local reference to the jobject for the argument
//
jobject JavaArgContainer::CreateJavaArgObject(_In_ JNIEnv *env, _In_ const JavaArg *arg)
{
	jobject obj = nullptr;

	switch (arg->GetType())
	{
	case SQL_C_SLONG:
	{
		jint javaData = *(reinterpret_cast<const int*>(arg->GetValue()));

		jclass objectClass = env->FindClass("java/lang/Integer");

		jmethodID init = env->GetMethodID(objectClass, "<init>", "(I)V");

		obj = env->NewObject(objectClass, init, javaData);

		break;
	}
	case SQL_C_BIT:
	{
		jboolean javaData = *(reinterpret_cast<const bool*>(arg->GetValue()));

		jclass objectClass = env->FindClass("java/lang/Boolean");
		jmethodID init = env->GetMethodID(objectClass, "<init>", "(Z)V");

		obj = env->NewObject(objectClass, init, javaData);

		break;
	}
	case SQL_C_DOUBLE:
	{
		jdouble javaData = *(reinterpret_cast<const double*>(arg->GetValue()));

		jclass objectClass = env->FindClass("java/lang/Double");
		jmethodID init = env->GetMethodID(objectClass, "<init>", "(D)V");

		obj = env->NewObject(objectClass, init, javaData);

		break;
	}
	case SQL_C_FLOAT:
	{
		jfloat javaData = *(reinterpret_cast<const float*>(arg->GetValue()));

		jclass objectClass = env->FindClass("java/lang/Float");
		jmethodID init = env->GetMethodID(objectClass, "<init>", "(F)V");

		obj = env->NewObject(objectClass, init, javaData);

		break;
	}

	case SQL_C_SBIGINT:
	{
		jlong javaData = *(reinterpret_cast<const long long*>(arg->GetValue()));

		jclass objectClass = env->FindClass("java/lang/Long");
		jmethodID init = env->GetMethodID(objectClass, "<init>", "(J)V");

		obj = env->NewObject(objectClass, init, javaData);

		break;
	}
	case SQL_C_UTINYINT:
	{
		const unsigned char data = *(reinterpret_cast<const unsigned char*>(arg->GetValue()));
		jshort javaData = static_cast<short>(data);

		jclass objectClass = env->FindClass("java/lang/Short");
		jmethodID init = env->GetMethodID(objectClass, "<init>", "(S)V");

		obj = env->NewObject(objectClass, init, javaData);

		break;
	}
	case SQL_C_SSHORT:
	{
		jshort javaData = *(reinterpret_cast<const short*>(arg->GetValue()));

		jclass objectClass = env->FindClass("java/lang/Short");
		jmethodID init = env->GetMethodID(objectClass, "<init>", "(S)V");

		obj = env->NewObject(objectClass, init, javaData);

		break;
	}
	case SQL_C_CHAR:
	{
		obj = JniTypeHelper::CreateString<true>(env, arg->GetValue(), arg->GetStrLenOrInd());

		break;
	}
	case SQL_C_WCHAR:
	{
		// Get the length of the unicode string, the size is the total number of bytes and
		// unicode characters are two bytes wide.
		//
		jsize strLen = static_cast<jsize>(arg->GetStrLenOrInd() / sizeof(jchar));
		const jchar *data = reinterpret_cast<const jchar*>(arg->GetValue());

		obj = static_cast<jobject>(env->NewString(data, strLen));

		break;
	}
	case SQL_C_BINARY:
	{
		jsize jSize = static_cast<jsize>(arg->GetStrLenOrInd());

		// Create a new java binary string
		//
		jbyteArray jArr = env->NewByteArray(jSize);

		// Copy the data into the java array
		//
		env->SetByteArrayRegion(jArr, 0, jSize, static_cast<jbyte*>(arg->GetValue()));

		obj = static_cast<jobject>(jArr);

		break;
	}
	case SQL_C_GUID:
	{
		const SQLGUID *odbcGuid = reinterpret_cast<const SQLGUID*>(arg->GetValue());

		// Convert the guid to string
		//
		string guidString = JniTypeHelper::ConvertGuidToString(odbcGuid);

		obj = static_cast<jobject>(env->NewStringUTF(guidString.c_str()));

		break;
	}
	case SQL_C_TYPE_DATE:
	{
		const SQL_DATE_STRUCT *odbcDate = reinterpret_cast<const SQL_DATE_STRUCT*>(arg->GetValue());

		// Convert the ODBC date to java.sql.date
		//
		jclass dateClass = env->FindClass("java/sql/Date");
		jmethodID dateMethod = env->GetStaticMethodID(dateClass,
													  "valueOf",
													  "(Ljava/lang/String;)Ljava/sql/Date;");

		JniTypeHelper::DateStructToJavaSqlDate(env,
											   odbcDate,
											   dateClass,
											   dateMethod,
											   obj);

		break;
	}
	case SQL_C_NUMERIC:
	{
		const SQL_NUMERIC_STRUCT *odbcNumeric = static_cast<SQL_NUMERIC_STRUCT*>(arg->GetValue());

		jclass bigDecimalClass = env->FindClass("java/math/BigDecimal");
		jclass bigIntegerClass = env->FindClass("java/math/BigInteger");
		jmethodID bigIntegerCtor = JniHelper::FindMethod(env, bigIntegerClass, "<init>", "(I[B)V");
		jmethodID bigDecimalCtor = JniHelper::FindMethod(env,
														 bigDecimalClass,
														 "<init>",
														 "(Ljava/math/BigInteger;I)V");

		JniTypeHelper::NumericStructToBigDecimal(env,
												 odbcNumeric,
												 arg->GetDecimalDigits(),
												 bigDecimalClass,
												 bigDecimalCtor,
												 bigIntegerClass,
												 bigIntegerCtor,
												 obj);

		break;
	}
	case SQL_C_TYPE_TIMESTAMP:
	{
		const SQL_TIMESTAMP_STRUCT *odbcTimestamp =
			static_cast<SQL_TIMESTAMP_STRUCT*>(arg->GetValue());

		jclass timestampClass = env->FindClass("java/sql/Timestamp");
		jmethodID tsValueOfMethod = env->GetStaticMethodID(timestampClass,
														   "valueOf",
														   "(Ljava/lang/String;)Ljava/sql/Timestamp;");

		JniTypeHelper::TimestampStructToJavaTimestamp(env,
													  odbcTimestamp,
													  timestampClass,
													  tsValueOfMethod,
													  obj);

		break;
	}
	default:
		throw runtime_error("Unsupported parameter type encountered");
	}

	return obj;
}