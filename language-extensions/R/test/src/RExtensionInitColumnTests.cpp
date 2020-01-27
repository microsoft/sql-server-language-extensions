//*************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server
// external language communication protocol.
// Copyright (C) 2019 Microsoft Corporation.
//
// This file is part of RExtension-test.
//
// RExtension-test is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RExtension-test is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RExtension-test.  If not, see <https://www.gnu.org/licenses/>.
//
// @File: RExtensionApiInitColumnTests.cpp
//
// Purpose:
//  Tests the RExtension's implementation of the external language InitColumn API.
//
//*************************************************************************************************
#ifdef _WIN64
#include <windows.h>
#endif
#include <sqlext.h>
#include <sqltypes.h>
#include "gtest/gtest.h"

#include "RExtensionApiTest.h"

namespace ExtensionApiTest
{
	// Positive test
	// Test InitColumn() API with valid values
	//
	TEST_F(RExtensionApiTest, InitColumnTest)
	{
		InitializeSession();

		SQLCHAR * columnName = reinterpret_cast<SQLCHAR*>(const_cast<char *>("Column1"));

		// columnNameLength is length of columnName + the null terminator
		//
		SQLSMALLINT columnNameLength = 8;

		SQLRETURN result = SQL_ERROR;

		result = (*m_initColumnFuncPtr)(
				*m_sessionId,
				m_taskId,
				0,
				columnName,
				columnNameLength,
				SQL_C_SLONG,   // dataType
				sizeof(int),   // columnSize
				0,             // decimalDigits
				1,             // nullable
				-1,            // partitionByNumber
				-1);           // orderByNumber
		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Negative test
	// Test InitColumn() API with null column name
	//
	TEST_F(RExtensionApiTest, InitInvalidColumnTest)
	{
		InitializeSession();

		SQLRETURN result = SQL_SUCCESS;
		result = (*m_initColumnFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,           // columnNumber
			nullptr,     // NULL name
			0,           // columnNameLength
			SQL_C_SLONG, // dataType
			sizeof(int), // columnSize
			0,           // decimalDigits
			1,           // nullable
			-1,          // partitionByNumber
			-1);         // orderByNumber

		EXPECT_EQ(result, SQL_ERROR);
	}

	// Negative test
	// Test InitColumn() API with bad column numbers (negative or too big)
	//
	TEST_F(RExtensionApiTest, InitInvalidColumnNumberTest)
	{
		InitializeSession();

		SQLCHAR * columnName = reinterpret_cast<SQLCHAR*>(const_cast<char *>("Hello"));
		SQLSMALLINT columnNameLength = 6;
		SQLRETURN result = SQL_SUCCESS;
		result = (*m_initColumnFuncPtr)(
				*m_sessionId,
				m_taskId,
				-1,             // negative column number
				columnName,
				columnNameLength,
				SQL_C_SLONG,   // dataType
				sizeof(int),   // columnSize
				0,             // decimalDigits
				1,             // nullable
				-1,            // partitionByNumber
				-1);           // orderByNumber
		EXPECT_EQ(result, SQL_ERROR);

		result = SQL_SUCCESS;
		result = (*m_initColumnFuncPtr)(
				*m_sessionId,
				m_taskId,
				m_inputSchemaColumnsNumber + 1, // column number greater than inputSchemaColumnsNumber
				columnName,
				columnNameLength,
				SQL_C_SLONG,   // dataType
				sizeof(int),   // columnSize
				0,             // decimalDigits
				1,             // nullable
				-1,            // partitionByNumber
				-1);           // orderByNumber
		EXPECT_EQ(result, SQL_ERROR);
	}
}