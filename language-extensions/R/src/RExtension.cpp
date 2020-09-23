//**************************************************************************************************
// RExtension : A language extension implementing the SQL Server
// external language communication protocol for R.
// Copyright (C) 2020 Microsoft Corporation.
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
//**************************************************************************************************

#include "Common.h"

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

#include "RColumn.h"
#include "RDataSet.h"
#include "RLibrarySession.h"
#include "RParam.h"
#include "RParamContainer.h"
#include "RSession.h"

// ODBCVER is defined in sql.h undefine it to avoid redefinition warnings when it is defined in
// sqlexternallanguage.h
//
#ifdef ODBCVER
	#undef ODBCVER
#endif
#include "sqlexternallanguage.h"
#include "sqlexternallibrary.h"

#ifndef _WIN64
#include <linux/limits.h>
#include <string.h>
#endif

using namespace std;

// A global map to keep track of the input/output data information per session.
//
static unordered_map<string, unique_ptr<RSession>> g_RSessionMap;

//--------------------------------------------------------------------------------------------------
// Name: GetInterfaceVersion
//
// Description:
//  Returns the API interface version for the extension
//
// Returns:
//  EXTERNAL_LANGUAGE_EXTENSION_API
//
SQLUSMALLINT GetInterfaceVersion()
{
	return EXTERNAL_LANGUAGE_EXTENSION_API;
}

//--------------------------------------------------------------------------------------------------
// Name: Init
//
// Description:
//  Initializes the RExtension.
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
		RPathSettings::CheckAndSetTZDir();

		REnvironment::Init(extensionParamsLength);

		if (R_GlobalEnv != nullptr)
		{
			result = SQL_SUCCESS;
		}

		LOG("RExtension::Init done with return code " + to_string(result));
	}
	catch (exception &ex)
	{
		LOG_EXCEPTION(ex);
	}
	catch (...)
	{
		LOG_ERROR("Unexpected exception occurred in function Init().");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: CheckAndGetSession
//
// Description:
//  Looks up the session map for an RSession pointer corresponding to sessionId after converting
//  it into a string.
//  If it is not found, initializes the session only if the funcName is InitSession.
//  else, throws a runtime_error exception stating that the given funcName is called
//  before the session environment is initialized.
//  On the other hand, if it is found, returns the found session only if funcName is NOT InitSession.
//  else, throws a runtime_error exception stating that session cannot reinitialized before cleanup.
//
// Returns:
//  The RSession pointer for given sessionId and taskId
//
RSession* CheckAndGetSession(string &&funcName, SQLGUID sessionId, SQLUSMALLINT taskId)
{
	string sessionIdString = Utilities::ConvertGuidToString(&sessionId);
	unordered_map<string, unique_ptr<RSession>>::const_iterator it =
		g_RSessionMap.find(sessionIdString);

	RSession* session = nullptr;
	bool isInitSession = funcName.compare("InitSession") == 0;
	bool isCleanupSession = funcName.compare("CleanupSession") == 0;

	if (it == g_RSessionMap.end())
	{
		if (isInitSession)
		{
			string msg = "Starting session: " + sessionIdString + " with task id: " +
				to_string(taskId) + ".";
			LOG(msg);

			g_RSessionMap[sessionIdString] = make_unique<RSession>();
			session = g_RSessionMap[sessionIdString].get();
		}
		else if (!isCleanupSession)
		{
			// Cleanup session can be called before session is initialized in case it is a
			// library session.
			//
			throw runtime_error("Function " + funcName + " called before session id "
				+ sessionIdString + " is initialized.");
		}
	}
	else
	{
		if (!isInitSession)
		{
			session = it->second.get();
		}
		else
		{
			throw runtime_error("Session " + sessionIdString + " cannot be reinitialized"
				" without cleaning up first.");
		}
	}

	return session;
}

// --------------------------------------------------------------------------------------------------
// Name: InitSession

// Description:
//  Initializes session-specific data. We store the schema, parameter info,
//  output input data info here.

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
	string msg = "RExtension::InitSession";
	LOG(msg);

	SQLRETURN result = SQL_ERROR;

	try
	{
		// R_GlobalEnv is defined in Rinternals.h available for use once Rf_initEmbeddedR has been called
		//
		if (R_GlobalEnv == nullptr)
		{
			throw runtime_error("Function InitSession() called before R environment is initialized.");
		}

		RSession* session = CheckAndGetSession("InitSession", sessionId, taskId);
		session->Init(
			sessionId,
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

		result = SQL_SUCCESS;
	}
	catch (exception &ex)
	{
		LOG_EXCEPTION(ex);
	}
	catch (...)
	{
		LOG_ERROR("Unexpected exception occurred in function InitSession().");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: InitColumn
//
// Description:
//  Initializes column-specific data. We store the name and the data type of the column here.
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

	SQLRETURN result = SQL_ERROR;

	try
	{
		RSession* session = CheckAndGetSession("InitColumn", sessionId, taskId);
		session->InitColumn(
			columnNumber,
			columnName,
			columnNameLength,
			dataType,
			columnSize,
			decimalDigits,
			nullable,
			partitionByNumber,
			orderByNumber);

		result = SQL_SUCCESS;
	}
	catch (exception &ex)
	{
		LOG_EXCEPTION(ex);
	}
	catch (...)
	{
		LOG_ERROR("Unexpected exception occurred in function InitColumn().");
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
	SQLSMALLINT  inputOutputType
)
{
	LOG("RExtension::InitParam");

	SQLRETURN result = SQL_ERROR;

	try
	{
		RSession* session = CheckAndGetSession("InitParam", sessionId, taskId);
		session->InitParam(
			paramNumber,
			paramName,
			paramNameLength,
			dataType,
			paramSize,
			decimalDigits,
			paramValue,
			strLen_or_Ind,
			inputOutputType);

		result = SQL_SUCCESS;
	}
	catch (exception &ex)
	{
		LOG_EXCEPTION(ex);
	}
	catch (...)
	{
		LOG_ERROR("Unexpected exception occurred in function InitParam().");
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

	SQLRETURN result = SQL_ERROR;
	*outputSchemaColumnsNumber = 0;

	try
	{
		RSession* session = CheckAndGetSession("Execute", sessionId, taskId);
		session->ExecuteWorkflow(
			rowsNumber,
			data,
			strLen_or_Ind,
			outputSchemaColumnsNumber);

		result = SQL_SUCCESS;
	}
	catch (exception &ex)
	{
		LOG_EXCEPTION(ex);
	}
	catch (...)
	{
		LOG_ERROR("Unexpected exception occurred in function Execute().");
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
//  SQL_SUCCESS on success, else SQL_ERROR
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

	SQLRETURN result = SQL_ERROR;

	try
	{
		RSession* session = CheckAndGetSession("GetResultColumn", sessionId, taskId);
		session->GetResultColumn(
			columnNumber,
			dataType,
			columnSize,
			decimalDigits,
			nullable);

		result = SQL_SUCCESS;
	}
	catch (exception &ex)
	{
		LOG_EXCEPTION(ex);
	}
	catch (...)
	{
		LOG_ERROR("Unexpected exception occurred in function GetResultColumn().");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: GetResults
//
// Description:
//  Returns the output data as well as the null map retrieved from the user program
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
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

	SQLRETURN result = SQL_ERROR;

	try
	{
		RSession* session = CheckAndGetSession("GetResults", sessionId, taskId);
		session->GetResults(
			rowsNumber,
			data,
			strLen_or_Ind);

		result = SQL_SUCCESS;
	}
	catch (exception &ex)
	{
		result = SQL_ERROR;

		LOG_EXCEPTION(ex);
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function GetResults().");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: GetOutputParam
//
// Description:
//  Returns the output parameter's data.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN GetOutputParam(
	SQLGUID      sessionId,
	SQLUSMALLINT taskId,
	SQLUSMALLINT paramNumber,
	SQLPOINTER   *paramValue,
	SQLINTEGER   *strLen_or_Ind)
{
	LOG("RExtension::GetOutputParam");

	SQLRETURN result = SQL_ERROR;

	try
	{
		RSession* session = CheckAndGetSession("GetOutputParam", sessionId, taskId);
		session->GetOutputParam(
			paramNumber,
			paramValue,
			strLen_or_Ind);

		result = SQL_SUCCESS;
	}
	catch (const exception &ex)
	{
		LOG_EXCEPTION(ex);
	}
	catch (...)
	{
		LOG_ERROR("Unexpected exception occurred in function GetOutputParam().");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: CleanupSession
//
// Description:
//  Cleans up the output data buffers that we persist for
//  ExtHost to finish processing the data
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN CleanupSession(
	SQLGUID      sessionId,
	SQLUSMALLINT taskId
)
{
	LOG("RExtension::CleanupSession");

	SQLRETURN result = SQL_ERROR;

	try
	{
		RSession* session = CheckAndGetSession("CleanupSession", sessionId, taskId);

		// Clean up the session; it could be nullptr in case of library sessions
		// because we don't add library sessions to the map.
		//
		if (session != nullptr)
		{
			session->Cleanup();

			string sessionIdString = Utilities::ConvertGuidToString(&sessionId);
			g_RSessionMap.erase(sessionIdString);
		}

		result = SQL_SUCCESS;
	}
	catch (const exception &ex)
	{
		LOG_EXCEPTION(ex);
	}
	catch (...)
	{
		LOG_ERROR("Unexpected exception occurred in function CleanupSession().");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: Cleanup
//
// Description:
//  Completely cleans up the extension
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN Cleanup()
{
	LOG("RExtension::Cleanup");

	SQLRETURN result = SQL_ERROR;

	try
	{
		// End Embedded R - usually done by calling Rf_endEmbdeddR(0)
		// However, with RInside, that is not needed here.
		// When REnvironment::sm_embeddedREnvPtr - the unique pointer to RInside goes out of scope,
		// it will be destructed and that will end embeddedR.
		//
		REnvironment::Cleanup();

		result = SQL_SUCCESS;
	}
	catch (const exception &ex)
	{
		LOG_EXCEPTION(ex);
	}
	catch (...)
	{
		LOG_ERROR("Unexpected exception occurred in function Cleanup().");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// External Library APIs
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: InstallExternalLibrary
//
// Description:
//  Installs an external library to the specified directory
//  The library file is expected to be a zip containing the R package
//  which itself could be a zip or tar.gz file.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN InstallExternalLibrary(
	SQLGUID    setupSessionId,
	SQLCHAR    *libraryName,
	SQLINTEGER libraryNameLength,
	SQLCHAR    *libraryFile,
	SQLINTEGER libraryFileLength,
	SQLCHAR    *libraryInstallDirectory,
	SQLINTEGER libraryInstallDirectoryLength,
	SQLCHAR    **libraryError,
	SQLINTEGER *libraryErrorLength)
{
	LOG("RExtension::InstallExternalLibrary");
	SQLRETURN result = SQL_ERROR;
	string exceptionString;

	try
	{
		RLibrarySession librarySession;

		librarySession.Init(
			setupSessionId,
			libraryName,
			libraryNameLength);

		result = librarySession.InstallLibrary(
			libraryFile,
			libraryFileLength,
			libraryInstallDirectory,
			libraryInstallDirectoryLength);
	}
	catch (const exception & ex)
	{
		LOG_EXCEPTION(ex);
		exceptionString = string(ex.what());
	}
	catch (...)
	{
		exceptionString = "Unexpected exception in function InstallExternalLibrary().";
		LOG_ERROR(exceptionString);
	}

	if (!exceptionString.empty())
	{
		*libraryErrorLength = exceptionString.length();
		string *pError = new string(exceptionString);
		SQLCHAR *error = const_cast<SQLCHAR*>(static_cast<const SQLCHAR *>
			(static_cast<const void*>(pError->c_str())));

		*libraryError = error;
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: UninstallExternalLibrary
//
// Description:
//  Uninstalls an external library from the specified directory.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN UninstallExternalLibrary(
	SQLGUID    setupSessionId,
	SQLCHAR    *libraryName,
	SQLINTEGER libraryNameLength,
	SQLCHAR    *libraryInstallDirectory,
	SQLINTEGER libraryInstallDirectoryLength,
	SQLCHAR    **libraryError,
	SQLINTEGER *libraryErrorLength)
{
	LOG("RExtension::UninstallExternalLibrary");
	SQLRETURN result = SQL_ERROR;
	string exceptionString;

	try
	{
		RLibrarySession librarySession;

		librarySession.Init(
			setupSessionId,
			libraryName,
			libraryNameLength);

		result = librarySession.UninstallLibrary(
			libraryInstallDirectory,
			libraryInstallDirectoryLength);
	}
	catch (const exception & ex)
	{
		exceptionString = string(ex.what());
		LOG_EXCEPTION(ex);
	}
	catch (...)
	{
		exceptionString = "Unexpected exception in function UninstallExternalLibrary().";
		LOG_ERROR(exceptionString);
	}

	if (!exceptionString.empty())
	{
		*libraryErrorLength = exceptionString.length();
		string *pError = new string(exceptionString);
		SQLCHAR *error = const_cast<SQLCHAR*>(static_cast<const SQLCHAR *>
			(static_cast<const void*>(pError->c_str())));

		*libraryError = error;
	}

	return result;
}
