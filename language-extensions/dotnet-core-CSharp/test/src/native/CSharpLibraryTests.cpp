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
#include <fstream>
#include <vector>

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

    // Helper: call InstallExternalLibrary and capture the error message (if any).
    // Returns SQL result; populates errorMessage with the UTF-8 error text.
    //
    static SQLRETURN CallInstallCaptureError(
        FN_installExternalLibrary *installFunc,
        const string &libName,
        const string &libFilePath,
        const string &installDir,
        string &errorMessage)
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

        errorMessage.clear();
        if (libError != nullptr && libErrorLength > 0)
        {
            errorMessage.assign(reinterpret_cast<char *>(libError),
                static_cast<size_t>(libErrorLength));
        }

        return result;
    }

    // Helper: count GUID-shaped subdirectories in a directory (temp folders
    // created by the install code during ZIP extraction).
    //
    static int CountGuidTempDirs(const string &dir)
    {
        int count = 0;
        if (!fs::exists(dir))
        {
            return 0;
        }
        for (const auto &entry : fs::directory_iterator(dir))
        {
            if (!fs::is_directory(entry.path()))
            {
                continue;
            }
            string name = entry.path().filename().string();
            // GUID format: 8-4-4-4-12 = 36 chars with hyphens at 8,13,18,23
            if (name.length() == 36 &&
                name[8] == '-' && name[13] == '-' &&
                name[18] == '-' && name[23] == '-')
            {
                ++count;
            }
        }
        return count;
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
    //  Tests that installing a file with .zip extension that is not actually
    //  a valid ZIP (magic bytes are not PK) copies it to the install directory
    //  as a raw file named after the library and returns SQL_SUCCESS.
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

        EXPECT_TRUE(fs::exists(fs::path(installDir) / "bad-package.dll"))
            << "Raw file not found in install directory as bad-package.dll";

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
    //  Tests that installing a raw DLL file (not a zip) copies it to the
    //  install directory named after the library and returns SQL_SUCCESS.
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

        // The raw DLL should be copied using the library name with .dll extension.
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "rawdllpackage.dll"))
            << "Raw DLL not found in install directory as rawdllpackage.dll";

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
        EXPECT_EQ(dllCount, 50) << "Expected 50 extracted DLL files, found " << dllCount;

        // Verify the alias exists so DllUtils can discover the library by name.
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "manyfilespackage.dll"));

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

    //----------------------------------------------------------------------------------------------
    // Helper: read manifest file into a vector of relative paths
    //
    static vector<string> ReadManifest(const string &manifestPath)
    {
        vector<string> lines;
        ifstream f(manifestPath);
        string line;
        while (getline(f, line))
        {
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }
            if (!line.empty())
            {
                lines.push_back(line);
            }
        }
        return lines;
    }

    //----------------------------------------------------------------------------------------------
    // Name: ManifestWrittenTest
    //
    // Description:
    //  Verifies that after installing a ZIP package, a manifest file named
    //  "{libName}.manifest" is written in the install directory and lists
    //  every file extracted from the package.
    //
    TEST_F(CSharpExtensionApiTests, ManifestWrittenTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "testpackageB-DLL.zip").string();
        ASSERT_TRUE(fs::exists(packagePath));

        string installDir = CreateInstallDir();

        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "testpackageB", packagePath, installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        string manifestPath = (fs::path(installDir) / "testpackageB.manifest").string();
        ASSERT_TRUE(fs::exists(manifestPath)) << "Manifest file not created";

        vector<string> entries = ReadManifest(manifestPath);
        EXPECT_GE(entries.size(), 2u) << "Manifest should list at least 2 extracted files";

        // Manifest should contain the actual extracted file names
        bool hasDll = false;
        bool hasDeps = false;
        for (const auto &e : entries)
        {
            if (e.find("testpackageB.dll") != string::npos) hasDll = true;
            if (e.find("testpackageB.deps.json") != string::npos) hasDeps = true;
        }
        EXPECT_TRUE(hasDll) << "Manifest missing testpackageB.dll";
        EXPECT_TRUE(hasDeps) << "Manifest missing testpackageB.deps.json";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ManifestListsNestedFilesTest
    //
    // Description:
    //  Verifies that the manifest records nested file paths using the
    //  relative path form so that uninstall can locate the files.
    //
    TEST_F(CSharpExtensionApiTests, ManifestListsNestedFilesTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "testpackageD-NESTED.zip").string();
        ASSERT_TRUE(fs::exists(packagePath));

        string installDir = CreateInstallDir();

        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "nestedlib", packagePath, installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        string manifestPath = (fs::path(installDir) / "nestedlib.manifest").string();
        ASSERT_TRUE(fs::exists(manifestPath));

        vector<string> entries = ReadManifest(manifestPath);

        bool hasNestedDll = false;
        bool hasRuntimeDll = false;
        for (const auto &e : entries)
        {
            // Accept either separator for cross-platform resilience
            if (e.find("MyLib.dll") != string::npos &&
                (e.find("net8.0") != string::npos))
            {
                hasNestedDll = true;
            }
            if (e.find("native.dll") != string::npos &&
                e.find("win-x64") != string::npos)
            {
                hasRuntimeDll = true;
            }
        }
        EXPECT_TRUE(hasNestedDll) << "Manifest missing lib/net8.0/MyLib.dll entry";
        EXPECT_TRUE(hasRuntimeDll) << "Manifest missing runtimes/win-x64/native.dll entry";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallLibNameAliasTest
    //
    // Description:
    //  When the ZIP does not contain a file matching "{libName}.*", the
    //  install routine creates an alias named "{libName}.dll" so that
    //  DllUtils.CreateDllList (which searches "{libName}.*") can find it.
    //
    TEST_F(CSharpExtensionApiTests, InstallLibNameAliasTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "testpackageB-DLL.zip").string();
        ASSERT_TRUE(fs::exists(packagePath));

        string installDir = CreateInstallDir();

        // Library name "myAlias" does not match the package's testpackageB.*
        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "myAlias", packagePath, installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        EXPECT_TRUE(fs::exists(fs::path(installDir) / "myAlias.dll"))
            << "Expected alias file 'myAlias.dll' not found";

        // Manifest should include the alias.
        vector<string> entries = ReadManifest(
            (fs::path(installDir) / "myAlias.manifest").string());
        bool hasAlias = false;
        for (const auto &e : entries)
        {
            if (e == "myAlias.dll") { hasAlias = true; break; }
        }
        EXPECT_TRUE(hasAlias) << "Manifest missing alias entry 'myAlias.dll'";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: DirectoryOverlapAllowedTest
    //
    // Description:
    //  Installing two libraries into the same install directory succeeds
    //  as long as they do not share any filenames. Directory (folder)
    //  overlap is explicitly permitted.
    //
    TEST_F(CSharpExtensionApiTests, DirectoryOverlapAllowedTest)
    {
        string packagesPath = GetPackagesPath();
        string pkgA = (fs::path(packagesPath) / "testpackageA-ZIP.zip").string();
        string pkgB = (fs::path(packagesPath) / "testpackageB-DLL.zip").string();
        ASSERT_TRUE(fs::exists(pkgA));
        ASSERT_TRUE(fs::exists(pkgB));

        string installDir = CreateInstallDir();

        // Install lib1 from package A (contents: testpackageA.dll, testpackageA.txt)
        SQLRETURN r1 = CallInstall(sm_installExternalLibraryFuncPtr,
            "lib1", pkgA, installDir);
        EXPECT_EQ(r1, SQL_SUCCESS);

        // Install lib2 from package B (contents: testpackageB.dll, testpackageB.deps.json)
        // No filename conflict => succeeds
        SQLRETURN r2 = CallInstall(sm_installExternalLibraryFuncPtr,
            "lib2", pkgB, installDir);
        EXPECT_EQ(r2, SQL_SUCCESS);

        // Both libraries' files coexist
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageA.dll"));
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.dll"));
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "lib1.manifest"));
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "lib2.manifest"));

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: FileConflictFailsTest
    //
    // Description:
    //  Installing a second library that would overwrite a file already
    //  installed by another library must fail with SQL_ERROR. The first
    //  library's files must remain intact.
    //
    TEST_F(CSharpExtensionApiTests, FileConflictFailsTest)
    {
        string packagesPath = GetPackagesPath();
        string pkgB = (fs::path(packagesPath) / "testpackageB-DLL.zip").string();
        ASSERT_TRUE(fs::exists(pkgB));

        string installDir = CreateInstallDir();

        // Install "lib1" from package B
        SQLRETURN r1 = CallInstall(sm_installExternalLibraryFuncPtr,
            "lib1", pkgB, installDir);
        EXPECT_EQ(r1, SQL_SUCCESS);
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.dll"));

        // Install a DIFFERENT library "lib2" from the same package.
        // Both would write testpackageB.dll => conflict, must fail.
        SQLRETURN r2 = CallInstall(sm_installExternalLibraryFuncPtr,
            "lib2", pkgB, installDir);
        EXPECT_EQ(r2, SQL_ERROR) << "Expected conflict error on duplicate filename";

        // lib1's files must survive
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.dll"));
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "lib1.manifest"));
        // lib2 must not have written a manifest
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "lib2.manifest"));

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: UninstallPreservesOtherLibrariesTest
    //
    // Description:
    //  Uninstalling one library must only delete that library's files
    //  (as listed in its manifest). Other libraries' files in the same
    //  directory must remain untouched.
    //
    TEST_F(CSharpExtensionApiTests, UninstallPreservesOtherLibrariesTest)
    {
        string packagesPath = GetPackagesPath();
        string pkgA = (fs::path(packagesPath) / "testpackageA-ZIP.zip").string();
        string pkgB = (fs::path(packagesPath) / "testpackageB-DLL.zip").string();

        string installDir = CreateInstallDir();

        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "lib1", pkgA, installDir), SQL_SUCCESS);
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "lib2", pkgB, installDir), SQL_SUCCESS);

        // Uninstall lib1 only
        SQLRETURN r = CallUninstall(sm_uninstallExternalLibraryFuncPtr,
            "lib1", installDir);
        EXPECT_EQ(r, SQL_SUCCESS);

        // lib1's files + manifest gone
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "testpackageA.dll"));
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "testpackageA.txt"));
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "lib1.manifest"));

        // lib2's files + manifest intact
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.dll"));
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.deps.json"));
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "lib2.manifest"));

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: UninstallRemovesEmptyNestedDirsTest
    //
    // Description:
    //  After uninstalling a library whose files lived in nested
    //  subdirectories, the now-empty nested directories are removed
    //  bottom-up.
    //
    TEST_F(CSharpExtensionApiTests, UninstallRemovesEmptyNestedDirsTest)
    {
        string packagesPath = GetPackagesPath();
        string pkgD = (fs::path(packagesPath) / "testpackageD-NESTED.zip").string();
        ASSERT_TRUE(fs::exists(pkgD));

        string installDir = CreateInstallDir();

        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "nestedlib", pkgD, installDir), SQL_SUCCESS);
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "lib" / "net8.0"));
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "runtimes" / "win-x64"));

        // Uninstall
        SQLRETURN r = CallUninstall(sm_uninstallExternalLibraryFuncPtr,
            "nestedlib", installDir);
        EXPECT_EQ(r, SQL_SUCCESS);

        // Nested directories should have been removed when they became empty
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "lib" / "net8.0"));
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "lib"));
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "runtimes" / "win-x64"));
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "runtimes"));

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: AlterExternalLibraryTest
    //
    // Description:
    //  Installing a library a second time with the same libName (simulating
    //  ALTER EXTERNAL LIBRARY) removes the old content tracked by the
    //  previous manifest before extracting the new package, even when the
    //  new package would otherwise conflict with leftover files.
    //
    TEST_F(CSharpExtensionApiTests, AlterExternalLibraryTest)
    {
        string packagesPath = GetPackagesPath();
        string pkgA = (fs::path(packagesPath) / "testpackageA-ZIP.zip").string();
        string pkgB = (fs::path(packagesPath) / "testpackageB-DLL.zip").string();

        string installDir = CreateInstallDir();

        // v1: install as "myLib" from package A => testpackageA.dll, testpackageA.txt
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "myLib", pkgA, installDir), SQL_SUCCESS);
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "testpackageA.dll"));

        // v2: install again as "myLib" from package B (ALTER)
        SQLRETURN r = CallInstall(sm_installExternalLibraryFuncPtr,
            "myLib", pkgB, installDir);
        EXPECT_EQ(r, SQL_SUCCESS) << "ALTER-style reinstall should succeed";

        // v1's unique files gone
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "testpackageA.dll"));
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "testpackageA.txt"));

        // v2's files present
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.dll"));
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.deps.json"));

        // Manifest reflects v2 content only
        vector<string> entries = ReadManifest(
            (fs::path(installDir) / "myLib.manifest").string());
        bool hasA = false, hasB = false;
        for (const auto &e : entries)
        {
            if (e.find("testpackageA") != string::npos) hasA = true;
            if (e.find("testpackageB") != string::npos) hasB = true;
        }
        EXPECT_FALSE(hasA) << "Manifest still references v1 files";
        EXPECT_TRUE(hasB) << "Manifest missing v2 files";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ErrorMessagePopulatedOnFailureTest
    //
    // Description:
    //  SQL Server surfaces the libraryError string to end users. Every failure
    //  path must populate libError with a non-empty, UTF-8-decodable message.
    //  Validates this for three distinct failure modes: non-existent source file,
    //  zip-slip attack, and file-level conflict.
    //
    TEST_F(CSharpExtensionApiTests, ErrorMessagePopulatedOnFailureTest)
    {
        string packagesPath = GetPackagesPath();
        string installDir = CreateInstallDir();
        string msg;

        // Failure mode 1: non-existent source file
        SQLRETURN r = CallInstallCaptureError(sm_installExternalLibraryFuncPtr,
            "missing", "C:\\does\\not\\exist.zip", installDir, msg);
        EXPECT_EQ(r, SQL_ERROR);
        EXPECT_FALSE(msg.empty()) << "No error message populated for missing file";

        // Failure mode 2: zip-slip attack
        string zipSlip = (fs::path(packagesPath) / "testpackageH-ZIPSLIP.zip").string();
        r = CallInstallCaptureError(sm_installExternalLibraryFuncPtr,
            "slip", zipSlip, installDir, msg);
        EXPECT_EQ(r, SQL_ERROR);
        EXPECT_FALSE(msg.empty()) << "No error message populated for zip-slip";

        // Failure mode 3: file-level conflict
        string pkgB = (fs::path(packagesPath) / "testpackageB-DLL.zip").string();
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "libA", pkgB, installDir), SQL_SUCCESS);
        r = CallInstallCaptureError(sm_installExternalLibraryFuncPtr,
            "libB", pkgB, installDir, msg);
        EXPECT_EQ(r, SQL_ERROR);
        EXPECT_FALSE(msg.empty()) << "No error message populated for conflict";
        // Message should mention the conflicting library name for diagnosability
        EXPECT_NE(msg.find("libB"), string::npos)
            << "Conflict message should include library name. Got: " << msg;

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: UninstallNonZipLibraryTest
    //
    // Description:
    //  When a raw DLL (non-ZIP) is installed, no manifest is written. Uninstall
    //  must still remove the library file by its libName.dll path.
    //  Exercises the File.Delete(libraryFile) branch in UninstallExternalLibrary.
    //
    TEST_F(CSharpExtensionApiTests, UninstallNonZipLibraryTest)
    {
        string packagesPath = GetPackagesPath();
        string rawDll = (fs::path(packagesPath) / "testpackageE-RAWDLL.dll").string();
        ASSERT_TRUE(fs::exists(rawDll));

        string installDir = CreateInstallDir();

        // Install the raw DLL as "rawlib" — no manifest should be written.
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "rawlib", rawDll, installDir), SQL_SUCCESS);
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "rawlib.dll"));
        ASSERT_FALSE(fs::exists(fs::path(installDir) / "rawlib.manifest"))
            << "Raw-DLL install should not write a manifest";

        // Uninstall must still delete the libName.dll file.
        SQLRETURN r = CallUninstall(sm_uninstallExternalLibraryFuncPtr,
            "rawlib", installDir);
        EXPECT_EQ(r, SQL_SUCCESS);
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "rawlib.dll"))
            << "Raw library file not removed by uninstall";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InnerZipFileConflictFailsTest
    //
    // Description:
    //  FileConflictFailsTest exercises the direct-files code path (package B,
    //  no inner zip). This test exercises the inner-zip code path (package A)
    //  which has its own separate conflict-detection loop. Regressing only
    //  one path must be caught.
    //
    TEST_F(CSharpExtensionApiTests, InnerZipFileConflictFailsTest)
    {
        string packagesPath = GetPackagesPath();
        string pkgA = (fs::path(packagesPath) / "testpackageA-ZIP.zip").string();
        ASSERT_TRUE(fs::exists(pkgA));

        string installDir = CreateInstallDir();

        // Install "lib1" from package A (inner-zip path) => testpackageA.dll + .txt
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "lib1", pkgA, installDir), SQL_SUCCESS);
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "testpackageA.dll"));
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "lib1.manifest"));

        // Install "lib2" from the same package — inner-zip conflict must fail
        SQLRETURN r = CallInstall(sm_installExternalLibraryFuncPtr,
            "lib2", pkgA, installDir);
        EXPECT_EQ(r, SQL_ERROR) << "Inner-zip path must detect filename conflict";

        // lib1's state must be untouched
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageA.dll"));
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "lib1.manifest"));
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "lib2.manifest"));

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: TempFolderCleanedUpAfterConflictTest
    //
    // Description:
    //  After a failed install (conflict or otherwise), the GUID-named temp
    //  folder used for outer-zip extraction must be cleaned up by the finally
    //  block. Regression would slowly leak disk space on every failed install.
    //
    TEST_F(CSharpExtensionApiTests, TempFolderCleanedUpAfterConflictTest)
    {
        string packagesPath = GetPackagesPath();
        string pkgA = (fs::path(packagesPath) / "testpackageA-ZIP.zip").string();
        string pkgB = (fs::path(packagesPath) / "testpackageB-DLL.zip").string();

        string installDir = CreateInstallDir();

        // Trigger a conflict: install then reinstall same package under different names
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "lib1", pkgB, installDir), SQL_SUCCESS);
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "lib2", pkgB, installDir), SQL_ERROR);

        // Also trigger inner-zip path conflict
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "libA", pkgA, installDir), SQL_SUCCESS);
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "libB", pkgA, installDir), SQL_ERROR);

        // Also trigger zip-slip failure
        string zipSlip = (fs::path(packagesPath) / "testpackageH-ZIPSLIP.zip").string();
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "slip", zipSlip, installDir), SQL_ERROR);

        EXPECT_EQ(CountGuidTempDirs(installDir), 0)
            << "GUID-named temp folder leaked after failed install(s)";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: AlterFromNonZipToZipTest
    //
    // Description:
    //  ALTER EXTERNAL LIBRARY scenario where v1 was a raw DLL (no manifest)
    //  and v2 is a ZIP package. Install code must handle the missing-manifest
    //  case gracefully and complete the new install. The old lib file remains
    //  until explicit uninstall — that matches sicongliu's baseline semantics.
    //
    TEST_F(CSharpExtensionApiTests, AlterFromNonZipToZipTest)
    {
        string packagesPath = GetPackagesPath();
        string rawDll = (fs::path(packagesPath) / "testpackageE-RAWDLL.dll").string();
        string pkgB = (fs::path(packagesPath) / "testpackageB-DLL.zip").string();

        string installDir = CreateInstallDir();

        // v1: raw DLL install (no manifest created).
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "myLib", rawDll, installDir), SQL_SUCCESS);
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "myLib.dll"));
        ASSERT_FALSE(fs::exists(fs::path(installDir) / "myLib.manifest"));

        // v2: ZIP install of the same libName — must NOT crash on missing manifest.
        SQLRETURN r = CallInstall(sm_installExternalLibraryFuncPtr,
            "myLib", pkgB, installDir);
        EXPECT_EQ(r, SQL_SUCCESS)
            << "ALTER from non-ZIP to ZIP should succeed even without prior manifest";

        // v2's files must be present + v2 manifest must exist.
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.dll"));
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "myLib.manifest"));

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: AliasFileRemovedOnUninstallTest
    //
    // Description:
    //  When a ZIP contains DLLs whose names don't match the library name,
    //  the install code creates an alias file named {libName}.dll so
    //  DllUtils.CreateDllList can discover it. This alias must be recorded
    //  in the manifest and removed on uninstall — otherwise orphaned alias
    //  files accumulate over install/uninstall cycles.
    //
    TEST_F(CSharpExtensionApiTests, AliasFileRemovedOnUninstallTest)
    {
        string packagesPath = GetPackagesPath();
        string pkgA = (fs::path(packagesPath) / "testpackageA-ZIP.zip").string();

        string installDir = CreateInstallDir();

        // Install package A (contains testpackageA.dll) under a different libName.
        // Since no file matches "aliaslib.*", the install code must create an
        // "aliaslib.dll" alias file copied from the first DLL.
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "aliaslib", pkgA, installDir), SQL_SUCCESS);
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "aliaslib.dll"))
            << "Alias file not created";

        // The alias must also be listed in the manifest.
        vector<string> entries = ReadManifest(
            (fs::path(installDir) / "aliaslib.manifest").string());
        bool hasAlias = false;
        for (const auto &e : entries)
        {
            if (e == "aliaslib.dll")
            {
                hasAlias = true;
                break;
            }
        }
        EXPECT_TRUE(hasAlias) << "Alias file not recorded in manifest";

        // Uninstall must remove both the content and the alias.
        ASSERT_EQ(CallUninstall(sm_uninstallExternalLibraryFuncPtr,
            "aliaslib", installDir), SQL_SUCCESS);
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "aliaslib.dll"))
            << "Alias file leaked after uninstall";
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "testpackageA.dll"))
            << "Content file leaked after uninstall";
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "aliaslib.manifest"))
            << "Manifest file leaked after uninstall";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallRawDllWithDllSuffixedLibNameTest
    //
    // Description:
    //  Regression test for the double-".dll" bug. When a caller invokes
    //  CREATE EXTERNAL LIBRARY [foo.dll] (i.e. the registered library name
    //  already ends in ".dll") with raw-DLL CONTENT, the install code must
    //  write the file as "foo.dll" — NOT "foo.dll.dll". The CLR assembly
    //  resolver looks up assemblies by simple name and will not find
    //  "foo.dll.dll" when asked to load "foo".
    //
    //  Symmetric behavior is required on uninstall: the file removed must
    //  be "foo.dll", not "foo.dll.dll".
    //
    TEST_F(CSharpExtensionApiTests, InstallRawDllWithDllSuffixedLibNameTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "testpackageE-RAWDLL.dll").string();
        ASSERT_TRUE(fs::exists(packagePath)) << "Test package not found: " << packagePath;

        string installDir = CreateInstallDir();

        // libName already ends in ".dll" — must NOT get a second ".dll" appended.
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "rawdllpackage.dll", packagePath, installDir), SQL_SUCCESS);

        EXPECT_TRUE(fs::exists(fs::path(installDir) / "rawdllpackage.dll"))
            << "Raw DLL not found at expected single-.dll path";
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "rawdllpackage.dll.dll"))
            << "Raw DLL incorrectly written with double-.dll extension; "
               "CLR assembly resolver would fail to locate it.";

        // Uninstall must also use the single-.dll path.
        ASSERT_EQ(CallUninstall(sm_uninstallExternalLibraryFuncPtr,
            "rawdllpackage.dll", installDir), SQL_SUCCESS);
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "rawdllpackage.dll"))
            << "Raw DLL not removed by uninstall";

        CleanupInstallDir(installDir);
    }
}
