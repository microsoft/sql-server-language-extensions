//*********************************************************************
//                Copyright (C) Microsoft Corporation.
//
// @File: PythonExtensionApiTest.h
//
// Purpose:
//  Define common methods and variables needed to test the Extension API
//
//*********************************************************************
#pragma once
#include "Common.h"

namespace ExtensionApiTest
{
	// All the tests in the PythonextensionApiTest suite run one after the other
	//
	class PythonExtensionApiTest : public ::testing::Test
	{
	protected:

		// Code here will be called immediately after the constructor (right
		// before each test).
		//
		void SetUp() override;

		// Code here will be called immediately after each test (right
		// before the destructor).
		//
		void TearDown() override;

		// Initialize a valid, default session for later tests
		//
		void InitializeSession();

		// Set up default, valid variables for use in tests
		//
		void SetupVariables();

		// Call Cleanup on the PythonExtension.
		// Testing if Cleanup is implemented correctly.
		//
		void DoCleanup();

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
	};
}