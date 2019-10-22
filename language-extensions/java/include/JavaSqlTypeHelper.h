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

#include "Common.h"

class JavaSqlTypeHelper
{
public:
	static bool GetJavaSqlType(SQLSMALLINT sqlType, jint &jdbcType);

	static bool GetOdbcType(jint jdbcType, SQLSMALLINT &odbcType);
};