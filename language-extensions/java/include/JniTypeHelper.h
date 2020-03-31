//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JniTypeHelper.h
//
// Purpose:
//  Helper for manipulating data through JNI
//
//*********************************************************************
#pragma once

#include "Common.h"

class JniTypeHelper
{
public:
	// Creates a JNI array
	//
	template<typename jArrayType>
	static jArrayType CreateJniArray(JNIEnv *env, jsize numRows);

	// Releases JNI array elements
	//
	template<typename jArrayType, typename jType>
	static void ReleaseJniArrayElems(JNIEnv *env, jArrayType jArray, jType *jElems);

	// Gets JNI array elements
	//
	template<typename jArrayType, typename jType>
	static jType* GetJniArrayElems(JNIEnv *env, jArrayType jArray);

	// Get length of type
	//
	template<typename jType, bool isUTF8 = false>
	static jsize GetSizeInBytes(JNIEnv *env, jType value);

	// Copies data to JNI
	//
	template<typename jType, typename cType>
	static void CopyInputData(
		JNIEnv           *env,
		jsize            numRows,
		const SQLPOINTER values,
		const SQLINTEGER *nullMap,
		jType            *jData);

	// Copies string data from JNI
	//
	template<bool isUTF8>
	static void CopyStringOutputData(
		JNIEnv             *env,
		jobjectArray       source,
		jsize              numRows,
		unsigned long long totalSizeInBytes,
		char               *target,
		SQLINTEGER         *nullMap);

	// Copies UTF-8 string data to JNI
	//
	template<bool isUTF8>
	static void CopyStringInputData(
		JNIEnv           *env,
		SQLULEN          numRows,
		SQLPOINTER       values,
		const SQLINTEGER *nullMap,
		jobjectArray     jArray);

	// Create a string object from value
	//
	template<bool isUTF8>
	static jstring CreateString(JNIEnv *env, const SQLPOINTER value, const jsize len);

	// Copies binary data to JNI
	//
	static void CopyBinaryInputData(
		JNIEnv           *env,
		SQLULEN          numRows,
		SQLPOINTER       values,
		const SQLINTEGER *nullMap,
		jobjectArray     jArray);

	// Copies GUID data to JNI
	//
	static void CopyGuidInputData(
		JNIEnv           *env,
		SQLULEN          numRows,
		SQLPOINTER       values,
		const SQLINTEGER *nullMap,
		jobjectArray     jArray);

	// Copies date data to JNI
	//
	static void CopyDateInputData(
		JNIEnv           *env,
		jclass           dateClass,
		SQLULEN          numRows,
		SQLPOINTER       values,
		const SQLINTEGER *nullMap,
		jobjectArray     jArray);

	// Copies numeric data to JNI
	//
	static void CopyNumericInputData(
		JNIEnv           *env,
		jclass           bigDecimalClass,
		SQLULEN          numRows,
		SQLPOINTER       values,
		const SQLINTEGER *nullMap,
		SQLSMALLINT      decimalDigits,
		jobjectArray     jArray);

	// Copies timestamp data to JNI
	//
	static void CopyTimestampInputData(
		JNIEnv           *env,
		jclass           timestampClass,
		SQLULEN          numRows,
		SQLPOINTER       values,
		const SQLINTEGER *nullMap,
		jobjectArray     jArray);

	// Copies data from JNI
	//
	template<typename jType, typename jArrayType, typename cType>
	static void* CopyOutputData(
		JNIEnv     *env,
		jType      *outputColData,
		jArrayType jArray,
		SQLULEN    &numRows,
		SQLULEN    &dataSizeInBytes);

	// Copies string data from JNI
	//
	static void CopyUnicodeStringOutputData(
		JNIEnv             *env,
		jobjectArray       source,
		jsize              numRows,
		unsigned long long totalSizeInChars,
		jchar              *target,
		SQLINTEGER         *nullMap);

	// Copies binary data from JNI
	//
	static void CopyBinaryOutputData(
		JNIEnv             *env,
		jobjectArray       source,
		jsize              numRows,
		unsigned long long totalSizeInBytes,
		jbyte              *target,
		SQLINTEGER         *nullMap);

	// Copies date data from JNI
	//
	static void CopyDateOutputData(
		JNIEnv          *env,
		jobjectArray    source,
		jsize           numRows,
		SQL_DATE_STRUCT *target,
		SQLINTEGER      *nullMap);

	// Copies numeric data from JNI
	//
	static void CopyNumericOutputData(
		JNIEnv             *env,
		jobjectArray       source,
		jsize              numRows,
		SQLCHAR            colPrecision,
		SQLSMALLINT        colScale,
		jint               colId,
		SQL_NUMERIC_STRUCT *target,
		SQLINTEGER         *nullMap);

	// Copies timestamp data from JNI
	//
	static void CopyTimestampOutputData(
		JNIEnv               *env,
		jobjectArray         source,
		jsize                numRows,
		SQL_TIMESTAMP_STRUCT *target,
		SQLINTEGER           *nullMap);

	// Converts a GUID to string
	//
	static std::string ConvertGuidToString(const SQLGUID *guid);

	// Convert a java string to an ODBC SQLGUID struct
	//
	static SQLGUID* JavaStringToGuidStruct(
		JNIEnv        *env,
		const jstring jStr,
		jclass        uuidClass,
		jmethodID     fromStringMethod,
		jmethodID     lsbMethod,
		jmethodID     msbMethod);

	// Converts an ODBC SQL_DATE_STRUCT to a java.sql.Date object
	//
	static void DateStructToJavaSqlDate(
		JNIEnv                *env,
		const SQL_DATE_STRUCT *odbcDate,
		const jclass          dateClass,
		const jmethodID       dateValueOfMethod,
		jobject               &javaDate);

	// Converts a java.sql.Date object to an ODBC SQL_DATE_STRUCT
	//
	static void JavaSqlDateToDateStruct(
		JNIEnv          *env,
		jobject         jDate,
		jclass          dateClass,
		jmethodID       dateToStringMethod,
		jmethodID       dateValueOfMethod,
		SQL_DATE_STRUCT &odbcDate);

	// Converts an ODBC SQL_NUMERIC_STRUCT to a java.math.BigDecimal object
	//
	static void NumericStructToBigDecimal(
		JNIEnv                   *env,
		const SQL_NUMERIC_STRUCT *odbcNumeric,
		SQLSMALLINT              decimalDigits,
		const jclass             bigDecimalClass,
		const jmethodID          bigDecimalCtor,
		const jclass             bigIntegerClass,
		const jmethodID          bigIntegerCtor,
		jobject                  &javaBigDecimal);

	// Converts a java.math.BigDecimal object to an ODBC SQL_NUMERIC_STRUCT
	//
	static void BigDecimalToNumericStruct(
		JNIEnv             *env,
		jobject            jBigDecimal,
		SQLCHAR            colPrecision,
		SQLSMALLINT        colScale,
		jmethodID          bigDecUnscaledValue,
		jmethodID          bigIntToByteArr,
		jmethodID          bigIntSignum,
		jmethodID          bigIntAbs,
		std::string        &&argDescription,
		SQL_NUMERIC_STRUCT &odbcNumeric);

	// Converts an ODBC SQL_TIMESTAMP_STRUCT to a java.sql.Timestamp object
	//
	static void TimestampStructToJavaTimestamp(
		JNIEnv                     *env,
		const SQL_TIMESTAMP_STRUCT *odbcTimestamp,
		const jclass               timestampClass,
		const jmethodID            tsValueOfMethod,
		jobject                    &javaTimestamp);

	// Converts a java.sql.Timestamp object to an ODBC SQL_TIMESTAMP_STRUCT
	//
	static void JavaTimestampToTimestampStruct(
		JNIEnv               *env,
		jobject              jTimestamp,
		jclass               tsClass,
		jmethodID            tsToStringMethod,
		jmethodID            tsGetNanosMethod,
		jmethodID            tsValueOfMethod,
		SQL_TIMESTAMP_STRUCT &odbcTimestamp);
};

#include "JniTypeHelper.inl"
