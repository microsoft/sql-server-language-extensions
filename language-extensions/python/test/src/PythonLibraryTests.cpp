//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonLibraryTests.cpp
//
// Purpose:
//  Define the gtest overrides and the External Library tests
//
//*************************************************************************************************
#include "PythonLibraryTests.h"
#include "PythonTestUtilities.h"

using namespace std;
namespace bp = boost::python;

namespace LibraryApiTests
{
	// Name: SetUp
	//
	// Description:
	//  Code here will be called immediately after the constructor (right
	//  before each test).
	//
	void ExternalLibraryApiTests::SetUp()
	{
		fs::path libPath = fs::absolute(m_libraryPath);

		if (fs::exists(libPath))
		{
			fs::remove_all(libPath);
		}

		EXPECT_FALSE(fs::exists(libPath));

		SetupVariables();
		Initialize();
	}

	// Name: TearDown
	//
	// Description:
	//  Code here will be called immediately after each test (right
	//  before the destructor).
	//
	void ExternalLibraryApiTests::TearDown()
	{
		fs::path libPath = fs::absolute(m_libraryPath);

		if (fs::exists(libPath))
		{
			fs::remove_all(libPath);
		}

		EXPECT_FALSE(fs::exists(libPath));

		DoCleanup();
	}

	// Name: SetupVariables
	//
	// Description:
	//  Set up default, valid variables for use in tests
	//
	void ExternalLibraryApiTests::SetupVariables()
	{
		m_libraryPath = fs::absolute(m_libraryPath).string();

		string enlRoot = getenv("ENL_ROOT");

		m_packagesPath = fs::absolute(enlRoot);

		m_packagesPath = m_packagesPath / "language-extensions" / "python" / "test" / "test_packages";

		ASSERT_TRUE(fs::exists(m_packagesPath));

		fs::path libPath = m_libraryPath;
		m_publicLibraryPath = (libPath / "1").string();
		m_privateLibraryPath = (libPath / "2").string();
	}

	// Name: Initialize
	//
	// Description:
	//  Initialize pythonextension for library management
	//
	void ExternalLibraryApiTests::Initialize()
	{
		SQLRETURN result = SQL_ERROR;

		result = Init(
			nullptr, // Extension Params
			0,       // Extension Params Length
			nullptr, // Extension Path
			0,       // Extension Path Length
			reinterpret_cast<SQLCHAR *>(const_cast<char *>(m_publicLibraryPath.c_str())),
			m_publicLibraryPath.length(),
			reinterpret_cast<SQLCHAR *>(const_cast<char *>(m_privateLibraryPath.c_str())),
			m_privateLibraryPath.length()
		);

		EXPECT_EQ(result, SQL_SUCCESS);

		m_mainModule = bp::import("__main__");
		m_mainNamespace = bp::extract<bp::dict>(m_mainModule.attr("__dict__"));

		if (!m_mainNamespace.has_key("originalKeys"))
		{
			string getKeysScript = "import sys\n"
				"originalKeys = list(sys.modules.keys())";
			bp::exec(getKeysScript.c_str(), m_mainNamespace);
		}

		ASSERT_NE(m_mainModule, boost::python::object());
		ASSERT_NE(m_mainNamespace, boost::python::object());
	}

	// Name: DoCleanup
	//
	// Description:
	//  Call Cleanup on the PythonExtension.
	//  Testing if Cleanup is implemented correctly.
	//
	void ExternalLibraryApiTests::DoCleanup()
	{
		SQLRETURN result = Cleanup();
		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Name: CleanModules
	//
	// Description:
	//  Clean sys.modules in python to remove import traces.
	//  This keeps the system clean for the next installation and import test.
	//
	void ExternalLibraryApiTests::CleanModules(
		string extLibName,
		string moduleName)
	{
		try
		{
			string delScript = "import sys\n"
				"keys = [key for key in sys.modules if '" + extLibName +
				"' in key or '" + moduleName + "' in key]\n"
				"for k in keys:"
				"    del sys.modules[k]";

			bp::exec(delScript.c_str(), m_mainNamespace);
		}
		catch (const bp::error_already_set &)
		{
			string s = PythonTestUtilities::ParsePythonException();
			cout << s << endl;
		}
	}

	// Name: InstallAndTest
	//
	// Description:
	//  Install a package and test it.
	//  If we expect a successful install, then we check the version and location.
	//  We then try importing the package and check whether it imports as we expect.
	//
	void ExternalLibraryApiTests::InstallAndTest(
		string extLibName,
		string moduleName,
		string pathToPackage,
		string installDir,
		string expectedVersion,
		string expectedLocation,
		bool   successfulInstall,
		bool   successfulImport)
	{
		SQLCHAR *libError = nullptr;
		SQLINTEGER libErrorLength = 0;

		SQLRETURN result = InstallExternalLibrary(
			SQLGUID(),
			reinterpret_cast<SQLCHAR *>(const_cast<char *>(extLibName.c_str())),
			extLibName.length(),
			reinterpret_cast<SQLCHAR *>(const_cast<char *>(pathToPackage.c_str())),
			pathToPackage.length(),
			reinterpret_cast<SQLCHAR *>(const_cast<char *>(installDir.c_str())),
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
				CleanModules(extLibName, moduleName);

				string infoScript = "import pkg_resources; import importlib;import sys\n"
					"importlib.invalidate_caches();\n"
					"pkg_resources._initialize_master_working_set()\n"
					"dist = pkg_resources.get_distribution('" + extLibName + "');\n"
					"location = dist.location; version = dist.version;";

				bp::exec(infoScript.c_str(), m_mainNamespace);
				string version = bp::extract<string>(m_mainNamespace["version"]);
				string location = bp::extract<string>(m_mainNamespace["location"]);

				if (expectedLocation.empty())
				{
					expectedLocation = installDir;
				}

				EXPECT_TRUE(fs::equivalent(location, expectedLocation));
				EXPECT_EQ(version, expectedVersion);

				// Import the module then delete it.
				//
				string importScript = "import " + moduleName + "; del " + moduleName;
				bp::exec(importScript.c_str(), m_mainNamespace);

				EXPECT_TRUE(successfulImport);
			}
			catch (const bp::error_already_set &)
			{
				EXPECT_FALSE(successfulImport);
				string s = PythonTestUtilities::ParsePythonException();
				cout << s << endl;
			}

			CleanModules(extLibName, moduleName);
		}
		else
		{
			EXPECT_EQ(result, SQL_ERROR);
		}
	}

	// Name: UninstallAndTest
	//
	// Description:
	//  Uninstall and test a package.
	//  If there is a different installation of the same package, then uninstalling won't
	//  fail on import, but the version in our specified dir should be gone.
	//
	void ExternalLibraryApiTests::UninstallAndTest(
		string extLibName,
		string moduleName,
		string installDir,
		bool   otherInstallationExists)
	{
		SQLCHAR *libError = nullptr;
		SQLINTEGER libErrorLength = 0;

		SQLRETURN result = UninstallExternalLibrary(
			SQLGUID(),
			reinterpret_cast<SQLCHAR *>(const_cast<char *>(extLibName.c_str())),
			extLibName.length(),
			reinterpret_cast<SQLCHAR *>(const_cast<char *>(installDir.c_str())),
			installDir.length(),
			&libError,
			&libErrorLength);

		// Try to find the package - if it successfully imports, 
		// the uninstall either failed or there is another
		// installation of the same package in a different location.
		//
		try
		{
			CleanModules(extLibName, moduleName);

			string infoScript = "import pkg_resources; import importlib;import sys\n"
				"importlib.invalidate_caches();\n"
				"pkg_resources._initialize_master_working_set()\n"
				"dist = pkg_resources.get_distribution('" + extLibName + "');\n"
				"location = dist.location; version = dist.version;";

			bp::exec(infoScript.c_str(), m_mainNamespace);

			string version = bp::extract<string>(m_mainNamespace["version"]);
			string location = bp::extract<string>(m_mainNamespace["location"]);

			// If another installation exists, 
			// then the uninstall should succeed and import should still work
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
		catch (const bp::error_already_set &)
		{
			EXPECT_EQ(result, SQL_SUCCESS);
			EXPECT_EQ(libError, nullptr);
			EXPECT_EQ(libErrorLength, 0);
		}
	}

	// Name: BasicZipInstallTest
	//
	// Description:
	//  Install a ZIP python package.
	//  This zip package also has some random extra_file.txt, 
	//  which we should ignore without failure.
	//
	TEST_F(ExternalLibraryApiTests, BasicZipInstallTest)
	{
		string libName = "testpackageA";
		fs::path pkgPath = m_packagesPath / "testpackageA-ZIP.zip";
		string version = "0.0.1";

		EXPECT_TRUE(fs::exists(pkgPath));

		InstallAndTest(
			libName,             // extLibName
			libName,             // moduleName
			pkgPath.string(),
			m_publicLibraryPath,
			version);

		UninstallAndTest(
			libName,             // extLibName
			libName,             // moduleName
			m_publicLibraryPath);
	}

	// Name: BasicWhlInstallTest
	//
	// Description:
	//  Install a WHL python package
	//
	TEST_F(ExternalLibraryApiTests, BasicWhlInstallTest)
	{
		string libName = "astor";
		fs::path pkgPath = m_packagesPath / "astor-0.7.1-WHL.zip";
		string version = "0.7.1";

		EXPECT_TRUE(fs::exists(pkgPath));

		InstallAndTest(
			libName,             // extLibName
			libName,             // moduleName
			pkgPath.string(),
			m_publicLibraryPath,
			version);

		UninstallAndTest(
			libName,             // extLibName
			libName,             // moduleName
			m_publicLibraryPath);
	}

	// Name: BasicTarInstallTest
	//
	// Description:
	//  Install a TAR GZ python package
	//
	TEST_F(ExternalLibraryApiTests, BasicTarInstallTest)
	{
		// The library name and module name are not the same
		// We install with "pip install absl-py", but import with "import absl"
		//
		string libName = "absl-py";
		string moduleName = "absl";
		fs::path pkgPath = m_packagesPath / "absl-py-1.0.0-TAR.zip";
		string version = "1.0.0";

		EXPECT_TRUE(fs::exists(pkgPath));

		InstallAndTest(libName, moduleName, pkgPath.string(), m_publicLibraryPath, version);

		UninstallAndTest(libName, moduleName, m_publicLibraryPath);
	}

	// Name: InstallMultipleTest
	//
	// Description:
	//  Install multiple packages of different types without cleaning
	//
	TEST_F(ExternalLibraryApiTests, InstallMultipleTest)
	{
		vector<string> libNames{ "absl-py", "astor", "testpackageA" };
		vector<string> moduleNames{ "absl", "astor", "testpackageA" };

		vector<fs::path> pkgPaths{ m_packagesPath / "absl-py-1.0.0-TAR.zip",
			 m_packagesPath / "astor-0.7.1-WHL.zip",
			 m_packagesPath / "testpackageA-ZIP.zip" };

		vector<string> versions{ "1.0.0", "0.7.1", "0.0.1" };

		for (size_t i = 0; i < libNames.size(); ++i)
		{
			EXPECT_TRUE(fs::exists(pkgPaths[i]));
			InstallAndTest(libNames[i], moduleNames[i], pkgPaths[i].string(), m_publicLibraryPath, versions[i]);
		}

		for (size_t i = 0; i < libNames.size(); ++i)
		{
			UninstallAndTest(libNames[i], moduleNames[i], m_publicLibraryPath);
		}
	}

	// Name: InstallPublicPrivate
	//
	// Description:
	//  Install package to public then private and make sure public is loaded, then private second time
	//
	TEST_F(ExternalLibraryApiTests, InstallPublicPrivate)
	{
		string libName = "testpackageA";

		fs::path pkgPath;
		string version;

		// First install in public path
		//
		pkgPath = m_packagesPath / "testpackageA-v2-ZIP.zip";
		version = "0.0.2";

		EXPECT_TRUE(fs::exists(pkgPath));

		InstallAndTest(
			libName,             // extLibName
			libName,             // moduleName
			pkgPath.string(),
			m_publicLibraryPath,
			version);

		// Install same package in different context to see if it is loaded instead of public
		//
		pkgPath = m_packagesPath / "testpackageA-ZIP.zip";
		version = "0.0.1";
		EXPECT_TRUE(fs::exists(pkgPath));

		InstallAndTest(
			libName,              // extLibName
			libName,              // moduleName
			pkgPath.string(),
			m_privateLibraryPath, // USE PRIVATE LIB PATH
			version);

		// Uninstall private then public, make sure we get still get the public after
		// uninstalling private.
		//
		UninstallAndTest(libName, libName, m_privateLibraryPath, true); // otherInstallationExists
		UninstallAndTest(libName, libName, m_publicLibraryPath, false);
	}

	// Name: InstallPrivatePublic
	//
	// Description:
	//  Install package to private then public and make sure private is loaded BOTH times
	//
	TEST_F(ExternalLibraryApiTests, InstallPrivatePublic)
	{
		string libName = "testpackageA";

		fs::path pkgPath;
		string version;

		// First install in public path
		//
		pkgPath = m_packagesPath / "testpackageA-v2-ZIP.zip";
		version = "0.0.2";

		EXPECT_TRUE(fs::exists(pkgPath));

		InstallAndTest(
			libName,               // extLibName
			libName,               // moduleName
			pkgPath.string(),
			m_privateLibraryPath,  // USE PRIVATE LIB PATH
			version,
			m_privateLibraryPath); // Expected location

		// Install same package in different context
		//
		pkgPath = m_packagesPath / "testpackageA-ZIP.zip";
		version = "0.0.1";
		EXPECT_TRUE(fs::exists(pkgPath));

		// We install this package into the PUBLIC scope, but since we have already installed a package into PRIVATE,
		// we should still be loading PRIVATE over PUBLIC.
		//
		InstallAndTest(
			libName,               // extLibName
			libName,               // moduleName
			pkgPath.string(),
			m_publicLibraryPath,   // Install to the public path
			"0.0.2",               // Expect that we are still importing the private version and location
			m_privateLibraryPath); // Expected location

		// Uninstall private then public, make sure we get still get the public after
		// uninstalling private.
		//
		UninstallAndTest(libName, libName, m_privateLibraryPath, true); // otherInstallationExists
		UninstallAndTest(libName, libName, m_publicLibraryPath, false);
	}

	// Name: DependencyInstallTest
	//
	// Description:
	//  Install a package that requires a dependency
	//
	TEST_F(ExternalLibraryApiTests, DependencyInstallTest)
	{
		// telemetry depends on statsd - install dep first
		//
		string libName = "statsd";
		fs::path pkgPath = m_packagesPath / "statsd-3.3.0-WHL.zip";
		string version = "3.3.0";

		EXPECT_TRUE(fs::exists(pkgPath));

		InstallAndTest(
			libName,             // extLibName
			libName,             // moduleName
			pkgPath.string(),
			m_publicLibraryPath,
			version,
			m_publicLibraryPath, // expectedLocation
			true,                // successfulInstall
			true                 // successfulImport
		);

		// Install telemetry now that the dependency is installed.
		// Here we try installing the top package into private, which will get the dependency
		// from public.
		//
		libName = "telemetry";
		pkgPath = m_packagesPath / "telemetry-0.3.2-TAR.zip";
		version = "0.3.2";

		EXPECT_TRUE(fs::exists(pkgPath));

		InstallAndTest(
			libName,              // extLibName
			libName,              // moduleName
			pkgPath.string(),
			m_privateLibraryPath, // expectedLocation
			version);

		// Clean statsd (which was imported by telemetry) 
		// from the imported modules to not interfere with later tests.
		//
		CleanModules(
			"statsd",  // extLibName
			"statsd"); // moduleName
	}

	// Name: UninstallNonexistentTest
	//
	// Description:
	//  Uninstall nonexistent package. pip should automatically passthrough successfully.
	//
	TEST_F(ExternalLibraryApiTests, UninstallNonexistentTest)
	{
		string libName = "noPackage";

		UninstallAndTest(
			libName,             // extLibName
			libName,             // moduleName
			m_publicLibraryPath
		);
	}

	//
	// Negative Tests
	//

	// Name: NoSetupPackageInstallTest
	//
	// Description:
	//  Try to install a package without setup.py
	//
	TEST_F(ExternalLibraryApiTests, NoSetupPackageInstallTest)
	{
		string libName = "testpackagebad";
		fs::path pkgPath = m_packagesPath / "testpackagebad-ZIP.zip";
		string version = "0.7.1";

		EXPECT_TRUE(fs::exists(pkgPath));

		InstallAndTest(
			libName,             // extLibName
			libName,             // moduleName
			pkgPath.string(),
			m_publicLibraryPath,
			version,
			m_publicLibraryPath, // expectedLocation
			false);              // Successful install
	}

	// Name: NotAZipInstallTest
	//
	// Description:
	//  Try to install a non-zip package
	//
	TEST_F(ExternalLibraryApiTests, NotAZipInstallTest)
	{
		string libName = "badpackage";
		fs::path pkgPath = m_packagesPath / "bad-package-ZIP.zip";
		string version = "0.7.1";

		EXPECT_TRUE(fs::exists(pkgPath));

		InstallAndTest(
			libName,             // extLibName
			libName,             // moduleName
			pkgPath.string(),
			m_publicLibraryPath,
			version,
			m_publicLibraryPath, // expectedLocation
			false);              // Successful install
	}

	// Name: NoPrezipInstallTest
	//
	// Description:
	//  Try to install a whl directly, with no zip over it
	//  This fails because we expect all our packages to be a zip, with the whl file inside the zip.
	//  When we get packages from SQL Server, they will not have any special filename, and whl files
	//  hold information in the name. By requiring a zip on top of the whl, we preserve the name.
	//
	TEST_F(ExternalLibraryApiTests, NoPrezipInstallTest)
	{
		string libName = "astor";
		fs::path pkgPath = m_packagesPath / "astor.whl";
		string version = "0.7.1";

		EXPECT_TRUE(fs::exists(pkgPath));

		InstallAndTest(
			libName,             // extLibName
			libName,             // moduleName
			pkgPath.string(),
			m_publicLibraryPath,
			version,
			m_publicLibraryPath, // expectedLocation
			false);              // Successful install
	}

	// Name: NoDependencyInstallTest
	//
	// Description:
	//  Install a package that requires a dependency, but without the dependency
	//
	TEST_F(ExternalLibraryApiTests, NoDependencyInstallTest)
	{
		string libName = "telemetry";
		fs::path pkgPath = m_packagesPath / "telemetry-0.3.2-TAR.zip";
		string version = "0.3.2";

		EXPECT_TRUE(fs::exists(pkgPath));

		// Import fails because it depends on statsd
		//
		InstallAndTest(
			libName,             // extLibName
			libName,             // moduleName
			pkgPath.string(),
			m_publicLibraryPath,
			version,
			m_publicLibraryPath, // expectedLocation
			true,                // Successful install
			false);              // Successful import
	}
}
