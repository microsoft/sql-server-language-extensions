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
// @File: RExtensionApiTest.cpp
//
// Purpose:
//  Defines the base test fixture members and tests RExtension's implementation of
//  the external language initialization and cleanup APIs.
//
//*************************************************************************************************
#ifdef _WIN64
#include <windows.h>
#endif

#include <memory>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"

#include "Rcpp.h"

#include "RExtensionApiTest.h"
#include "Utilities.h"

using namespace std;

#ifdef _WIN64
	const string RExtensionLibName = "libRExtension.dll";
#else
	const string RExtensionLibName = "libRExtension.so.1.0";
#endif // _WIN64

extern int g_argc;
extern const char **g_argv;

namespace ExtensionApiTest
{
	// Initialize all the static members
	//
	void *RExtensionApiTest::m_libHandle = nullptr;
	FN_init *RExtensionApiTest::m_initFuncPtr = nullptr;
	FN_initSession *RExtensionApiTest::m_initSessionFuncPtr = nullptr;
	FN_initColumn *RExtensionApiTest::m_initColumnFuncPtr = nullptr;
	FN_initParam *RExtensionApiTest::m_initParamFuncPtr = nullptr;
	FN_execute *RExtensionApiTest::m_executeFuncPtr = nullptr;
	FN_cleanupSession *RExtensionApiTest::m_cleanupSessionFuncPtr = nullptr;
	FN_cleanup *RExtensionApiTest::m_cleanupFuncPtr = nullptr;

	// Per-test-suite set-up.
	// Called before the first test in this test suite.
	//
	void RExtensionApiTest::SetUpTestSuite()
	{
		CheckAndSetRHome();
		ASSERT_NO_THROW(GetHandles());
		DoInit();
	}

	// Per-test-suite tear-down.
	// Called after the last test in this test suite.
	//
	void RExtensionApiTest::TearDownTestSuite()
	{
		DoCleanup();
		ASSERT_NO_THROW(ReleaseHandles());
	}

	// Code here will be called immediately after the constructor (right
	// before each test).
	//
	void RExtensionApiTest::SetUp()
	{
		SetupVariables();
	}

	// Code here will be called immediately after each test (right
	// before the destructor).
	//
	void RExtensionApiTest::TearDown()
	{
		CleanupVariables();
		CleanupSession();
	}

	// Name: CheckAndSetRHome()
	//
	// Description:
	// Check R_HOME and set it if not defined.
	//
	void RExtensionApiTest::CheckAndSetRHome()
	{
		std::cout << "Checking and setting R_HOME.\n";
		char *RHome = getenv("R_HOME");

		// If RHome is not defined it could be passed as the first argument to the test executable
		// In terms of gtest, the first argument is the name of the test executable
		// and RHome then becomes the second argument.
		//
		if (RHome == nullptr && g_argc >= 2)
		{
			RHome = const_cast<char *>(g_argv[1]);
			putenv(RHome);
		}
	}

	// Name: GetHandles
	//
	// Description:
	// Load library and get handles to different functions
	// Testing if RExtension is successfully loaded dynamically
	//
	void RExtensionApiTest::GetHandles()
	{
		std::cout << "Loading the RExtension and getting handles for extension APIs.\n";
		m_libHandle = Utilities::CrossPlatLoadLibrary(RExtensionLibName.c_str());
		ASSERT_TRUE(m_libHandle != nullptr);

		m_initFuncPtr = reinterpret_cast<FN_init*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				m_libHandle,
				"Init"));
		ASSERT_TRUE(m_initFuncPtr != nullptr);

		m_initSessionFuncPtr = reinterpret_cast<FN_initSession*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				m_libHandle,
				"InitSession"));
		ASSERT_TRUE(m_initSessionFuncPtr != nullptr);

		m_initColumnFuncPtr = reinterpret_cast<FN_initColumn*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				m_libHandle,
				"InitColumn"));
		ASSERT_TRUE(m_initColumnFuncPtr != nullptr);

		m_initParamFuncPtr = reinterpret_cast<FN_initParam*>(
		Utilities::CrossPlatGetFunctionFromLibHandle(
			m_libHandle,
			"InitParam"));
		ASSERT_TRUE(m_initParamFuncPtr != nullptr);

		m_executeFuncPtr = reinterpret_cast<FN_execute*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				m_libHandle,
				"Execute"));
		ASSERT_TRUE(m_executeFuncPtr != nullptr);

		m_cleanupSessionFuncPtr = reinterpret_cast<FN_cleanupSession*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				m_libHandle,
				"CleanupSession"));
		ASSERT_TRUE(m_cleanupSessionFuncPtr != nullptr);

		m_cleanupFuncPtr = reinterpret_cast<FN_cleanup*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				m_libHandle,
				"Cleanup"));
		ASSERT_TRUE(m_cleanupFuncPtr != nullptr);
	}

	// Name: DoInit
	//
	// Description:
	// Do Init where embedded R is initialized - can be called only once in the test suite.
	// Testing if Init is implemented correctly.
	//
	void RExtensionApiTest::DoInit()
	{
		std::cout << "Calling Init.\n";
		SQLRETURN result = SQL_ERROR;
		const string cmdLine = "dummyInputScript --no-save";
		int paramsLength = strlen(cmdLine.c_str());
		unique_ptr<SQLCHAR> extensionParams(new SQLCHAR[paramsLength]);
		memcpy(extensionParams.get(), cmdLine.c_str(), paramsLength);
		result = (*m_initFuncPtr)(
			extensionParams.get(),
			paramsLength,
			nullptr,
			0,
			nullptr,
			0,
			nullptr,
			0);
		ASSERT_EQ(result, SQL_SUCCESS);
	}

	// Name: SetupVariables
	//
	// Description:
	// Set up default, valid variables for use in tests
	//
	void RExtensionApiTest::SetupVariables()
	{
		m_sessionId = new SQLGUID();
		m_taskId = 0;
		m_numTasks = 1;
		m_inputSchemaColumnsNumber = 1;
		m_parametersNumber = 1;

		m_paramNameString = "@param1";
		m_paramName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_paramNameString.c_str()))
			);

		m_columnNameString = "Column1";
		m_columnName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_columnNameString.c_str()))
			);

		m_scriptString = "print('" + m_printMessage + "');"
			"OutputDataSet <- InputDataSet;"
			"print('InputDataSet:'); print(InputDataSet);"
			"print('OutputDataSet:'); print(OutputDataSet);";
		m_script = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_scriptString.c_str()))
			);

		m_inputDataNameString = "InputDataSet";
		m_inputDataName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_inputDataNameString.c_str()))
			);

		m_outputDataNameString = "OutputDataSet";
		m_outputDataName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_outputDataNameString.c_str()))
			);

		// Retrieve the global environment
		//
		m_globalEnvironment = Rcpp::Environment::global_env();
	}

	// Name: CleanupVariables
	//
	// Description:
	// Delete the memory allocated to default variables.
	//
	void RExtensionApiTest::CleanupVariables()
	{
		if (m_sessionId != nullptr)
		{
			delete m_sessionId;
		}
	}

	// Name: ReleaseHandles
	//
	// Description:
	// Close the handle to the library.
	//
	void RExtensionApiTest::ReleaseHandles()
	{
		std::cout << "Closing the library handle.\n";
		Utilities::CrossPlatCloseLibrary(m_libHandle);
	}

	// Name: DoCleanup
	//
	// Description:
	// Call Cleanup on the RExtension.
	// Testing if Cleanup is implemented correctly.
	//
	void RExtensionApiTest::DoCleanup()
	{
		std::cout << "Calling Cleanup.\n";
		SQLRETURN result = SQL_ERROR;
		result = (*m_cleanupFuncPtr)();
		ASSERT_EQ(result, SQL_SUCCESS);
	}

	// Name: InitializeSession
	//
	// Description:
	// Initialize a valid, default session for later tests
	//
	void RExtensionApiTest::InitializeSession(SQLUSMALLINT inputSchemaColumnsNumber)
	{
		SQLRETURN result = SQL_SUCCESS;

		result = (*m_initSessionFuncPtr)(
			*m_sessionId,
			m_taskId,
			m_numTasks,
			m_script,
			m_scriptString.length(),
			inputSchemaColumnsNumber,
			m_parametersNumber,
			m_inputDataName,
			m_inputDataNameString.length(),
			m_outputDataName,
			m_outputDataNameString.length());

		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Name: CleanupSession
	//
	// Description:
	// Cleanup a valid, default session for later tests
	//
	void RExtensionApiTest::CleanupSession()
	{
		SQLRETURN result = SQL_SUCCESS;

		result = (*m_cleanupSessionFuncPtr)(
			*m_sessionId,
			m_taskId);

		EXPECT_EQ(result, SQL_SUCCESS);
	}
}
