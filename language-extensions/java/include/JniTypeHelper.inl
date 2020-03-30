//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JniTypeHelper.inl
//
// Purpose:
//  Inline functions for JniTypeHelper.h
//
//*********************************************************************
#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include <cstring> // Needed for memcpy in Linux

// The format used to log the session id (guid)
//
#define GuidFormat "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X"

// The format used for DATE data type
//
#define DateFormat "%04hd-%02hu-%02hu"

// The format used for TIMESTAMP data type
//
#define TimestampFormat "%04hd-%02hu-%02hu %02hu:%02hu:%02hu.%09u"

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CreateJniArray
//
// Description:
//	Specialized template function for creating double array in Java
//
template<>
inline jdoubleArray JniTypeHelper::CreateJniArray<jdoubleArray>(JNIEnv *env, jsize numRows)
{
	return env->NewDoubleArray(numRows);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CreateJniArray
//
// Description:
//	Specialized template function for creating int array in Java
//
template<>
inline jintArray JniTypeHelper::CreateJniArray<jintArray>(JNIEnv *env, jsize numRows)
{
	return env->NewIntArray(numRows);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CreateJniArray
//
// Description:
//	Specialized template function for creating boolean array in Java
//
template<>
inline jbooleanArray JniTypeHelper::CreateJniArray<jbooleanArray>(JNIEnv *env, jsize numRows)
{
	return env->NewBooleanArray(numRows);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CreateJniArray
//
// Description:
//	Specialized template function for creating short array in Java
//
template<>
inline jshortArray JniTypeHelper::CreateJniArray<jshortArray>(JNIEnv *env, jsize numRows)
{
	return env->NewShortArray(numRows);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CreateJniArray
//
// Description:
//	Specialized template function for creating float array in Java
//
template<>
inline jfloatArray JniTypeHelper::CreateJniArray<jfloatArray>(JNIEnv *env, jsize numRows)
{
	return env->NewFloatArray(numRows);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CreateJniArray
//
// Description:
//	Specialized template function for creating long array in Java
//
template<>
inline jlongArray JniTypeHelper::CreateJniArray<jlongArray>(JNIEnv *env, jsize numRows)
{
	return env->NewLongArray(numRows);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::ReleaseJniArrayElems
//
// Description:
//	Specialized template function for releasing long array in Java
//
template<>
inline void JniTypeHelper::ReleaseJniArrayElems<jlongArray, jlong>(JNIEnv *env, jlongArray jArray, jlong *jElems)
{
	env->ReleaseLongArrayElements(jArray, jElems, 0);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::ReleaseJniArrayElems
//
// Description:
//	Specialized template function for releasing int array in Java
//
template<>
inline void JniTypeHelper::ReleaseJniArrayElems<jintArray, jint>(JNIEnv *env, jintArray jArray, jint *jElems)
{
	env->ReleaseIntArrayElements(jArray, jElems, 0);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::ReleaseJniArrayElems
//
// Description:
//	Specialized template function for releasing double array in Java
//
template<>
inline void JniTypeHelper::ReleaseJniArrayElems<jdoubleArray, jdouble>(JNIEnv *env, jdoubleArray jArray, jdouble *jElems)
{
	env->ReleaseDoubleArrayElements(jArray, jElems, 0);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::ReleaseJniArrayElems
//
// Description:
//	Specialized template function for releasing short array in Java
//
template<>
inline void JniTypeHelper::ReleaseJniArrayElems<jshortArray, jshort>(JNIEnv *env, jshortArray jArray, jshort *jElems)
{
	env->ReleaseShortArrayElements(jArray, jElems, 0);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::ReleaseJniArrayElems
//
// Description:
//	Specialized template function for releasing boolean array in Java
//
template<>
inline void JniTypeHelper::ReleaseJniArrayElems<jbooleanArray, jboolean>(JNIEnv *env, jbooleanArray jArray, jboolean *jElems)
{
	env->ReleaseBooleanArrayElements(jArray, jElems, 0);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::ReleaseJniArrayElems
//
// Description:
//	Specialized template function for releasing float array in Java
//
template<>
inline void JniTypeHelper::ReleaseJniArrayElems<jfloatArray, jfloat>(JNIEnv *env, jfloatArray jArray, jfloat *jElems)
{
	env->ReleaseFloatArrayElements(jArray, jElems, 0);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::GetJniArrayElems
//
// Description:
//	Specialized template function for getting long array elements in Java
//
template<>
inline jlong* JniTypeHelper::GetJniArrayElems<jlongArray, jlong>(JNIEnv *env, jlongArray jArray)
{
	return env->GetLongArrayElements(jArray, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::GetJniArrayElems
//
// Description:
//	Specialized template function for getting int array elements in Java
//
template<>
inline jint* JniTypeHelper::GetJniArrayElems<jintArray, jint>(JNIEnv *env, jintArray jArray)
{
	return env->GetIntArrayElements(jArray, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::GetJniArrayElems
//
// Description:
//	Specialized template function for getting double array elements in Java
//
template<>
inline jdouble* JniTypeHelper::GetJniArrayElems<jdoubleArray, jdouble>(JNIEnv *env, jdoubleArray jArray)
{
	return env->GetDoubleArrayElements(jArray, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::GetJniArrayElems
//
// Description:
//	Specialized template function for getting short array elements in Java
//
template<>
inline jshort* JniTypeHelper::GetJniArrayElems<jshortArray, jshort>(JNIEnv *env, jshortArray jArray)
{
	return env->GetShortArrayElements(jArray, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::GetJniArrayElems
//
// Description:
//	Specialized template function for getting boolean array elements in Java
//
template<>
inline jboolean* JniTypeHelper::GetJniArrayElems<jbooleanArray, jboolean>(JNIEnv *env, jbooleanArray jArray)
{
	return env->GetBooleanArrayElements(jArray, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::GetJniArrayElems
//
// Description:
//	Specialized template function for getting float array elements in Java
//
template<>
inline jfloat* JniTypeHelper::GetJniArrayElems<jfloatArray, jfloat>(JNIEnv *env, jfloatArray jArray)
{
	return env->GetFloatArrayElements(jArray, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::GetSizeInBytes
//
// Description:
//	Specialized template function for getting the size in bytes of Java byte array
//
template<>
inline jsize JniTypeHelper::GetSizeInBytes<jbyteArray>(JNIEnv *env, jbyteArray value)
{
	return env->GetArrayLength(value);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::GetSizeInBytes
//
// Description:
//	Specialized template function for getting the size in bytes of Java Unicode string
//
template<>
inline jsize JniTypeHelper::GetSizeInBytes<jstring, false>(JNIEnv *env, jstring value)
{
	return env->GetStringLength(value) * sizeof(jchar);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::GetSizeInBytes
//
// Description:
//	Specialized template function for getting the size in bytes of Java UTF8 string
//
template<>
inline jsize JniTypeHelper::GetSizeInBytes<jstring, true>(JNIEnv *env, jstring value)
{
	jsize totalBytes = 0;

	// Get the byte array with the character set as UTF-8
	//
	jclass jClass = env->FindClass("java/lang/String");
	jstring jUtf8Label = env->NewStringUTF("UTF-8");
	JniHelper::ThrowOnJavaException(env);

	jmethodID jMethod = JniHelper::FindMethod(env,
											  jClass,
											  "getBytes",
											  "(Ljava/lang/String;)[B");

	jobject jUtf8Bytes = env->CallObjectMethod(value, jMethod, jUtf8Label);
	JniHelper::ThrowOnJavaException(env);

	totalBytes = env->GetArrayLength(static_cast<jbyteArray>(jUtf8Bytes));

	env->DeleteLocalRef(jClass);
	env->DeleteLocalRef(jUtf8Bytes);
	env->DeleteLocalRef(jUtf8Label);

	return totalBytes;
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CopyInputData
//
// Description:
//  Copies data value one by one into the Java array and make sure that the default value is set if
//  the value from SQL is null
//
template<typename jType, typename cType>
inline void JniTypeHelper::CopyInputData(
	JNIEnv				*env,
	jsize				numRows,
	const SQLPOINTER	values,
	const SQLINTEGER	*nullMap,
	jType				*jData)
{
	char defaultNullVal = 0;
	const cType *odbcData = static_cast<const cType*>(values);

	if (odbcData != nullptr)
	{
		// Convert data and populate java array
		//
		for (SQLULEN i = 0; i < numRows; ++i)
		{
			jData[i] = odbcData[i];
			if (nullMap != nullptr && nullMap[i] == SQL_NULL_DATA)
			{
				jData[i] = defaultNullVal;
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CopyUnicodeStringInputData
//
// Description:
//  Copy unicode string values one by one into the Java array
//
template<bool isUTF8>
inline void JniTypeHelper::CopyStringInputData(
	JNIEnv				*env,
	SQLULEN				numRows,
	SQLPOINTER			values,
	const SQLINTEGER	*nullMap,
	jobjectArray		jData)
{
	// The address to start reading the next value from
	//
	jbyte *pos = static_cast<jbyte*>(values);

	if (pos != nullptr && nullMap != nullptr)
	{
		for (jsize i = 0; i < numRows; ++i)
		{
			// The current string being copied
			//
			jstring jStr = nullptr;

			if (nullMap[i] != SQL_NULL_DATA)
			{
				assert(pos != nullptr);

				// The length (in bytes) of the current string
				//
				SQLINTEGER cbLen = nullMap[i];

				jStr = CreateString<isUTF8>(env, pos, cbLen);

				// Advance the pointer to the next string
				//
				pos += cbLen;
			}

			env->SetObjectArrayElement(jData, i, jStr);

			if (jStr != nullptr)
			{
				env->DeleteLocalRef(jStr);
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CreateString
//
// Description:
// Creates a Java string from a Unicode string and returns a local reference
//
// Note:
// The caller of this function is responsible for freeing the local reference returned
//
template<>
inline jstring JniTypeHelper::CreateString<false>(
	JNIEnv				*env,
	const SQLPOINTER	value,
	const jsize			len)
{
	const jchar *dataPos = reinterpret_cast<const jchar*>(value);

	// Create a new java string
	//
	jstring jStr = env->NewString(dataPos, len / sizeof(jchar));
	JniHelper::ThrowOnJavaException(env);

	return jStr;
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CreateString
//
// Description:
// Creates a Java string from a UTF-8 string and returns a local reference
//
// Note:
// JNI has a function NewStringUTF, but this uses Java modified UTF-8, which doesn't support
// all of the UTF-8 characters such as supplementary characters.
//
// The caller of this function is responsible for freeing the local reference returned
//
template<>
inline jstring JniTypeHelper::CreateString<true>(
	JNIEnv				*env,
	const SQLPOINTER	value,
	const jsize			len)
{
	jstring jStrResult = nullptr;
	jclass jClass = env->FindClass("java/lang/String");
	jstring jUtf8Label = env->NewStringUTF("UTF-8");
	JniHelper::ThrowOnJavaException(env);

	// Create a UTF-8 string by initializing a string from bytes with a character set of UTF-8
	//
	jbyteArray jByteArr = env->NewByteArray(len);
	JniHelper::ThrowOnJavaException(env);

	jbyte *jByteData = env->GetByteArrayElements(jByteArr, nullptr);

	// Copy the UTF-8 bytes into the java array
	//
	CopyInputData<jbyte, char>(env,
							   len,
							   reinterpret_cast<SQLPOINTER>(value),
							   nullptr /* nullMap */,
							   jByteData);

	env->ReleaseByteArrayElements(jByteArr, jByteData, 0);

	// Create the string from bytes with UTF-8 as the character set
	//
	jmethodID jMethod = JniHelper::FindMethod(env,
											  jClass,
											  "<init>",
											  "([BLjava/lang/String;)V");

	jStrResult = static_cast<jstring>(env->NewObject(jClass, jMethod, jByteArr, jUtf8Label));
	JniHelper::ThrowOnJavaException(env);

	env->DeleteLocalRef(jByteArr);
	env->DeleteLocalRef(jUtf8Label);
	env->DeleteLocalRef(jClass);

	return jStrResult;
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CopyBinaryInputData
//
// Description:
//  Copy binary string values one by one into the Java array
//
inline void JniTypeHelper::CopyBinaryInputData(
	JNIEnv				*env,
	SQLULEN				numRows,
	SQLPOINTER			values,
	const SQLINTEGER	*nullMap,
	jobjectArray		jData)
{
	// The address to start reading the next value from
	//
	jbyte *pos = static_cast<jbyte*>(values);

	if (pos != nullptr && nullMap != nullptr)
	{
		for (jsize i = 0; i < numRows; ++i)
		{
			// The current binary string being copied
			//
			jbyteArray jByteArray = nullptr;

			if (nullMap[i] != SQL_NULL_DATA)
			{
				assert(pos != nullptr);

				// The length (in bytes) of the current binary string
				//
				SQLINTEGER cbLen = nullMap[i];

				// Create a new java binary string
				//
				jByteArray = env->NewByteArray(cbLen);
				jbyte *jByteData = env->GetByteArrayElements(jByteArray, nullptr);

				// Copy the data into the java array and release
				//
				CopyInputData<jbyte, char>(env,
										   cbLen,
										   pos,
										   nullptr /* nullMap */,
										   jByteData);

				// Set the reference and release array for this value
				//
				env->ReleaseByteArrayElements(jByteArray, jByteData, 0);

				// Advance the pointer to the next value
				//
				pos += cbLen;
			}

			env->SetObjectArrayElement(jData, i, jByteArray);

			if (jByteArray != nullptr)
			{
				env->DeleteLocalRef(jByteArray);
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CopyOutputData
//
// Description:
//   Copy the data one by one out of the Java array into the array to return back to
//   ExtHost. Make sure to set the data size and set the number of output rows given
//   the length of the java array.
//
// Returns:
//	Pointer to the output data
//
template<typename jType, typename jArrayType, typename cType>
inline void* JniTypeHelper::CopyOutputData(
	JNIEnv		*env,
	jType		*outputColData,
	jArrayType	jArray,
	SQLULEN		&numRows,
	SQLULEN		&dataSizeInBytes)
{
	// Set the size of the column
	//
	dataSizeInBytes = sizeof(cType);
	cType *returnData = nullptr;

	// Get the array length and declare a new buffer to hold the return data
	//
	if (jArray != nullptr)
	{
		jint len = env->GetArrayLength(jArray);
		numRows = static_cast<SQLULEN>(len);
		returnData = new cType[len];

		for (jint i = 0; i < len; ++i)
		{
			returnData[i] = static_cast<cType>(outputColData[i]);
		}
	}
	else
	{
		numRows = 0;
	}

	return returnData;
}

//--------------------------------------------------------------------------------------------------
// Name: CopyStringOutputData
//
// Description:
//	Copy Unicode strings one by one out of the Java array into the array to return back to ExtHost.
//	This is done by iterating over all the strings in the java array while copying them to
//	the allocated buffer
//
template<>
inline void JniTypeHelper::CopyStringOutputData<false>(
	JNIEnv				*env,
	jobjectArray		source,
	jsize				numRows,
	unsigned long long	totalSizeInBytes,
	char				*target,
	SQLINTEGER			*nullMap)
{
	for (jsize i = 0; i < numRows; ++i)
	{
		// Get the string
		//
		jstring str =
			reinterpret_cast<jstring>(env->GetObjectArrayElement(source, i));

		if (str != nullptr)
		{
			// Get the size of the string
			//
			jsize strLenInChars = env->GetStringLength(str);

			SQLINTEGER strLenInBytes = static_cast<SQLINTEGER>(strLenInChars * sizeof(jchar));

			// Commit the string to the output buffer
			//
			env->GetStringRegion(str, 0, strLenInChars, reinterpret_cast<jchar*>(target));
			env->DeleteLocalRef(str);

			// Advance the pointer past the string
			//
			target += strLenInBytes;

			nullMap[i] = strLenInBytes;
		}
		else
		{
			nullMap[i] = SQL_NULL_DATA;
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: CopyStringOutputData
//
// Description:
//	Copy UTF-8 strings one by one out of the Java array into the array to return back to ExtHost.
//	This is done by iterating over all the strings in the java array while copying them to
//	the allocated buffer
//
template<>
inline void JniTypeHelper::CopyStringOutputData<true>(
	JNIEnv				*env,
	jobjectArray		source,
	jsize				numRows,
	unsigned long long	totalSizeInBytes,
	char				*target,
	SQLINTEGER			*nullMap)
{
	jclass jClass = env->FindClass("java/lang/String");
	jstring jUtf8Str = env->NewStringUTF("UTF-8");
	JniHelper::ThrowOnJavaException(env);

	jmethodID jMethod = JniHelper::FindMethod(env,
											  jClass,
											  "getBytes",
											  "(Ljava/lang/String;)[B");

	for (jsize i = 0; i < numRows; ++i)
	{
		// Get the string
		//
		jstring str =
			reinterpret_cast<jstring>(env->GetObjectArrayElement(source, i));

		if (str != nullptr)
		{
			jbyteArray jUtf8BytesArr =
				static_cast<jbyteArray>(env->CallObjectMethod(str, jMethod, jUtf8Str));
			JniHelper::ThrowOnJavaException(env);

			jsize jNumOfBytes = env->GetArrayLength(static_cast<jarray>(jUtf8BytesArr));

			SQLINTEGER strLenInBytes = static_cast<SQLINTEGER>(jNumOfBytes);

			// Commit the size of the string to the output buffer
			//
			nullMap[i] = strLenInBytes;

			// Commit the string to the output buffer
			//
			jbyte *byteData =
				static_cast<jbyte*>(env->GetPrimitiveArrayCritical(jUtf8BytesArr, nullptr));
			if (byteData)
			{
				memcpy(target,
					   byteData,
					   strLenInBytes);

				env->ReleasePrimitiveArrayCritical(jUtf8BytesArr, byteData, 0);
			}
			else
			{
				JniHelper::ThrowOnJavaException(env);
			}

			env->DeleteLocalRef(jUtf8BytesArr);
			env->DeleteLocalRef(str);

			// Advance the pointer past the string
			//
			target += strLenInBytes;
		}
		else
		{
			nullMap[i] = SQL_NULL_DATA;
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CopyBinaryOutputData
//
// Description:
//	Copy binary strings one by one out of the Java array into the array to return back to
//	ExtHost.
//	This is done by iterating over all the strings in the java array while copying them to
//	the allocated buffer
//
inline void JniTypeHelper::CopyBinaryOutputData(
	JNIEnv				*env,
	jobjectArray		source,
	jsize				numRows,
	unsigned long long	totalSizeInBytes,
	jbyte				*target,
	SQLINTEGER			*nullMap)
{
	for (jsize i = 0; i < numRows; ++i)
	{
		// Get the element
		//
		jbyteArray byteArray =
			reinterpret_cast<jbyteArray>(env->GetObjectArrayElement(source, i));

		if (byteArray != nullptr)
		{
			// Get the size of the element
			//
			jsize elemSizeInBytes = env->GetArrayLength(byteArray);

			// Commit the element to the output buffer
			//
			jbyte *byteData =
				static_cast<jbyte*>(env->GetPrimitiveArrayCritical(byteArray, nullptr));
			if (byteData)
			{
				memcpy(target,
					   byteData,
					   elemSizeInBytes);

				env->ReleasePrimitiveArrayCritical(byteArray, byteData, 0);
			}
			else
			{
				JniHelper::ThrowOnJavaException(env);
			}

			env->DeleteLocalRef(byteArray);

			// Advance the pointer past the binary string
			//
			target += elemSizeInBytes;

			nullMap[i] = elemSizeInBytes;
		}
		else
		{
			// The element is null. Commit the size as 0xff
			//
			nullMap[i] = SQL_NULL_DATA;
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CopyDateOutputData
//
// Description:
//	Copy date values one by one out of the Java array into the array to return back to
//	ExtHost.
//	This is done by iterating over all the dates in the java array and processing each
//	as follows:
//	1. Convert to string via toString() method
//	2. Parse the elements of the string (day, month, year) into the target SQL_DATE_STRUCT buffer
//
inline void JniTypeHelper::CopyDateOutputData(
	JNIEnv				*env,
	jobjectArray		source,
	jsize				numRows,
	SQL_DATE_STRUCT		*target,
	SQLINTEGER			*nullMap)
{
	jclass dateClass = env->FindClass("java/sql/Date");
	jmethodID dateToStringMethod = env->GetMethodID(dateClass,
													"toString",
													"()Ljava/lang/String;");
	jmethodID dateValueOfMethod = env->GetStaticMethodID(dateClass,
														 "valueOf",
														 "(Ljava/lang/String;)Ljava/sql/Date;");

	// Date.toString is a system function and should always be available
	//
	assert(dateToStringMethod);

	for (jsize i = 0; i < numRows; ++i)
	{
		// Get the element
		//
		jobject date = env->GetObjectArrayElement(source, i);

		if (date != nullptr)
		{
			JavaSqlDateToDateStruct(env,
									date,
									dateClass,
									dateToStringMethod,
									dateValueOfMethod,
									target[i]);

			env->DeleteLocalRef(date);

			nullMap[i] = 0;
		}
		else
		{
			// The element is null. Mark null map
			//
			nullMap[i] = SQL_NULL_DATA;
			memset(&target[i], 0, sizeof(SQL_DATE_STRUCT));
		}
	}

	env->DeleteLocalRef(dateClass);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::ConvertGuidToString
//
// Description:
//  Converts a SQLGUID to a string
//
// Returns:
//	string of the guid
//
inline std::string JniTypeHelper::ConvertGuidToString(const SQLGUID *guid)
{
	// 32 hex chars + 4 hyphens + null terminator, so 37 characters.
	//
	char guidString[37];
#ifdef _WIN64
	sprintf_s(guidString, sizeof(guidString) / sizeof(guidString[0]),
			  GuidFormat,
			  guid->Data1, guid->Data2, guid->Data3,
			  guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
			  guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
#elif __linux__
	snprintf(guidString, sizeof(guidString) / sizeof(guidString[0]),
			 GuidFormat,
			 static_cast<unsigned long>(guid->Data1), guid->Data2, guid->Data3,
			 guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
			 guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
#else
	throw "The current platform is not supported.";
#endif

	std::string s(guidString);
	return s;
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CopyGuidInputData
//
// Description:
//  Copy SQLGUID values as strings one by one into the Java array
//
inline void JniTypeHelper::CopyGuidInputData(
	JNIEnv				*env,
	SQLULEN				numRows,
	SQLPOINTER			values,
	const SQLINTEGER	*nullMap,
	jobjectArray		jArray)
{
	const SQLGUID *odbcData = static_cast<SQLGUID*>(values);

	for (jsize i = 0; i < numRows; ++i)
	{
		jstring str = nullptr;

		if (nullMap == nullptr || nullMap[i] != SQL_NULL_DATA)
		{
			// Convert the guid to string
			//
			std::string guidString = ConvertGuidToString(&(odbcData[i]));

			// Create the java string
			//
			str = env->NewStringUTF(guidString.c_str());
		}

		env->SetObjectArrayElement(jArray, i, str);

		if (str != nullptr)
		{
			env->DeleteLocalRef(str);
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CopyDateInputData
//
// Description:
//  Copy SQL_DATE_STRUCT values as java.sql.Date instances one by one into the Java array
//
inline void JniTypeHelper::CopyDateInputData(
	JNIEnv				*env,
	jclass				dateClass,
	SQLULEN				numRows,
	SQLPOINTER			values,
	const SQLINTEGER	*nullMap,
	jobjectArray		jArray)
{
	jmethodID dateValueOfMethod = env->GetStaticMethodID(dateClass,
														 "valueOf",
														 "(Ljava/lang/String;)Ljava/sql/Date;");

	// Date.valueOf is a system function and should always be available
	//
	assert(dateValueOfMethod);

	const SQL_DATE_STRUCT *odbcData = static_cast<SQL_DATE_STRUCT*>(values);

	for (jsize i = 0; i < numRows; ++i)
	{
		jobject dateJava = nullptr;

		if (nullMap == nullptr || nullMap[i] != SQL_NULL_DATA)
		{
			DateStructToJavaSqlDate(env,
									&(odbcData[i]),
									dateClass,
									dateValueOfMethod,
									dateJava);
		}

		env->SetObjectArrayElement(jArray, i, dateJava);

		if (dateJava != nullptr)
		{
			env->DeleteLocalRef(dateJava);
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::ODBCDateToJavaSqlDate
//
// Description:
//  Converts an ODBC SQL_DATE_STRUCT to a java.sql.Date object
//
// Returns:
//	A jobject containing a java.sql.Date instance
//
inline void JniTypeHelper::DateStructToJavaSqlDate(
	JNIEnv					*env,
	const SQL_DATE_STRUCT	*odbcDate,
	const jclass			dateClass,
	const jmethodID			dateValueOfMethod,
	jobject					&javaDate)
{
	// 4 numbers for year + 2 numbers for month + 2 numbers for day
	// + 2 hyphens + null terminator, overall 11 characters
	//
	char dateCString[11];

	// Convert the date to a C string
	//
	snprintf(dateCString,
			 sizeof(dateCString) / sizeof(dateCString[0]),
			 DateFormat,
			 odbcDate->year,
			 odbcDate->month,
			 odbcDate->day);

	// Convert the C string to java string
	//
	jstring dateJavaString = env->NewStringUTF(dateCString);

	// Convert the java string to java.sql.date object
	//
	javaDate = env->CallStaticObjectMethod(dateClass, dateValueOfMethod, dateJavaString);
	JniHelper::ThrowOnJavaException(env);

	env->DeleteLocalRef(dateJavaString);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::JavaSqlDateToDateStruct
//
// Description:
//  Converts a java.sql.Date object to an ODBC SQL_DATE_STRUCT
//
inline void JniTypeHelper::JavaSqlDateToDateStruct(
	JNIEnv				*env,
	jobject				jDate,
	jclass				dateClass,
	jmethodID			dateToStringMethod,
	jmethodID			dateValueOfMethod,
	SQL_DATE_STRUCT		&odbcDate)
{
	odbcDate = {0};

	// Convert the date to a java string
	//
	jstring dateJString =
		reinterpret_cast<jstring>(env->CallObjectMethod(jDate, dateToStringMethod));
	JniHelper::ThrowOnJavaException(env);

	// Sanity check:
	// Convert the string back to java.sql.date object to verify its validity
	//
	jobject jDateValidate = env->CallStaticObjectMethod(dateClass, dateValueOfMethod, dateJString);
	JniHelper::ThrowOnJavaException(env);

	// Commit the date fields to the output buffer
	//
	const char *dateCStr = env->GetStringUTFChars(dateJString, nullptr);
	JniHelper::ThrowOnJavaException(env);

	std::string dateStr(dateCStr);
	std::istringstream iss(dateStr);

	iss >> odbcDate.year;
	iss.ignore(1);	// ignore '-'
	iss >> odbcDate.month;
	iss.ignore(1);	// ignore '-'
	iss >> odbcDate.day;

	env->ReleaseStringUTFChars(dateJString, dateCStr);
	env->DeleteLocalRef(jDateValidate);
	env->DeleteLocalRef(dateJString);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CopyDateInputData
//
// Description:
//  Copy SQL_NUMERIC_STRUCT values as java.math.BigDecimal instances one by one into the Java array
//
inline void JniTypeHelper::CopyNumericInputData(
	JNIEnv				*env,
	jclass				bigDecimalClass,
	SQLULEN				numRows,
	SQLPOINTER			values,
	const SQLINTEGER	*nullMap,
	SQLSMALLINT			decimalDigits,
	jobjectArray		jArray)
{
	// Number of required references:
	// 1. BigInteger class
	// 2. 1 BigDecimal object allocated for each row
	// 3. 1 BigInteger object allocated for each row
	// 4. 1 jbyteArray allocated for each row
	//
	jint maxLocalReferences = 4;
	AutoJniLocalFrame jFrame(env, maxLocalReferences);

	jclass bigIntegerClass = env->FindClass("java/math/BigInteger");
	jmethodID bigIntegerCtor = JniHelper::FindMethod(env, bigIntegerClass, "<init>", "(I[B)V");
	jmethodID bigDecimalCtor = JniHelper::FindMethod(env,
													 bigDecimalClass,
													 "<init>",
													 "(Ljava/math/BigInteger;I)V");

	// Those are system functions and should always be available
	//
	assert(bigIntegerCtor);
	assert(bigDecimalCtor);

	const SQL_NUMERIC_STRUCT *odbcData = static_cast<SQL_NUMERIC_STRUCT*>(values);

	for (jsize i = 0; i < numRows; ++i)
	{
		jobject bigDecimalJava = nullptr;

		if (nullMap == nullptr || nullMap[i] != SQL_NULL_DATA)
		{
			NumericStructToBigDecimal(env,
									  &(odbcData[i]),
									  decimalDigits,
									  bigDecimalClass,
									  bigDecimalCtor,
									  bigIntegerClass,
									  bigIntegerCtor,
									  bigDecimalJava);
		}

		env->SetObjectArrayElement(jArray, i, bigDecimalJava);

		if (bigDecimalJava != nullptr)
		{
			env->DeleteLocalRef(bigDecimalJava);
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::ODBCDateToJavaSqlDate
//
// Description:
//  Converts an ODBC SQL_DATE_STRUCT to a java.sql.Date object
//
// Returns:
//	A jobject containing a java.math.BigDecimal instance
//
inline void JniTypeHelper::NumericStructToBigDecimal(
	JNIEnv						*env,
	const SQL_NUMERIC_STRUCT	*odbcNumeric,
	SQLSMALLINT					decimalDigits,
	const jclass				bigDecimalClass,
	const jmethodID				bigDecimalCtor,
	const jclass				bigIntegerClass,
	const jmethodID				bigIntegerCtor,
	jobject						&javaBigDecimal)
{
	int byteCount = 0;
	for (int i = 0; i < SQL_MAX_NUMERIC_LEN; ++i)
	{
		if (odbcNumeric->val[i] != 0)
		{
			byteCount = i + 1;
		}
	}

	// Create byte array for BigInteger ctor
	//
	jbyteArray jByteArray = env->NewByteArray(byteCount);
	jbyte *jByteData = static_cast<jbyte*>(env->GetPrimitiveArrayCritical(jByteArray, nullptr));

	if (jByteData)
	{
		// Copy data to byte array.
		//
		// For BigInteger, the byte array in big endian byte order.
		// In SQL_NUMERIC_STRUCT, the number stored in the val field is a scaled integer in little
		// endian order.
		// So to prepare the byte array for the BigInteger ctor, we flip the byte order from little
		// endian to big endian
		//
		for (size_t i = 0; i < byteCount; ++i)
		{
			jByteData[i] = odbcNumeric->val[byteCount - 1 - i];
		}

		// Set the reference and release array for this value
		//
		env->ReleasePrimitiveArrayCritical(jByteArray, jByteData, 0);
	}
	else
	{
		JniHelper::ThrowOnJavaException(env);
	}

	// Create BigInteger
	//
	jint signum = odbcNumeric->sign == 0 ? -1 : 1;
	jobject javaBigInteger = env->NewObject(bigIntegerClass, bigIntegerCtor, signum, jByteArray);
	JniHelper::ThrowOnJavaException(env);

	// Create BigDecimal
	//
	javaBigDecimal = env->NewObject(bigDecimalClass, bigDecimalCtor, javaBigInteger, decimalDigits);
	JniHelper::ThrowOnJavaException(env);

	env->DeleteLocalRef(jByteArray);
	env->DeleteLocalRef(javaBigInteger);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CopyNumericOutputData
//
// Description:
//	Copy date values one by one out of the Java array into the array to return back to
//	ExtHost.
//	This is done by iterating over all the dates in the java array and processing each
//	as follows:
//	1. Convert to string via toString() method
//	2. Parse the elements of the string (day, month, year) into the target SQL_DATE_STRUCT buffer
//
inline void JniTypeHelper::CopyNumericOutputData(
	JNIEnv				*env,
	jobjectArray		source,
	jsize				numRows,
	SQLCHAR				colPrecision,
	SQLSMALLINT			colScale,
	jint				colId,
	SQL_NUMERIC_STRUCT	*target,
	SQLINTEGER			*nullMap)
{
	// Number of required references:
	// 1. BigDecimal class
	// 2. BigInteger class
	// 3. 1 BigDecimal object allocated for each row
	//
	jint maxLocalReferences = 3;
	AutoJniLocalFrame jFrame(env, maxLocalReferences);

	jclass bigDecimalClass = env->FindClass("java/math/BigDecimal");
	jmethodID bigDecUnscaledValue = JniHelper::FindMethod(env,
														  bigDecimalClass,
														  "unscaledValue",
														  "()Ljava/math/BigInteger;");

	jclass bigIntegerClass = env->FindClass("java/math/BigInteger");
	jmethodID bigIntToByteArr = JniHelper::FindMethod(env, bigIntegerClass, "toByteArray", "()[B");
	jmethodID bigIntSignum = JniHelper::FindMethod(env, bigIntegerClass, "signum", "()I");
	jmethodID bigIntAbs = JniHelper::FindMethod(env,
												bigIntegerClass,
												"abs",
												"()Ljava/math/BigInteger;");

	for (jsize i = 0; i < numRows; ++i)
	{
		// Get the element
		//
		jobject bigDecimal = env->GetObjectArrayElement(source, i);

		if (bigDecimal != nullptr)
		{
			BigDecimalToNumericStruct(
				env,
				bigDecimal,
				colPrecision,
				colScale,
				bigDecUnscaledValue,
				bigIntToByteArr,
				bigIntSignum,
				bigIntAbs,
				"output column ID " + std::to_string(colId) + " and row ID " + std::to_string(i),
				target[i]);

			env->DeleteLocalRef(bigDecimal);

			nullMap[i] = 0;
		}
		else
		{
			// The element is null. Mark null map
			//
			nullMap[i] = SQL_NULL_DATA;
			memset(&target[i], 0, sizeof(SQL_NUMERIC_STRUCT));
		}
	}
}

//-----------------------------------------------------------------------------------------------
// Name: JniTypeHelper::BigDecimalToNumericStruct
//
// Description:
//  Converts a java.math.BigDecimal object to an ODBC SQL_NUMERIC_STRUCT
//
inline void JniTypeHelper::BigDecimalToNumericStruct(
	JNIEnv				*env,
	jobject				jBigDecimal,
	SQLCHAR				colPrecision,
	SQLSMALLINT			colScale,
	jmethodID			bigDecUnscaledValue,
	jmethodID			bigIntToByteArr,
	jmethodID			bigIntSignum,
	jmethodID			bigIntAbs,
	std::string			&&argDescription,
	SQL_NUMERIC_STRUCT	&odbcNumeric)
{
	// Number of required references:
	// 1. 1 BigInteger object allocated for each row
	// 2. 1 BigInteger object representing the absolute value allocated for each row
	// 3. 1 jbyteArray allocated for each row
	//
	jint maxLocalReferences = 3;
	AutoJniLocalFrame jFrame(env, maxLocalReferences);

	odbcNumeric = {0};

	// Commit the precision and scale fields to the output buffer
	//
	odbcNumeric.precision = colPrecision;
	odbcNumeric.scale = static_cast<SQLCHAR>(colScale);

	jobject jBigInteger = env->CallObjectMethod(jBigDecimal, bigDecUnscaledValue);
	JniHelper::ThrowOnJavaException(env);

	// Commit the sign field to the output buffer
	//
	jint signum = env->CallIntMethod(jBigInteger, bigIntSignum);
	JniHelper::ThrowOnJavaException(env);

	// Positive and 0 values are marked as positive
	//
	if (signum == 1 || signum == 0)
	{
		odbcNumeric.sign = 1;
	}

	// Prepare the val field for the output buffer:

	// Get the absolute value of the BigInteger
	//
	jobject jAbsBigInteger = env->CallObjectMethod(jBigInteger, bigIntAbs);
	JniHelper::ThrowOnJavaException(env);

	// Get the data
	//
	jbyteArray jByteArray =
		static_cast<jbyteArray>(env->CallObjectMethod(jAbsBigInteger, bigIntToByteArr));
	JniHelper::ThrowOnJavaException(env);

	jsize size = env->GetArrayLength(jByteArray);

	if (size > SQL_MAX_NUMERIC_LEN)
	{
		throw std::runtime_error("The numeric value in " + argDescription +
								 " is out of range for SQL Server numeric representation");
	}

	jbyte *jByteData = static_cast<jbyte*>(env->GetPrimitiveArrayCritical(jByteArray, nullptr));
	if (jByteData)
	{
		// Set the val field.
		//
		// For BigInteger, the byte array in big endian byte order.
		// In SQL_NUMERIC_STRUCT, the number stored in the val field is a scaled integer in little
		// endian order.
		// So to store the val field of the SQL_NUMERIC_STRUCT structure, we flip the byte order
		// we flip the byte order from big endian to little endian
		//
		for (jsize i = 0; i < size; ++i)
		{
			odbcNumeric.val[i] = jByteData[size - 1 - i];
		}

		// Set the reference and release array for this value
		//
		env->ReleasePrimitiveArrayCritical(jByteArray, jByteData, 0);
	}

	JniHelper::ThrowOnJavaException(env);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CopyTimestampInputData
//
// Description:
//  Copy SQL_TIMESTAMP_STRUCT values as java.sql.Timestamp instances one by one into the Java array
//
inline void JniTypeHelper::CopyTimestampInputData(
	JNIEnv				*env,
	jclass				timestampClass,
	SQLULEN				numRows,
	SQLPOINTER			values,
	const SQLINTEGER	*nullMap,
	jobjectArray		jArray)
{
	jmethodID tsValueOfMethod = env->GetStaticMethodID(timestampClass,
													   "valueOf",
													   "(Ljava/lang/String;)Ljava/sql/Timestamp;");

	// Timestamp.valueOf is a system function and should always be available
	//
	assert(tsValueOfMethod);

	const SQL_TIMESTAMP_STRUCT *odbcData = static_cast<SQL_TIMESTAMP_STRUCT*>(values);

	for (jsize i = 0; i < numRows; ++i)
	{
		jobject timestampJava = nullptr;

		if (nullMap == nullptr || nullMap[i] != SQL_NULL_DATA)
		{
			TimestampStructToJavaTimestamp(
				env,
				&(odbcData[i]),
				timestampClass,
				tsValueOfMethod,
				timestampJava);
		}

		env->SetObjectArrayElement(jArray, i, timestampJava);

		if (timestampJava != nullptr)
		{
			env->DeleteLocalRef(timestampJava);
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::TimestampStructToJavaTimestamp
//
// Description:
//  Converts an ODBC SQL_TIMESTAMP_STRUCT to a java.sql.Timestamp object
//
// Returns:
//	A jobject containing a java.sql.Timestamp instance
//
inline void JniTypeHelper::TimestampStructToJavaTimestamp(
	JNIEnv						*env,
	const SQL_TIMESTAMP_STRUCT	*odbcTimestamp,
	const jclass				timestampClass,
	const jmethodID				tsValueOfMethod,
	jobject						&javaTimestamp)
{
	// 4 numbers for year + 2 numbers for month + 2 numbers for day
	// + 2 hyphens + space + 2 numbers for hour + 2 numbers for minutes
	// + 2 numbers for seconds + 7 numbers for fractional seconds + 2 semicolons
	// + dot + null terminator, overall 11 characters
	//
	char timestampCString[28];

	// Convert the timestamp to a C string
	//
	snprintf(timestampCString,
			 sizeof(timestampCString) / sizeof(timestampCString[0]),
			 TimestampFormat,
			 odbcTimestamp->year,
			 odbcTimestamp->month,
			 odbcTimestamp->day,
			 odbcTimestamp->hour,
			 odbcTimestamp->minute,
			 odbcTimestamp->second,
			 odbcTimestamp->fraction);

	// Convert the C string to java string
	//
	jstring timestampJavaString = env->NewStringUTF(timestampCString);

	// Convert the java string to java.sql.timestamp object
	//
	javaTimestamp =
		env->CallStaticObjectMethod(timestampClass, tsValueOfMethod, timestampJavaString);
	JniHelper::ThrowOnJavaException(env);

	env->DeleteLocalRef(timestampJavaString);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::CopyTimestampOutputData
//
// Description:
//	Copy timestamp values one by one out of the Java array into the array to return back to
//	ExtHost.
//	This is done by iterating over all the timestamps in the java array and processing each
//	as follows:
//	1. Convert to string via toString() method
//	2. Parse the elements of the string into the target SQL_TIMESTAMP_STRUCT buffer
//
inline void JniTypeHelper::CopyTimestampOutputData(
	JNIEnv					*env,
	jobjectArray			source,
	jsize					numRows,
	SQL_TIMESTAMP_STRUCT	*target,
	SQLINTEGER				*nullMap)
{
	// Number of required references:
	// 1. Timestamp class
	// 3. 1 Timestamp object allocated for each row
	//
	jint maxLocalReferences = 2;
	AutoJniLocalFrame jFrame(env, maxLocalReferences);

	jclass timestampClass = env->FindClass("java/sql/Timestamp");

	jmethodID tsToStringMethod = JniHelper::FindMethod(env,
													   timestampClass,
													   "toString",
													   "()Ljava/lang/String;");
	jmethodID tsGetNanosMethod = JniHelper::FindMethod(env, timestampClass, "getNanos", "()I");

	jmethodID tsValueOfMethod = env->GetStaticMethodID(timestampClass,
													   "valueOf",
													   "(Ljava/lang/String;)Ljava/sql/Timestamp;");
	JniHelper::ThrowOnJavaException(env);

	for (jsize i = 0; i < numRows; ++i)
	{
		// Get the element
		//
		jobject timestamp = env->GetObjectArrayElement(source, i);

		if (timestamp != nullptr)
		{
			JavaTimestampToTimestampStruct(env,
										   timestamp,
										   timestampClass,
										   tsToStringMethod,
										   tsGetNanosMethod,
										   tsValueOfMethod,
										   target[i]);

			env->DeleteLocalRef(timestamp);

			nullMap[i] = 0;
		}
		else
		{
			// The element is null. Mark null map
			//
			nullMap[i] = SQL_NULL_DATA;
			memset(&target[i], 0, sizeof(SQL_TIMESTAMP_STRUCT));
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::JavaTimestampToTimestampStruct
//
// Description:
//  Converts a java.sql.Timestamp object to an ODBC SQL_TIMESTAMP_STRUCT
//
inline void JniTypeHelper::JavaTimestampToTimestampStruct(
	JNIEnv					*env,
	jobject					jTimestamp,
	jclass					tsClass,
	jmethodID				tsToStringMethod,
	jmethodID				tsGetNanosMethod,
	jmethodID				tsValueOfMethod,
	SQL_TIMESTAMP_STRUCT	&odbcTimestamp)
{
	// Number of required references:
	// 1. 1 jstring containing the timestamp's toString value
	// 2. 1 timestamp jobject containing the jstring's valueOf
	//
	jint maxLocalReferences = 2;
	AutoJniLocalFrame jFrame(env, maxLocalReferences);

	odbcTimestamp = {0};

	// Convert the timestamp to a java string
	//
	jstring timestampJString =
		reinterpret_cast<jstring>(env->CallObjectMethod(jTimestamp, tsToStringMethod));
	JniHelper::ThrowOnJavaException(env);

	// Sanity check:
	// Convert the string back to timestamp object to verify its validity
	//
	const jobject jTimestampValidate =
		env->CallStaticObjectMethod(tsClass, tsValueOfMethod, timestampJString);
	JniHelper::ThrowOnJavaException(env);

	jint nanos = env->CallIntMethod(jTimestamp, tsGetNanosMethod);
	JniHelper::ThrowOnJavaException(env);

	// Commit the timestamp fields to the output buffer
	//
	const char *timestampCStr = env->GetStringUTFChars(timestampJString, nullptr);
	JniHelper::ThrowOnJavaException(env);

	std::string timestampStr(timestampCStr);
	std::istringstream iss(timestampStr);

	iss >> odbcTimestamp.year;
	iss.ignore(1);	// ignore '-'
	iss >> odbcTimestamp.month;
	iss.ignore(1);	// ignore '-'
	iss >> odbcTimestamp.day;
	iss.ignore(1);	// ignore space
	iss >> odbcTimestamp.hour;
	iss.ignore(1);	// ignore ':'
	iss >> odbcTimestamp.minute;
	iss.ignore(1);	// ignore ':'
	iss >> odbcTimestamp.second;

	odbcTimestamp.fraction = nanos;

	env->ReleaseStringUTFChars(timestampJString, timestampCStr);
}

//--------------------------------------------------------------------------------------------------
// Name: JniTypeHelper::JavaStringToGuidStruct
//
// Description:
//  Converts a java string object that represents a guid to an ODBC SQLGUID struct
//
inline SQLGUID* JniTypeHelper::JavaStringToGuidStruct(
	JNIEnv			*env,
	const jstring	jStr,
	jclass			uuidClass,
	jmethodID		fromStringMethod,
	jmethodID		lsbMethod,
	jmethodID		msbMethod)
{
	std::unique_ptr<SQLGUID> odbcGuid(new SQLGUID());

	// Auto cleanup any local references
	// 1 reference for the UUID object
	//
	AutoJniLocalFrame jFrame(env, 1);

	// Convert the string to UUID object and make sure it's properly formatted
	//
	jobject uuidObj = env->CallStaticObjectMethod(uuidClass, fromStringMethod, jStr);
	JniHelper::ThrowOnJavaException(env);

	// Get the byte representation of the UUID - most and least significant bits
	//
	jlong msb = env->CallLongMethod(uuidObj, msbMethod);
	JniHelper::ThrowOnJavaException(env);

	jlong lsb = env->CallLongMethod(uuidObj, lsbMethod);
	JniHelper::ThrowOnJavaException(env);

	// Java uses big endian format, while SQL uses small endian.
	// So we need to reverse the byte order before sending to the server.
	//
	*reinterpret_cast<jint*>(&odbcGuid->Data1) = static_cast<jint>((msb >> 32) & 0xFFFFFFFF);
	*reinterpret_cast<jshort*>(&odbcGuid->Data2) = static_cast<jshort>((msb >> 16) & 0xFFFF);
	*reinterpret_cast<jshort*>(&odbcGuid->Data3) = static_cast<jshort>((msb & 0xFFFF));

	*reinterpret_cast<jlong*>(&odbcGuid->Data4) = lsb;
	char *iStart = reinterpret_cast<char*>(&odbcGuid->Data4);
	char *iEnd = iStart + sizeof(jlong);
	std::reverse(iStart, iEnd);

	return odbcGuid.release();
}
