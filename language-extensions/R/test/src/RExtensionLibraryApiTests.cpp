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
// @File: RExtensionLibraryApiTests.cpp
//
// Purpose:
//  Defines the base test fixture members and tests RExtension's implementation of
//  the external language library management APIs.
//
//**************************************************************************************************

#include "Common.h"

#include "RExtensionLibraryApiTests.h"

using namespace std;
namespace fs = experimental::filesystem;

namespace ExtensionApiTest
{
	// x_OsName, x_FileExtension are used to derive path of packages on respective platforms.
	//
#ifdef _WIN64
	const string x_OsName = "windows";
#else
	const string x_OsName = "linux";
#endif

	const string x_FileExtension = ".zip";

	// Static members
	//
	FN_installExternalLibrary *RExtensionLibraryApiTests::sm_installExternalLibraryFuncPtr = nullptr;
	FN_uninstallExternalLibrary *RExtensionLibraryApiTests::sm_uninstallExternalLibraryFuncPtr = nullptr;
	FN_executeScript *RExtensionLibraryApiTests::sm_executeScriptFuncPtr = nullptr;
	FN_executeScriptAndGetResult *RExtensionLibraryApiTests::sm_executeScriptAndGetResultFuncPtr = nullptr;
	const string RExtensionLibraryApiTests::sm_DbIdUserIdPrefix = "1_1_";

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionLibraryApiTests::SetUpTestSuite
	//
	// Description:
	//  Per-test-suite set-up. Called before the first test in this test suite.
	//  Calls the base SetUpTestSuite from here.
	//
	void RExtensionLibraryApiTests::SetUpTestSuite()
	{
		RExtensionApiTests::SetUpTestSuite();
		GetHandles();
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionLibraryApiTests::SetUp
	//
	// Description:
	//  Per-test set up. Code here will be called immediately after the constructor (right
	//  before each test).
	//
	void RExtensionLibraryApiTests::SetUp()
	{
		RExtensionApiTests::SetUp();
		SetupVariables();
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionLibraryApiTests::GetHandles
	//
	// Description:
	//  Gets handles to install and uninstall library API functions
	//  as well as to the Embedded R environment via RInside.
	//
	void RExtensionLibraryApiTests::GetHandles()
	{
		sm_installExternalLibraryFuncPtr = reinterpret_cast<FN_installExternalLibrary*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				sm_libHandle,
				"InstallExternalLibrary"));
		ASSERT_TRUE(sm_installExternalLibraryFuncPtr != nullptr);

		sm_uninstallExternalLibraryFuncPtr = reinterpret_cast<FN_uninstallExternalLibrary*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				sm_libHandle,
				"UninstallExternalLibrary"));
		ASSERT_TRUE(sm_uninstallExternalLibraryFuncPtr != nullptr);

		sm_executeScriptFuncPtr = reinterpret_cast<FN_executeScript*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				sm_libHandle,
				"ExecuteScript"));
		ASSERT_TRUE(sm_executeScriptFuncPtr != nullptr);

		sm_executeScriptAndGetResultFuncPtr = reinterpret_cast<FN_executeScriptAndGetResult*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				sm_libHandle,
				"ExecuteScriptAndGetResult"));
		ASSERT_TRUE(sm_executeScriptAndGetResultFuncPtr != nullptr);
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionLibraryApiTests::SetupVariables
	//
	// Description:
	//  Sets up default, valid variables for use in tests
	//
	void RExtensionLibraryApiTests::SetupVariables()
	{
		// First call the base SetupVariables
		//
		RExtensionApiTests::SetupVariables();

		string enlRoot = string(getenv("ENL_ROOT"));

		m_packagesPath = fs::absolute(enlRoot);
		m_packagesPath = m_packagesPath / "language-extensions" / "R" / "test" / "test-packages";
		ASSERT_TRUE(fs::exists(m_packagesPath));
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionLibraryApiTests::ExecuteScriptAndGetResultAsString
	//
	// Description:
	//  Executes and gets the result of the given script by invoking
	//  the function provided by RExtension. Expects the results of the script to be
	//  an Rcpp::CharacterVector of size > 0 and returns the first character string from it.
	//
	// Returns:
	//  First character string from the result.
	//
	string RExtensionLibraryApiTests::ExecuteScriptAndGetResultAsString(string script)
	{
		Rcpp::CharacterVector resultVector(
			sm_executeScriptAndGetResultFuncPtr(script));

		string result = "";

		bool hasResults = resultVector.size() > 0;
		if (hasResults)
		{
			result = string(static_cast<char *>(resultVector[0]));
		}

		return result;
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionLibraryApiTests::DetachLibrary
	//
	// Description:
	//  Removes the library from the search path and unload it from the session.
	//  This is needed so that for consecutive install and load tests
	//  without uninstalling in between. Library and package are synonyms and refer
	//  to the same object.
	//
	void RExtensionLibraryApiTests::DetachLibrary(string extLibName)
	{
		try
		{
			string detachScript = "detach(package:" + extLibName + " , unload = TRUE);";
			sm_executeScriptFuncPtr(detachScript);
		}
		catch (...)
		{
			// Detaching the package is best effort
			// This may fail if the package is not successfully loaded.
			// We ignore such failures and move on since we test
			// unsuccessful load scenarios as well.
			//
			cout << "Failed to detach library." << endl;
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionLibraryApiTests::GetLocationOfLibrary
	//
	// Description:
	//  Finds the location of the given extLibName by executing the script that finds the package
	//  location. Library and package are synonyms and refer to the same object.
	//
	// Returns:
	//  The first install location of the library if it is found or throws an exception if
	//  it is not installed and not found.
	//
	string RExtensionLibraryApiTests::GetLocationOfLibrary(string extLibName)
	{
		string locationScript = "find.package(package = '" + extLibName + "')[1];";
		string location = ExecuteScriptAndGetResultAsString(locationScript);

		return location;
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionLibraryApiTests::GetVersionOfLibrary
	//
	// Description:
	//  Finds the version of the given extLibName by executing the script that finds the package
	//  version. Library and package are synonyms and refer to the same object.
	//
	// Returns:
	//  The version of the library installed or throws an exception if not installed.
	//
	string RExtensionLibraryApiTests::GetVersionOfLibrary(string extLibName)
	{
		string versionScript = "as.character(packageVersion('" + extLibName + "'));";
		string version = ExecuteScriptAndGetResultAsString(versionScript);

		return version;
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionLibraryApiTests::InstallAndTest
	//
	// Description:
	//  Installs a package and tests it.
	//  If we expect a successful install, then we check the version and location.
	//  We then try loading the library and check whether it loads as we expect.
	//
	void RExtensionLibraryApiTests::InstallAndTest(
		string extLibName,
		string pathToPackage,
		string installDir,
		string expectedVersion,
		string expectedLocation,
		bool   successfulInstall,
		bool   successfulLoad)
	{
		SQLCHAR *libError = nullptr;
		SQLINTEGER libErrorLength = 0;

		SQLRETURN result = (*sm_installExternalLibraryFuncPtr)(
			*m_sessionId,
			static_cast<SQLCHAR *>(static_cast<void *>(
				const_cast<char *>(extLibName.c_str()))),
			extLibName.length(),
			static_cast<SQLCHAR *>(static_cast<void *>(
				const_cast<char *>(pathToPackage.c_str()))),
			pathToPackage.length(),
			static_cast<SQLCHAR *>(static_cast<void *>(
				const_cast<char *>(installDir.c_str()))),
			installDir.length(),
			&libError,
			&libErrorLength);

		if (successfulInstall)
		{
			EXPECT_EQ(result, SQL_SUCCESS);

			EXPECT_EQ(libError, nullptr);
			EXPECT_EQ(libErrorLength, 0);

			try
			{
				DetachLibrary(extLibName);

				string location = GetLocationOfLibrary(extLibName);
				if (expectedLocation.empty())
				{
					expectedLocation = fs::path(installDir).append(extLibName).string();
				}

				EXPECT_TRUE(fs::equivalent(location, expectedLocation));

				string version = GetVersionOfLibrary(extLibName);
				EXPECT_EQ(version, expectedVersion);

				// Load the library, this will throw an exception if the load fails.
				//
				string loadScript = "library('" + extLibName + "');";
				sm_executeScriptFuncPtr(loadScript);

				EXPECT_TRUE(successfulLoad);
			}
			catch (...)
			{
				EXPECT_FALSE(successfulLoad);
			}

			DetachLibrary(extLibName);
		}
		else
		{
			EXPECT_EQ(result, SQL_ERROR);
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionLibraryApiTests::UninstallAndTest
	//
	// Description:
	//  Uninstalls and tests a package.
	//  If there is a different installation of the same package, then uninstalling won't
	//  fail on import, but the version in our specified dir should be gone.
	//
	void RExtensionLibraryApiTests::UninstallAndTest(
		string extLibName,
		string installDir,
		bool   otherInstallationExists)
	{
		SQLCHAR *libError = nullptr;
		SQLINTEGER libErrorLength = 0;

		SQLRETURN result = (*sm_uninstallExternalLibraryFuncPtr)(
			*m_sessionId,
			static_cast<SQLCHAR *>(static_cast<void *>(
				const_cast<char *>(extLibName.c_str()))),
			extLibName.length(),
			static_cast<SQLCHAR *>(static_cast<void *>(
				const_cast<char *>(installDir.c_str()))),
			installDir.length(),
			&libError,
			&libErrorLength);

		// Try to find the package - if it successfully loads,
		// the uninstall either failed or there is another
		// installation of the same package in a different location.
		//
		try
		{
			DetachLibrary(extLibName);

			string location = GetLocationOfLibrary(extLibName);

			// If another installation exists,
			// then the uninstall should succeed and load should still work
			//
			if (otherInstallationExists)
			{
				EXPECT_EQ(result, SQL_SUCCESS);
				EXPECT_EQ(libError, nullptr);
				EXPECT_EQ(libErrorLength, 0);

				EXPECT_TRUE(fs::exists(location));
				if (fs::exists(installDir))
				{
					EXPECT_FALSE(fs::equivalent(location, installDir));
				}
			}
			else
			{
				EXPECT_EQ(result, SQL_ERROR);
			}
		}
		catch (...)
		{
			EXPECT_EQ(result, SQL_SUCCESS);
			EXPECT_EQ(libError, nullptr);
			EXPECT_EQ(libErrorLength, 0);
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: BasicZipInstallTest
	//
	// Description:
	//  Installs a ZIP R package. On Windows, the zip is a built R package.
	//  On Linux, the zip is actually a tar gz file containing the source of the R package
	//  and is built as part of install.
	//  This is because SQL Server names the external library files with .zip extension on all
	//  platforms and we want to test in a similar way.
	//
	TEST_F(RExtensionLibraryApiTests, BasicZipInstallTest)
	{
		string libName = "assertthat";
		string version = "0.2.1";
		string zipName = RExtensionLibraryApiTests::sm_DbIdUserIdPrefix +
			libName + "_" + version + x_FileExtension;

		fs::path pkgPath = m_packagesPath / x_OsName / zipName;

		EXPECT_TRUE(fs::exists(pkgPath));

		InstallAndTest(
			libName,
			pkgPath.string(),
			RExtensionApiTests::sm_publicLibraryPath,
			version);

		UninstallAndTest(
			libName,
			RExtensionApiTests::sm_publicLibraryPath);
	}
}
