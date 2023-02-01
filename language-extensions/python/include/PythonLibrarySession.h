//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonLibrarySession.h
//
// Purpose:
//  Class encapsulating operations performed in a library management session
//
//*************************************************************************************************

#pragma once
#include "Common.h"
#ifdef _WIN64
	#include <filesystem>
#else
	#include <experimental/filesystem>
#endif

class PythonLibrarySession
{
public:

	//  Initializes the Python library session, initialize the main namespace.
	//
	void Init(const SQLGUID *sessionId);

	// Install the specified library.
	//
	SQLRETURN InstallLibrary(
		std::string tempFolder,
		SQLCHAR     *libraryName,
		SQLINTEGER  libraryNameLength,
		SQLCHAR     *libraryFile,
		SQLINTEGER  libraryFileLength,
		SQLCHAR     *libraryInstallDirectory,
		SQLINTEGER  libraryInstallDirectoryLength);

	// Uninstall the specified library.
	//
	SQLRETURN UninstallLibrary(
		SQLCHAR    *libraryName,
		SQLINTEGER libraryNameLength,
		SQLCHAR    *libraryInstallDirectory,
		SQLINTEGER libraryInstallDirectoryLength);

	#ifdef _WIN64
		// Get top level directory/ies for a package
		//
		std::vector<std::filesystem::directory_entry> GetTopLevel(
			std::string libName,
			std::string installDir);

		// Get all the artifacts we can find of a package that are in the path
		//
		std::vector<std::filesystem::directory_entry> GetAllArtifacts(
			std::string libName,
			std::string path);
	#else
		// Get top level directory/ies for a package
		//
		std::vector<std::experimental::filesystem::directory_entry> GetTopLevel(
			std::string libName,
			std::string installDir);

		// Get all the artifacts we can find of a package that are in the path
		//
		std::vector<std::experimental::filesystem::directory_entry> GetAllArtifacts(
			std::string libName,
			std::string path);
	#endif

private:
	SQLGUID m_sessionId{ 0, 0, 0, {0} };

	// The underlying boost::python namespace, which contains all the python variables.
	// We execute any python scripts on this namespace.
	//
	boost::python::object m_mainNamespace;
};
