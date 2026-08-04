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

	// Packages authored on Windows can carry backslash-separated ZIP entry names. pip treats those
	// as literal characters on Linux, so setup.py is found but its package directory is not: pip
	// exits 0 and the caller reports SQL_SUCCESS for a broken layout.
	//
	// This runs in the satellite process, where an uncaught exception kills the process rather than
	// failing the statement, so failures stay inside Python and report back through a flag.
	// "needed" and "normalized" are separate so "nothing to do" is distinguishable from "tried and
	// failed"; only the latter throws.
	//
	// Entries are opened before entry.filename is rewritten, because ZipFile.open() compares the
	// central-directory name against the local header and raises BadZipFile when they differ - on
	// exactly the archives this exists to fix. They are streamed in bounded chunks with declared and
	// actual size caps, so a compressed bomb cannot exhaust the satellite. Both paths are bound as
	// namespace variables rather than spliced into the script text: a quote in a customer filename
	// would otherwise be a SyntaxError raised before the try: could catch it.
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
			"max_entry_size = 256 * 1024 * 1024\n"
			"max_total_size = 1024 * 1024 * 1024\n"
			"copy_chunk_size = 1024 * 1024\n"
			"try:\n"
			"    with zipfile.ZipFile(_normalize_src_zip_, 'r') as source_zip:\n"
			"        needed = any('\\\\' in n for n in source_zip.namelist())\n"
			"        if needed:\n"
			"            declared_total = 0\n"
			"            for entry in source_zip.infolist():\n"
			"                if entry.file_size > max_entry_size:\n"
			"                    raise ValueError('entry exceeds 256 MiB uncompressed-size limit: ' + entry.filename)\n"
			"                declared_total += entry.file_size\n"
			"                if declared_total > max_total_size:\n"
			"                    raise ValueError('archive exceeds 1 GiB aggregate uncompressed-size limit')\n"
			"            copied_total = 0\n"
			"            with zipfile.ZipFile(_normalize_dst_zip_, 'w') as normalized_zip:\n"
			"                for entry in source_zip.infolist():\n"
			"                    name = entry.filename.replace('\\\\', '/')\n"
			// Containment is checked after the rewrite: '..\\..\\.bashrc' is one inert flat
			// name on Linux, but a real traversal path once backslashes become separators.
			"                    if name.startswith('/') or '..' in name.split('/'):\n"
			"                        raise ValueError('unsafe entry name: ' + entry.filename)\n"
			"                    with source_zip.open(entry, 'r') as source_entry:\n"
			"                        entry.filename = name\n"
			"                        entry_copied = 0\n"
			"                        with normalized_zip.open(entry, 'w') as normalized_entry:\n"
			"                            while True:\n"
			"                                chunk = source_entry.read(copy_chunk_size)\n"
			"                                if not chunk:\n"
			"                                    break\n"
			"                                entry_copied += len(chunk)\n"
			"                                copied_total += len(chunk)\n"
			"                                if entry_copied > max_entry_size:\n"
			"                                    raise ValueError('entry exceeds 256 MiB uncompressed-size limit: ' + name)\n"
			"                                if copied_total > max_total_size:\n"
			"                                    raise ValueError('archive exceeds 1 GiB aggregate uncompressed-size limit')\n"
			"                                normalized_entry.write(chunk)\n"
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
			// Falling through would hand pip the ORIGINAL archive - the one that needs
			// repairing - so pip exits 0 and the caller reports SQL_SUCCESS for a broken
			// install. Logging cannot prevent that: LOG_ERROR never reaches *libraryError,
			// which InstallExternalLibrary fills only from its catch blocks. Throw instead.
			//
			string normalizeError = bp::extract<string>(m_mainNamespace["error"])();

			// The message embeds a customer-supplied entry name: strip CR/LF/NUL so it cannot
			// forge log records, and bound the length.
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
