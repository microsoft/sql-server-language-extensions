//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonExtensionApiTests.cpp
//
// Purpose:
//  Define the common member functions of the PythonExtensionApiTests class
//
//*************************************************************************************************
#include "PythonExtensionApiTests.h"

using namespace std;
namespace py = boost::python;

namespace ExtensionApiTest
{
	// Code here will be called immediately after the constructor (right
	// before each test).
	//
	void PythonExtensionApiTests::SetUp()
	{
		Py_Initialize();

		boost::python::numpy::initialize();

		SetupVariables();
	}

	// Code here will be called immediately after each test (right
	// before the destructor).
	//
	void PythonExtensionApiTests::TearDown()
	{
		DoCleanup();
	}

	// Set up default, valid variables for use in tests
	//
	void PythonExtensionApiTests::SetupVariables()
	{
		m_sessionId = new SQLGUID();
		m_taskId = 0;
		m_numTasks = 1;
		m_parametersNumber = 1;

		m_paramName = "param1";
		m_paramNameLength = m_paramName.length();

		m_scriptString = "print('" + m_printMessage + "');"
			"OutputDataSet = InputDataSet;"
			"print('InputDataSet:'); print(InputDataSet);"
			"print('OutputDataSet:'); print(OutputDataSet);";
		m_script = static_cast<SQLCHAR *>(static_cast<void *>(const_cast<char *>(m_scriptString.c_str())));
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
	// Tests InitSession API
	//
	void PythonExtensionApiTests::InitializeSession(SQLUSMALLINT inputSchemaColumnsNumber)
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
			inputSchemaColumnsNumber,
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
	void PythonExtensionApiTests::DoCleanup()
	{
		SQLRETURN result = SQL_ERROR;

		result = CleanupSession(*m_sessionId, m_taskId);
		EXPECT_EQ(result, SQL_SUCCESS);

		result = Cleanup();
		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Parses the value of the active python exception
	// Type, value, and traceback are in separate pointers
	//
	string PythonExtensionApiTests::ParsePythonException()
	{
		PyObject *pType = NULL;
		PyObject *pValue = NULL;
		PyObject *pTraceback = NULL;

		// Fetch the exception info from the Python C API
		//
		PyErr_Fetch(&pType, &pValue, &pTraceback);

		// Fallback error
		//
		string ret("Unfetchable Python error");

		// If the fetch got a type pointer, parse the type into the exception string
		//
		if (pType != NULL)
		{
			string type = ExtractString(pType);

			// If a valid string extraction is available, use it
			// otherwise use fallback string
			//
			if (type.empty())
			{
				ret = "Unknown exception type";
			}
			else
			{
				ret = type;
			}
		}

		// Do the same for the exception value (the stringification of the exception)
		//
		if (pValue != NULL)
		{
			string value = ExtractString(pValue);

			if (value.empty())
			{
				ret += string(": Unparseable Python error: ");
			}
			else
			{
				ret += ": " + value;
			}
		}

		// Parse lines from the traceback using the Python traceback module
		//
		if (pTraceback != NULL)
		{
			py::handle<> handleTrace(pTraceback);

			// Load the traceback module and the format_tb function
			//
			py::object traceback(py::import("traceback"));
			py::object format_tb(traceback.attr("format_tb"));

			// Call format_tb to get a list of traceback strings
			//
			py::object traceList(format_tb(handleTrace));

			// Join the traceback strings into a single string
			//
			py::object tracePyStr(py::str("\n").join(traceList));

			// Extract the string, check the extraction, and fallback if necessary
			//
			string trace = ExtractString(tracePyStr.ptr());

			if (trace.empty())
			{
				ret += string(": Unparseable Python traceback");
			}
			else
			{
				ret += ": " + trace;
			}
		}

		return ret;
	}

	// Extract the string from a boost::python object
	//
	string PythonExtensionApiTests::ExtractString(PyObject *pObj)
	{
		string ret;
		py::handle<> handle(pObj);
		py::str pyStr(handle);

		// Extract the string from the boost::python object
		//
		py::extract<string> extracted(pyStr);

		// If a valid string extraction is available, use it
		// otherwise return empty string
		//
		if (extracted.check())
		{
			ret = extracted();
		}

		return ret;
	}

}
