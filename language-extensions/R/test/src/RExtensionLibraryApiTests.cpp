//**************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server
// external language communication protocol.
// Copyright (C) 2020 Microsoft Corporation.
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

namespace ExtensionApiTest
{
	// x_OsName, x_FileExtension are used to derive path of packages on respective platforms.
	//
#ifdef _WIN64
	const string x_OsName = "windows";
	namespace fs = filesystem;
#else
	const string x_OsName = "linux";
	namespace fs = experimental::filesystem;
#endif

	const string x_FileExtension = ".zip";

	// Static members
	//
	FN_installExternalLibrary *RExtensionLibraryApiTests::sm_installExternalLibraryFuncPtr = nullptr;
	FN_uninstallExternalLibrary *RExtensionLibraryApiTests::sm_uninstallExternalLibraryFuncPtr = nullptr;
	FN_executeScript *RExtensionLibraryApiTests::sm_executeScriptFuncPtr = nullptr;
	FN_executeScriptAndGetResult *RExtensionLibraryApiTests::sm_executeScriptAndGetResultFuncPtr = nullptr;
	const string RExtensionLibraryApiTests::sm_PublicDbIdUserIdPrefix = "1_1_";
	const string RExtensionLibraryApiTests::sm_PrivateDbIdUserIdPrefix = "24_58_";
	fs::path RExtensionLibraryApiTests::sm_packagesSourcePath;

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
		SetPackageSourcePath();
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
	// Name: RExtensionLibraryApiTests::SetPackageSourcePath
	//
	// Description:
	// Sets the package source path root for use in tests.
	//
	void RExtensionLibraryApiTests::SetPackageSourcePath()
	{
		string enlRoot = string(getenv("ENL_ROOT"));
		sm_packagesSourcePath = fs::absolute(enlRoot);
		sm_packagesSourcePath = sm_packagesSourcePath / "language-extensions" / "R" / "test" / "test-packages";
		ASSERT_TRUE(fs::exists(sm_packagesSourcePath));
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
					expectedLocation = fs::path(installDir).string();
				}

				fs::path fullExpectedLocation = fs::path(expectedLocation) / extLibName;
				string normalizedExpectedLocation =
					Utilities::NormalizePathString(fullExpectedLocation.string());
				string normalizedFoundLocation = Utilities::NormalizePathString(location);

				EXPECT_TRUE(normalizedFoundLocation.compare(normalizedExpectedLocation) == 0);

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

			// We will reach here only if GetLocationOfLibrary succeeds
			// which either means other installation exists or uninstallation failed above.
			//
			fs::path previousFullInstallLocation = fs::path(installDir) / extLibName;
			string normalizedPreviousFullInstall =
				Utilities::NormalizePathString(previousFullInstallLocation.string());
			string normalizedFoundLocation = Utilities::NormalizePathString(location);

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
					EXPECT_FALSE(normalizedFoundLocation.compare(normalizedPreviousFullInstall) == 0);
				}
			}
			else
			{
				// Uninstallation failed.
				//
				EXPECT_EQ(result, SQL_ERROR);
				EXPECT_TRUE(normalizedFoundLocation.compare(normalizedPreviousFullInstall) == 0);
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
		string zipFileName = sm_PublicDbIdUserIdPrefix +
			libName + "_" + version + x_FileExtension;

		fs::path pkgPath = sm_packagesSourcePath / x_OsName / zipFileName;

		EXPECT_TRUE(fs::exists(pkgPath));

		InstallAndTest(
			libName,
			pkgPath.string(),
			sm_publicLibraryPath,
			version);

		UninstallAndTest(
			libName,
			sm_publicLibraryPath);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InstallMultipleTest
	//
	// Description:
	//  Installs multiple packages in different paths without cleaning.
	//
	TEST_F(RExtensionLibraryApiTests, InstallMultipleTest)
	{
		vector<string> libNames{ "assertthat", "pkgconfig", "rlang" };
		vector<string> versions{ "0.2.1", "2.0.3", "0.4.7"};

		vector<string> zipFileNames{
			sm_PublicDbIdUserIdPrefix + libNames[0] + "_"
				+ versions[0] + x_FileExtension,
			sm_PublicDbIdUserIdPrefix + libNames[1] + "_"
				+ versions[1] + x_FileExtension,
			sm_PrivateDbIdUserIdPrefix + libNames[2] + "_"
				+ versions[2] + x_FileExtension };

		vector<fs::path> pkgPaths{
			sm_packagesSourcePath / x_OsName / zipFileNames[0],
			sm_packagesSourcePath / x_OsName / zipFileNames[1],
			sm_packagesSourcePath / x_OsName / zipFileNames[2] };

		EXPECT_TRUE(fs::exists(pkgPaths[0]));
		InstallAndTest(
			libNames[0],
			pkgPaths[0].string(),
			sm_publicLibraryPath,
			versions[0]);

		EXPECT_TRUE(fs::exists(pkgPaths[1]));
		InstallAndTest(
			libNames[1],
			pkgPaths[1].string(),
			sm_publicLibraryPath,
			versions[1]);

		EXPECT_TRUE(fs::exists(pkgPaths[2]));
		InstallAndTest(
			libNames[2],
			pkgPaths[2].string(),
			sm_privateLibraryPath,
			versions[2]);

		UninstallAndTest(
			libNames[0],
			sm_publicLibraryPath);
		UninstallAndTest(
			libNames[1],
			sm_publicLibraryPath);
		UninstallAndTest(
			libNames[2],
			sm_privateLibraryPath);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InstallPublicPrivate
	//
	// Description:
	//  Installs package to public and makes sure its loaded from public path. 
	//  Then installs to private path without uninstalling from public.
	//  This time makes sure it is loaded from private.
	//
	TEST_F(RExtensionLibraryApiTests, InstallPublicPrivate)
	{
		string libName = "assertthat";
		string version = "0.2.1";
		string zipFileName =
			sm_PublicDbIdUserIdPrefix + libName + "_" + version + x_FileExtension;
		fs::path pkgPath;

		// First install in public path
		//
		pkgPath = sm_packagesSourcePath / x_OsName / zipFileName;

		EXPECT_TRUE(fs::exists(pkgPath));

		InstallAndTest(
			libName,
			pkgPath.string(),
			sm_publicLibraryPath,
			version);

		// Install same package in different context to see if it is loaded instead of public
		//
		EXPECT_TRUE(fs::exists(pkgPath));

		InstallAndTest(
			libName,
			pkgPath.string(),
			sm_privateLibraryPath, // USE PRIVATE LIB PATH
			version);

		// Uninstall private then public, make sure we at least get the public after
		// uninstalling private.
		//
		UninstallAndTest(libName, sm_privateLibraryPath, true); // otherInstallationExists
		UninstallAndTest(libName, sm_publicLibraryPath, false);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InstallPrivatePublic
	//
	// Description:
	//  Installs package to private and makes sure its loaded from private path.
	//  Then installs to public path without uninstalling from private.
	//  This time again makes sure it is loaded from private path since private takes precendence
	//  over public.
	//
	TEST_F(RExtensionLibraryApiTests, InstallPrivatePublic)
	{
		string libName = "lazyeval";
		string privateVersion = "0.2.1";
		string zipFileName =
			sm_PrivateDbIdUserIdPrefix + libName + "_" + privateVersion + x_FileExtension;
		fs::path pkgPath;

		pkgPath = sm_packagesSourcePath / x_OsName / zipFileName;

		EXPECT_TRUE(fs::exists(pkgPath));

		// First install in private path
		//
		InstallAndTest(
			libName,
			pkgPath.string(),
			sm_privateLibraryPath, // USE PRIVATE LIB PATH
			privateVersion,
			sm_privateLibraryPath); // Expected Location

		// Install same package in different context
		// We install this package into the PUBLIC scope, but since we have already
		// installed a package into PRIVATE,
		// we should still be loading PRIVATE over PUBLIC.
		//
		string publicVersion = "0.2.2";
		zipFileName = sm_PublicDbIdUserIdPrefix + libName + "_" + publicVersion + x_FileExtension;
		pkgPath = sm_packagesSourcePath / x_OsName / zipFileName;

		InstallAndTest(
			libName,
			pkgPath.string(),
			sm_publicLibraryPath,   // install to the public path
			privateVersion,         // expected version
			sm_privateLibraryPath); // expected location

		// Uninstall private then public, make sure we at least get the public after
		// uninstalling private.
		//
		UninstallAndTest(
			libName,
			sm_privateLibraryPath,
			true); // otherInstallationExists

		UninstallAndTest(
			libName,
			sm_publicLibraryPath,
			false);
	}

	//----------------------------------------------------------------------------------------------
	// Name: DependencyInstallTest
	//
	// Description:
	//  Installs a package that requires a dependency, without the dependency first - then with
	//  the dependency.
	//
	TEST_F(RExtensionLibraryApiTests, DependencyInstallTest)
	{
		string libName = "bindrcpp";
		string version = "0.2.2";
		string zipFileName =
			sm_PrivateDbIdUserIdPrefix + libName + "_" + version + x_FileExtension;

		fs::path pkgPath = sm_packagesSourcePath / x_OsName / zipFileName;

		EXPECT_TRUE(fs::exists(pkgPath));

		bool isLinux = x_OsName.compare("linux") == 0;

		InstallAndTest(
			libName,
			pkgPath.string(),
			sm_privateLibraryPath,
			version,
			sm_privateLibraryPath,
			// Successful install possible only on Windows since on Linux we can't even build
			// without the link library being installed.
			//
			!isLinux, // successfulInstall
			false);   // successfulLoad

		// bindrcpp depends on bindr and Rcpp; rcpp is already installed.
		// install bindr here.
		//
		string dependencyLibName = "bindr";
		string dependencyVersion = "0.1.1";
		string dependencyZipFileName =
			sm_PublicDbIdUserIdPrefix + dependencyLibName + "_"
			+ dependencyVersion + x_FileExtension;
		fs::path dependencyPkgPath = sm_packagesSourcePath / x_OsName / dependencyZipFileName;

		EXPECT_TRUE(fs::exists(dependencyPkgPath));

		// Install the dependency.
		//
		InstallAndTest(
			dependencyLibName,
			dependencyPkgPath.string(),
			sm_publicLibraryPath,
			dependencyVersion);

		string linkLibName = "plogr";
		if (isLinux)
		{
			// If it is linux, need to install plogr as well
			// since bindrcpp links to it while building.
			//
			string linkLibVersion = "0.2.0";
			string linkLibraryZipFileName =
				sm_PublicDbIdUserIdPrefix + linkLibName + "_"
				+ linkLibVersion + x_FileExtension;
			fs::path linkLibraryPkgPath = sm_packagesSourcePath / x_OsName / linkLibraryZipFileName;

			EXPECT_TRUE(fs::exists(linkLibraryPkgPath));

			// Install the link library.
			//
			InstallAndTest(
				linkLibName,
				linkLibraryPkgPath.string(),
				sm_publicLibraryPath,
				linkLibVersion);
		}

		// Install bindrcpp now that all the dependencies are installed.
		// Here we try installing the top package into private, which will get the dependency
		// from public.
		//
		InstallAndTest(
			libName,
			pkgPath.string(),
			sm_privateLibraryPath,
			version);

		// Detach bindr library which would have been loaded when bindrcpp is loaded.
		//
		DetachLibrary(dependencyLibName);

		UninstallAndTest(
			dependencyLibName,
			sm_publicLibraryPath);

		if (isLinux)
		{
			DetachLibrary(linkLibName);
			UninstallAndTest(
				linkLibName,
				sm_publicLibraryPath);
		}

		UninstallAndTest(
			libName,
			sm_privateLibraryPath);
	}

	//----------------------------------------------------------------------------------------------
	// Name: UninstallNonexistentTest
	//
	// Description:
	//  Uninstalls nonexistent package.
	//
	TEST_F(RExtensionLibraryApiTests, UninstallNonexistentTest)
	{
		string libName = "noPackage";

		// There will be an error while uninstalling via R
		// but we check for non-existence of folder to confirm the library is not installed.
		// So, this will still succeed since no such folder with "noPackage" exists.
		//
		UninstallAndTest(
			libName,
			sm_publicLibraryPath);
	}

	//
	// Negative Tests
	//

	//----------------------------------------------------------------------------------------------
	// Name: NoDescriptionPackageInstallTest
	//
	// Description:
	//  Tries to install a package without DESCRIPTION.
	//
	TEST_F(RExtensionLibraryApiTests, NoDescriptionPackageInstallTest)
	{
		string libName = "assertthatbad";
		string version = "0.2.1";
		string zipFileName =
			sm_PublicDbIdUserIdPrefix + libName + "_" + version + x_FileExtension;
		fs::path pkgPath;

		pkgPath = sm_packagesSourcePath / x_OsName / zipFileName;

		EXPECT_TRUE(fs::exists(pkgPath));

		// Installation should fail.
		//
		InstallAndTest(
			libName,
			pkgPath.string(),
			sm_publicLibraryPath,
			version,
			sm_publicLibraryPath,
			false); // successfulInstall
	}

	//----------------------------------------------------------------------------------------------
	// Name: InterchangedPackagesTest
	//
	// Description:
	//  Tries to install a source tar.gz package on Windows (succeeds since Rtools is installed)
	//  and a zip of windows built package on Linux (fails).
	//
	TEST_F(RExtensionLibraryApiTests, InterchangedPackagesTest)
	{
		string libName = "assertthat";
		string version = "0.2.1";
		string zipFileName =
			sm_PublicDbIdUserIdPrefix + libName + "interchange_" + version + x_FileExtension;
		fs::path pkgPath;

		pkgPath = sm_packagesSourcePath / x_OsName / zipFileName;

		EXPECT_TRUE(fs::exists(pkgPath));

		bool isWindows = x_OsName.compare("windows") == 0;

		// Installation should succeed on windows, fail on linux.
		//
		InstallAndTest(
			libName,
			pkgPath.string(),
			sm_publicLibraryPath,
			version,
			sm_publicLibraryPath,
			isWindows); // successfulInstall
	}

	//----------------------------------------------------------------------------------------------
	// Name: ZipWithinZipTest
	//
	// Description:
	//  Tries to install a wrongly structured zip i.e. zip within zip.
	//  This fails because we expect only one level of compression of the orginal package.
	//
	TEST_F(RExtensionLibraryApiTests, ZipWithinZipTest)
	{
		string libName = "assertthatzipwithinzip";
		string version = "0.2.1";
		string zipFileName =
			sm_PublicDbIdUserIdPrefix + libName + "_" + version + x_FileExtension;
		fs::path pkgPath;

		pkgPath = sm_packagesSourcePath / x_OsName / zipFileName;

		EXPECT_TRUE(fs::exists(pkgPath));

		// Installation should fail.
		//
		InstallAndTest(
			libName,
			pkgPath.string(),
			sm_publicLibraryPath,
			version,
			sm_publicLibraryPath,
			false); // successfulInstall
	}
}
