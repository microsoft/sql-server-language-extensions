//*********************************************************************
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
//*********************************************************************

#include "Logger.h"
#include "PythonSession.h"
#include "PythonTypeUtils.h"

using namespace std;
namespace py = boost::python;

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

	// Store the number of input columns and parameters
	//
	m_inputSchemaColumnsNumber = inputSchemaColumnsNumber;
	m_parametersNumber = parametersNumber;
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

	if (columnName == nullptr)
	{
		throw invalid_argument("Invalid input column name supplied");
	}
	else if (columnNumber != m_inputColumns.size() ||  // Confirm that columnNumber is the next column index
			columnNumber >= m_inputSchemaColumnsNumber)
	{
		throw invalid_argument("Invalid input column id supplied: " + to_string(columnNumber));
	}

	// Store the information for this column
	//
	string name(reinterpret_cast<const char*>(columnName), columnNameLength - 1); // Remove null terminator

	m_inputColumns.emplace_back(PythonColumn(name, dataType, columnSize, nullable, decimalDigits));
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
	SQLSMALLINT   InputOutputType)
{
	LOG("PythonSession::InitParam");

	if (paramName == nullptr)
	{
		throw invalid_argument("Invalid input parameter name supplied");
	}
	else if (paramNumber >= m_parametersNumber)
	{
		throw invalid_argument("Invalid input param id supplied: " + to_string(paramNumber));
	}

	// +1 removes the @ in front of the parameter name, -1 to remove it from the length
	//
	string name(reinterpret_cast<const char*>((paramName + 1)), paramNameLength - 1);

	LOG("PythonSession::InitParam: Initializing parameter " + name);

	// Add input parameters to the namespace
	//
	PythonTypeUtils::AddParamToNamespace(
		m_mainNamespace,
		name,
		dataType,
		paramSize,
		decimalDigits,
		paramValue,
		strLen_or_Ind);
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
	SQLPOINTER  *paramValue,
	SQLINTEGER  *strLen_or_Ind)
{
	LOG(("PythonSession::GetOutputParam"));
}

// Cleanup session
//
void PythonSession::Cleanup()
{
	LOG("PythonSession::Cleanup");
}
