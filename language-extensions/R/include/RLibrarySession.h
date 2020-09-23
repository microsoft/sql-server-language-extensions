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
//  @File: RLibrarySession.h
//
// Purpose:
//  Class encapsulating operations performed in a library management session.
//
//**************************************************************************************************

#pragma once

//--------------------------------------------------------------------------------------------------
// Name: RLibrarySession
//
// Description:
//  Class encapsulating operations performed in a library management session.
//
class RLibrarySession
{
public:

	// Initializes the R library session by storing the sessionId and the library name.
	//
	void Init(
		const SQLGUID &sessionId,
		const SQLCHAR *libraryName,
		SQLINTEGER    libraryNameLength);

	// Installs the contents of the libraryFile which is of the form <dbId>_<userId>_libName.zip
	// to the given installation directory.
	//
	SQLRETURN InstallLibrary(
		const SQLCHAR *libraryFile,
		SQLINTEGER    libraryFileLength,
		const SQLCHAR *libraryInstallDirectory,
		SQLINTEGER    libraryInstallDirectoryLength);

	// Uninstalls from the specified directory.
	//
	SQLRETURN UninstallLibrary(
		const SQLCHAR *libraryInstallDirectory,
		SQLINTEGER    libraryInstallDirectoryLength);

	// Checks if the library is installed at correct location or not.
	//
	bool IsLibraryInstalledAtPath(const std::string &installDir);

	// Creates a script that would set the libPaths to a character vector of strings.
	// These strings are the ordered members of the input Rcpp::CharacterVector
	// and are directory paths.
	//
	static std::string GenerateScriptToSetLibPath(
		Rcpp::CharacterVector pathsVector,
		bool                  isLastElementLibPaths);

	// Gets the correct file extension by looking up the first two magic bytes.
	//
	static std::string GetFileExtension(const std::string &libFilePathStr);

	// Tar Gzip extension.
	//
	static const std::string sm_TarGzExt;

	// Zip extension.
	//
	static const std::string sm_ZipExt;

	// The script to find the current library paths.
	//
	static const std::string sm_ScriptToGetLibPaths;

private:

	// Session ID.
	//
	SQLGUID m_sessionId { 0, 0, 0, {0} };

	// Library Name being operated upon in this session.
	//
	std::string m_libraryName;
};
