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

	// Older package fixtures and customer packages created on Windows can contain backslashes in
	// ZIP entry names. Modern pip treats those as literal characters on Linux, so setup.py is found
	// but its package directory is not. Normalize the inner package ZIP before passing it to pip.
	//
	// This is best effort on purpose. It runs while installing an external library inside the
	// satellite process, where an uncaught exception terminates the process rather than failing the
	// statement, so every failure path has to stay inside Python and report back through a flag:
	//
	//  - the entry data must be read BEFORE rewriting entry.filename. ZipFile.open() compares the
	//    central-directory name against the local header and raises BadZipFile when they differ, so
	//    reading after the rewrite throws on exactly the archives this code exists to fix.
	//  - the archive is only rewritten when an entry actually contains a backslash, so well-formed
	//    packages are passed through untouched.
	//  - if no backslash entries are present, installPath is left untouched. If backslash entries
	//    are present but rewrite fails (for example malformed zip or an unsafe rewritten name),
	//    normalized stays false and the caller throws instead of falling through to the original path.
	//  - the two paths are BOUND as namespace variables rather than spliced into the script
	//    text. A quote in a customer-supplied filename would otherwise break the script at
	//    COMPILE time, and a SyntaxError is raised before the try: below can catch anything -
	//    terminating the satellite process. Binding also removes the injection vector.
	//  - "needed" and "normalized" are tracked separately, so "no backslashes present" is
	//    distinguishable from "rewrite attempted and failed". The latter THROWS rather than
	//    falling through: the fallback archive is the very one that needs repairing, so pip
	//    would exit 0 having installed a package whose directory is never found, and the caller
	//    would report SQL_SUCCESS for a broken layout. Logging alone cannot prevent that -
	//    LOG_ERROR never reaches *libraryError.
	//
	if (fs::path(installPath).extension().generic_string() == ".zip")
	{
		string normalizedInstallPath =
			(fs::path(tempFolder) / ("normalized-" + fs::path(installPath).filename().string())).generic_string();

		m_mainNamespace["_normalize_src_zip_"] = installPath;
		m_mainNamespace["_normalize_dst_zip_"] = normalizedInstallPath;

		string normalizeScript = "import zipfile\n"
			"needed = False\n"
			"normalized = False\n"
			"error = ''\n"
			"try:\n"
			"    with zipfile.ZipFile(_normalize_src_zip_, 'r') as source_zip:\n"
			"        needed = any('\\\\' in n for n in source_zip.namelist())\n"
			"        if needed:\n"
			"            with zipfile.ZipFile(_normalize_dst_zip_, 'w') as normalized_zip:\n"
			"                for entry in source_zip.infolist():\n"
			"                    data = source_zip.read(entry)\n"
			"                    name = entry.filename.replace('\\\\', '/')\n"
			// Replacing backslashes turns a name that is inert on Linux (one flat file
			// called '..\\..\\.bashrc') into a real traversal path, so containment has to
			// be checked after the rewrite, not before.
			"                    if name.startswith('/') or '..' in name.split('/'):\n"
			"                        raise ValueError('unsafe entry name: ' + entry.filename)\n"
			"                    entry.filename = name\n"
			"                    normalized_zip.writestr(entry, data)\n"
			"            normalized = True\n"
			"except Exception as ex:\n"
			"    normalized = False\n"
			"    error = str(ex)";
		bp::exec(normalizeScript.c_str(), m_mainNamespace);

		bool normalizeNeeded = bp::extract<bool>(m_mainNamespace["needed"]);
		bool normalizeSucceeded = bp::extract<bool>(m_mainNamespace["normalized"]);

		if (normalizeNeeded && normalizeSucceeded)
		{
			installPath = normalizedInstallPath;
		}
		else if (normalizeNeeded)
		{
			// The archive needed normalization and we could not produce it. Falling through
			// would hand pip the ORIGINAL archive - precisely the archive this code exists to
			// repair - so pip finds setup.py, never finds the package directory, exits 0, and
			// the caller reports SQL_SUCCESS for a broken install. Logging alone does not
			// prevent that: LOG_ERROR goes to the satellite's stderr and never reaches
			// *libraryError, which InstallExternalLibrary fills only from its catch blocks.
			// Throw so that catch converts this to SQL_ERROR and surfaces the reason.
			//
			string normalizeError = bp::extract<string>(m_mainNamespace["error"])();

			// The message embeds a customer-supplied ZIP entry name. Strip CR/LF/NUL so it
			// cannot forge additional log records, and bound the length.
			//
			for (char &ch : normalizeError)
			{
				if (ch == '\r' || ch == '\n' || ch == '\0')
				{
					ch = ' ';
				}
			}
			if (normalizeError.size() > 512)
			{
				normalizeError.resize(512);
			}

			throw runtime_error("Failed to normalize backslash-separated entry names in the "
				"external library archive: " + normalizeError);
		}
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
