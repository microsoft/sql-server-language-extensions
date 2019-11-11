//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JavaSqlTypeHelper.h
//
// Purpose:
//  Handles converting ODBC C type to JDBC type
//
//*********************************************************************
#include "JavaSqlTypeHelper.h"

using namespace std;

// Constant values for SQL types defined in java.sql.Types
//
const int x_javaSqlTinyInt = -6;
const int x_javaSqlSmallInt = 5;
const int x_javaSqlBit = -7;
const int x_javaSqlInteger = 4;
const int x_javaSqlBigInt = -5;
const int x_javaSqlReal = 7;
const int x_javaSqlDouble = 8;
const int x_javaSqlNVarChar = -9;
const int x_javaSqlNChar = -15;
const int x_javaSqlVarChar = 12;
const int x_javaSqlChar = 1;
const int x_javaSqlVarBinary = -3;
const int x_javaSqlDate = 91;
const int x_javaSqlNumeric = 2;
const int x_javaSqlDecimal = 3;
const int x_javaSqlTimestamp = 93;

using namespace std;

// Maps the ODBC C type to java.sql.Type code
//
const unordered_map<SQLSMALLINT, jint> x_odbcCToJavaSqlTypeMap =
{
	{static_cast<SQLSMALLINT>(SQL_C_SSHORT), x_javaSqlSmallInt},
	{static_cast<SQLSMALLINT>(SQL_C_BIT), x_javaSqlBit},
	{static_cast<SQLSMALLINT>(SQL_C_SBIGINT), x_javaSqlBigInt},
	{static_cast<SQLSMALLINT>(SQL_C_SLONG), x_javaSqlInteger},
	{static_cast<SQLSMALLINT>(SQL_C_FLOAT), x_javaSqlReal},
	{static_cast<SQLSMALLINT>(SQL_C_DOUBLE), x_javaSqlDouble},
	{static_cast<SQLSMALLINT>(SQL_C_UTINYINT), x_javaSqlTinyInt},
	{static_cast<SQLSMALLINT>(SQL_C_CHAR), x_javaSqlVarChar},
	{static_cast<SQLSMALLINT>(SQL_C_WCHAR), x_javaSqlNVarChar},
	{static_cast<SQLSMALLINT>(SQL_C_BINARY), x_javaSqlVarBinary},
	{static_cast<SQLSMALLINT>(SQL_C_GUID), x_javaSqlChar},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_DATE), x_javaSqlDate},
	{static_cast<SQLSMALLINT>(SQL_C_NUMERIC), x_javaSqlNumeric},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_TIMESTAMP), x_javaSqlTimestamp},
};

// Maps the to java.sql.Type code to ODBC C type
//
const unordered_map<jint, SQLSMALLINT> x_javaSqlTypeToOdbcCMap =
{
	{x_javaSqlSmallInt, static_cast<SQLSMALLINT>(SQL_C_SSHORT)},
	{x_javaSqlBit, static_cast<SQLSMALLINT>(SQL_C_BIT)},
	{x_javaSqlBigInt, static_cast<SQLSMALLINT>(SQL_C_SBIGINT)},
	{x_javaSqlInteger, static_cast<SQLSMALLINT>(SQL_C_SLONG)},
	{x_javaSqlReal, static_cast<SQLSMALLINT>(SQL_C_FLOAT)},
	{x_javaSqlDouble, static_cast<SQLSMALLINT>(SQL_C_DOUBLE)},
	{x_javaSqlTinyInt, static_cast<SQLSMALLINT>(SQL_C_UTINYINT)},
	{x_javaSqlVarChar, static_cast<SQLSMALLINT>(SQL_C_CHAR)},
	{x_javaSqlChar, static_cast<SQLSMALLINT>(SQL_C_CHAR)},
	{x_javaSqlNVarChar, static_cast<SQLSMALLINT>(SQL_C_WCHAR)},
	{x_javaSqlNChar, static_cast<SQLSMALLINT>(SQL_C_WCHAR)},
	{x_javaSqlVarBinary, static_cast<SQLSMALLINT>(SQL_C_BINARY)},
	{x_javaSqlDate, static_cast<SQLSMALLINT>(SQL_C_TYPE_DATE)},
	{x_javaSqlNumeric, static_cast<SQLSMALLINT>(SQL_C_NUMERIC)},
	{x_javaSqlDecimal, static_cast<SQLSMALLINT>(SQL_C_NUMERIC)},
	{x_javaSqlTimestamp, static_cast<SQLSMALLINT>(SQL_C_TYPE_TIMESTAMP)},
};

//--------------------------------------------------------------------------------------------------
// Name: JavaSqlTypeHelper::GetJavaSqlType
//
// Description:
//	Get the Java SQL type from ODBC C type.
//
bool JavaSqlTypeHelper::GetJavaSqlType(SQLSMALLINT sqlType, jint &jdbcType)
{
	bool status = false;

	unordered_map<SQLSMALLINT, jint>::const_iterator it = x_odbcCToJavaSqlTypeMap.find(sqlType);

	if (it != x_odbcCToJavaSqlTypeMap.end())
	{
		jdbcType = it->second;
		status = true;
	}

	return status;
}

//--------------------------------------------------------------------------------------------------
// Name: JavaSqlTypeHelper::GetOdbcType
//
// Description:
//	Get the ODBC C type from Java SQL type.
//
bool JavaSqlTypeHelper::GetOdbcType(jint jdbcType, SQLSMALLINT &odbcType)
{
	bool status = false;

	unordered_map<jint, SQLSMALLINT>::const_iterator it = x_javaSqlTypeToOdbcCMap.find(jdbcType);

	if (it != x_javaSqlTypeToOdbcCMap.end())
	{
		odbcType = it->second;
		status = true;
	}

	return status;
}