//**************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server
// external language communication protocol.
// Copyright (C) 2020 Microsoft Corporation.
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
//**************************************************************************************************

#include "Common.h"

namespace ExtensionApiTest
{
	//----------------------------------------------------------------------------------------------
	// Name: InitColumnTest
	//
	// Description:
	//  Tests InitColumn() API with valid values, can initialize only once.
	//  A positive and negative test.
	//
	TEST_F(RExtensionApiTests, InitColumnTest)
	{
		InitializeSession(m_inputSchemaColumnsNumber);

		SQLRETURN result = SQL_ERROR;
		result = (*sm_initColumnFuncPtr)(
				*m_sessionId,
				m_taskId,
				0,
				m_columnName,
				m_columnNameString.length(),
				SQL_C_SLONG,   // dataType
				m_IntSize,     // columnSize
				0,             // decimalDigits
				1,             // nullable
				-1,            // partitionByNumber
				-1);           // orderByNumber
		EXPECT_EQ(result, SQL_SUCCESS);

		result = (*sm_initColumnFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
			m_columnName,
			m_columnNameString.length(),
			SQL_C_SLONG,   // dataType
			m_IntSize,     // columnSize
			0,             // decimalDigits
			1,             // nullable
			-1,            // partitionByNumber
			-1);           // orderByNumber
		EXPECT_EQ(result, SQL_ERROR);
	}

	//
	// Negative tests
	//
	//----------------------------------------------------------------------------------------------
	// Name: InitInvalidColumnTest
	//
	// Description:
	//  Tests InitColumn() API with null column name. A negative test.
	//
	TEST_F(RExtensionApiTests, InitInvalidColumnTest)
	{
		InitializeSession(m_inputSchemaColumnsNumber);

		SQLRETURN result = SQL_SUCCESS;
		result = (*sm_initColumnFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,           // columnNumber
			nullptr,     // NULL name
			0,           // columnNameLength
			SQL_C_SLONG, // dataType
			m_IntSize,   // columnSize
			0,           // decimalDigits
			1,           // nullable
			-1,          // partitionByNumber
			-1);         // orderByNumber

		EXPECT_EQ(result, SQL_ERROR);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InitInvalidColumnNumberTest
	//
	// Description:
	//  Tests InitColumn() API with bad column numbers (too big). A negative test.
	//
	TEST_F(RExtensionApiTests, InitInvalidColumnNumberTest)
	{
		InitializeSession(m_inputSchemaColumnsNumber);

		SQLRETURN result = SQL_SUCCESS;
		result = (*sm_initColumnFuncPtr)(
				*m_sessionId,
				m_taskId,
				m_inputSchemaColumnsNumber + 1, // column number greater than inputSchemaColumnsNumber
				m_columnName,
				m_columnNameString.length(),
				SQL_C_SLONG,   // dataType
				m_IntSize,     // columnSize
				0,             // decimalDigits
				1,             // nullable
				-1,            // partitionByNumber
				-1);           // orderByNumber
		EXPECT_EQ(result, SQL_ERROR);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InitInvalidColumnDataTypeTest
	//
	// Description:
	//  Tests InitColumn() API with unsupported column datatype. A negative test.
	//
	TEST_F(RExtensionApiTests, InitInvalidColumnDataTypeTest)
	{
		InitializeSession(m_inputSchemaColumnsNumber);

		SQLRETURN result = SQL_SUCCESS;
		result = (*sm_initColumnFuncPtr)(
				*m_sessionId,
				m_taskId,
				0,
				m_columnName,
				m_columnNameString.length(),
				SQL_C_BINARY,   // Unsupported dataType
				m_CharSize,     // columnSize
				0,              // decimalDigits
				1,              // nullable
				-1,             // partitionByNumber
				-1);            // orderByNumber
		EXPECT_EQ(result, SQL_ERROR);
	}
}
