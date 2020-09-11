//**************************************************************************************************
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
//  @File: RLibrarySession.h
//
// Purpose:
//  Class encapsulating operations performed in a library management session.
//
//**************************************************************************************************

#pragma once

class RLibrarySession
{
public:

	//  Initializes the R library session and a shared pointer to the EmbeddedR environment.
	//
	void Init(const SQLGUID &sessionId);

	// Install the specified library.
	//
	SQLRETURN InstallLibrary(
		const SQLCHAR *libraryName,
		SQLINTEGER    libraryNameLength,
		const SQLCHAR *libraryFile,
		SQLINTEGER    libraryFileLength,
		const SQLCHAR *libraryInstallDirectory,
		SQLINTEGER    libraryInstallDirectoryLength);

	// Uninstall the specified library.
	//
	/*SQLRETURN UninstallLibrary(
		SQLCHAR    *libraryName,
		SQLINTEGER libraryNameLength,
		SQLCHAR    *libraryInstallDirectory,
		SQLINTEGER libraryInstallDirectoryLength);

	// Get top level directory/ies for a package
	//
	std::vector<std::experimental::filesystem::directory_entry> GetTopLevel(
		std::string libName,
		std::string installDir);

	// Get all the artifacts we can find of a package that are in the path
	//
	std::vector<std::experimental::filesystem::directory_entry> GetAllArtifacts(
		std::string libName,
		std::string path);*/

private:

	// A pointer to the embedded R environment via RInside.
	// We execute all R scripts in this environment and there can only be a single
	// instance of RInside in the extension.
	//
	RInside* m_embeddedREnvPtr;

	SQLGUID m_sessionId { 0, 0, 0, {0} };
};

