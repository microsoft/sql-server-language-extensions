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
#include <experimental/filesystem>
#include <iostream>
#include <fstream>
#include <regex>
#include <unordered_map>

#include "Logger.h"
#include "PythonExtensionUtils.h"
#include "PythonNamespace.h"
#include "PythonPathSettings.h"
#include "PythonSession.h"
#include "sqlexternallanguage.h"
#include "sqlexternallibrary.h"

using namespace std;
namespace py = boost::python;
namespace fs = std::experimental::filesystem;

#ifndef _WIN64
#include <dlfcn.h>
const string x_PythonSoFile = "libpython3.7m.so.1.0";
#endif

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

		py::numpy::initialize();

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

	PythonNamespace::Cleanup();

	return result;
}


//--------------------------------------------------------------------------------------------------
// External Library APIs
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: InstallExternalLibrary
//
// Description:
//	Installs an external library to the specified directory
//	The library file is expected to be a zip containing the python package inside.
//	We unzip the file then install the python package inside using a subprocess call to pip.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
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

	string libName = string(reinterpret_cast<char *>(libraryName), libraryNameLength);

	string installDir = string(reinterpret_cast<char *>(libraryInstallDirectory), libraryInstallDirectoryLength);
	installDir = PythonExtensionUtils::NormalizePathString(installDir);
	string libFilePath = string(reinterpret_cast<char *>(libraryFile), libraryFileLength);
	libFilePath = PythonExtensionUtils::NormalizePathString(libFilePath);

	string tempFolder = installDir + "/tmp";

	try
	{
		py::object mainNamespace = PythonNamespace::MainNamespace();

		string extractScript = "import zipfile\n"
			"with zipfile.ZipFile('" + libFilePath + "') as zip:\n"
			"    zip.extractall('" + tempFolder + "')";

		py::exec(extractScript.c_str(), mainNamespace);

		string installPath = "";

		// Find the python package inside the zip to use for installation.
		//
		for (const fs::directory_entry &entry : fs::directory_iterator(tempFolder))
		{
			string type = entry.path().extension().generic_string();

			if (type.compare(".whl") == 0 ||
					type.compare(".zip") == 0 ||
					type.compare(".gz") == 0
				)
			{
				installPath = entry.path().generic_string();
				break;
			}
		}
		
		if (installPath.empty()) 
		{
			throw runtime_error("Could not find the package inside the zip - "
				"external library must be a python package inside a zip.");
		}

		string pathToPython = PythonExtensionUtils::GetPathToPython();

		string installScript = "import subprocess;pipresult = subprocess.run(['" + pathToPython +
			"', '-m', 'pip', 'install', '" + installPath +
			"', '--no-deps', '--ignore-installed', '--no-cache-dir', '-t', '" + installDir + "']).returncode";

		py::exec(installScript.c_str(), mainNamespace);

		int pipResult = py::extract<int>(mainNamespace["pipresult"]);

		if (pipResult != 0)
		{
			throw runtime_error("Pip failed to install the package with exit code " + to_string(pipResult));
		}

		result = SQL_SUCCESS;
	}
	catch (const exception & ex)
	{
		result = SQL_ERROR;

		errorString = string(ex.what());
		LOG_ERROR(errorString);
	}
	catch (const py::error_already_set &)
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


//--------------------------------------------------------------------------------------------------
// Name: GetTopLevel
//
// Description:
//	Get top level directory/ies for a package
//
// Returns:
//	A vector of directory_entries of the top level artifacts of the package
//
vector<fs::directory_entry> GetTopLevel(string libName, string installDir)
{
	vector<fs::directory_entry> artifacts;
	regex_constants::syntax_option_type caseInsensitive = regex_constants::icase;

	// Normalize library names by replacing all dashes and underscores with regex for either
	//
	string regexLibName = regex_replace(libName, regex("(-|_)"), "(-|_)");

	for (const fs::directory_entry &entry : fs::directory_iterator(installDir))
	{
		string pathFilename = entry.path().filename().string();

		// The top_level.txt file is in the egg-info or dist-info folder
		//
		regex egg("^" + regexLibName + "-(.*)egg(.*)", caseInsensitive);
		regex distinfo("^" + regexLibName + "-(.*)dist-info", caseInsensitive);

		if (regex_match(pathFilename, egg) ||
			regex_match(pathFilename, distinfo))
		{
			artifacts.push_back(entry);

			// The top_level.txt file tells us what items this package put into the installation directory
			// that we will need to delete to uninstall.
			//
			fs::path topLevelPath = entry.path();
			topLevelPath = topLevelPath.append("top_level.txt");

			if(fs::exists(topLevelPath))
			{
				// Read in the top_level file to find what the top_level folders and files are
				//
				ifstream topLevelFile(topLevelPath);
				string str;
				while (getline(topLevelFile, str))
				{
					if (str.size() > 0)
					{
						fs::path path(installDir);
						artifacts.push_back(fs::directory_entry(path.append(str)));
					}
				}

				topLevelFile.close();
				break;
			}

			break;
		}
	}

	return artifacts;
}

//--------------------------------------------------------------------------------------------------
// Name: GetAllArtifacts
//
// Description:
//	Get all the artifacts we can find of a package that are in the path
//
// Returns:
//	A vector of directory_entries of the artifacts
//
vector<fs::directory_entry> GetAllArtifacts(string libName, string path)
{
	vector<fs::directory_entry> artifacts;
	regex_constants::syntax_option_type caseInsensitive = regex_constants::icase;

	// Normalize library names by replacing all dashes with underscores
	//
	string regexLibName = regex_replace(libName, regex("(-|_)"), "(-|_)");

	if (fs::exists(path))
	{
		for (const fs::directory_entry &entry : fs::directory_iterator(path))
		{
			string pathFilename = entry.path().filename().string();

			regex pth("^" + regexLibName + "-(.*).pth", caseInsensitive);
			regex pyFile("^" + regexLibName + ".py", caseInsensitive);

			if (regex_match(pathFilename, pyFile) ||
				regex_match(pathFilename, pth))
			{
				artifacts.push_back(entry);
			}
		}
	}

	return artifacts;
}

//--------------------------------------------------------------------------------------------------
// Name: UninstallExternalLibrary
//
// Description:
//	Uninstalls an external library from the specified directory.
//	We add the directory to the python path, then call pip uninstall in a subprocess.
//	If pip fails for some reason, we try to manually uninstall the package by deleting the 
//	top level package folder as well as any dist/egg/.py files that were left behind.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
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
	SQLRETURN result = SQL_ERROR;

	string errorString;
	vector<fs::directory_entry> artifacts;

	py::object mainNamespace = PythonNamespace::MainNamespace();

	string libName = string(reinterpret_cast<char *>(libraryName), libraryNameLength);

	string installDir = string(reinterpret_cast<char *>(libraryInstallDirectory), libraryInstallDirectoryLength);
	installDir = PythonExtensionUtils::NormalizePathString(installDir);

	try
	{
		// Save the top_level items so we can delete them if the pip uninstall fails.
		// If pip uninstall succeeds, we won't need this.
		//
		artifacts = GetTopLevel(libName, installDir);

		string pathToPython = PythonExtensionUtils::GetPathToPython();

		string uninstallScript = 
		"newPath = ['" + installDir + "'] + _originalpath\n"
		"os.environ['PYTHONPATH'] = os.pathsep.join(newPath)\n"
		"import subprocess\n"
		"pipresult = subprocess.run(['" + pathToPython +
			"', '-m', 'pip', 'uninstall', '" + libName + "', '-y']).returncode\n";

		py::exec(uninstallScript.c_str(), mainNamespace);

		int pipResult = py::extract<int>(mainNamespace["pipresult"]);

		if (pipResult != 0)
		{
			throw runtime_error("Pip failed to fully uninstall the package with exit code " + to_string(pipResult));
		}

		result = SQL_SUCCESS;
	}
	catch (const exception & ex)
	{
		result = SQL_ERROR;

		errorString = string(ex.what());
	}
	catch (const py::error_already_set &)
	{
		result = SQL_ERROR;

		errorString = PythonExtensionUtils::ParsePythonException();
	}
	catch (...)
	{
		result = SQL_ERROR;

		errorString = "Unexpected exception occurred in function UninstallExternalLibrary()";
	}

	// If pip fails for some reason, we try to manually uninstall the package by deleting the 
	// top level package folder as well as any dist/egg/.py files that were left behind.
	//
	try
	{
		if (result != SQL_SUCCESS && fs::exists(installDir))
		{
			LOG("Failed to fully uninstall " + libName + " with pip, deleting files manually");

			for (fs::directory_entry entry : artifacts)
			{
				fs::remove_all(entry);
			}

			vector<fs::directory_entry> newArtifacts = GetAllArtifacts(libName, installDir);

			for (fs::directory_entry entry : newArtifacts)
			{
				fs::remove_all(entry);
			}
		}

		// We have successfully manually deleted the package, so we should remove the error.
		// 
		errorString = "";
		result = SQL_SUCCESS;
	}
	catch (...)
	{
		result = SQL_ERROR;
	}

	if (!errorString.empty())
	{
		LOG_ERROR(errorString);
		result = SQL_ERROR;

		*libraryErrorLength = errorString.length();

		string *pError = new string(errorString);
		SQLCHAR *error = const_cast<SQLCHAR *>(reinterpret_cast<const SQLCHAR *>(pError->c_str()));

		*libraryError = error;
	}

	return result;
}
