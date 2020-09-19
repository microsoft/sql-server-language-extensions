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
// @File: RExtensionApiInitSessionTests.cpp
//
// Purpose:
//  Tests the RExtension's implementation of the external language InitSession API.
//
//*************************************************************************************************

#include "Common.h"

namespace ExtensionApiTest
{
	// Test InitSession() API with valid values
	//
	TEST_F(RExtensionApiTest, InitSessionTest)
	{
		SQLRETURN result = SQL_ERROR;
		result = (*m_initSessionFuncPtr)(
			*m_sessionId,
			m_taskId,
			m_numTasks,
			m_script,
			m_scriptString.length(),
			m_inputSchemaColumnsNumber,
			m_parametersNumber,
			m_inputDataName,
			m_inputDataNameString.length(),
			m_outputDataName,
			m_outputDataNameString.length());

		EXPECT_EQ(result, SQL_SUCCESS);

		// Try to reinitialize with the same session Id should fail
		//
		result = (*m_initSessionFuncPtr)(
			*m_sessionId,
			m_taskId,
			m_numTasks,
			m_script,
			m_scriptString.length(),
			m_inputSchemaColumnsNumber,
			m_parametersNumber,
			m_inputDataName,
			m_inputDataNameString.length(),
			m_outputDataName,
			m_outputDataNameString.length());
		EXPECT_EQ(result, SQL_ERROR);


		SQLGUID sessionId = { 0, 0, 1, {1} };

		// Try cleanup without first initializing a different session should still pass
		// since while cleaning up we can't differentiate if the cleanup is for
		// library session or not. For library sessions, we don't do initialization.
		//
		result = (*m_cleanupSessionFuncPtr)(
			sessionId,
			m_taskId);
		EXPECT_EQ(result, SQL_SUCCESS);

		// Initialize the different session; make sure this also inits independently
		// even though previous session is still in progress.
		//
		result = (*m_initSessionFuncPtr)(
			sessionId,
			m_taskId,
			m_numTasks,
			m_script,
			m_scriptString.length(),
			m_inputSchemaColumnsNumber,
			m_parametersNumber,
			m_inputDataName,
			m_inputDataNameString.length(),
			m_outputDataName,
			m_outputDataNameString.length());

		EXPECT_EQ(result, SQL_SUCCESS);

		result = (*m_cleanupSessionFuncPtr)(
			sessionId,
			m_taskId);

		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Negative test
	// Test InitSession() API with invalid values
	//
	TEST_F(RExtensionApiTest, InitInvalidSessionTest)
	{
		SQLRETURN result = SQL_SUCCESS;
		result = (*m_initSessionFuncPtr)(
			*m_sessionId,
			m_taskId,
			m_numTasks,
			nullptr,                    // script
			0,                          // scriptLength
			m_inputSchemaColumnsNumber,
			m_parametersNumber,
			m_inputDataName,
			m_inputDataNameString.length(),
			m_outputDataName,
			m_outputDataNameString.length());
		EXPECT_EQ(result, SQL_ERROR);
		CleanupSession();

		result = SQL_SUCCESS;
		result = (*m_initSessionFuncPtr)(
			*m_sessionId,
			m_taskId,
			m_numTasks,
			m_script,
			m_scriptString.length(),
			m_inputSchemaColumnsNumber,
			m_parametersNumber,
			nullptr,                    // inputDataName
			0,                          // inputDataNameLength
			m_outputDataName,
			m_outputDataNameString.length());
		EXPECT_EQ(result, SQL_ERROR);
		CleanupSession();

		result = SQL_SUCCESS;
		result = (*m_initSessionFuncPtr)(
			*m_sessionId,
			m_taskId,
			m_numTasks,
			m_script,
			m_scriptString.length(),
			m_inputSchemaColumnsNumber,
			m_parametersNumber,
			m_inputDataName,
			m_inputDataNameString.length(),
			nullptr,                    // outputDataName
			0);                         // outputDataNameLength
		EXPECT_EQ(result, SQL_ERROR);
	}
}
