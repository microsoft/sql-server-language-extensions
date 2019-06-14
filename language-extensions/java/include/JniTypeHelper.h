//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: JniTypeHelper.h
// @Owner: brnieb
//
// Purpose:
//	 Helper for manipulating data through JNI
//
//*********************************************************************
#pragma once

class JniTypeHelper
{
public:
	// Creates a JNI array
	//
	template<typename jArrayType>
	static jArrayType CreateJniArray(_In_ JNIEnv *env, _In_ jsize numRows);

	// Releases JNI array elements
	//
	template<typename jArrayType, typename jType>
	static void ReleaseJniArrayElems(_In_ JNIEnv *env, _In_ jArrayType jArray, _In_ jType *jElems);

	// Gets JNI array elements
	//
	template<typename jArrayType, typename jType>
	static jType* GetJniArrayElems(_In_ JNIEnv *env, _In_ jArrayType jArray);

	// Get length of type
	//
	template<typename jType, bool isUTF8 = false>
	static jsize GetSizeInBytes(_In_ JNIEnv *env, _In_ jType value);

	// Copies data to JNI
	//
	template<typename jType, typename cType>
	static void CopyInputData(
		_In_ JNIEnv                         * env,
		_In_ jsize numRows,
		_In_reads_(numRows) const SQLPOINTER values,
		_In_reads_opt_(numRows) const SQLINTEGER  * nullMap,
		_Out_writes_(numRows) jType         * jData);

	// Copies string data from JNI
	//
	template<bool isUTF8>
	static void CopyStringOutputData(
		_In_ JNIEnv                             * env,
		_In_ jobjectArray source,
		_In_ jsize numRows,
		_In_ unsigned long long totalSizeInBytes,
		_Out_writes_(totalSizeInBytes) char    *target,
		_Out_writes_(numRows) SQLINTEGER        * nullMap);

	// Copies UTF-8 string data to JNI
	//
	template<bool isUTF8>
	static void CopyStringInputData(
		_In_ JNIEnv                                 * env,
		_In_ SQLULEN numRows,
		_In_reads_(numRows) SQLPOINTER values,
		_In_reads_opt_(numRows) const SQLINTEGER    * nullMap,
		_Inout_ jobjectArray jArray);

	// Create a string object from value
	//
	template<bool isUTF8>
	static jstring CreateString(
		_In_ JNIEnv			  *env,
		_In_ const SQLPOINTER value,
		_In_ const jsize	  len);

	// Copies binary data to JNI
	//
	static void CopyBinaryInputData(
		_In_ JNIEnv                                 * env,
		_In_ SQLULEN numRows,
		_In_reads_(numRows) SQLPOINTER values,
		_In_reads_opt_(numRows) const SQLINTEGER    * nullMap,
		_Inout_ jobjectArray jArray);

	// Copies GUID data to JNI
	//
	static void CopyGuidInputData(
		_In_ JNIEnv                                 * env,
		_In_ SQLULEN numRows,
		_In_reads_(numRows) SQLPOINTER values,
		_In_reads_opt_(numRows) const SQLINTEGER    * nullMap,
		_Inout_ jobjectArray jArray);

	// Copies date data to JNI
	//
	static void CopyDateInputData(
		_In_ JNIEnv                                 * env,
		_In_ jclass dateClass,
		_In_ SQLULEN numRows,
		_In_reads_(numRows) SQLPOINTER values,
		_In_reads_opt_(numRows) const SQLINTEGER    * nullMap,
		_Inout_ jobjectArray jArray);

	// Copies numeric data to JNI
	//
	static void CopyNumericInputData(
		_In_ JNIEnv                                 * env,
		_In_ jclass bigDecimalClass,
		_In_ SQLULEN numRows,
		_In_reads_(numRows) SQLPOINTER values,
		_In_reads_opt_(numRows) const SQLINTEGER    * nullMap,
		_In_ SQLSMALLINT decimalDigits,
		_Inout_ jobjectArray jArray);

	// Copies timestamp data to JNI
	//
	static void CopyTimestampInputData(
		_In_ JNIEnv                                 * env,
		_In_ jclass timestampClass,
		_In_ SQLULEN numRows,
		_In_reads_(numRows) SQLPOINTER values,
		_In_reads_opt_(numRows) const SQLINTEGER    * nullMap,
		_Inout_ jobjectArray jArray);

	// Copies data from JNI
	//
	template<typename jType, typename jArrayType, typename cType>
	static void* CopyOutputData(
		_In_ JNIEnv		*env,
		_In_ jType		*outputColData,
		_In_ jArrayType jArray,
		_Out_ SQLULEN	&numRows,
		_Out_ SQLULEN	&dataSizeInBytes);

	// Copies string data from JNI
	//
	static void CopyUnicodeStringOutputData(
		_In_ JNIEnv                             * env,
		_In_ jobjectArray source,
		_In_ jsize numRows,
		_In_ unsigned long long totalSizeInChars,
		_Out_writes_(totalSizeInChars) jchar    * target,
		_Out_writes_(numRows) SQLINTEGER        * nullMap);

	// Copies binary data from JNI
	//
	static void CopyBinaryOutputData(
		_In_ JNIEnv                             * env,
		_In_ jobjectArray source,
		_In_ jsize numRows,
		_In_ unsigned long long totalSizeInBytes,
		_Out_writes_bytes_(totalSizeInBytes) jbyte    * target,
		_Out_writes_(numRows) SQLINTEGER        * nullMap);

	// Copies date data from JNI
	//
	static void CopyDateOutputData(
		_In_ JNIEnv                             * env,
		_In_ jobjectArray source,
		_In_ jsize numRows,
		_Out_writes_(numRows) SQL_DATE_STRUCT * target,
		_Out_writes_(numRows) SQLINTEGER        * nullMap);

	// Copies numeric data from JNI
	//
	static void CopyNumericOutputData(
		_In_ JNIEnv                           * env,
		_In_ jobjectArray source,
		_In_ jsize numRows,
		_In_ SQLCHAR colPrecision,
		_In_ SQLSMALLINT colScale,
		_In_ jint colId,
		_Out_writes_(numRows) SQL_NUMERIC_STRUCT * target,
		_Out_writes_(numRows) SQLINTEGER      * nullMap);

	// Copies timestamp data from JNI
	//
	static void CopyTimestampOutputData(
		_In_ JNIEnv                             * env,
		_In_ jobjectArray source,
		_In_ jsize numRows,
		_Out_writes_(numRows) SQL_TIMESTAMP_STRUCT * target,
		_Out_writes_(numRows) SQLINTEGER        * nullMap);

	// Converts a GUID to string
	//
	static std::string ConvertGuidToString(_In_ const SQLGUID *guid);

	// Convert a java string to an ODBC SQLGUID struct
	//
	static SQLGUID* JavaStringToGuidStruct(
		_In_ JNIEnv		   *env,
		_In_ const jstring jStr,
		_In_ jclass		   uuidClass,
		_In_ jmethodID	   fromStringMethod,
		_In_ jmethodID	   lsbMethod,
		_In_ jmethodID	   msbMethod);

	// Converts an ODBC SQL_DATE_STRUCT to a java.sql.Date object
	//
	static void DateStructToJavaSqlDate(
		_In_ JNIEnv				   *env,
		_In_ const SQL_DATE_STRUCT *odbcDate,
		_In_ const jclass		   dateClass,
		_In_ const jmethodID	   dateValueOfMethod,
		_Out_ jobject			   &javaDate);

	// Converts a java.sql.Date object to an ODBC SQL_DATE_STRUCT
	//
	static void JavaSqlDateToDateStruct(
		_In_ JNIEnv			  *env,
		_In_ jobject		  jDate,
		_In_ jclass			  dateClass,
		_In_ jmethodID		  dateToStringMethod,
		_In_ jmethodID		  dateValueOfMethod,
		_Out_ SQL_DATE_STRUCT &odbcDate);

	// Converts an ODBC SQL_NUMERIC_STRUCT to a java.math.BigDecimal object
	//
	static void NumericStructToBigDecimal(
		_In_ JNIEnv					  *env,
		_In_ const SQL_NUMERIC_STRUCT *odbcNumeric,
		_In_ SQLSMALLINT			  decimalDigits,
		_In_ const jclass			  bigDecimalClass,
		_In_ const jmethodID		  bigDecimalCtor,
		_In_ const jclass			  bigIntegerClass,
		_In_ const jmethodID		  bigIntegerCtor,
		_Out_ jobject				  &javaBigDecimal);

	// Converts a java.math.BigDecimal object to an ODBC SQL_NUMERIC_STRUCT
	//
	static void BigDecimalToNumericStruct(
		_In_ JNIEnv				 *env,
		_In_ jobject			 jBigDecimal,
		_In_ SQLCHAR			 colPrecision,
		_In_ SQLSMALLINT		 colScale,
		_In_ jmethodID			 bigDecUnscaledValue,
		_In_ jmethodID			 bigIntToByteArr,
		_In_ jmethodID			 bigIntSignum,
		_In_ jmethodID			 bigIntAbs,
		_In_ jint				 colId,
		_In_ jsize				 rowId,
		_Out_ SQL_NUMERIC_STRUCT &odbcNumeric);

	// Converts an ODBC SQL_TIMESTAMP_STRUCT to a java.sql.Timestamp object
	//
	static void TimestampStructToJavaTimestamp(
		_In_ JNIEnv						*env,
		_In_ const SQL_TIMESTAMP_STRUCT *odbcTimestamp,
		_In_ const jclass				timestampClass,
		_In_ const jmethodID			tsValueOfMethod,
		_Out_ jobject					&javaTimestamp);

	// Converts a java.sql.Timestamp object to an ODBC SQL_TIMESTAMP_STRUCT
	//
	static void JavaTimestampToTimestampStruct(
		_In_ JNIEnv				   *env,
		_In_ jobject			   jTimestamp,
		_In_ jclass				   tsClass,
		_In_ jmethodID			   tsToStringMethod,
		_In_ jmethodID			   tsGetNanosMethod,
		_In_ jmethodID			   tsValueOfMethod,
		_Out_ SQL_TIMESTAMP_STRUCT &odbcTimestamp);
};

#include "JniTypeHelper.inl"
