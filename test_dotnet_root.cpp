// test_dotnet_root.cpp
// Standalone test for DotnetEnvironment::get_dotnet host_path fix.
// Loads nativecsharpextension.dll and calls Init() WITHOUT DOTNET_ROOT set.
//
// Build:
//   cl.exe test_dotnet_root.cpp /EHsc /Fe:test_dotnet_root.exe
//
// Run: test_dotnet_root.exe <path_to_extension_dir>
//   e.g.: test_dotnet_root.exe C:\repos\sql-server-language-extensions\build-output\dotnet-core-CSharp-extension\windows\debug
//
#include <Windows.h>
#include <stdio.h>
#include <string.h>

typedef short SQLRETURN;
typedef unsigned short SQLUSMALLINT;
typedef unsigned char SQLCHAR;
typedef unsigned long long SQLULEN;

typedef SQLUSMALLINT (*GetInterfaceVersionFn)();
typedef SQLRETURN (*InitFn)(
    SQLCHAR *ExtensionParams, SQLULEN ExtensionParamsLength,
    SQLCHAR *ExtensionPath, SQLULEN ExtensionPathLength,
    SQLCHAR *PublicLibraryPath, SQLULEN PublicLibraryPathLength,
    SQLCHAR *PrivateLibraryPath, SQLULEN PrivateLibraryPathLength);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: test_dotnet_root.exe <extension_dir>\n");
        printf("  extension_dir: path containing nativecsharpextension.dll + managed DLLs\n");
        return 1;
    }

    const char *extDir = argv[1];

    // Remove DOTNET_ROOT to simulate the bug scenario
    SetEnvironmentVariableA("DOTNET_ROOT", NULL);
    SetEnvironmentVariableW(L"DOTNET_ROOT", NULL);

    printf("[TEST] DOTNET_ROOT cleared from environment\n");
    printf("[TEST] Extension dir: %s\n", extDir);

    // Build DLL path
    char dllPath[MAX_PATH];
    snprintf(dllPath, MAX_PATH, "%s\\nativecsharpextension.dll", extDir);

    printf("[TEST] Loading DLL: %s\n", dllPath);
    HMODULE hDll = LoadLibraryA(dllPath);
    if (!hDll)
    {
        printf("[FAIL] LoadLibrary failed: 0x%08x\n", GetLastError());
        return 1;
    }

    GetInterfaceVersionFn getVersion = (GetInterfaceVersionFn)GetProcAddress(hDll, "GetInterfaceVersion");
    InitFn initFn = (InitFn)GetProcAddress(hDll, "Init");

    if (!getVersion || !initFn)
    {
        printf("[FAIL] Missing exports: GetInterfaceVersion=%p Init=%p\n", getVersion, initFn);
        FreeLibrary(hDll);
        return 1;
    }

    printf("[TEST] Extension API version: %d\n", getVersion());

    // Call Init with the extension dir as all paths.
    // Init creates DotnetEnvironment and calls Init() which calls
    // load_hostfxr() then get_dotnet() — the code path we're testing.
    //
    SQLCHAR emptyParams[] = "";
    SQLRETURN rc = initFn(
        emptyParams, 0,                                               // ExtensionParams
        (SQLCHAR *)extDir, (SQLULEN)strlen(extDir),                   // ExtensionPath (language path)
        (SQLCHAR *)extDir, (SQLULEN)strlen(extDir),                   // PublicLibraryPath
        (SQLCHAR *)extDir, (SQLULEN)strlen(extDir));                  // PrivateLibraryPath

    if (rc == 0)
    {
        printf("[PASS] Init succeeded (rc=0) — DotnetEnvironment initialized without DOTNET_ROOT\n");
    }
    else
    {
        printf("[FAIL] Init failed (rc=%d, 0x%04x) — DotnetEnvironment could not initialize\n", rc, (unsigned short)rc);
    }

    FreeLibrary(hDll);
    return (rc == 0) ? 0 : 1;
}
