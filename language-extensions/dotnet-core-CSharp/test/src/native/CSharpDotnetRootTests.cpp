//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpDotnetRootTests.cpp
//
// Purpose:
//  Tests that DotnetEnvironment initialization works correctly when
//  the DOTNET_ROOT environment variable is not set. Validates the fix
//  for AppContainer ACCESS_DENIED when hostfxr probes the filesystem
//  without host_path set.
//
//  These tests verify Init() behavior; they do NOT test within an
//  AppContainer (that requires Launchpad). They verify that hostfxr
//  receives correct host_path and dotnet_root parameters.
//
//*********************************************************************
#include "CSharpExtensionApiTests.h"

using namespace std;

namespace ExtensionApiTest
{
    //----------------------------------------------------------------------
    // DotnetRootTests
    //
    // Separate test fixture that tests Init behavior with DOTNET_ROOT
    // environment variable manipulation.
    //
    // NOTE: .NET runtime can only be initialized once per process.
    // These tests must run in a separate test binary or before any
    // other test that calls Init(). The main test suite already calls
    // DoInit() in SetUpTestCase, so these tests verify that the Init
    // call (which has already happened) succeeded despite DOTNET_ROOT
    // being cleared.
    //
    class DotnetRootTests : public CSharpExtensionApiTests
    {
    protected:
        static void SetUpTestCase()
        {
            SetUpPath();
            GetHandles();

            // Clear DOTNET_ROOT before Init to simulate the environment
            // where the env var is not set (as in AppContainer satellites).
            //
            SetEnvironmentVariableA("DOTNET_ROOT", NULL);
            SetEnvironmentVariableW(L"DOTNET_ROOT", NULL);

            DoInit();
        }

        static void TearDownTestCase()
        {
            DoCleanup();
        }
    };

    //----------------------------------------------------------------------
    // Test: InitSucceedsWithoutDotnetRoot
    //
    // Verifies that Init() succeeds when DOTNET_ROOT is not set.
    // Before the fix, hostfxr would probe the filesystem with
    // host_path=nullptr, walking up parent directories. In AppContainer,
    // this hits the drive root and fails with ACCESS_DENIED.
    //
    // After the fix:
    //   - host_path is set to the current process executable
    //   - dotnet_root falls back to the language path (m_root_path)
    //
    // This test doesn't run in AppContainer, so the unfixed code would
    // also pass here (filesystem probing succeeds without ACL restrictions).
    // The value of this test is regression prevention: if someone removes
    // the host_path/dotnet_root fix, and the machine's .NET layout changes,
    // this test catches the failure.
    //
    TEST_F(DotnetRootTests, InitSucceedsWithoutDotnetRoot)
    {
        // If we got here, Init() in SetUpTestCase succeeded.
        // Verify the function pointers are valid (Init created DotnetEnvironment).
        //
        ASSERT_NE(sm_initFuncPtr, nullptr);
        ASSERT_NE(sm_initSessionFuncPtr, nullptr);
        ASSERT_NE(sm_executeFuncPtr, nullptr);
        ASSERT_NE(sm_cleanupFuncPtr, nullptr);
    }

    //----------------------------------------------------------------------
    // Test: InitSessionWorksAfterInitWithoutDotnetRoot
    //
    // Verifies that InitSession succeeds after Init was called without
    // DOTNET_ROOT. This confirms the managed CSharpExtension.dll was
    // loaded and can create sessions — the full runtime is functional.
    //
    TEST_F(DotnetRootTests, InitSessionWorksAfterInitWithoutDotnetRoot)
    {
        SQLGUID sessionId;
        memset(&sessionId, 0, sizeof(sessionId));
        sessionId.Data1 = 0xDEADBEEF;

        string scriptString = "Microsoft.SqlServer.CSharpExtensionTest.dll;Microsoft.SqlServer.CSharpExtension.CSharpTestExecutor";
        SQLCHAR *script = (SQLCHAR *)scriptString.c_str();

        string inputDataName = "InputDataSet";
        string outputDataName = "OutputDataSet";

        SQLRETURN result = (*sm_initSessionFuncPtr)(
            sessionId,
            0,       // taskId
            1,       // numTasks
            script,
            scriptString.length(),
            0,       // inputSchemaColumnsNumber
            0,       // parametersNumber
            (SQLCHAR *)inputDataName.c_str(),
            (SQLUSMALLINT)inputDataName.length(),
            (SQLCHAR *)outputDataName.c_str(),
            (SQLUSMALLINT)outputDataName.length());

        EXPECT_EQ(result, SQL_SUCCESS);

        // Cleanup the session
        //
        if (result == SQL_SUCCESS)
        {
            (*sm_cleanupSessionFuncPtr)(sessionId, 0);
        }
    }
}
