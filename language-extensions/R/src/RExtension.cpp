//*************************************************************************************************
// RExtension : A language extension implementing the SQL Server
// external language communication protocol for R.
// Copyright (C) 2019 Microsoft Corporation.
//
// This file is part of RExtension.
//
// RExtension is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RExtension is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RExtension.  If not, see <https://www.gnu.org/licenses/>.
//
//  @File: RExtension.cpp
//
// Purpose:
//  RExtension DLL that can be loaded by ExtHost. This library inits embedded R,
//  handles communication with ExtHost, and executes user-specified R script
//
//*************************************************************************************************
#include "Common.h"

#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>

#include "RColumn.h"
#include "RDataSet.h"
#include "RParam.h"
#include "RParamContainer.h"
#include "RPathSettings.h"
#include "RSession.h"
#include "sqlexternallanguage.h"

#ifndef _WIN64
#include <linux/limits.h>
#include <string.h>
#endif

using namespace std;

// A global object to keep track of the input/output data information
//
static RSession *g_sessionData = nullptr;

// A reference to the embedded R environment via RInside.
//
unique_ptr<RInside> g_embeddedRPtr = nullptr;

//--------------------------------------------------------------------------------------------------
// Name: CheckSessionEnvInitialized
//
// Description:
//	Throws a runtime_error exception of a function is called before the session environment has
//	been initialized
//
void CheckSessionEnvInitialized(string &&FuncName)
{
	// R_GlobalEnv is defined in Rinternals.h available for use once Rf_initEmbeddedR has been called
	//
	if (R_GlobalEnv == nullptr)
	{
		throw runtime_error("Function " + FuncName + " called before extension is initialized");
	}
	else if (g_sessionData == nullptr)
	{
		throw runtime_error("Function " + FuncName + " called before session is initialized");
	}
}

//--------------------------------------------------------------------------------------------------
// Name: GetInterfaceVersion
//
// Description:
//	Returns the API interface version for the extension
//
// Returns:
//	EXTERNAL_LANGUAGE_EXTENSION_API
//
SQLUSMALLINT GetInterfaceVersion()
{
	return EXTERNAL_LANGUAGE_EXTENSION_API;
}

//--------------------------------------------------------------------------------------------------
// Name: Init
//
// Description:
//	Initialize the RExtension.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
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
	LOG("RExtension::Init");

	SQLRETURN result = SQL_ERROR;

	try
	{
		RPathSettings::Init(
			extensionParams,
			extensionPath,
			publicLibraryPath,
			privateLibraryPath);

		RPathSettings::CheckAndSetRHome();

		// Setting up the parameters to be passed to the R runtime.
		//
		vector<char*> argsForR;
		unique_ptr<char[]> dummyInputScriptPtr = Utilities::GenerateUniquePtr("dummyInputScript");
		unique_ptr<char[]> noSaveOptionPtr = Utilities::GenerateUniquePtr("--no-save");
		unique_ptr<char[]> extensionParamsPtr = Utilities::GenerateUniquePtr(RPathSettings::Params());

		if (extensionParamsLength > 0)
		{
			// Tokenize the command line to find the arguments for R.
			//
			Utilities::Tokenize(extensionParamsPtr.get(), " ", &argsForR);
		}
		else
		{
			// If no command line, pass in a dummy input script
			// Rf_initEmbeddedR needs an input script parameter as the first argument even if it isn't used.
			//
			argsForR.push_back(dummyInputScriptPtr.get());
		}

		// --no-save is required to indicate that it is a non-interactive session. It can't be the first
		// argument otherwise it is considered as the script.
		//
		argsForR.push_back(noSaveOptionPtr.get());

		// Initialize the R runtime using the parameters set above.
		//
		g_embeddedRPtr = make_unique<RInside>(argsForR.size(), argsForR.data());

		if (R_GlobalEnv != nullptr && g_embeddedRPtr != nullptr)
		{
			result = SQL_SUCCESS;
		}

		LOG("RExtension::Init done with return code " + to_string(result));
	}
	catch (exception &ex)
	{
		result = SQL_ERROR;

		LOG_EXCEPTION(ex);
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function Init()");
	}

	return result;
}

// --------------------------------------------------------------------------------------------------
// Name: InitSession

// Description:
// Initializes session-specific data. We store the schema, parameter info, output input data info here

// Returns:
// SQL_SUCCESS on success, else SQL_ERROR

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
	string msg = "RExtension::InitSession";
	LOG(msg);

	SQLRETURN result = SQL_SUCCESS;

	try
	{
		// R_GlobalEnv is defined in Rinternals.h available for use once Rf_initEmbeddedR has been called
		//
		if (R_GlobalEnv == nullptr)
		{
			throw runtime_error("Function InitSession() called before extension is initialized");
		}

		g_sessionData = new RSession();

		string guidToString = Utilities::ConvertGuidToString(&sessionId);

		msg = "Starting session: " + guidToString + " with task id: " +
			to_string(taskId) + ".";
		LOG(msg);

		g_sessionData->Init(sessionId,
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
	catch (exception &ex)
	{
		result = SQL_ERROR;

		LOG_EXCEPTION(ex);
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
	LOG("RExtension::InitColumn");

	SQLRETURN result = SQL_SUCCESS;

	try
	{
		CheckSessionEnvInitialized("InitColumn");

		g_sessionData->InitColumn(
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
	catch (exception &ex)
	{
		result = SQL_ERROR;

		LOG_EXCEPTION(ex);
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
//	Initializes parameter-specific data.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
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
	SQLSMALLINT  inputOutputType
)
{
	LOG("RExtension::InitParam");

	SQLRETURN result = SQL_SUCCESS;

	try
	{
		CheckSessionEnvInitialized("InitParam");

		g_sessionData->InitParam(
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
	catch (exception &ex)
	{
		result = SQL_ERROR;

		LOG_EXCEPTION(ex);
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
//  Given the data from ExtHost, convert and populate the arrays in the user script. Then,
//  invoke the specified script and retrieve the output schema and convert the data back to
//  ODBC types.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
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
	LOG("RExtension::Execute");

	SQLRETURN result = SQL_SUCCESS;
	*outputSchemaColumnsNumber = 0;

	try
	{
		CheckSessionEnvInitialized("Execute");

		g_sessionData->ExecuteWorkflow(
			rowsNumber,
			data,
			strLen_or_Ind,
			outputSchemaColumnsNumber);
	}
	catch (exception &ex)
	{
		result = SQL_ERROR;

		LOG_EXCEPTION(ex);
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
	LOG("RExtension::GetResultColumn");

	SQLRETURN result = SQL_SUCCESS;

	try
	{
		CheckSessionEnvInitialized("GetResultColumn");

		g_sessionData->GetResultColumn(
			columnNumber,
			dataType,
			columnSize,
			decimalDigits,
			nullable);
	}
	catch (exception &ex)
	{
		result = SQL_ERROR;

		LOG_EXCEPTION(ex);
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function GetResultColumn()");
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
	LOG("RExtension::GetResults");

	SQLRETURN result = SQL_SUCCESS;

	try
	{
		CheckSessionEnvInitialized("GetResults");

		g_sessionData->GetResults(
			rowsNumber,
			data,
			strLen_or_Ind);
	}
	catch (exception &ex)
	{
		result = SQL_ERROR;

		LOG_EXCEPTION(ex);
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
	SQLINTEGER   *strLen_or_Ind)
{
	LOG("RExtension::GetOutputParam");

	SQLRETURN result = SQL_SUCCESS;

	try
	{
		CheckSessionEnvInitialized("GetOutputParam");

		g_sessionData->GetOutputParam(
			paramNumber,
			paramValue,
			strLen_or_Ind);
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_EXCEPTION(ex);
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
	LOG("RExtension::CleanupSession");

	// Clean up the session
	//
	if (g_sessionData != nullptr)
	{
		g_sessionData->Cleanup();

		delete g_sessionData;
		g_sessionData = nullptr;
	}

	return SQL_SUCCESS;
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
	LOG("RExtension::Cleanup");

	// End Embedded R - usually done by calling Rf_endEmbdeddR(0)
	// However, with RInside, that is not needed here.
	// When g_embeddedRPtr goes out of scope, its destructor will end embeddedR
	//

	return SQL_SUCCESS;
}
