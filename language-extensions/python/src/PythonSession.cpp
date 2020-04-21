//**************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonSession.cpp
//
// Purpose:
//  Class encapsulating operations performed per session
//
//**************************************************************************************************

#include "Logger.h"
#include "PythonSession.h"
#include "PythonExtensionUtils.h"

using namespace std;
namespace py = boost::python;
namespace np = boost::python::numpy;

// Init the session
//
void PythonSession::Init(
	const SQLGUID *sessionId,
	SQLUSMALLINT  taskId,
	SQLUSMALLINT  numTasks,
	SQLCHAR       *script,
	SQLULEN       scriptLength,
	SQLUSMALLINT  inputSchemaColumnsNumber,
	SQLUSMALLINT  parametersNumber,
	SQLCHAR       *inputDataName,
	SQLUSMALLINT  inputDataNameLength,
	SQLCHAR       *outputDataName,
	SQLUSMALLINT  outputDataNameLength)
{
	LOG("PythonSession::Init");

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

	// Initialize the script
	//
	if (script == nullptr)
	{
		throw invalid_argument("Invalid script, the script value cannot be NULL");
	}

	// Initialize and store the user script
	//
	m_script = string(reinterpret_cast<const char*>(script), scriptLength);
	m_scriptLength = scriptLength;

	// Initialize the parameters container.
	//
	m_paramContainer.Init(parametersNumber);

	// Initialize the InputDataSet
	//
	m_inputDataSet.Init(inputDataName, inputDataNameLength, inputSchemaColumnsNumber);

	// Initialize the OutputDataSet
	//
	m_outputDataSet.Init(outputDataName, outputDataNameLength, 0);

}

// Init the input column
//
void PythonSession::InitColumn(
	SQLUSMALLINT  columnNumber,
	const SQLCHAR *columnName,
	SQLSMALLINT   columnNameLength,
	SQLSMALLINT   dataType,
	SQLULEN       columnSize,
	SQLSMALLINT   decimalDigits,
	SQLSMALLINT   nullable,
	SQLSMALLINT   partitionByNumber,
	SQLSMALLINT   orderByNumber)
{
	LOG("PythonSession::InitColumn #" + to_string(columnNumber));

	m_inputDataSet.InitColumn(
		columnNumber,
		columnName,
		columnNameLength,
		dataType,
		columnSize,
		decimalDigits,
		nullable);
 }

// Init the input parameter
//
void PythonSession::InitParam(
	SQLUSMALLINT  paramNumber,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   dataType,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType)
{
	LOG("PythonSession::InitParam #" + to_string(paramNumber));

	if (paramName == nullptr)
	{
		throw invalid_argument("Invalid input parameter name supplied");
	}
	else if (paramNumber >= m_paramContainer.GetSize())
	{
		throw invalid_argument("Invalid input param id supplied: " + to_string(paramNumber));
	}

	// Add parameter to the container and boost::python nameSpace.
	//
	m_paramContainer.AddParamToNamespace(
		m_mainNamespace,
		paramNumber,
		paramName,
		paramNameLength,
		dataType,
		paramSize,
		decimalDigits,
		paramValue,
		strLen_or_Ind,
		inputOutputType);
}

// Execute the workflow for the session
//
void PythonSession::ExecuteWorkflow(
	SQLULEN      rowsNumber,
	SQLPOINTER   *data,
	SQLINTEGER   **strLen_or_Ind,
	SQLUSMALLINT *outputSchemaColumnsNumber)
{
	LOG("PythonSession::ExecuteWorkflow");

	*outputSchemaColumnsNumber = 0;

	// Add columns to the input DataFrame.
	//
	m_inputDataSet.AddColumnsToDictionary(rowsNumber, data, strLen_or_Ind);

	// Add the dictionary for InputDataSet to the python namespace and convert to a DataFrame.
	//
	m_inputDataSet.AddDictionaryToNamespace(m_mainNamespace);

	// Execute the script, capturing the output and error
	//
	try
	{
		// Scripts to redirect stdout and stderr to variables to extract afterwards
		//
		string redirectPyOut = "import sys; from io import StringIO\n"
			"_temp_out_ = StringIO(); _temp_err_ = StringIO()\n"
			"sys.stdout = _temp_out_; sys.stderr = _temp_err_\n";

		string resetPyOut = "sys.stdout = sys.__stdout__\n"
			"sys.stderr = sys.__stderr__\n"
			"_temp_out_ = _temp_out_.getvalue()\n"
			"_temp_err_ = _temp_err_.getvalue()";

		// Execute script and capture output
		//
		py::exec(redirectPyOut.c_str(), m_mainNamespace);
		py::exec(m_script.c_str(), m_mainNamespace);
		py::exec(resetPyOut.c_str(), m_mainNamespace);

		string pyStdOut = py::extract<string>(m_mainNamespace["_temp_out_"]);
		string pyStdErr = py::extract<string>(m_mainNamespace["_temp_err_"]);

		cout << pyStdOut << endl;
		cerr << pyStdErr << endl;
	}
	catch (py::error_already_set &)
	{
		string pyError = PythonExtensionUtils::ParsePythonException();
		LOG_ERROR(pyError);
		throw runtime_error("Error running python:\n" + pyError);
	}
}

// Get the metadata for the output column
//
void PythonSession::GetResultColumn(
	SQLUSMALLINT columnNumber,
	SQLSMALLINT  *dataType,
	SQLULEN      *columnSize,
	SQLSMALLINT  *decimalDigits,
	SQLSMALLINT  *nullable)
{
	LOG("PythonSession::GetResultColumn");
}

// Get the results
//
void PythonSession::GetResults(
	SQLULEN    *rowsNumber,
	SQLPOINTER **data,
	SQLINTEGER ***strLen_or_Ind)
{
	LOG("PythonSession::GetResults");
}

// Get the the output parameter
//
void PythonSession::GetOutputParam(
	SQLUSMALLINT paramNumber,
	SQLPOINTER   *paramValue,
	SQLINTEGER   *strLen_or_Ind)
{
	LOG("PythonSession::GetOutputParam");
}

// Cleanup session
//
void PythonSession::Cleanup()
{
	LOG("PythonSession::Cleanup");
}
