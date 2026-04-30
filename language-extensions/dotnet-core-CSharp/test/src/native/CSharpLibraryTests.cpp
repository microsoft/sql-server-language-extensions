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
#include <filesystem>
#include <fstream>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

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
    static bool DoesDirectoryHaveFiles(const string &dir)
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

        // Assert on the specific files we expect from the inner-zip
        // contents rather than just "directory is non-empty" -- the weaker
        // form passes even if the install extracts the wrong package or
        // leaves stale files behind from a prior test run.
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageA.dll"))
            << "Expected testpackageA.dll not extracted from inner zip";
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageA.txt"))
            << "Expected testpackageA.txt not extracted from inner zip";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InnerZipFutureSymlinkRejectedTest
    //
    // Description:
    //  Regression for PR #85 / option α. The inner-zip install path must
    //  route through tempFolder + IsReparsePoint-guarded CopyDirectory --
    //  NOT a direct ZipFile.ExtractToDirectory(innerZip, installDir) call.
    //  testpackageK-SYMLINK.zip is an outer zip whose inner zip contains:
    //
    //    legitfile.dll       regular file
    //    evil-symlink.dll    Unix mode 0o120755 (symbolic link, target
    //                        "/etc/passwd")
    //
    //  Today's .NET ZipFile.ExtractToDirectory ignores Unix mode bits on
    //  every platform: it writes evil-symlink.dll as a regular file
    //  containing the literal text "/etc/passwd". A future .NET runtime
    //  (or a switch to a different extraction library) might honor those
    //  bits and create a real symlink in the staged tree -- at which point
    //  a direct extract-to-installDir would silently land an attacker-
    //  controlled symlink in the live library directory.
    //
    //  This test asserts the future-proofing invariant: regardless of how
    //  the runtime materializes the symlink-mode entry, installDir must
    //  contain NO reparse points after install. If the runtime starts
    //  honoring the bits, the IsReparsePoint guard in CopyDirectory must
    //  skip evil-symlink.dll; if the runtime keeps writing it as a regular
    //  file, the test still passes (no reparse points exist). The test
    //  fails only if BOTH (a) the runtime honors the bits AND (b) the
    //  IsReparsePoint guard regresses.
    //
    //  Also asserts that install completes successfully and that the
    //  legitimate file lands in installDir, so a regression that simply
    //  fails the install on encountering a symlink-mode entry is also
    //  caught.
    //
    TEST_F(CSharpExtensionApiTests, InnerZipFutureSymlinkRejectedTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "testpackageK-SYMLINK.zip").string();
        ASSERT_TRUE(fs::exists(packagePath))
            << "Symlink fixture missing -- regenerate with build-symlink-fixture.ps1";

        string installDir = CreateInstallDir();

        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "symlinklib", packagePath, installDir);
        ASSERT_EQ(result, SQL_SUCCESS)
            << "Install must succeed -- symlink-mode entries are skipped, not fatal";

        // The legitimate file must be present (proves install actually ran).
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "legitfile.dll"))
            << "Expected legitfile.dll missing -- inner-zip path may not have run";

        // The future-proofing invariant: nothing in installDir is a reparse
        // point. If a future runtime materializes evil-symlink.dll as a
        // real symlink in tempFolder, the IsReparsePoint guard in
        // CopyDirectory must skip it so installDir stays clean.
        std::error_code ec;
        for (const auto &entry : fs::recursive_directory_iterator(installDir, ec))
        {
            // is_symlink follows std::filesystem semantics: returns true
            // for both file and directory symlinks on Linux, and for
            // SYMLINK / SYMLINKD reparse points on Windows.
            EXPECT_FALSE(fs::is_symlink(entry.symlink_status()))
                << "installDir contains a symlink (regression): " << entry.path();
        }

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallZipContainingDllsTest
    //
    // Description:
    //  Tests installing an outer zip that contains DLLs directly (no inner zip).
    //  Verifies that the specific expected DLLs are extracted to the install
    //  directory.
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

        // Assert on the specific files we expect from package B's contents.
        // "any DLL exists" is too weak: it would pass even if the install
        // extracted the wrong package or a stale file from a prior test
        // survived.
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.dll"))
            << "Expected testpackageB.dll not extracted";
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.deps.json"))
            << "Expected testpackageB.deps.json not extracted";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallZipExtensionWithBadContentFailsLoudlyTest
    //
    // Description:
    //  Tests that installing a file with the .zip extension whose bytes
    //  are NOT a valid ZIP archive returns SQL_ERROR. Dispatch is by
    //  extension (not by content sniff), so a file the user named
    //  "bad-package-ZIP.zip" must be treated as a ZIP -- and if its bytes
    //  are not a real archive, ZipFile.ExtractToDirectory throws and the
    //  install must fail loudly. We must NOT silently rewrite the user's
    //  upload into a "{libName}.dll" raw install (which the previous
    //  content-sniff implementation did).
    //
    TEST_F(CSharpExtensionApiTests, InstallZipExtensionWithBadContentFailsLoudlyTest)
    {
        string packagesPath = GetPackagesPath();
        string packagePath = (fs::path(packagesPath) / "bad-package-ZIP.zip").string();
        ASSERT_TRUE(fs::exists(packagePath)) << "Test package not found: " << packagePath;

        string installDir = CreateInstallDir();

        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "bad-package", packagePath, installDir);
        EXPECT_EQ(result, SQL_ERROR);

        // The user's file must NOT have been silently installed under a
        // ".dll" rename. Pre-fix behavior was to copy bad-package-ZIP.zip
        // to "bad-package.dll" -- assert that did not happen.
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "bad-package.dll"))
            << "Bad ZIP should not be silently rewritten as bad-package.dll";

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
        EXPECT_TRUE(DoesDirectoryHaveFiles(installDir)) << "No files found after installation";

        // Uninstall
        result = CallUninstall(sm_uninstallExternalLibraryFuncPtr,
            "testpackageB", installDir);
        EXPECT_EQ(result, SQL_SUCCESS);

        // Verify directory is empty
        EXPECT_FALSE(DoesDirectoryHaveFiles(installDir)) << "Files still present after uninstall";

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

        // Verify v1's unique files were removed by the manifest cleanup,
        // and v2's expected file is present. The earlier "any .dll exists"
        // loop is redundant given these explicit checks.
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "testpackageA.dll"))
            << "Stale testpackageA.dll left behind after reinstall";
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "testpackageA.txt"))
            << "Stale testpackageA.txt left behind after reinstall";
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.dll"))
            << "Expected testpackageB.dll not present after reinstall";

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
    // Name: AlterToEmptyDirsZipPreservesV1Test
    //
    // Description:
    //  Regression test for the silent-data-loss scenario on ALTER. A ZIP
    //  containing only directory entries (no file entries) used to slip
    //  past the empty-archive guard because Directory.GetDirectories on
    //  the extracted tempFolder is non-empty. CollectRelativeFiles then
    //  returns 0 entries; CleanupManifest deletes the previous version's
    //  content; nothing is copied; and the manifest write is skipped
    //  because it is gated on extractedFiles.Count > 0. The library would
    //  end up GONE with no replacement and no manifest.
    //
    //  After the fix, install fails with SQL_ERROR before any cleanup
    //  runs, so v1 must remain byte-for-byte intact.
    //
    TEST_F(CSharpExtensionApiTests, AlterToEmptyDirsZipPreservesV1Test)
    {
        string packagesPath = GetPackagesPath();
        string pkgB = (fs::path(packagesPath) / "testpackageB-DLL.zip").string();
        string emptyDirsZip = (fs::path(packagesPath) / "testpackageI-EMPTYDIRS.zip").string();
        ASSERT_TRUE(fs::exists(pkgB));
        ASSERT_TRUE(fs::exists(emptyDirsZip));

        string installDir = CreateInstallDir();

        // v1: install a real ZIP package as "myLib".
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "myLib", pkgB, installDir), SQL_SUCCESS);
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.dll"));
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.deps.json"));
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "myLib.manifest"));

        // v2: attempt ALTER with a ZIP whose only entries are empty
        // directories. Must FAIL -- and v1 must survive intact.
        SQLRETURN r = CallInstall(sm_installExternalLibraryFuncPtr,
            "myLib", emptyDirsZip, installDir);
        EXPECT_EQ(r, SQL_ERROR)
            << "ALTER to empty-dirs ZIP must fail rather than silently delete v1";

        // v1's content must be untouched.
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.dll"))
            << "v1's testpackageB.dll was deleted by failed ALTER";
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.deps.json"))
            << "v1's testpackageB.deps.json was deleted by failed ALTER";
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "myLib.manifest"))
            << "v1's manifest was deleted by failed ALTER";

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
    //  Tests that installing a ZIP containing many files extracts all of
    //  them correctly AND that the alias is created on top.
    //
    //  Package contents: testpackageG-MANYFILES.zip contains exactly 50
    //  DLLs (Module1.dll .. Module50.dll) and no other files. None of
    //  them matches "manyfilespackage.*", so install must clone the first
    //  DLL as an alias named "manyfilespackage.dll". The DLL count in
    //  installDir is therefore 50 (extracted) + 1 (alias) = 51.
    //
    //  Historical note: a pre-PR version of this test asserted
    //  EXPECT_EQ(dllCount, 50) and EXPECT_TRUE(fs::exists(... /
    //  "manyfilespackage")) (alias with no .dll extension). It passed
    //  legitimately at the time because the install code created the
    //  alias as "{libName}" with no extension -- so dllCount stayed at 50
    //  and the no-extension EXPECT_TRUE matched. Test and code agreed,
    //  but the alias was un-loadable as a DLL on Windows. The test
    //  caught no regression in alias naming because its assertions were
    //  written to match the buggy behavior. Current assertions
    //  (51 + .dll extension + per-module name check below) pin the
    //  correct shape so a future regression toward "alias with wrong
    //  extension" or "extracted file silently dropped" is caught.
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

        // Count DLL files in installDir. 50 from the package + 1 alias = 51.
        int dllCount = 0;
        for (const fs::directory_entry &entry : fs::directory_iterator(installDir))
        {
            if (entry.path().extension() == ".dll")
            {
                dllCount++;
            }
        }
        EXPECT_EQ(dllCount, 51)
            << "Expected 51 DLL files (50 extracted + 1 alias), found " << dllCount;

        // Per-module existence check. Catches a "silently dropped one
        // extracted file but added another spurious .dll so the count
        // still totals 51" regression that the bare count above would miss.
        for (int i = 1; i <= 50; ++i)
        {
            string moduleName = "Module" + std::to_string(i) + ".dll";
            EXPECT_TRUE(fs::exists(fs::path(installDir) / moduleName))
                << "Expected extracted module missing: " << moduleName;
        }

        // Verify the alias exists with the .dll extension so DllUtils can
        // discover the library by name. The .dll extension is critical:
        // an alias without it would still be findable by DllUtils's
        // "{libName}.*" wildcard but would be un-loadable as a DLL on
        // Windows, which is exactly the bug the historical test missed.
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

        // Manifest entries are EXACT relative paths -- assert equality, not
        // substring containment. Substring match would accept "x.dll",
        // "testpackageB.dll.bak", etc. and miss real bugs.
        bool hasDll = false;
        bool hasDeps = false;
        for (const string &e : entries)
        {
            if (e == "testpackageB.dll")       { hasDll  = true; }
            if (e == "testpackageB.deps.json") { hasDeps = true; }
        }
        EXPECT_TRUE(hasDll)  << "Manifest missing exact entry 'testpackageB.dll'";
        EXPECT_TRUE(hasDeps) << "Manifest missing exact entry 'testpackageB.deps.json'";

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

        fs::path aliasFile = fs::path(installDir) / "myAlias.dll";
        fs::path sourceDll = fs::path(installDir) / "testpackageB.dll";
        ASSERT_TRUE(fs::exists(aliasFile))
            << "Expected alias file 'myAlias.dll' not found";
        ASSERT_TRUE(fs::exists(sourceDll))
            << "Expected source file 'testpackageB.dll' not extracted";

        // Alias must be a byte-for-byte copy of the source DLL (the first
        // .dll discovered in the package). A zero-length alias, a copy of
        // the wrong file, or a partial write would all silently pass the
        // "file exists" check above without this content check.
        EXPECT_EQ(fs::file_size(aliasFile), fs::file_size(sourceDll))
            << "Alias file size differs from source DLL";

        std::ifstream aliasStream(aliasFile.string(), std::ios::binary);
        std::ifstream sourceStream(sourceDll.string(), std::ios::binary);
        std::string aliasContents(
            (std::istreambuf_iterator<char>(aliasStream)),
             std::istreambuf_iterator<char>());
        std::string sourceContents(
            (std::istreambuf_iterator<char>(sourceStream)),
             std::istreambuf_iterator<char>());
        // Explicit close (rather than relying on RAII at end-of-scope) so
        // the file handles are released before the EXPECT_EQ comparison
        // -- gtest macros that fail can run arbitrary code in the
        // diagnostic path, and the comparison itself is cheaper to debug
        // when the streams are known-closed. Same pattern is used at
        // every read/write site below.
        aliasStream.close();
        sourceStream.close();
        EXPECT_EQ(aliasContents, sourceContents)
            << "Alias file contents differ from source DLL";

        // Manifest should include the alias.
        vector<string> entries = ReadManifest(
            (fs::path(installDir) / "myAlias.manifest").string());
        bool hasAlias = false;
        for (const string &e : entries)
        {
            if (e == "myAlias.dll")
            {
                hasAlias = true;
                break;
            }
        }
        EXPECT_TRUE(hasAlias) << "Manifest missing alias entry 'myAlias.dll'";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: NonConflictingFlatFilesCoexistTest
    //
    // Description:
    //  Installing two libraries into the same install directory succeeds
    //  as long as they do not share any filenames. Both packages used here
    //  are flat (no nested directories), so the test exercises the
    //  flat-file coexistence case only -- nested-directory overlap is
    //  covered separately by ManifestListsNestedFilesTest +
    //  InnerZipFileConflictFailsTest.
    //
    TEST_F(CSharpExtensionApiTests, NonConflictingFlatFilesCoexistTest)
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
        for (const string &e : entries)
        {
            if (e.find("testpackageA") != string::npos)
            {
                hasA = true;
            }

            if (e.find("testpackageB") != string::npos)
            {
                hasB = true;
            }
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
        string missingPath = "C:\\does\\not\\exist.zip";
        SQLRETURN r = CallInstallCaptureError(sm_installExternalLibraryFuncPtr,
            "missing", missingPath, installDir, msg);
        EXPECT_EQ(r, SQL_ERROR);
        EXPECT_FALSE(msg.empty()) << "No error message populated for missing file";
        // Message should mention the path that was not found, so the user
        // can fix the input rather than guess.
        EXPECT_NE(msg.find("exist.zip"), string::npos)
            << "Missing-file message should reference the path. Got: " << msg;

        // Failure mode 2: zip-slip attack
        string zipSlip = (fs::path(packagesPath) / "testpackageH-ZIPSLIP.zip").string();
        r = CallInstallCaptureError(sm_installExternalLibraryFuncPtr,
            "slip", zipSlip, installDir, msg);
        EXPECT_EQ(r, SQL_ERROR);
        EXPECT_FALSE(msg.empty()) << "No error message populated for zip-slip";
        // Message should describe the rejection. The exception comes from
        // .NET's ZipFile.ExtractToDirectory built-in zip-slip guard, which
        // throws with "outside the specified destination directory". Our
        // own ValidateRelativePath defense-in-depth check ("invalid path")
        // is never reached because the .NET extractor catches it first.
        // Either substring is acceptable -- assert the .NET form.
        EXPECT_NE(msg.find("outside"), string::npos)
            << "Zip-slip message should describe the rejection. Got: " << msg;

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
    //  Raw-DLL (non-ZIP) installs write a single-entry manifest tracking
    //  "{libName}.dll" so that ALTER from raw->ZIP can clean up the prior
    //  install. Uninstall must remove both the file and its manifest.
    //
    TEST_F(CSharpExtensionApiTests, UninstallNonZipLibraryTest)
    {
        string packagesPath = GetPackagesPath();
        string rawDll = (fs::path(packagesPath) / "testpackageE-RAWDLL.dll").string();
        ASSERT_TRUE(fs::exists(rawDll));

        string installDir = CreateInstallDir();

        // Install the raw DLL as "rawlib" — manifest must be written so that
        // ALTER scenarios can track ownership of the {libName}.dll file.
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "rawlib", rawDll, installDir), SQL_SUCCESS);
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "rawlib.dll"));
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "rawlib.manifest"))
            << "Raw-DLL install should write a single-entry manifest";

        // Uninstall must delete both the file and the manifest.
        SQLRETURN r = CallUninstall(sm_uninstallExternalLibraryFuncPtr,
            "rawlib", installDir);
        EXPECT_EQ(r, SQL_SUCCESS);
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "rawlib.dll"))
            << "Raw library file not removed by uninstall";
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "rawlib.manifest"))
            << "Raw-DLL manifest file not removed by uninstall";

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
    //  ALTER EXTERNAL LIBRARY scenario where v1 was a raw DLL and v2 is a
    //  ZIP package. Raw-DLL installs now write a manifest with one entry
    //  ("{libName}.dll"), so the v2 ZIP install can detect and clean up the
    //  v1 file before extracting. Without that manifest tracking, the ZIP
    //  install would either silently overwrite v1's DLL or trip the alias
    //  conflict check on the pre-existing "{libName}.dll".
    //
    TEST_F(CSharpExtensionApiTests, AlterFromNonZipToZipTest)
    {
        string packagesPath = GetPackagesPath();
        string rawDll = (fs::path(packagesPath) / "testpackageE-RAWDLL.dll").string();
        string pkgB = (fs::path(packagesPath) / "testpackageB-DLL.zip").string();

        string installDir = CreateInstallDir();

        // v1: raw DLL install. The new behavior writes a manifest tracking
        // the single "{libName}.dll" entry, so ALTER can clean it up below.
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "myLib", rawDll, installDir), SQL_SUCCESS);
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "myLib.dll"));
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "myLib.manifest"))
            << "Raw-DLL install should write a manifest so ALTER can track it";

        // v2: ZIP install of the same libName. The pre-existing myLib.dll
        // must NOT trip the alias conflict check -- it is tracked in v1's
        // manifest as owned-by-previous and gets cleaned up first.
        SQLRETURN r = CallInstall(sm_installExternalLibraryFuncPtr,
            "myLib", pkgB, installDir);
        EXPECT_EQ(r, SQL_SUCCESS)
            << "ALTER from non-ZIP to ZIP should succeed";

        // v2's content must be present + manifest exists.
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.dll"));
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.deps.json"));
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "myLib.manifest"));

        // v1's raw DLL bytes must be GONE. Both the file and the alias for
        // v2 are named myLib.dll, so a broken cleanup (overwrite-instead-of-
        // clean-then-install) would be masked by mere existence of the file.
        // Compare bytes against testpackageB.dll: if the alias was created
        // correctly from v2's content, it must equal the source DLL.
        fs::path myLibDll = fs::path(installDir) / "myLib.dll";
        fs::path sourceDll = fs::path(installDir) / "testpackageB.dll";
        ASSERT_TRUE(fs::exists(myLibDll))
            << "v2's alias myLib.dll missing";
        EXPECT_EQ(fs::file_size(myLibDll), fs::file_size(sourceDll))
            << "myLib.dll size differs from v2's source -- likely still v1's bytes";

        std::ifstream myLibStream(myLibDll.string(), std::ios::binary);
        std::ifstream sourceStream(sourceDll.string(), std::ios::binary);
        std::string myLibBytes(
            (std::istreambuf_iterator<char>(myLibStream)),
             std::istreambuf_iterator<char>());
        std::string sourceBytes(
            (std::istreambuf_iterator<char>(sourceStream)),
             std::istreambuf_iterator<char>());
        myLibStream.close();
        sourceStream.close();
        EXPECT_EQ(myLibBytes, sourceBytes)
            << "myLib.dll content differs from v2's source -- v1's raw DLL was not cleaned up";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: AlterFromZipToNonZipTest
    //
    // Description:
    //  ALTER EXTERNAL LIBRARY scenario where v1 was a ZIP package and v2 is
    //  a raw DLL with the same libName. The v1 manifest must be cleaned up
    //  before v2 writes "{libName}.dll", and v2 must end up with its own
    //  one-entry manifest. This is the inverse of AlterFromNonZipToZipTest.
    //
    TEST_F(CSharpExtensionApiTests, AlterFromZipToNonZipTest)
    {
        string packagesPath = GetPackagesPath();
        string pkgB = (fs::path(packagesPath) / "testpackageB-DLL.zip").string();
        string rawDll = (fs::path(packagesPath) / "testpackageE-RAWDLL.dll").string();

        string installDir = CreateInstallDir();

        // v1: install ZIP package B as "myLib". This drops testpackageB.dll,
        // testpackageB.deps.json, an alias myLib.dll, and a myLib.manifest.
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "myLib", pkgB, installDir), SQL_SUCCESS);
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "testpackageB.dll"));
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "myLib.manifest"));

        // v2: raw-DLL install of the same libName.
        SQLRETURN r = CallInstall(sm_installExternalLibraryFuncPtr,
            "myLib", rawDll, installDir);
        EXPECT_EQ(r, SQL_SUCCESS)
            << "ALTER from ZIP to non-ZIP should succeed";

        // v1's ZIP-only files must be gone (cleaned up via v1's manifest
        // before v2 was written).
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "testpackageB.deps.json"))
            << "Stale v1 deps.json left behind after ALTER zip->raw";

        // v2's file must be present and its manifest must list exactly that file.
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "myLib.dll"));
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "myLib.manifest"));
        vector<string> entries = ReadManifest(
            (fs::path(installDir) / "myLib.manifest").string());
        EXPECT_EQ(entries.size(), 1u) << "Raw-DLL manifest should list exactly one entry";
        if (!entries.empty())
        {
            EXPECT_EQ(entries[0], "myLib.dll");
        }

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

    //----------------------------------------------------------------------------------------------
    // Name: UninstallRejectsPathTraversalLibNameTest
    //
    // Description:
    //  UninstallExternalLibrary must reject libNames that contain path
    //  separators or traversal segments before using them to build
    //  manifestPath / libraryFile via Path.Combine. Without validation,
    //  a malicious libName like "../foo" would resolve outside installDir
    //  and allow unintended file reads/deletes.
    //
    TEST_F(CSharpExtensionApiTests, UninstallRejectsPathTraversalLibNameTest)
    {
        string installDir = CreateInstallDir();

        // Create a sentinel file OUTSIDE installDir that uninstall must not touch.
        fs::path sentinelDir = fs::path(installDir).parent_path();
        fs::path sentinel = sentinelDir / "do-not-delete.manifest";
        std::ofstream sentinelStream(sentinel.string());
        sentinelStream << "sentinel";
        // Close before fs::exists -- the assertion only checks the dir
        // entry, but planting the file via an unflushed stream and then
        // exercising uninstall would race against the OS commit. Keep
        // the explicit close-before-assert pattern at every write site
        // for consistency with the byte-comparison sites further down.
        sentinelStream.close();
        ASSERT_TRUE(fs::exists(sentinel));

        // Attempt uninstall with a traversal libName.
        SQLRETURN result = CallUninstall(sm_uninstallExternalLibraryFuncPtr,
            "../do-not-delete", installDir);
        EXPECT_EQ(result, SQL_ERROR) << "Uninstall must reject libName with traversal";
        EXPECT_TRUE(fs::exists(sentinel)) << "Sentinel file outside installDir was deleted";

        fs::remove(sentinel);
        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallRejectsExtensionOnlyLibNameTest
    //
    // Description:
    //  ValidateLibraryName must reject libNames that are only an extension
    //  (e.g. ".dll"). Without this check, DllFileNameFor(".dll") returns
    //  ".dll", producing hidden dotfiles like "{installDir}/.dll" and
    //  "{installDir}/.dll.manifest" that are opaque on Windows and hidden
    //  on Linux.
    //
    TEST_F(CSharpExtensionApiTests, InstallRejectsExtensionOnlyLibNameTest)
    {
        string packagesPath = GetPackagesPath();
        string rawDll = (fs::path(packagesPath) / "testpackageE-RAWDLL.dll").string();
        ASSERT_TRUE(fs::exists(rawDll));

        string installDir = CreateInstallDir();

        // libName is just an extension -- no stem. Must be rejected before
        // any filesystem operation.
        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            ".dll", rawDll, installDir);
        EXPECT_EQ(result, SQL_ERROR) << "Install must reject extension-only libName";

        // No file should have been created at "installDir/.dll" or its manifest.
        EXPECT_FALSE(fs::exists(fs::path(installDir) / ".dll"))
            << "Hidden dotfile created from extension-only libName";
        EXPECT_FALSE(fs::exists(fs::path(installDir) / ".dll.manifest"))
            << "Hidden dotfile manifest created from extension-only libName";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: AliasConflictDetectedBeforeExtractionTest
    //
    // Description:
    //  If library A has already installed a file "shared.dll" at the root of
    //  installDir, and library B (whose package contains DLLs but none named
    //  "shared.*") is then installed with libName "shared", the install code
    //  would normally create a "shared.dll" alias. That alias now collides
    //  with A's file. Install must fail during the conflict-check phase
    //  BEFORE any of B's content is written into installDir (no partial
    //  state left behind).
    //
    TEST_F(CSharpExtensionApiTests, AliasConflictDetectedBeforeExtractionTest)
    {
        string packagesPath = GetPackagesPath();
        string pkgA = (fs::path(packagesPath) / "testpackageA-ZIP.zip").string();

        string installDir = CreateInstallDir();

        // Install library A normally.
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "libA", pkgA, installDir), SQL_SUCCESS);

        // Plant a "shared.dll" file in installDir (simulates ownership by another library).
        fs::path squatter = fs::path(installDir) / "shared.dll";
        std::ofstream squatterStream(squatter.string());
        squatterStream << "squatter";
        squatterStream.close();
        ASSERT_TRUE(fs::exists(squatter));

        // Count files currently in installDir; install of B must not add any.
        size_t fileCountBefore = 0;
        for (const fs::directory_entry &p : fs::recursive_directory_iterator(installDir))
        {
            if (fs::is_regular_file(p))
            {
                ++fileCountBefore;
            }
        }

        // Install B with libName "shared" - it has no shared.* file, so install
        // would create a "shared.dll" alias, which collides with squatter.
        SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
            "shared", pkgA, installDir);
        EXPECT_EQ(result, SQL_ERROR) << "Alias conflict must be detected and install must fail";

        // No B content should have been written.
        size_t fileCountAfter = 0;
        for (const fs::directory_entry &p : fs::recursive_directory_iterator(installDir))
        {
            if (fs::is_regular_file(p))
            {
                ++fileCountAfter;
            }
        }
        EXPECT_EQ(fileCountBefore, fileCountAfter)
            << "Failed install left partial state in installDir";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: RawDllInstallFailsIfForeignFileExistsTest
    //
    // Description:
    //  Restores the pre-PR ExtHost CopyFileW(..., bFailIfExists=TRUE)
    //  contract for raw-DLL installs. If "{libName}.dll" already exists in
    //  installDir AND we cannot prove ownership via this library's manifest
    //  (e.g. another library or an external process planted the file),
    //  install must FAIL rather than silently overwrite. This prevents one
    //  library from clobbering another's content in a shared install dir.
    //
    TEST_F(CSharpExtensionApiTests, RawDllInstallFailsIfForeignFileExistsTest)
    {
        string packagesPath = GetPackagesPath();
        string rawDll = (fs::path(packagesPath) / "testpackageE-RAWDLL.dll").string();
        ASSERT_TRUE(fs::exists(rawDll));

        string installDir = CreateInstallDir();

        // Plant a foreign file at the target path with no matching manifest
        // -- simulates ownership by another library or external tooling.
        fs::path foreign = fs::path(installDir) / "owned.dll";
        std::ofstream foreignStream(foreign.string());
        foreignStream << "FOREIGN-CONTENT";
        foreignStream.close();
        ASSERT_TRUE(fs::exists(foreign));

        // Install must FAIL rather than overwrite the foreign file.
        SQLRETURN r = CallInstall(sm_installExternalLibraryFuncPtr,
            "owned", rawDll, installDir);
        EXPECT_EQ(r, SQL_ERROR)
            << "Raw-DLL install must fail when {libName}.dll already exists "
               "and is not owned by this library";

        // Foreign file content must be byte-for-byte unchanged.
        ASSERT_TRUE(fs::exists(foreign));
        std::ifstream ifs(foreign.string());
        std::string contents((std::istreambuf_iterator<char>(ifs)),
                             std::istreambuf_iterator<char>());
        ifs.close();
        EXPECT_EQ(contents, "FOREIGN-CONTENT")
            << "Foreign file was overwritten by failed raw-DLL install";

        // No manifest should have been written for the failed install.
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "owned.manifest"))
            << "Manifest leaked from a failed raw-DLL install";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: AlterFromNonZipToNonZipTest
    //
    // Description:
    //  ALTER EXTERNAL LIBRARY scenario where both v1 and v2 are raw DLLs
    //  with the same libName. v1 writes a single-entry manifest; v2 must
    //  see that manifest, treat the existing "{libName}.dll" as
    //  owned-by-previous, clean it up, then copy v2's bytes into place.
    //  Completes the ALTER coverage matrix (ZIP->ZIP, raw->ZIP, ZIP->raw,
    //  and now raw->raw).
    //
    TEST_F(CSharpExtensionApiTests, AlterFromNonZipToNonZipTest)
    {
        string packagesPath = GetPackagesPath();
        string rawDll = (fs::path(packagesPath) / "testpackageE-RAWDLL.dll").string();
        ASSERT_TRUE(fs::exists(rawDll));

        string installDir = CreateInstallDir();

        // v1: raw DLL install. Writes myLib.dll plus myLib.manifest.
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "myLib", rawDll, installDir), SQL_SUCCESS);
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "myLib.dll"));
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "myLib.manifest"));

        // v2: raw DLL install of the same libName (same source bytes is fine
        // -- the test exercises the manifest cleanup + re-install path).
        SQLRETURN r = CallInstall(sm_installExternalLibraryFuncPtr,
            "myLib", rawDll, installDir);
        EXPECT_EQ(r, SQL_SUCCESS) << "ALTER raw->raw should succeed";

        // The file and manifest must still be present, and the manifest
        // should still contain exactly one entry (no duplication from a
        // missed cleanup of the previous manifest).
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "myLib.dll"));
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "myLib.manifest"));
        vector<string> entries = ReadManifest(
            (fs::path(installDir) / "myLib.manifest").string());
        EXPECT_EQ(entries.size(), 1u)
            << "Raw->raw ALTER must not duplicate manifest entries";
        if (!entries.empty())
        {
            EXPECT_EQ(entries[0], "myLib.dll");
        }

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallRejectsInvalidLibNameTest
    //
    // Description:
    //  ValidateLibraryName must reject all of the following on the install
    //  side (uninstall is covered separately by
    //  UninstallRejectsPathTraversalLibNameTest):
    //   - empty string
    //   - whitespace-only
    //   - path-traversal segment (".." resolved against installDir)
    //   - embedded null character
    //   - absolute path
    //   - extension-only (covered also by InstallRejectsExtensionOnlyLibNameTest)
    //   - Windows reserved DOS device names (CON, NUL, COM1, LPT1, ...)
    //  A regression in ValidateLibraryName must trip at least one of these
    //  cases.
    //
    TEST_F(CSharpExtensionApiTests, InstallRejectsInvalidLibNameTest)
    {
        string packagesPath = GetPackagesPath();
        string rawDll = (fs::path(packagesPath) / "testpackageE-RAWDLL.dll").string();
        ASSERT_TRUE(fs::exists(rawDll));

        struct Case
        {
            string libName;
            const char *label;
        };
        // Note: "foo\0bar" must be constructed via the (data, length) ctor
        // so the embedded NUL survives. CallInstall forwards libName.length().
        Case cases[] = {
            { string(""),                                 "empty" },
            { string("   "),                              "whitespace-only" },
            { string("../escape"),                        "path-traversal" },
            { string("foo\0bar", 7),                      "null-character" },
#ifdef _WIN32
            { string("C:\\Windows\\foo"),                 "absolute-path-windows" },
#else
            { string("/etc/foo"),                         "absolute-path-posix" },
#endif
            { string(".dll"),                             "extension-only" },
            // Reserved DOS device names. ValidateLibraryName checks the
            // stem (Path.GetFileNameWithoutExtension), so both bare "CON"
            // and "CON.dll" / "nul.txt" must be rejected. Mixed case must
            // also be rejected (s_reservedDeviceNames uses
            // OrdinalIgnoreCase).
            { string("CON"),                              "reserved-device-CON" },
            { string("nul"),                              "reserved-device-nul-lower" },
            { string("Aux"),                              "reserved-device-Aux-mixed" },
            { string("PRN"),                              "reserved-device-PRN" },
            { string("COM1"),                             "reserved-device-COM1" },
            { string("LPT9"),                             "reserved-device-LPT9" },
            { string("CON.dll"),                          "reserved-device-CON-with-ext" },
            { string("nul.manifest"),                     "reserved-device-nul-with-ext" },
        };

        for (const Case &c : cases)
        {
            string installDir = CreateInstallDir();

            SQLRETURN result = CallInstall(sm_installExternalLibraryFuncPtr,
                c.libName, rawDll, installDir);
            EXPECT_EQ(result, SQL_ERROR)
                << "ValidateLibraryName should reject case: " << c.label;

            CleanupInstallDir(installDir);
        }
    }

    //----------------------------------------------------------------------------------------------
    // Name: InstallZipWithDllSuffixedLibNameTest
    //
    // Description:
    //  Symmetric coverage with InstallRawDllWithDllSuffixedLibNameTest. When
    //  CREATE EXTERNAL LIBRARY [foo.dll] is paired with a ZIP package whose
    //  contents do NOT include a "foo.dll" file at the root, install must
    //  create a single "foo.dll" alias (NOT "foo.dll.dll") and the alias
    //  must be tracked in the manifest as "foo.dll".
    //
    TEST_F(CSharpExtensionApiTests, InstallZipWithDllSuffixedLibNameTest)
    {
        string packagesPath = GetPackagesPath();
        string pkgB = (fs::path(packagesPath) / "testpackageB-DLL.zip").string();
        ASSERT_TRUE(fs::exists(pkgB));

        string installDir = CreateInstallDir();

        // libName ends in .dll. The package contains testpackageB.dll +
        // testpackageB.deps.json -- nothing matching "foo.*" -- so the
        // install code must create an alias.
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "foo.dll", pkgB, installDir), SQL_SUCCESS);

        // Single .dll suffix -- not "foo.dll.dll".
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "foo.dll"))
            << "Alias not created at expected single-.dll path";
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "foo.dll.dll"))
            << "Alias incorrectly written with double-.dll extension";

        // Manifest tracks the alias under its single-.dll name.
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "foo.dll.manifest"));
        vector<string> entries = ReadManifest(
            (fs::path(installDir) / "foo.dll.manifest").string());
        bool aliasInManifest = false;
        for (const string &e : entries)
        {
            if (e == "foo.dll")
            {
                aliasInManifest = true;
                break;
            }
        }
        EXPECT_TRUE(aliasInManifest)
            << "Manifest must list the alias under its single-.dll name";

        CleanupInstallDir(installDir);
    }

    //----------------------------------------------------------------------------------------------
    // Name: UninstallWithMissingInstallDirTest
    //
    // Description:
    //  Uninstall must succeed (no-op) when installDir does not exist on
    //  disk at all. The Directory.Exists guard in UninstallExternalLibrary
    //  short-circuits manifest cleanup; this test pins that contract so a
    //  future change can't accidentally throw on a missing directory.
    //  Distinct from UninstallNonExistentLibraryTest which creates the
    //  installDir first.
    //
    TEST_F(CSharpExtensionApiTests, UninstallWithMissingInstallDirTest)
    {
        // Construct a path that does NOT exist. Use a sibling of the
        // standard testInstallLibs path so we know the parent directory
        // is writable but the target itself is absent.
        char path[MAX_PATH + 1] = { 0 };
        GetModuleFileName(NULL, path, MAX_PATH);
        fs::path missing = fs::path(path).parent_path() / "testInstallLibs-missing";
        if (fs::exists(missing))
        {
            fs::remove_all(missing);
        }
        ASSERT_FALSE(fs::exists(missing))
            << "Test setup error: chosen installDir must not exist";

        SQLRETURN r = CallUninstall(sm_uninstallExternalLibraryFuncPtr,
            "anything", missing.string());
        EXPECT_EQ(r, SQL_SUCCESS)
            << "Uninstall against a missing installDir must succeed (no-op)";

        // Should NOT have created the directory as a side effect.
        EXPECT_FALSE(fs::exists(missing))
            << "Uninstall must not create installDir";
    }

    //----------------------------------------------------------------------------------------------
    // Name: UninstallPreservesSharedNestedDirsTest
    //
    // Description:
    //  Two libraries can share a nested parent directory (e.g. both contribute
    //  files under "lib/net8.0/"). Uninstalling one must leave the other's
    //  files AND the shared parent directory itself intact.
    //
    //  Library 1 = testpackageD-NESTED.zip  -> lib/net8.0/MyLib.dll, runtimes/win-x64/native.dll, MyLib.deps.json
    //  Library 2 = testpackageJ-NESTED2.zip -> lib/net8.0/Other.dll, Other.deps.json
    //
    //  After installing both: lib/net8.0/ contains MyLib.dll AND Other.dll.
    //  After uninstalling library 1: lib/net8.0/Other.dll AND lib/net8.0/
    //  AND lib/ must all still exist.
    //
    TEST_F(CSharpExtensionApiTests, UninstallPreservesSharedNestedDirsTest)
    {
        string packagesPath = GetPackagesPath();
        string pkgD = (fs::path(packagesPath) / "testpackageD-NESTED.zip").string();
        string pkgJ = (fs::path(packagesPath) / "testpackageJ-NESTED2.zip").string();
        ASSERT_TRUE(fs::exists(pkgD));
        ASSERT_TRUE(fs::exists(pkgJ));

        string installDir = CreateInstallDir();

        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "lib1", pkgD, installDir), SQL_SUCCESS);
        ASSERT_EQ(CallInstall(sm_installExternalLibraryFuncPtr,
            "lib2", pkgJ, installDir), SQL_SUCCESS);

        // Pre-uninstall sanity: both libraries' files coexist in lib/net8.0/.
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "lib" / "net8.0" / "MyLib.dll"));
        ASSERT_TRUE(fs::exists(fs::path(installDir) / "lib" / "net8.0" / "Other.dll"));

        // Uninstall library 1.
        ASSERT_EQ(CallUninstall(sm_uninstallExternalLibraryFuncPtr,
            "lib1", installDir), SQL_SUCCESS);

        // Library 1's unique files must be gone.
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "lib" / "net8.0" / "MyLib.dll"))
            << "lib1's MyLib.dll leaked after uninstall";
        EXPECT_FALSE(fs::exists(fs::path(installDir) / "MyLib.deps.json"))
            << "lib1's MyLib.deps.json leaked after uninstall";

        // Library 2's files must be untouched.
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "lib" / "net8.0" / "Other.dll"))
            << "lib2's Other.dll was wrongly removed by lib1's uninstall";
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "Other.deps.json"))
            << "lib2's Other.deps.json was wrongly removed by lib1's uninstall";
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "lib2.manifest"))
            << "lib2's manifest was wrongly removed by lib1's uninstall";

        // The shared parent directory MUST still exist (lib2 still has
        // content there). Empty-dir cleanup must only fire on dirs that
        // become empty, not on dirs that still have content from another
        // library.
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "lib" / "net8.0"))
            << "Shared parent directory lib/net8.0/ wrongly removed by lib1's uninstall";
        EXPECT_TRUE(fs::exists(fs::path(installDir) / "lib"))
            << "Shared parent directory lib/ wrongly removed by lib1's uninstall";

        CleanupInstallDir(installDir);
    }
}
