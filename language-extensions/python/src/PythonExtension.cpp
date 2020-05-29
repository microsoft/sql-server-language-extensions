//**************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonExtension.cpp
//
// Purpose:
//  Python extension DLL that can be loaded by ExtHost. This library loads the
//  Python dll, handles communication with ExtHost, and executes user-specified
//  Python script
//
//**************************************************************************************************

#include <boost/python.hpp>
#include <unordered_map>

#include "Logger.h"
#include "PythonExtensionUtils.h"
#include "PythonSession.h"
#include "sqlexternallanguage.h"

using namespace std;
namespace py = boost::python;

static unordered_map<string, PythonSession *> g_pySessionMap;

//--------------------------------------------------------------------------------------------------
// Name: GetInterfaceVersion
//
// Description:
// Returns the API interface version for the extension
//
// Returns:
// EXTERNAL_LANGUAGE_EXTENSION_API
//
SQLUSMALLINT
GetInterfaceVersion()
{
	return EXTERNAL_LANGUAGE_EXTENSION_API;
}

//--------------------------------------------------------------------------------------------------
// Name: Init
//
// Description:
//  Initialize the python extension. Until registration, nothing is needed here.
//  We call Py_Initialize to initialize python in C++ and allow boost to work
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN Init(
	SQLCHAR *extensionParams,
	SQLULEN extensionParamsLength,
	SQLCHAR *extensionPath,
	SQLULEN extensionPathLength,
	SQLCHAR *publicLibraryPath,
	SQLULEN publicLibraryPathLength,
	SQLCHAR *privateLibraryPath,
	SQLULEN privateLibraryPathLength
)
{
	LOG("Init");

	SQLRETURN result = SQL_SUCCESS;

	try
	{
		// Initialize Python using the Python/C API.
		// This allows us to start using Python API and boost functions.
		//
		Py_Initialize();

		if (!Py_IsInitialized()) 
		{
			throw runtime_error("Python did not initialize properly, "
				"check paths and dependencies.");
		}

		py::numpy::initialize();
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (const py::error_already_set&)
	{
		result = SQL_ERROR;

		LOG_ERROR("Python error: " + PythonExtensionUtils::ParsePythonException());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function Init()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: InitSession
//
// Description:
//  Initializes session-specific data. We store the schema and find the main class and
//  method to execute here.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN InitSession(
	SQLGUID      sessionId,
	SQLUSMALLINT taskId,
	SQLUSMALLINT numTasks,
	SQLCHAR      *script,
	SQLULEN      scriptLength,
	SQLUSMALLINT inputSchemaColumnsNumber,
	SQLUSMALLINT parametersNumber,
	SQLCHAR      *inputDataName,
	SQLUSMALLINT inputDataNameLength,
	SQLCHAR      *outputDataName,
	SQLUSMALLINT outputDataNameLength
)
{
	LOG("InitSession");
	SQLRETURN result = SQL_SUCCESS;

	string session = PythonExtensionUtils::ConvertGuidToString(&sessionId);
	try
	{
		g_pySessionMap[session] = new PythonSession();
		g_pySessionMap[session]->Init(
			&sessionId,
			taskId,
			numTasks,
			script,
			scriptLength,
			inputSchemaColumnsNumber,
			parametersNumber,
			inputDataName,
			inputDataNameLength,
			outputDataName,
			outputDataNameLength);
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (const py::error_already_set&)
	{
		result = SQL_ERROR;

		LOG_ERROR("Python error: " + PythonExtensionUtils::ParsePythonException());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function InitSession()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: InitColumn
//
// Description:
//  Initializes column-specific data. We store the name and the data type of the column
//  here.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN InitColumn(
	SQLGUID      sessionId,
	SQLUSMALLINT taskId,
	SQLUSMALLINT columnNumber,
	SQLCHAR      *columnName,
	SQLSMALLINT  columnNameLength,
	SQLSMALLINT  dataType,
	SQLULEN      columnSize,
	SQLSMALLINT  decimalDigits,
	SQLSMALLINT  nullable,
	SQLSMALLINT  partitionByNumber,
	SQLSMALLINT  orderByNumber
)
{
	LOG("InitColumn");
	SQLRETURN result = SQL_SUCCESS;
	string session = PythonExtensionUtils::ConvertGuidToString(&sessionId);
	try
	{
		g_pySessionMap[session]->InitColumn(
			columnNumber,
			columnName,
			columnNameLength,
			dataType,
			columnSize,
			decimalDigits,
			nullable,
			partitionByNumber,
			orderByNumber);
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (const py::error_already_set&)
	{
		result = SQL_ERROR;

		LOG_ERROR("Python error: " + PythonExtensionUtils::ParsePythonException());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function InitColumn()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: InitParam
//
// Description:
//  Initializes parameter-specific data.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN InitParam(
	SQLGUID      sessionId,
	SQLUSMALLINT taskId,
	SQLUSMALLINT paramNumber,
	SQLCHAR      *paramName,
	SQLSMALLINT  paramNameLength,
	SQLSMALLINT  dataType,
	SQLULEN      paramSize,
	SQLSMALLINT  decimalDigits,
	SQLPOINTER   paramValue,
	SQLINTEGER   strLen_or_Ind,
	SQLSMALLINT  inputOutputType)
{
	LOG("InitParam");
	SQLRETURN result = SQL_SUCCESS;
	string session = PythonExtensionUtils::ConvertGuidToString(&sessionId);
	try
	{
		g_pySessionMap[session]->InitParam(
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
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (const py::error_already_set&)
	{
		result = SQL_ERROR;

		LOG_ERROR("Python error: " + PythonExtensionUtils::ParsePythonException());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function InitParam()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: Execute
//
// Description:
//  Given the data from ExtHost, convert and populate the arrays in the user python program. Then,
//  invoke the specified function and retrieve the output schema and convert the data back to
//  ODBC types.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN Execute(
	SQLGUID      sessionId,
	SQLUSMALLINT taskId,
	SQLULEN      rowsNumber,
	SQLPOINTER   *data,
	SQLINTEGER   **strLen_or_Ind,
	SQLUSMALLINT *outputSchemaColumnsNumber
)
{
	LOG("Execute");
	SQLRETURN result = SQL_SUCCESS;
	string session = PythonExtensionUtils::ConvertGuidToString(&sessionId);
	try
	{
		g_pySessionMap[session]->ExecuteWorkflow(rowsNumber,
									data,
									strLen_or_Ind,
									outputSchemaColumnsNumber);
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (const py::error_already_set&)
	{
		result = SQL_ERROR;

		LOG_ERROR("Python error: " + PythonExtensionUtils::ParsePythonException());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function Execute()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: GetResultColumn
//
// Description:
//  Returns information about the output column
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN GetResultColumn(
	SQLGUID      sessionId,
	SQLUSMALLINT taskId,
	SQLUSMALLINT columnNumber,
	SQLSMALLINT  *dataType,
	SQLULEN      *columnSize,
	SQLSMALLINT  *decimalDigits,
	SQLSMALLINT  *nullable
)
{
	LOG("GetResultColumn");
	SQLRETURN result = SQL_SUCCESS;
	string session = PythonExtensionUtils::ConvertGuidToString(&sessionId);
	try
	{
		g_pySessionMap[session]->GetResultColumn(
			columnNumber,
			dataType,
			columnSize,
			decimalDigits,
			nullable);
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (const py::error_already_set&)
	{
		result = SQL_ERROR;

		LOG_ERROR("Python error: " + PythonExtensionUtils::ParsePythonException());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function Execute()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: GetResults
//
// Description:
//	Returns the output data as well as the null map retrieved from the user program
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN GetResults(
	SQLGUID      sessionId,
	SQLUSMALLINT taskId,
	SQLULEN      *rowsNumber,
	SQLPOINTER   **data,
	SQLINTEGER   ***strLen_or_Ind
)
{
	LOG("GetResults");
	SQLRETURN result = SQL_SUCCESS;
	string session = PythonExtensionUtils::ConvertGuidToString(&sessionId);
	try
	{
		g_pySessionMap[session]->GetResults(
			rowsNumber,
			data,
			strLen_or_Ind);
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (const py::error_already_set&)
	{
		result = SQL_ERROR;

		LOG_ERROR("Python error: " + PythonExtensionUtils::ParsePythonException());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function GetResults()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: GetOutputParam
//
// Description:
//	Returns the output parameter's data.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN GetOutputParam(
	SQLGUID      sessionId,
	SQLUSMALLINT taskId,
	SQLUSMALLINT paramNumber,
	SQLPOINTER   *paramValue,
	SQLINTEGER   *strLen_or_Ind
)
{
	LOG("GetOutputParam");
	SQLRETURN result = SQL_SUCCESS;
	string session = PythonExtensionUtils::ConvertGuidToString(&sessionId);
	try
	{
		g_pySessionMap[session]->GetOutputParam(
			paramNumber,
			paramValue,
			strLen_or_Ind);
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (const py::error_already_set&)
	{
		result = SQL_ERROR;

		LOG_ERROR("Python error: " + PythonExtensionUtils::ParsePythonException());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function GetOutputParam()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: CleanupSession
//
// Description:
//	Cleans up the output data buffers that we persist for
//	ExtHost to finish processing the data
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN CleanupSession(
	SQLGUID      sessionId,
	SQLUSMALLINT taskId
)
{
	LOG("CleanupSession");
	SQLRETURN result = SQL_SUCCESS;
	string session = PythonExtensionUtils::ConvertGuidToString(&sessionId);
	try
	{
		if (g_pySessionMap.count(session) > 0)
		{
			g_pySessionMap[session]->Cleanup();
			delete g_pySessionMap[session];
			g_pySessionMap.erase(session);
		}
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (const py::error_already_set&)
	{
		result = SQL_ERROR;

		LOG_ERROR("Python error: " + PythonExtensionUtils::ParsePythonException());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function CleanupSession()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: Cleanup
//
// Description:
//	Completely clean up the extension
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN Cleanup()
{
	LOG("Cleanup");
	SQLRETURN result = SQL_SUCCESS;

	return result;
}
