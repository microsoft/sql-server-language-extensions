//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonInitColumnTests.cpp
//
// Purpose:
//  Test the Python extension columns using the Extension API
//
//*************************************************************************************************
#include "PythonExtensionApiTests.h"

namespace ExtensionApiTest
{
	// Name: InitColumnTest
	//
	// Description:
	//  Test InitColumn() API with valid values
	//
	TEST_F(PythonExtensionApiTests, InitColumnTest)
	{
		InitializeSession(0, // paramsNumber
			1); // inputSchemaColumns

		SQLCHAR * columnName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>("Column1")));
		SQLSMALLINT columnNameLength = 7;
		SQLRETURN result = InitColumn(
			*m_sessionId,
			m_taskId,
			0,                // Column Number
			columnName,
			columnNameLength,
			SQL_C_SLONG,      // Data Type
			m_IntSize,        // Column Size
			0,                // Decimal Digits
			1,                // Nullable
			-1,               // PartitionByNumber
			-1                // OrderByNumber
		);
		EXPECT_EQ(result, SQL_SUCCESS);
	}
	
	//
	// Negative Tests
	//

	// Name: InitInvalidColumnTest
	//
	// Description:
	//  Test InitColumn() API with null column name
	//
	TEST_F(PythonExtensionApiTests, InitInvalidColumnTest)
	{
		InitializeSession(0, // paramsNumber
			1); // inputSchemaColumns

		SQLRETURN result = InitColumn(
			*m_sessionId,
			m_taskId,
			0,           // Column Number
			nullptr,     // Column Name
			0,           // Column Name Length
			SQL_C_SLONG, // Data Type
			m_IntSize,   // Column Size
			0,           // Decimal Digits
			1,           // Nullable
			-1,          // PartitionByNumber
			-1           // OrderByNumber
		);
		EXPECT_EQ(result, SQL_ERROR);
	}

	// Name: InitInvalidColumnNumberTest
	//
	// Description:
	//  Test InitColumn() API with bad column numbers (too big)
	//
	TEST_F(PythonExtensionApiTests, InitInvalidColumnNumberTest)
	{
		InitializeSession(1);

		SQLCHAR * columnName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>("Column1")));
		SQLSMALLINT columnNameLength = 7;

		SQLRETURN result = InitColumn(
			*m_sessionId,
			m_taskId,
			2,                // column number greater than initialized columns
			columnName,
			columnNameLength,
			SQL_C_SLONG,      // Data Type
			m_IntSize,        // Column Size
			0,                // Decimal Digits
			1,                // Nullable
			-1,               // PartitionByNumber
			-1                // OrderByNumber
		);
		EXPECT_EQ(result, SQL_ERROR);
	}
}
