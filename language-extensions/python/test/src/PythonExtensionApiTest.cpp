//*********************************************************************
//                Copyright (C) Microsoft Corporation.
//
// @File: PythonExtensionApiTest.cpp
//
// Purpose:
//  Define the common member functions of the PythonExtensionApiTest class
//
//*********************************************************************
#include "PythonExtensionApiTest.h"

using namespace std;

namespace ExtensionApiTest
{
	// Code here will be called immediately after the constructor (right
	// before each test).
	//
	void PythonExtensionApiTest::SetUp()
	{
		SetupVariables();
	}

	// Code here will be called immediately after each test (right
	// before the destructor).
	//
	void PythonExtensionApiTest::TearDown()
	{
		DoCleanup();
	}

	// Set up default, valid variables for use in tests
	//
	void PythonExtensionApiTest::SetupVariables()
	{
		m_sessionId = new SQLGUID();
		m_taskId = 0;
		m_numTasks = 1;
		m_inputSchemaColumnsNumber = 1;
		m_parametersNumber = 1;

		string scriptString = "print('Hello')";
		m_script = reinterpret_cast<SQLCHAR *>(const_cast<char *>(scriptString.c_str()));
		m_scriptLength = scriptString.length() + 1; // null terminator

		string inputDataNameString = "InputDataSet";
		m_inputDataName = reinterpret_cast<SQLCHAR *>(const_cast<char *>(inputDataNameString.c_str()));
		m_inputDataNameLength = inputDataNameString.length() + 1; // null terminator

		string outputDataNameString = "OutputDataSet";
		m_outputDataName = reinterpret_cast<SQLCHAR *>(const_cast<char *>(outputDataNameString.c_str()));
		m_outputDataNameLength = outputDataNameString.length() + 1; // null terminator
	}

	// Initialize a valid, default session for non-Init tests
	//
	void PythonExtensionApiTest::InitializeSession()
	{
		SQLRETURN result = SQL_ERROR;

		result = Init(
			nullptr, // Extension Params
			0,		 // Extension Params Length
			nullptr, // Extension Path
			0,		 // Extension Path Length
			nullptr, // Public Library Path
			0,		 // Public Library Path Length
			nullptr, // Private Library Path
			0		 // Private Library Path Length
		);
		EXPECT_EQ(result, SQL_SUCCESS);

		result = InitSession(
			*m_sessionId,
			m_taskId,
			m_numTasks,
			m_script,
			m_scriptLength,
			m_inputSchemaColumnsNumber,
			m_parametersNumber,
			m_inputDataName,
			m_inputDataNameLength,
			m_outputDataName,
			m_outputDataNameLength
		);
		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Call Cleanup on the PythonExtension.
	// Testing if Cleanup is implemented correctly.
	//
	void PythonExtensionApiTest::DoCleanup()
	{
		SQLRETURN result = SQL_ERROR;

		result = CleanupSession(*m_sessionId, m_taskId);
		EXPECT_EQ(result, SQL_SUCCESS);

		result = Cleanup();
		EXPECT_EQ(result, SQL_SUCCESS);
	}
}
