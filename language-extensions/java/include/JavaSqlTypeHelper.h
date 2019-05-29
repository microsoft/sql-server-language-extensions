//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: JavaSqlTypeHelper.h
// @Owner: brnieb
//
// Purpose:
//	 Handles converting ODBC C type to JDBC type
//
//*********************************************************************
#pragma once

class JavaSqlTypeHelper
{
public:
	_Success_(return )
	static bool GetJavaSqlType(_In_ SQLSMALLINT sqlType, _Out_ jint &jdbcType);

	_Success_(return )
	static bool GetOdbcType(_In_ jint jdbcType, _Out_ SQLSMALLINT &odbcType);
};