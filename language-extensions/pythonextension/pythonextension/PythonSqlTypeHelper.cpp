//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: PySqlTypeHelper.h
// @Owner: joz
//
// Purpose:
//	 Handles converting ODBC C type to JDBC type
//
//*********************************************************************
#ifdef _WIN64
#include <windows.h>
#endif
#include <sqlext.h>
#include <sqltypes.h>
#include <stdlib.h>
#include <unordered_map>
#ifndef _WIN64
// These sal include headers must follow the standard c++ headers, or there
// will be compilation issues. This is because headers like iostream/algorithm use
// variables like __in which are the same as a SAL annotation causing redefinition issues.
//
#include <sal_def.h>
#include <xplat_sal.h>
#endif

// Constant values for SQL types defined in py.sql.Types
//
const int x_pySqlTinyInt = -6;
const int x_pySqlSmallInt = 5;
const int x_pySqlBit = -7;
const int x_pySqlInteger = 4;
const int x_pySqlBigInt = -5;
const int x_pySqlReal = 7;
const int x_pySqlDouble = 8;
const int x_pySqlNVarChar = -9;
const int x_pySqlNChar = -15;
const int x_pySqlVarChar = 12;
const int x_pySqlChar = 1;
const int x_pySqlVarBinary = -3;
const int x_pySqlDate = 91;
const int x_pySqlNumeric = 2;
const int x_pySqlDecimal = 3;
const int x_pySqlTimestamp = 93;

using namespace std;

// Maps the ODBC C type to py.sql.Type code
//
const unordered_map<SQLSMALLINT, INT> x_odbcCToPySqlTypeMap =
{
    {static_cast<SQLSMALLINT>(SQL_C_SSHORT), x_pySqlSmallInt},
    {static_cast<SQLSMALLINT>(SQL_C_BIT), x_pySqlBit},
    {static_cast<SQLSMALLINT>(SQL_C_SBIGINT), x_pySqlBigInt},
    {static_cast<SQLSMALLINT>(SQL_C_SLONG), x_pySqlInteger},
    {static_cast<SQLSMALLINT>(SQL_C_FLOAT), x_pySqlReal},
    {static_cast<SQLSMALLINT>(SQL_C_DOUBLE), x_pySqlDouble},
    {static_cast<SQLSMALLINT>(SQL_C_UTINYINT), x_pySqlTinyInt},
    {static_cast<SQLSMALLINT>(SQL_C_CHAR), x_pySqlVarChar},
    {static_cast<SQLSMALLINT>(SQL_C_WCHAR), x_pySqlNVarChar},
    {static_cast<SQLSMALLINT>(SQL_C_BINARY), x_pySqlVarBinary},
    {static_cast<SQLSMALLINT>(SQL_C_GUID), x_pySqlChar},
    {static_cast<SQLSMALLINT>(SQL_C_TYPE_DATE), x_pySqlDate},
    {static_cast<SQLSMALLINT>(SQL_C_NUMERIC), x_pySqlNumeric},
    {static_cast<SQLSMALLINT>(SQL_C_TYPE_TIMESTAMP), x_pySqlTimestamp},
};

// Maps the to py.sql.Type code to ODBC C type
//
const unordered_map<INT, SQLSMALLINT> x_pySqlTypeToOdbcCMap =
{
    {x_pySqlSmallInt, static_cast<SQLSMALLINT>(SQL_C_SSHORT)},
    {x_pySqlBit, static_cast<SQLSMALLINT>(SQL_C_BIT)},
    {x_pySqlBigInt, static_cast<SQLSMALLINT>(SQL_C_SBIGINT)},
    {x_pySqlInteger, static_cast<SQLSMALLINT>(SQL_C_SLONG)},
    {x_pySqlReal, static_cast<SQLSMALLINT>(SQL_C_FLOAT)},
    {x_pySqlDouble, static_cast<SQLSMALLINT>(SQL_C_DOUBLE)},
    {x_pySqlTinyInt, static_cast<SQLSMALLINT>(SQL_C_UTINYINT)},
    {x_pySqlVarChar, static_cast<SQLSMALLINT>(SQL_C_CHAR)},
    {x_pySqlChar, static_cast<SQLSMALLINT>(SQL_C_CHAR)},
    {x_pySqlNVarChar, static_cast<SQLSMALLINT>(SQL_C_WCHAR)},
    {x_pySqlNChar, static_cast<SQLSMALLINT>(SQL_C_WCHAR)},
    {x_pySqlVarBinary, static_cast<SQLSMALLINT>(SQL_C_BINARY)},
    {x_pySqlDate, static_cast<SQLSMALLINT>(SQL_C_TYPE_DATE)},
    {x_pySqlNumeric, static_cast<SQLSMALLINT>(SQL_C_NUMERIC)},
    {x_pySqlDecimal, static_cast<SQLSMALLINT>(SQL_C_NUMERIC)},
    {x_pySqlTimestamp, static_cast<SQLSMALLINT>(SQL_C_TYPE_TIMESTAMP)},
};
