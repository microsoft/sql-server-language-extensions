//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonLibraryTests.h
//
// Purpose:
//  Define the gtest overrides and the External Library tests
//
//*************************************************************************************************
#pragma once
#include "Common.h"
#ifdef _WIN64
	#include <filesystem>
	namespace fs = std::filesystem;
#else
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#endif

namespace LibraryApiTests
{
	// All the tests in the LibraryApiTests suite run one after the other
	//
	class ExternalLibraryApiTests : public ::testing::Test
	{
	protected:
		// Code here will be called immediately after the constructor (right
		// before each test).
		//
		void SetUp() override;

		// Code here will be called immediately after each test (right
		// before the destructor).
		//
		void TearDown() override;

		// Set up default, valid variables for use in tests
		//
		void SetupVariables();

		// Call Cleanup on the PythonExtension.
		// Testing if Cleanup is implemented correctly.
		//
		void DoCleanup();

		// Cleanup sys.modules in python to remove import traces
		//
		void CleanModules(std::string extLibName, std::string moduleName);

		// Install and test a package
		//
		void InstallAndTest(
			std::string extLibName,
			std::string moduleName,
			std::string pathToPackage,
			std::string installDir,
			std::string expectedVersion,
			std::string expectedLocation = "",
			bool successfulInstall = true,
			bool successfulImport = true);

		// Uninstall and test a package
		//
		void UninstallAndTest(
			std::string extLibName,
			std::string moduleName,
			std::string installDir,
			bool otherInstallationExists = false);

		// Initialize pythonextension for library management
		//
		void Initialize();

		// Filesystem path to the packages we will install
		//
		fs::path m_packagesPath;

		// Some temp paths for public and private libraries
		//
		std::string m_libraryPath = "testInstallPkgs";
		std::string m_publicLibraryPath;
		std::string m_privateLibraryPath;	
		std::string m_pathToPython;

		// The boost python module; python will run in this object
		//
		boost::python::object m_mainModule;

		// The boost python namespace; backup dictionary containing all builtins
		//
		boost::python::dict m_backupNamespace;

		// The main namespace. We use this for all execution
		//
		boost::python::dict m_mainNamespace;

	};
}
