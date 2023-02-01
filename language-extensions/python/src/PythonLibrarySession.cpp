//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonLibrarySession.cpp
//
// Purpose:
//  Class encapsulating operations performed in a library management session
//
//*************************************************************************************************

#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_map>

#include "Logger.h"
#include "PythonExtensionUtils.h"
#include "PythonLibrarySession.h"
#include "PythonNamespace.h"

using namespace std;
namespace bp = boost::python;

#ifdef _WIN64
	#include <filesystem>
	namespace fs = std::filesystem;
#else
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#endif

//-------------------------------------------------------------------------------------------------
// Name: PythonLibrarySession::Init
//
// Description:
//  Initializes the Python library session, initialize the main namespace.
//
void PythonLibrarySession::Init(
	const SQLGUID *sessionId)
{
	LOG("PythonLibrarySession::Init");

	m_mainNamespace = PythonNamespace::MainNamespace();
}

//-------------------------------------------------------------------------------------------------
// Name: PythonLibrarySession::InstallLibrary
//
// Description:
//  Install the specified library.
//
// Returns:
//  The result of installation
//
SQLRETURN PythonLibrarySession::InstallLibrary(
	string     tempFolder,
	SQLCHAR    *libraryName,
	SQLINTEGER libraryNameLength,
	SQLCHAR    *libraryFile,
	SQLINTEGER libraryFileLength,
	SQLCHAR    *libraryInstallDirectory,
	SQLINTEGER libraryInstallDirectoryLength)
{
	LOG("PythonLibrarySession::InstallExternalLibrary");
	SQLRETURN result = SQL_ERROR;

	string errorString;

	string libName = string(reinterpret_cast<char *>(libraryName), libraryNameLength);

	string installDir = string(reinterpret_cast<char *>(libraryInstallDirectory), libraryInstallDirectoryLength);
	installDir = PythonExtensionUtils::NormalizePathString(installDir);
	string libFilePath = string(reinterpret_cast<char *>(libraryFile), libraryFileLength);
	libFilePath = PythonExtensionUtils::NormalizePathString(libFilePath);
	
	string extractScript = "import zipfile\n"
		"with zipfile.ZipFile('" + libFilePath + "') as zip:\n"
		"    zip.extractall('" + tempFolder + "')";

	bp::exec(extractScript.c_str(), m_mainNamespace);

	string installPath = "";

	// Find the python package inside the zip to use for installation.
	//
	for (const fs::directory_entry &entry : fs::directory_iterator(tempFolder))
	{
		string type = entry.path().extension().generic_string();

		if (type.compare(".whl") == 0 ||
			type.compare(".zip") == 0 ||
			type.compare(".gz") == 0)
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

	// Set the TMPDIR so that pip uses our destination as temp. This allows us to use a
	// non-default instance.
	// Without this, TMPDIR will have MSSQL##$INSTANCE in the path, and the $ causes problems
	// with pip because they interpret $INSTANCE as a variable, not part of the path.
	//
	string setTemp = "import os;oldtemp = os.environ['TMPDIR'] if 'TMPDIR' in os.environ else None;"
		"os.environ['TMPDIR'] = '" + tempFolder + "'";
	bp::exec(setTemp.c_str(), m_mainNamespace);

	string installScript = 
		"import subprocess;pipresult = subprocess.run(['" + pathToPython +
		"', '-m', 'pip', 'install', '" + installPath +
		"', '--no-deps', '--ignore-installed', '--no-cache-dir'"
		", '-t', '" + installDir + "']).returncode";

	bp::exec(installScript.c_str(), m_mainNamespace);

	int pipResult = bp::extract<int>(m_mainNamespace["pipresult"]);

	string resetTemp =  "if oldtemp: \n"
						"    os.environ['TMPDIR'] = oldtemp\n"
						"else:\n"
						"    del os.environ['TMPDIR']";
	bp::exec(resetTemp.c_str(), m_mainNamespace);


	if (pipResult != 0)
	{
		throw runtime_error("Pip failed to install the package with exit code " +
			to_string(pipResult));
	}

	result = SQL_SUCCESS;

	return result;
}

//-------------------------------------------------------------------------------------------------
// Name: PythonLibrarySession::UninstallLibrary
//
// Description:
//  Uninstall the specified library from the specified library directory
//
// Returns:
//  The result of uninstallation
//
SQLRETURN PythonLibrarySession::UninstallLibrary(
	SQLCHAR    *libraryName,
	SQLINTEGER libraryNameLength,
	SQLCHAR    *libraryInstallDirectory,
	SQLINTEGER libraryInstallDirectoryLength
)
{
	LOG("PythonLibrarySession::UninstallExternalLibrary");
	SQLRETURN result = SQL_ERROR;

	string errorString;
	vector<fs::directory_entry> artifacts;

	string libName = string(reinterpret_cast<char *>(libraryName), libraryNameLength);

	string installDir = string(reinterpret_cast<char *>(libraryInstallDirectory),
		libraryInstallDirectoryLength);
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

		bp::exec(uninstallScript.c_str(), m_mainNamespace);

		int pipResult = bp::extract<int>(m_mainNamespace["pipresult"]);

		if (pipResult == 0)
		{
			result = SQL_SUCCESS;
		}
		else
		{
			throw runtime_error("Pip failed to fully uninstall the package with exit code " +
				to_string(pipResult));
		}
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

	// If pip fails for some reason, we try to manually uninstall the package by deleting the
	// top level package folder as well as any dist/egg/.py files that were left behind.
	//
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

		// If we successfully removed all the files, then we have a SUCCESS result.
		//
		result = SQL_SUCCESS;
	}

	return result;
}

//-------------------------------------------------------------------------------------------------
// Name: PythonLibrarySession::GetTopLevel
//
// Description:
//  Get top level directory/ies for a package
//
// Returns:
//  A vector of directory_entries of the top level artifacts of the package
//
vector<fs::directory_entry> PythonLibrarySession::GetTopLevel(string libName, string installDir)
{
	vector<fs::directory_entry> artifacts;
	regex_constants::syntax_option_type caseInsensitive = regex_constants::icase;

	// Normalize library names by replacing all dashes and underscores with regex for either
	//
	string regexLibName = regex_replace(libName, regex("(-|_)"), "(-|_)");

	if (fs::exists(installDir))
	{
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

				// The top_level.txt file tells us what items this package put into the 
				// installation directory that we will need to delete to uninstall.
				//
				fs::path topLevelPath = entry.path();
				topLevelPath = topLevelPath.append("top_level.txt");

				if (fs::exists(topLevelPath))
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
	}

	return artifacts;
}

//-------------------------------------------------------------------------------------------------
// Name: PythonLibrarySession::GetAllArtifacts
//
// Description:
//  Get all the artifacts we can find of a package that are in the path
//
// Returns:
//  A vector of directory_entries of the artifacts
//
vector<fs::directory_entry> PythonLibrarySession::GetAllArtifacts(string libName, string path)
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
