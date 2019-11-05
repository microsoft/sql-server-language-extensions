//*********************************************************************
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// @File: JavaSqlTypeHelper.h
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