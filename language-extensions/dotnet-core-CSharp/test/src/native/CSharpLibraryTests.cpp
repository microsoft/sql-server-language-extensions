//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpLibraryTests.cpp
//
// Purpose:
//  Tests the .NET Core CSharpExtension's implementation of the
//  InstallExternalLibrary and UninstallExternalLibrary APIs.
//  Uses the CSharpExtensionApiTests fixture to share the single
//  .NET runtime initialization.
//
//*********************************************************************
#include "CSharpExtensionApiTests.h"

using namespace std;
namespace fs = experimental::filesystem;

namespace ExtensionApiTest
{
    // Helper: get path to test_packages directory
    //
    static string GetPackagesPath()
    {
        string result;
        const char *enlRoot = getenv("ENL_ROOT");

        if (enlRoot != nullptr)
        {
            result = (fs::path(enlRoot) /
                "language-extensions" / "dotnet-core-CSharp" / "test" / "test_packages").string();
        }
        else
        {
            // Fallback: navigate from executable path
            char path[MAX_PATH + 1] = { 0 };
            GetModuleFileName(NULL, path, MAX_PATH);
            fs::path buildOutputPath = fs::path(path).parent_path().parent_path().parent_path().parent_path();
            result = (buildOutputPath.parent_path().parent_path() /
                "language-extensions" / "dotnet-core-CSharp" / "test" / "test_packages").string();
        }

        return result;
    }

    // Helper: create a clean temporary install directory
    //
    static string CreateInstallDir()
    {
        char path[MAX_PATH + 1] = { 0 };
        GetModuleFileName(NULL, path, MAX_PATH);
        string installDir = (fs::path(path).parent_path() / "testInstallLibs").string();

        if (fs::exists(installDir))
        {
            fs::remove_all(installDir);
        }
        fs::create_directories(installDir);

        return installDir;
    }

    // Helper: clean up install directory
    //
    static void CleanupInstallDir(const string &installDir)
    {
        if (fs::exists(installDir))
        {
            fs::remove_all(installDir);
        }
    }

    // Helper: call InstallExternalLibrary and check result
    //
    static SQLRETURN CallInstall(
        FN_installExternalLibrary *installFunc,
        const string &libName,
        const string &libFilePath,
        const string &installDir)
    {
        SQLCHAR *libError = nullptr;
        SQLINTEGER libErrorLength = 0;

        SQLRETURN result = (*installFunc)(
            SQLGUID(),
            reinterpret_cast<SQLCHAR *>(const_cast<char *>(libName.c_str())),
            static_cast<SQLINTEGER>(libName.length()),
            reinterpret_cast<SQLCHAR *>(const_cast<char *>(libFilePath.c_str())),
            static_cast<SQLINTEGER>(libFilePath.length()),
            reinterpret_cast<SQLCHAR *>(const_cast<char *>(installDir.c_str())),
            static_cast<SQLINTEGER>(installDir.length()),
            &libError,
            &libErrorLength);

        return result;
    }

    // Helper: call UninstallExternalLibrary and check result
    //
    static SQLRETURN CallUninstall(
        FN_uninstallExternalLibrary *uninstallFunc,
        const string &libName,
        const string &installDir)
    {
        SQLCHAR *libError = nullptr;
        SQLINTEGER libErrorLength = 0;

        SQLRETURN result = (*uninstallFunc)(
            SQLGUID(),
            reinterpret_cast<SQLCHAR *>(const_cast<char *>(libName.c_str())),
            static_cast<SQLINTEGER>(libName.length()),
            reinterpret_cast<SQLCHAR *>(const_cast<char *>(installDir.c_str())),
            static_cast<SQLINTEGER>(installDir.length()),
            &libError,
            &libErrorLength);

        return result;
    }

    // Helper: check if directory has any files or subdirectories
    //
    static bool DirectoryHasFiles(const string &dir)
    {
        bool hasFiles = false;

        for (const auto &entry : fs::directory_iterator(dir))
        {
            hasFiles = true;
            break;
        }

        return hasFiles;
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallZipContainingZipTest
    //
    // Description:
    //  Tests installing an outer zip that contains an inner zip.
    //  Verifies that files from the inner zip are extracted to the install directory.
    //
    TEST_F(CSharpExtensionApiTests, InstallZipContainingZipTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "testpackageA-ZIP.zip").string();
        ASSERT_TRUE(fs::exists(packagePath)) << "Test package not found: " << packagePath;

        string installDir = CreateInstallDir();

        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "testpackageA", packagePath, installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        EXPECT_TRUE(DirectoryHasFiles(installDir))
            << "No files found in install directory after installation";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallZipContainingDllsTest
    //
    // Description:
    //  Tests installing an outer zip that contains DLLs directly (no inner zip).
    //  Verifies that DLLs are copied to the install directory.
    //
    TEST_F(CSharpExtensionApiTests, InstallZipContainingDllsTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "testpackageB-DLL.zip").string();
        ASSERT_TRUE(fs::exists(packagePath)) << "Test package not found: " << packagePath;

        string installDir = CreateInstallDir();

        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "testpackageB", packagePath, installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        // Verify that DLL files were copied to the install directory
        bool hasDll = false;
        for (const auto &entry : fs::directory_iterator(installDir))
        {
            if (entry.path().extension() == ".dll")
            {
                hasDll = true;
                break;
            }
        }
        EXPECT_TRUE(hasDll) << "No DLL files found in install directory after installation";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallInvalidZipTest
    //
    // Description:
    //  Tests that installing a file that has a .zip extension but is not actually
    //  a valid ZIP (magic bytes are not PK) is treated as a raw file and copied
    //  directly to the install directory, returning SQL_SUCCESS.
    //
    TEST_F(CSharpExtensionApiTests, InstallInvalidZipTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "bad-package-ZIP.zip").string();
        ASSERT_TRUE(fs::exists(packagePath)) << "Test package not found: " << packagePath;

        string installDir = CreateInstallDir();

        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "bad-package", packagePath, installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        // The file should have been copied as a raw file
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "bad-package-ZIP.zip"))
            << "Raw file not found in install directory";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallNonExistentFileTest
    //
    // Description:
    //  Tests that installing a non-existent file returns SQL_ERROR.
    //
    TEST_F(CSharpExtensionApiTests, InstallNonExistentFileTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "nonexistent.zip").string();

        string installDir = CreateInstallDir();

        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "nonexistent", packagePath, installDir);
        EXPECT_EQ(result, SQL_ERROR);

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: UninstallLibraryTest
    //
    // Description:
    //  Tests installing a library, then uninstalling it, and verifying the
    //  install directory is empty.
    //
    TEST_F(CSharpExtensionApiTests, UninstallLibraryTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "testpackageB-DLL.zip").string();
        ASSERT_TRUE(fs::exists(packagePath)) << "Test package not found: " << packagePath;

        string installDir = CreateInstallDir();

        // Install first
        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "testpackageB", packagePath, installDir);
        EXPECT_EQ(result, SQL_SUCCESS);
        EXPECT_TRUE(DirectoryHasFiles(installDir)) << "No files found after installation";

        // Uninstall
        result = CallUninstall(sm_uninstallExternalLibraryFuncPtr,
            "testpackageB", installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        // Verify directory is empty
        EXPECT_FALSE(DirectoryHasFiles(installDir)) << "Files still present after uninstall";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ReinstallLibraryTest
    //
    // Description:
    //  Tests installing a library, then reinstalling with a different package
    //  to verify overwrite behavior works correctly.
    //
    TEST_F(CSharpExtensionApiTests, ReinstallLibraryTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePathA = (fs::path(packagesPath) / "testpackageA-ZIP.zip").string();
        string packagePathB = (fs::path(packagesPath) / "testpackageB-DLL.zip").string();
        ASSERT_TRUE(fs::exists(packagePathA)) << "Test package not found: " << packagePathA;
        ASSERT_TRUE(fs::exists(packagePathB)) << "Test package not found: " << packagePathB;

        string installDir = CreateInstallDir();

        // Install first package
        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "testpackage", packagePathA, installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        // Install second package (overwrite)
        result = CallInstall(sm_installExternalLibraryFuncPtr,
            "testpackage", packagePathB, installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        // Verify the install directory has files from the second package
        bool hasDll = false;
        for (const auto &entry : fs::directory_iterator(installDir))
        {
            if (entry.path().extension() == ".dll")
            {
                hasDll = true;
                break;
            }
        }
        EXPECT_TRUE(hasDll) << "DLL from second package not found after reinstall";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallEmptyZipTest
    //
    // Description:
    //  Tests that installing an empty zip (zero entries) returns SQL_ERROR.
    //  The install logic detects that the archive contains no entries and
    //  explicitly fails the operation rather than proceeding with an empty
    //  set of files.
    //
    TEST_F(CSharpExtensionApiTests, InstallEmptyZipTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "testpackageC-EMPTY.zip").string();
        ASSERT_TRUE(fs::exists(packagePath)) << "Test package not found: " << packagePath;

        string installDir = CreateInstallDir();

        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "emptypackage", packagePath, installDir);
        EXPECT_EQ(result, SQL_ERROR);

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallZipWithNestedDirectoriesTest
    //
    // Description:
    //  Tests that installing a zip with nested subdirectories (e.g.,
    //  lib/net8.0/, runtimes/win-x64/) preserves the full directory tree.
    //
    TEST_F(CSharpExtensionApiTests, InstallZipWithNestedDirectoriesTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "testpackageD-NESTED.zip").string();
        ASSERT_TRUE(fs::exists(packagePath)) << "Test package not found: " << packagePath;

        string installDir = CreateInstallDir();

        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "nestedpackage", packagePath, installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        // Verify subdirectories were preserved
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "lib" / "net8.0" / "MyLib.dll"))
            << "Nested file lib/net8.0/MyLib.dll not found";
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "runtimes" / "win-x64" / "native.dll"))
            << "Nested file runtimes/win-x64/native.dll not found";
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "MyLib.deps.json"))
            << "Root file MyLib.deps.json not found";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallRawDllNotZipTest
    //
    // Description:
    //  Tests that installing a raw DLL file (not a zip) copies the file directly
    //  to the install directory and returns SQL_SUCCESS.
    //
    TEST_F(CSharpExtensionApiTests, InstallRawDllNotZipTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "testpackageE-RAWDLL.dll").string();
        ASSERT_TRUE(fs::exists(packagePath)) << "Test package not found: " << packagePath;

        string installDir = CreateInstallDir();

        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "rawdllpackage", packagePath, installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        // Verify the raw DLL was copied to the install directory
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageE-RAWDLL.dll"))
            << "Raw DLL not found in install directory";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallZipWithSpacesInFilenamesTest
    //
    // Description:
    //  Tests that installing a zip whose entries contain spaces in filenames
    //  extracts correctly.
    //
    TEST_F(CSharpExtensionApiTests, InstallZipWithSpacesInFilenamesTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "testpackageF-SPACES.zip").string();
        ASSERT_TRUE(fs::exists(packagePath)) << "Test package not found: " << packagePath;

        string installDir = CreateInstallDir();

        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "spacespackage", packagePath, installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        EXPECT_TRUE(fs::exists(fs::path(installDir) / "My Library.dll"))
            << "File with spaces 'My Library.dll' not found";
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "config file.json"))
            << "File with spaces 'config file.json' not found";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallZipWithManyFilesTest
    //
    // Description:
    //  Tests that installing a zip containing many files (50 DLLs) extracts
    //  all of them correctly.
    //
    TEST_F(CSharpExtensionApiTests, InstallZipWithManyFilesTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "testpackageG-MANYFILES.zip").string();
        ASSERT_TRUE(fs::exists(packagePath)) << "Test package not found: " << packagePath;

        string installDir = CreateInstallDir();

        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "manyfilespackage", packagePath, installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        // Count extracted DLL files
        int dllCount = 0;
        for (const auto &entry : fs::directory_iterator(installDir))
        {
            if (entry.path().extension() == ".dll")
            {
                dllCount++;
            }
        }
        EXPECT_EQ(dllCount, 50) << "Expected 50 DLL files, found " << dllCount;

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallZipSlipTest
    //
    // Description:
    //  Tests that installing a zip containing path-traversal entries (e.g., "../../malicious.dll")
    //  returns SQL_ERROR. .NET 8's ZipFile.ExtractToDirectory has built-in protection against
    //  zip-slip attacks and throws IOException for such entries.
    //
    TEST_F(CSharpExtensionApiTests, InstallZipSlipTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "testpackageH-ZIPSLIP.zip").string();
        ASSERT_TRUE(fs::exists(packagePath)) << "Test package not found: " << packagePath;

        string installDir = CreateInstallDir();

        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "zipslippackage", packagePath, installDir);
        EXPECT_EQ(result, SQL_ERROR);

        // Verify no files were written outside the install directory
        fs::path parentDir = fs::path(installDir).parent_path();
        EXPECT_FALSE(fs::exists(parentDir / "malicious.dll"))
            << "Zip-slip attack: file escaped install directory";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: DoubleUninstallTest
    //
    // Description:
    //  Tests that uninstalling a library twice is idempotent. The second
    //  uninstall should succeed even though the library is already gone.
    //
    TEST_F(CSharpExtensionApiTests, DoubleUninstallTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "testpackageA-ZIP.zip").string();
        ASSERT_TRUE(fs::exists(packagePath)) << "Test package not found: " << packagePath;

        string installDir = CreateInstallDir();

        // Install, then uninstall twice
        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "doubleuninstall", packagePath, installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        result = CallUninstall(sm_uninstallExternalLibraryFuncPtr,
            "doubleuninstall", installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        result = CallUninstall(sm_uninstallExternalLibraryFuncPtr,
            "doubleuninstall", installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: UninstallNonExistentLibraryTest
    //
    // Description:
    //  Tests that uninstalling a library that was never installed succeeds.
    //  The install directory may or may not exist; either way the operation
    //  should not fail.
    //
    TEST_F(CSharpExtensionApiTests, UninstallNonExistentLibraryTest)
    {
        string installDir = CreateInstallDir();

        SQLRETURN result = CallUninstall(sm_uninstallExternalLibraryFuncPtr,
            "neverinstalled", installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        CleanupInstallDir(installDir);
    }
}
