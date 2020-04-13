//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonExtensionApiTest.cpp
//
// Purpose:
//  Define the common member functions of the PythonExtensionApiTest class
//
//*************************************************************************************************
#include "PythonExtensionApiTest.h"

using namespace std;
namespace py = boost::python;

namespace ExtensionApiTest
{
	// Code here will be called immediately after the constructor (right
	// before each test).
	//
	void PythonExtensionApiTest::SetUp()
	{
		Py_Initialize();

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

		m_paramName = "param1";
		m_paramNameLength = m_paramName.length();

		m_scriptString = "print('Hello')";
		m_script = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_scriptString.c_str())));
		m_scriptLength = m_scriptString.length();

		m_inputDataNameString = "InputDataSet";
		m_inputDataName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_inputDataNameString.c_str())));
		m_inputDataNameLength = m_inputDataNameString.length();

		m_outputDataNameString = "OutputDataSet";
		m_outputDataName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_outputDataNameString.c_str())));
		m_outputDataNameLength = m_outputDataNameString.length();

		try
		{
			m_mainModule = py::import("__main__");
			m_mainNamespace = m_mainModule.attr("__dict__");
		}
		catch (py::error_already_set&)
		{
			throw runtime_error("Error loading main module and namespace");
		}

		// Check that the module and namespace are populated, not None objects
		//
		if (m_mainModule == boost::python::object() ||
			m_mainNamespace == boost::python::object())
		{
			throw runtime_error("Main module or namespace was None");
		}
	}

	// Initialize a valid, default session for non-Init tests
	//
	void PythonExtensionApiTest::InitializeSession()
	{
		SQLRETURN result = SQL_ERROR;

		result = Init(
			nullptr, // Extension Params
			0,       // Extension Params Length
			nullptr, // Extension Path
			0,       // Extension Path Length
			nullptr, // Public Library Path
			0,       // Public Library Path Length
			nullptr, // Private Library Path
			0        // Private Library Path Length
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
