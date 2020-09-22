//**************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server
// external language communication protocol.
// Copyright (C) 2019 Microsoft Corporation.
//
// This file is part of RExtension-test.
//
// RExtension-test is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RExtension-test is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RExtension-test.  If not, see <https://www.gnu.org/licenses/>.
//
// @File: RExtensionLibraryApiTests.h
//
// Purpose:
//  Defines the base test fixture members and tests RExtension's implementation of
//  the external language library management APIs.
//
//**************************************************************************************************

#pragma once

// Type definitions of functions exposed from RExtension and useful in RExtensionLibaryApiTests.
//
typedef SQLRETURN FN_installExternalLibrary(
	SQLGUID,      // setupSessionId
	SQLCHAR *,    // libraryName
	SQLINTEGER,   // libraryNameLength
	SQLCHAR*,     // libraryFile
	SQLINTEGER,   // libraryFileLength
	SQLCHAR*,     // libraryInstallDirectory
	SQLINTEGER,   // libraryInstallDirectoryLength
	SQLCHAR**,    // libraryError
	SQLINTEGER*); // libraryErrorLength

typedef SQLRETURN FN_uninstallExternalLibrary(
	SQLGUID,      // setupSessionId
	SQLCHAR*,     // libraryName
	SQLINTEGER,   // libraryNameLength
	SQLCHAR*,     // libraryInstallDirectory
	SQLINTEGER,   // libraryInstallDirectoryLength
	SQLCHAR**,    // libraryError
	SQLINTEGER*); // libraryErrorLength

typedef void FN_executeScript(
	std::string script);

typedef SEXP FN_executeScriptAndGetResult(
	std::string script);

namespace ExtensionApiTest
{
	// All the tests in the RExtensionLibraryApiTests suite run one after the other
	//
	class RExtensionLibraryApiTests : public RExtensionApiTests
	{
	protected:

		// Per-test-suite set-up.
		// Called before the first test in this test suite.
		//
		static void SetUpTestSuite();

		// Code here will be called immediately after the constructor (right
		// before each test).
		//
		void SetUp() override;

		// Get handles to install and uninstall library API functions.
		//
		static void GetHandles();

		// Executes and gets the result of the given script by invoking
		// the function provided by RExtension.
		//
		static std::string ExecuteScriptAndGetResultAsString(std::string script);

		// Set up default, valid variables for use in tests
		//
		void SetupVariables() override;

		// Detach package from R runtime to remove load traces.
		//
		void DetachLibrary(std::string extLibName);

		// Find the location of the given extLibName.
		//
		std::string GetLocationOfLibrary(std::string extLibName);

		// Find the version of the given extLibName.
		//
		std::string GetVersionOfLibrary(std::string extLibName);

		// Install and test a package
		//
		void InstallAndTest(
			std::string extLibName,
			std::string pathToPackage,
			std::string installDir,
			std::string expectedVersion,
			std::string expectedLocation = "",
			bool        successfulInstall = true,
			bool        successfulLoad = true);

		// Uninstall and test a package
		//
		void UninstallAndTest(
			std::string extLibName,
			std::string installDir,
			bool        otherInstallationExists = false);

		// Filesystem path of the source location of the directory containing the
		// packages we will install.
		//
		std::experimental::filesystem::path m_packagesPath;

		// The prefix appended to zip file paths.
		//
		static const std::string sm_DbIdUserIdPrefix;

		// Pointer to the InstallExternalLibrary function.
		//
		static FN_installExternalLibrary *sm_installExternalLibraryFuncPtr;

		// Pointer to the UninstallExternalLibrary function.
		//
		static FN_uninstallExternalLibrary *sm_uninstallExternalLibraryFuncPtr;

		// Pointer to ExecuteScriptAndGetResult function from RExtension.
		//
		static FN_executeScript *sm_executeScriptFuncPtr;

		// Pointer to ExecuteScriptAndGetResult function.
		//
		static FN_executeScriptAndGetResult *sm_executeScriptAndGetResultFuncPtr;
	};
}
