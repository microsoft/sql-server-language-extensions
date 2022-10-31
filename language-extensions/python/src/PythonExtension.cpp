//*************************************************************************************************
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
//*************************************************************************************************

#include <boost/python.hpp>
#include <iostream>
#include <fstream>
#include <unordered_map>

#include "Logger.h"
#include "PythonExtensionUtils.h"
#include "PythonLibrarySession.h"
#include "PythonNamespace.h"
#include "PythonPathSettings.h"
#include "PythonSession.h"
#include "sqlexternallanguage.h"
#include "sqlexternallibrary.h"

using namespace std;
namespace bp = boost::python;

#ifdef _WIN64
	#include <filesystem>
	namespace fs = std::filesystem;
#else
	#include <experimental/filesystem>
	namespace fs = experimental::filesystem;
	#include <dlfcn.h>
	const string x_PythonSoFile = "libpython3.10.so.1.0";
#endif

static unordered_map<string, PythonSession *> g_pySessionMap;

//-------------------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------------------
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

#ifndef _WIN64
		// Preload the python so in Linux so that numpy knows about it.
		// Without this line, the numpy .so cannot find python and will fail to load.
		//
		dlopen(x_PythonSoFile.c_str(), RTLD_LAZY | RTLD_GLOBAL);
#endif

		// Initialize Python using the Python/C API.
		// This allows us to start using Python API and boost functions.
		//
		Py_Initialize();

		if (!Py_IsInitialized())
		{
			throw runtime_error("Python did not initialize properly, "
				"check paths and dependencies.");
		}

		bp::numpy::initialize();

		PythonPathSettings::Init(
			extensionParams,
			extensionPath,
			publicLibraryPath,
			privateLibraryPath);

		PythonNamespace::Init();
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (const bp::error_already_set&)
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

//-------------------------------------------------------------------------------------------------
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
	catch (const bp::error_already_set&)
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

//-------------------------------------------------------------------------------------------------
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
	catch (const bp::error_already_set&)
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

//-------------------------------------------------------------------------------------------------
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
	catch (const bp::error_already_set&)
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

//-------------------------------------------------------------------------------------------------
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
	catch (const bp::error_already_set&)
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

//-------------------------------------------------------------------------------------------------
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
	catch (const bp::error_already_set&)
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

//-------------------------------------------------------------------------------------------------
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
	catch (const bp::error_already_set&)
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

//-------------------------------------------------------------------------------------------------
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
	catch (const bp::error_already_set&)
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

//-------------------------------------------------------------------------------------------------
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
	catch (const bp::error_already_set&)
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

//-------------------------------------------------------------------------------------------------
// Name: Cleanup
//
// Description:
//  Completely clean up the extension
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN Cleanup()
{
	LOG("Cleanup");
	SQLRETURN result = SQL_SUCCESS;

	PythonNamespace::Cleanup();

	return result;
}


//-------------------------------------------------------------------------------------------------
// External Library APIs
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Name: InstallExternalLibrary
//
// Description:
//  Installs an external library to the specified directory
//  The library file is expected to be a zip containing the python package inside.
//  We unzip the file then install the python package inside using a subprocess call to pip.
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
	LOG("InstallExternalLibrary");
	SQLRETURN result = SQL_ERROR;

	string errorString;

	string installDir = string(reinterpret_cast<char *>(libraryInstallDirectory), 
		libraryInstallDirectoryLength);
	installDir = PythonExtensionUtils::NormalizePathString(installDir);

	string tempFolder = PythonExtensionUtils::NormalizePathString(
		fs::path(installDir).append("tmp").string());

	try
	{
		PythonLibrarySession librarySession = PythonLibrarySession();

		librarySession.Init(&setupSessionId);

		result = librarySession.InstallLibrary(
			tempFolder,
			libraryName,
			libraryNameLength,
			libraryFile,
			libraryFileLength,
			libraryInstallDirectory,
			libraryInstallDirectoryLength);
	}
	catch (const exception & ex)
	{
		result = SQL_ERROR;

		errorString = string(ex.what());
		LOG_ERROR(errorString);
	}
	catch (const bp::error_already_set &)
	{
		result = SQL_ERROR;

		errorString = PythonExtensionUtils::ParsePythonException();

		LOG_ERROR("Python error: " + errorString);
	}
	catch (...)
	{
		result = SQL_ERROR;

		errorString = "Unexpected exception occurred in function InstallExternalLibrary()";

		LOG_ERROR(errorString);
	}

	// Clean up the temp installation folder
	//
	if (fs::exists(tempFolder))
	{
		fs::remove_all(tempFolder);
	}

	if (!errorString.empty())
	{
		*libraryErrorLength = errorString.length();

		string *pError = new string(errorString);
		SQLCHAR *error = const_cast<SQLCHAR*>(reinterpret_cast<const SQLCHAR *>(pError->c_str()));

		*libraryError = error;
	}

	return result;
}

//-------------------------------------------------------------------------------------------------
// Name: UninstallExternalLibrary
//
// Description:
//  Uninstalls an external library from the specified directory.
//  We add the directory to the python path, then call pip uninstall in a subprocess.
//  If pip fails for some reason, we try to manually uninstall the package by deleting the
//  top level package folder as well as any dist/egg/.py files that were left behind.
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
	LOG("UninstallExternalLibrary");
	SQLRETURN result = SQL_SUCCESS;

	string errorString;

	try
	{
		PythonLibrarySession librarySession = PythonLibrarySession();

		librarySession.Init(&setupSessionId);
		
		result = librarySession.UninstallLibrary(
			libraryName,
			libraryNameLength,
			libraryInstallDirectory,
			libraryInstallDirectoryLength);
	}
	catch (const exception & ex)
	{
		result = SQL_ERROR;

		errorString = string(ex.what());
	}
	catch (const bp::error_already_set &)
	{
		result = SQL_ERROR;

		errorString = PythonExtensionUtils::ParsePythonException();
	}
	catch (...)
	{
		result = SQL_ERROR;

		errorString = "Unexpected exception occurred in function UninstallExternalLibrary()";
	}

	if (!errorString.empty())
	{
		LOG_ERROR(errorString);
		result = SQL_ERROR;

		*libraryErrorLength = errorString.length();

		string *pError = new string(errorString);
		SQLCHAR *error = const_cast<SQLCHAR *>(
			reinterpret_cast<const SQLCHAR *>(pError->c_str()));

		*libraryError = error;
	}

	return result;
}
