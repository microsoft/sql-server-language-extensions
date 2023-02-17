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
//  @File: RLibrarySession.cpp
//
// Purpose:
//  Class encapsulating operations performed in a library management session
//
//**************************************************************************************************

#include "Common.h"
#include <fstream>

#include "RLibrarySession.h"

using namespace std;
#ifdef _WIN64
	#include <filesystem>
	namespace fs = filesystem;
#else
	#include <experimental/filesystem>
	namespace fs = experimental::filesystem;
#endif

const int x_NumberOfMagicBytes = 2;
const std::byte x_FirstMagicByte = static_cast<std::byte>(0x1f);
const std::byte x_SecondMagicByte = static_cast<std::byte>(0x8b);

const string RLibrarySession::sm_TarGzExt = ".tar.gz";
const string RLibrarySession::sm_ZipExt = ".zip";
const string RLibrarySession::sm_ScriptToGetLibPaths = ".libPaths()";

//--------------------------------------------------------------------------------------------------
// Name: RLibrarySession::GenerateScriptToSetLibPath
//
// Description:
//  Creates a script that would set the libPaths to a character vector of strings.
//  These strings are the ordered members of the input Rcpp::CharacterVector and are directory paths.
//  If the given vector is empty, we simply return the default value of the script
//  which is .libPaths()
//
// Returns:
//  An R script that can be used to set the libPaths.
//
string RLibrarySession::GenerateScriptToSetLibPath(
	Rcpp::CharacterVector pathsVector,
	bool                  isLastElementLibPaths)
{
	stringstream setLibPaths;

	if (pathsVector.size() > 0)
	{
		// The script looks like either of the two scenarios :
		// A) .libPaths( c( 'Path1', 'Path2', .libPaths())); OR
		// B) .libPaths( c( 'Path1', 'Path2', 'Path3'));
		//
		setLibPaths << ".libPaths( c( ";

		for (int index = 0; index < pathsVector.size(); ++index)
		{
			// Do the processing only if the path is not empty
			//
			if (!pathsVector[index].empty())
			{
				string normalizedPath = Utilities::NormalizePathString(
					string(pathsVector[index]));

				if (index != pathsVector.size() - 1)
				{
					// This is the case for all paths except the last one
					//
					setLibPaths << "'" + normalizedPath + "', ";
				}
				else
				{
					if (isLastElementLibPaths)
					{
						// This is scenario A: if the last element is .libPaths()
						// then we don't need to enclose it in quotes.
						//
						setLibPaths << normalizedPath;
					}
					else
					{
						// This is scenario B: if the last element is a regular path,
						// we need to enclose it in quotes but not add a ','
						//
						setLibPaths << "'" + normalizedPath + "'";
					}
				}
			}
		}

		setLibPaths << " ) );";
	}
	else
	{
		// Use the default get lib paths scripts as the same script to set lib paths
		// when size of given Rcpp::CharacterVector is 0.
		//
		setLibPaths << sm_ScriptToGetLibPaths;
	}

	return setLibPaths.str();
}

//--------------------------------------------------------------------------------------------------
// Name: RLibrarySession::GetFileExtension
//
// Description:
//  Gets the correct file extension by looking up the first two magic bytes from the file header
//  and if they are equal to 0x1f and 0x8b, returns '.tar.gz.' Otherwise returns '.zip'.
//
// Returns:
//  The correct extension either .tar.gz or .zip
//
string RLibrarySession::GetFileExtension(const string &libFilePathStr)
{
	// File extensions are not stored by CREATE EXTERNAL LIBRARY.
	// ".tar.gz" source packages will have ".zip" extension
	// the code below examines the header of the ".zip" files and
	// renames the file with the proper extension
	//
	bool isGzip = false;
	ifstream file(libFilePathStr, (ios::binary | ios::in));

	if (file.is_open())
	{
		std::byte header[x_NumberOfMagicBytes] = { static_cast<std::byte>(0) };

		file.read(reinterpret_cast<char *>(header), x_NumberOfMagicBytes);

		// gzip headers start with bytes 0x1f, 0x8b
		//
		isGzip = ((header[0] == x_FirstMagicByte) && (header[1] == x_SecondMagicByte));
	}

	string extension = (isGzip ? sm_TarGzExt : sm_ZipExt);

	return extension;
}

//--------------------------------------------------------------------------------------------------
// Name: RLibrarySession::Init
//
// Description:
//  Initializes the R library session by storing the session Id and the library name.
//
void RLibrarySession::Init(
	const SQLGUID &sessionId,
	const SQLCHAR *libraryName,
	SQLINTEGER    libraryNameLength)
{
	LOG("RLibrarySession::Init");

	m_sessionId = sessionId;

	m_libraryName = string(static_cast<const char*>(
			static_cast<const void*>(libraryName)),
			libraryNameLength);
}

//--------------------------------------------------------------------------------------------------
// Name: RLibrarySession::InstallLibrary
//
// Description:
//  Installs the contents of the libraryFile which is of the form <dbId>_<userId>_libName.zip
//  to the given installation directory. We copy the same contents into a new zip
//  whose name is of the form libName.zip. We use the R function install.packages specifying this
//  new zip as the value for its 'pkgs' argument and 'lib' pointing to the install directory.
//
// Returns:
//  SQL_SUCCESS if installation completes, throws an exception otherwise.
//
SQLRETURN RLibrarySession::InstallLibrary(
	const SQLCHAR *libraryFile,
	SQLINTEGER    libraryFileLength,
	const SQLCHAR *libraryInstallDirectory,
	SQLINTEGER    libraryInstallDirectoryLength)
{
	LOG("RLibrarySession::InstallLibrary");

	string installDir = string(static_cast<const char*>(
			static_cast<const void*>(libraryInstallDirectory)), libraryInstallDirectoryLength);
	installDir = Utilities::NormalizePathString(installDir);

	string libFilePathStr = string(static_cast<const char*>(
			static_cast<const void*>(libraryFile)), libraryFileLength);
	libFilePathStr = Utilities::NormalizePathString(libFilePathStr);

	// The original library file name is of the form <dbId>_<userId>_libName.zip
	// Generate the new name of the library file in the form of libName.zip or libName.tar.gz
	//
	string extension = GetFileExtension(libFilePathStr);
	string renamedLibFilePathStr = Utilities::NormalizePathString(
		fs::path(installDir).append(m_libraryName + extension).string());

	try
	{
		// Copy the library file to the install directory with the expected name.
		//
		string copyScript = "file.copy(from = '" + libFilePathStr + "', to = '"
			+ renamedLibFilePathStr + "');";
		ExecuteScript(copyScript);

		// Install the package using the new renamedLibFilePathStr.
		//
		string installScript =
			"install.packages(pkgs = '" + renamedLibFilePathStr + "', "
				"lib = '" + installDir + "', repos = NULL, clean = TRUE, quiet = FALSE, "
				"verbose = TRUE, INSTALL_opts=c('--debug'));";
		ExecuteScript(installScript);

		// Confirm the installation.
		//
		bool isInstalled = IsLibraryInstalledAtPath(installDir);
		if (!isInstalled)
		{
			throw runtime_error("install.packages() failed to install the package "
				+ m_libraryName + ".");
		}
	}
	catch (exception &ex)
	{
		// Clean up the copied library file path with the new name.
		//
		if (fs::exists(renamedLibFilePathStr))
		{
			fs::remove_all(renamedLibFilePathStr);
		}

		// Rethrow this exception, since the install has failed.
		// We denote SQL_ERROR, by rethrowing this exception so that it can be caught by the caller
		// and sent back to SQL Server to record the errors.
		//
		throw(ex);
	}

	return SQL_SUCCESS;
}

//--------------------------------------------------------------------------------------------------
// Name: RLibrarySession::UninstallLibrary
//
// Description:
//  Uninstalls from the specified library directory.
//  We use remove.packages R function with its 'lib' argument pointing to the directory from which
//  to install. If that fails for some reason, we try to manually uninstall the package
//  by deleting the top level package folder.
//
// Returns:
//  SQL_SUCCESS on complete uninstallation. In case of any failures, returns SQL_ERROR.
//
SQLRETURN RLibrarySession::UninstallLibrary(
	const SQLCHAR *libraryInstallDirectory,
	SQLINTEGER    libraryInstallDirectoryLength)
{
	LOG("RLibrarySession::UninstallLibrary");
	SQLRETURN result = SQL_ERROR;

	string errorString;

	string installDir = string(static_cast<const char*>(
			static_cast<const void*>(libraryInstallDirectory)), libraryInstallDirectoryLength);
	installDir = Utilities::NormalizePathString(installDir);

	try
	{
		// Uninstall the given libName from the installDir path.
		//
		string uninstallScript = "remove.packages(pkgs = '" + m_libraryName +
			"', lib = '" + installDir + "');";
		ExecuteScript(uninstallScript);

		bool isInstalled = IsLibraryInstalledAtPath(installDir);
		if (isInstalled)
		{
			throw runtime_error("remove.packages() failed to uninstall the package " +
				m_libraryName + ".");
		}

		result = SQL_SUCCESS;
	}
	catch (const exception & ex)
	{
		result = SQL_ERROR;
		errorString = string(ex.what());
	}
	catch (...)
	{
		result = SQL_ERROR;
		errorString = "Unexpected exception occurred in function UninstallLibrary().";
	}

	// If remove.packages fails for some reason, we try to manually uninstall the package
	// by deleting the top level package folder.
	//
	if (result != SQL_SUCCESS && fs::exists(installDir))
	{
		LOG_ERROR(errorString);
		string libraryDir = Utilities::NormalizePathString(
			fs::path(installDir).append(m_libraryName).string());

		// Clean up the library directory folder
		//
		if (fs::exists(libraryDir))
		{
			LOG("Failed to completely uninstall " + m_libraryName + " with remove.packages,"
				" so deleting files manually...");

			error_code errorCode;

			// returnValue is SQL_ERROR(-1) if there is an error, otherwise it is the
			// number of files deleted.
			//
			uintmax_t returnValue = fs::remove_all(libraryDir, errorCode);
			if (returnValue == static_cast<uintmax_t>(SQL_ERROR))
			{
				result = errorCode.value();
				LOG_ERROR("Failed to manually delete the directory with error code "
					+ to_string(result) + ".");
			}
			else
			{
				// If we successfully removed all the files, then we have a SUCCESS result.
				//
				result = SQL_SUCCESS;
				LOG("Manual deletion succeeded.");
			}
		}
		else
		{
			result = SQL_SUCCESS;
			LOG("Even though there was an error, library directory does not exist.");
		}
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: RLibrarySession::IsLibraryInstalledAtPath
//
// Description:
//  Checks if the library is installed at the correct location or not by executing
//  the script find.package()
//  If the library is installed, this script returns its first installation location.
//  If its installed location is same as the installDir, it returns true.
//  Otherwise, it returns false.
//
// Returns:
//  True if installed at correct location, false otherwise.
//
bool RLibrarySession::IsLibraryInstalledAtPath(const string &installDir)
{
	LOG("RLibrarySession::IsLibraryInstalledAtPath");

	string getInstallationLocationScript = "find.package(package = '" + m_libraryName + "'"
		", lib.loc = '" + installDir + "')[1];";

	bool isInstalled = false;

	try
	{
		Rcpp::CharacterVector installedLocationVector(
			ExecuteScriptAndGetResult(getInstallationLocationScript));
		
		if (installedLocationVector.size() > 0)
		{
			string firstInstallPath(installedLocationVector[0]);
			LOG(m_libraryName + " is found installed at " + firstInstallPath);
			fs::path fullExpectedInstallPath = fs::path(installDir) / m_libraryName;

			string normalizedFoundPath =
				Utilities::NormalizePathString(firstInstallPath);
			string normalizedExpectedInstallPath =
				Utilities::NormalizePathString(fullExpectedInstallPath.string());
			isInstalled =
				normalizedFoundPath.compare(normalizedExpectedInstallPath) == 0;
		}
	}
	catch (...)
	{
		// If the script throws an exception, it means we could not find the package.
		// Keep isInstalled = false.
		//
		LOG(m_libraryName + " is not installed.");
	}
	
	return isInstalled;
}
