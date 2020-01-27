//******************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server external language communication protocol.
// Copyright (C) 2019 Microsoft Corporation.

// This file is part of RExtension-test.

// RExtension-test is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// RExtension-test is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with RExtension-test.  If not, see <https://www.gnu.org/licenses/>.

// @File: RExtensionApiTest.h
//
// Purpose:
//  Specifies some typedefinitions to be used in the test.
//  And declares the base test fixture class RExtensionApiTest.
//
//******************************************************************************************************
#pragma once

typedef SQLRETURN FN_init(
	SQLCHAR *, // extensionParams
	SQLULEN,   // extensionParamsLength
	SQLCHAR *, // extensionPath
	SQLULEN,   // extensionPathLength
	SQLCHAR *, // publicLibraryPath
	SQLULEN,   // publicLibraryPathLength
	SQLCHAR *, // privateLibraryPath
	SQLULEN);  // privateLibraryPathLength

typedef SQLRETURN FN_initSession(
	SQLGUID,       // sessionId
	SQLUSMALLINT,  // taskId
	SQLUSMALLINT,  // numTasks
	SQLCHAR *,     // script
	SQLULEN,       // scriptLength
	SQLUSMALLINT,  // inputSchemaColumnsNumber
	SQLUSMALLINT,  // parametersNumber
	SQLCHAR*,      // inputDataName
	SQLUSMALLINT,  // inputDataNameLength
	SQLCHAR*,      // outputDataName
	SQLUSMALLINT); // outputDataNameLength

typedef SQLRETURN FN_initColumn(
	SQLGUID,      // sessionId
	SQLUSMALLINT, // taskId
	SQLUSMALLINT, // columnNumber
	SQLCHAR *,    // columnName
	SQLSMALLINT,  // columnNameLenth
	SQLSMALLINT,  // dataType
	SQLULEN,      // columnSize,
	SQLSMALLINT,  // decimalDigits
	SQLSMALLINT,  // nullable
	SQLSMALLINT,  // partitionByNumber
	SQLSMALLINT); // orderByNumber

typedef SQLRETURN FN_cleanup();

typedef SQLRETURN FN_cleanupSession(
	SQLGUID,       // sessionId
	SQLUSMALLINT); // taskId

namespace ExtensionApiTest
{
	// All the tests in the RExtensionApiTest suite run one after the other
	//
	class RExtensionApiTest : public ::testing::Test
	{
	protected:

		// Per-test-suite set-up.
		// Called before the first test in this test suite.
		//
		static void SetUpTestCase();

		// Per-test-suite tear-down.
		// Called after the last test in this test suite.
		//
		static void TearDownTestCase();

		// Code here will be called immediately after the constructor (right
		// before each test).
		//
		void SetUp() override;

		// Code here will be called immediately after each test (right
		// before the destructor).
		//
		void TearDown() override;

		// Check R_HOME and set it if not defined.
		//
		static void CheckAndSetRHome();

		// Load library and get handles to different functions
		// Testing if RExtension is successfully loaded dynamically
		//
		static void GetHandles();

		// Do Init where embedded R is initialized - can be called only once in the test suite.
		// Testing if Init is implemented correctly.
		//
		static void DoInit();

		// Set up default, valid variables for use in tests
		//
		void SetupVariables();

		// Close the handle to the library.
		//
		static void ReleaseHandles();

		// Call Cleanup on the RExtension.
		// Testing if Cleanup is implemented correctly.
		//
		static void DoCleanup();

		// Initialize a valid, default session for later tests
		//
		void InitializeSession();

		// Cleanup a valid, default session for later tests
		//
		void CleanupSession();

		// Objects declared here can be used by all tests in the test suite
		//
		SQLGUID *m_sessionId;
		SQLUSMALLINT m_taskId;
		SQLUSMALLINT m_numTasks;
		SQLUSMALLINT m_parametersNumber;

		SQLCHAR *m_script;
		SQLSMALLINT m_scriptLength;

		SQLUSMALLINT m_inputSchemaColumnsNumber;
		SQLCHAR *m_inputDataName;
		SQLSMALLINT m_inputDataNameLength;

		SQLCHAR *m_outputDataName;
		SQLSMALLINT m_outputDataNameLength;

		// Pointer handle to the library libRextension
		//
		static void *m_libHandle;

		// Pointer to the Init function
		//
		static FN_init *m_initFuncPtr;

		// Pointer to the InitSession function
		//
		static FN_initSession *m_initSessionFuncPtr;

		// Pointer to the InitColumn function
		//
		static FN_initColumn *m_initColumnFuncPtr;

		// Pointer to the CleanupSession function
		//
		static FN_cleanupSession *m_cleanupSessionFuncPtr;

		// Pointer to the Cleanup function
		//
		static FN_cleanup *m_cleanupFuncPtr;
	};
}