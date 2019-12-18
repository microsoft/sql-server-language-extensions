//*********************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: pythonextension.cpp
//
// Purpose:
//  Python extension DLL that can be loaded by ExtHost. This library loads the
//  Python dll, handles communication with ExtHost, and executes user-specified
//  Python script
//
//*********************************************************************

#include "Logger.h"
#include "PythonSession.h"
#include "PythonTypeUtils.h"
#include "sqlexternallanguage.h"
#include "PythonExtensionUtils.h"

using namespace std;

// The version of python to load as DLL
//
static const string x_PythonVersion = "python37";

static void *g_pyDLL = nullptr;
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
	SQLCHAR *ExtensionParams,
	SQLULEN ExtensionParamsLength,
	SQLCHAR *ExtensionPath,
	SQLULEN ExtensionPathLength,
	SQLCHAR *PublicLibraryPath,
	SQLULEN PublicLibraryPathLength,
	SQLCHAR *PrivateLibraryPath,
	SQLULEN PrivateLibraryPathLength
)
{
	LOG("Init");

	SQLRETURN result = SQL_SUCCESS;
	string pythonHomePath = "";

	try
	{
		pythonHomePath = string(reinterpret_cast<char*>(ExtensionPath), ExtensionPathLength - 1);
		pythonHomePath = PythonExtensionUtils::GetEnvVariable("PYTHONHOME");
	}
	catch (...)
	{
		LOG("Falling back to default python path: " + pythonHomePath);
	}

	try
	{
		// Initialize Python using the Python/C API.
		// This allows us to start using Python API and boost functions.
		//
		Py_Initialize();
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
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
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT NumTasks,
	SQLCHAR		 *Script,
	SQLULEN		 ScriptLength,
	SQLUSMALLINT InputSchemaColumnsNumber,
	SQLUSMALLINT ParametersNumber,
	SQLCHAR		 *InputDataName,
	SQLUSMALLINT InputDataNameLength,
	SQLCHAR		 *OutputDataName,
	SQLUSMALLINT OutputDataNameLength
)
{
	LOG("InitSession");
	SQLRETURN result = SQL_SUCCESS;

	string session = PythonExtensionUtils::ConvertGuidToString(&SessionId);
	try
	{
		g_pySessionMap[session] = new PythonSession();
		g_pySessionMap[session]->Init(
			&SessionId,
			TaskId,
			NumTasks,
			Script,
			ScriptLength,
			InputSchemaColumnsNumber,
			ParametersNumber,
			InputDataName,
			InputDataNameLength,
			OutputDataName,
			OutputDataNameLength);
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
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
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ColumnNumber,
	SQLCHAR		 *ColumnName,
	SQLSMALLINT	 ColumnNameLength,
	SQLSMALLINT	 DataType,
	SQLULEN		 ColumnSize,
	SQLSMALLINT	 DecimalDigits,
	SQLSMALLINT	 Nullable,
	SQLSMALLINT	 PartitionByNumber,
	SQLSMALLINT	 OrderByNumber
)
{
	LOG("InitColumn");
	SQLRETURN result = SQL_SUCCESS;
	string session = PythonExtensionUtils::ConvertGuidToString(&SessionId);
	try
	{
		g_pySessionMap[session]->InitColumn(
			ColumnNumber,
			ColumnName,
			ColumnNameLength,
			DataType,
			ColumnSize,
			DecimalDigits,
			Nullable,
			PartitionByNumber,
			OrderByNumber);
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
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
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ParamNumber,
	SQLCHAR		 *ParamName,
	SQLSMALLINT	 ParamNameLength,
	SQLSMALLINT	 DataType,
	SQLULEN		 ParamSize,
	SQLSMALLINT	 DecimalDigits,
	SQLPOINTER	 ParamValue,
	SQLINTEGER	 StrLen_or_Ind,
	SQLSMALLINT	 InputOutputType)
{
	LOG("InitParam");
	SQLRETURN result = SQL_SUCCESS;
	string session = PythonExtensionUtils::ConvertGuidToString(&SessionId);
	try
	{
		g_pySessionMap[session]->InitParam(
			ParamNumber,
			ParamName,
			ParamNameLength,
			DataType,
			ParamSize,
			DecimalDigits,
			ParamValue,
			StrLen_or_Ind,
			InputOutputType);
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
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
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId,
	SQLULEN		 RowsNumber,
	SQLPOINTER	 *Data,
	SQLINTEGER	 **StrLen_or_Ind,
	SQLUSMALLINT *OutputSchemaColumnsNumber
)
{
	LOG("Execute");
	SQLRETURN result = SQL_SUCCESS;
	string session = PythonExtensionUtils::ConvertGuidToString(&SessionId);
	try
	{
		g_pySessionMap[session]->ExecuteWorkflow(RowsNumber,
									Data,
									StrLen_or_Ind,
									OutputSchemaColumnsNumber);
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function Execute()");
	}

	return result;
}

SQLRETURN GetResultColumn(
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ColumnNumber,
	SQLSMALLINT	 *DataType,
	SQLULEN		 *ColumnSize,
	SQLSMALLINT	 *DecimalDigits,
	SQLSMALLINT	 *Nullable
)
{
	LOG("GetResultColumn");
	SQLRETURN result = SQL_SUCCESS;
	string session = PythonExtensionUtils::ConvertGuidToString(&SessionId);
	try
	{
		g_pySessionMap[session]->GetResultColumn(
			ColumnNumber,
			DataType,
			ColumnSize,
			DecimalDigits,
			Nullable);
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function Execute()");
	}

	return result;
}

SQLRETURN GetResults(
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId,
	SQLULEN		 *RowsNumber,
	SQLPOINTER	 **Data,
	SQLINTEGER	 ***StrLen_or_Ind
)
{
	LOG("GetResults");
	SQLRETURN result = SQL_SUCCESS;
	string session = PythonExtensionUtils::ConvertGuidToString(&SessionId);
	try
	{
		g_pySessionMap[session]->GetResults(
			RowsNumber,
			Data,
			StrLen_or_Ind);
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function GetResults()");
	}

	return result;
}

SQLRETURN GetOutputParam(
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ParamNumber,
	SQLPOINTER	 *ParamValue,
	SQLINTEGER	 *StrLen_or_Ind
)
{
	LOG("GetOutputParam");
	SQLRETURN result = SQL_SUCCESS;
	string session = PythonExtensionUtils::ConvertGuidToString(&SessionId);
	try
	{
		g_pySessionMap[session]->GetOutputParam(
			ParamNumber,
			ParamValue,
			StrLen_or_Ind);
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function GetOutputParam()");
	}

	return result;
}

SQLRETURN CleanupSession(
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId
)
{
	LOG("CleanupSession");
	SQLRETURN result = SQL_SUCCESS;
	string session = PythonExtensionUtils::ConvertGuidToString(&SessionId);
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
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function CleanupSession()");
	}

	return result;
}

SQLRETURN Cleanup()
{
	LOG("Cleanup");
	SQLRETURN result = SQL_SUCCESS;
	try
	{
		PythonExtensionUtils::FreeDLL(g_pyDLL);
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function Cleanup()");
	}

	return result;
}
