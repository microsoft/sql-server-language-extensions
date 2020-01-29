//*********************************************************************
//                Copyright (C) Microsoft Corporation.
//
// @File: PythonColumnTest.cpp
//
// Purpose:
//  Test the Python extension columns using the Extension API
//
//*********************************************************************
#include "PythonExtensionApiTest.h"

namespace ExtensionApiTest
{
	// Negative test
	// Test InitColumn() API with null column name
	//
	TEST_F(PythonExtensionApiTest, InitInvalidColumnTest)
	{
		InitializeSession();

		SQLRETURN result = InitColumn(
			*m_sessionId,
			m_taskId,
			0,				// Column Number
			nullptr,		// Column Name
			0,				// Column Name Length
			SQL_C_SLONG,	// Data Type
			sizeof(int),	// Column Size
			0,				// Decimal Digits
			1,				// Nullable
			-1,				// PartitionByNumber
			-1				// OrderByNumber
		);
		EXPECT_EQ(result, SQL_ERROR);
	}

	// Negative test
	// Test InitColumn() API with bad column numbers (too big)
	//
	TEST_F(PythonExtensionApiTest, InitInvalidColumnNumberTest)
	{
		InitializeSession();

		SQLCHAR * columnName = reinterpret_cast<SQLCHAR *>("Hello");
		SQLSMALLINT columnNameLength = 6;

		SQLRETURN result = InitColumn(
			*m_sessionId,
			m_taskId,
			m_inputSchemaColumnsNumber + 1, // column number greater than m_inputSchemaColumnsNumber
			columnName,
			columnNameLength,
			SQL_C_SLONG,	// Data Type
			sizeof(int),	// Column Size
			0,				// Decimal Digits
			1,				// Nullable
			-1,				// PartitionByNumber
			-1				// OrderByNumber
		);
		EXPECT_EQ(result, SQL_ERROR);
	}

	// Test InitColumn() API with valid values
	//
	TEST_F(PythonExtensionApiTest, InitColumnTest)
	{
		InitializeSession();

		SQLCHAR * columnName = reinterpret_cast<SQLCHAR *>("Hello");
		SQLSMALLINT columnNameLength = 6;
		SQLRETURN result = InitColumn(
			*m_sessionId,
			m_taskId,
			0,				// Column Number
			columnName,
			columnNameLength,
			SQL_C_SLONG,	// Data Type
			sizeof(int),	// Column Size
			0,				// Decimal Digits
			1,				// Nullable
			-1,				// PartitionByNumber
			-1				// OrderByNumber
		);
		EXPECT_EQ(result, SQL_SUCCESS);
	}
}
