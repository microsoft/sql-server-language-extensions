//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpExtensionApiTests.cpp
//
// Purpose:
//  Defines the base test fixture members and tests .NET Core CSharpExtension's implementation of
//  the external language initialization and cleanup APIs.
//
//*********************************************************************
#include "CSharpExtensionApiTests.h"

using namespace std;
namespace fs = experimental::filesystem;

namespace ExtensionApiTest
{
    // Initialize all the static members
    //
    string CSharpExtensionApiTests::sm_extensionPath;
    HINSTANCE CSharpExtensionApiTests::sm_libHandle;
    FN_init *CSharpExtensionApiTests::sm_initFuncPtr = nullptr;
    FN_initSession *CSharpExtensionApiTests::sm_initSessionFuncPtr = nullptr;
    FN_initColumn *CSharpExtensionApiTests::sm_initColumnFuncPtr = nullptr;
    FN_initParam *CSharpExtensionApiTests::sm_initParamFuncPtr = nullptr;
    FN_execute *CSharpExtensionApiTests::sm_executeFuncPtr = nullptr;
    FN_getResultColumn *CSharpExtensionApiTests::sm_getResultColumnFuncPtr = nullptr;
    FN_getResults *CSharpExtensionApiTests::sm_getResultsFuncPtr = nullptr;
    FN_getOutputParam *CSharpExtensionApiTests::sm_getOutputParamFuncPtr = nullptr;
    FN_cleanupSession *CSharpExtensionApiTests::sm_cleanupSessionFuncPtr = nullptr;
    FN_cleanup *CSharpExtensionApiTests::sm_cleanupFuncPtr = nullptr;

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::SetUpTestCase
    //
    // Description:
    //  Per-test-suite set-up. Called before the first test in every test suite.
    //  But we want to execute this only once in the entire test run before the first test case.
    //  since .NET can be initialized only once.
    //
    void CSharpExtensionApiTests::SetUpTestCase()
    {
        SetUpPath();
        GetHandles();
        DoInit();
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::TearDownTestCase
    //
    // Description:
    //  Per-test-suite tear-down. Called after the last test in every test suite.
    //  But we want to execute this only once in the entire test run after the last test case
    //  since .NET can be torn down only once.
    //
    void CSharpExtensionApiTests::TearDownTestCase()
    {
        DoCleanup();
    }

    // Name: CSharpExtensionApiTest::SetUp
    //
    // Description:
    //  Code here will be called immediately after the constructor (right
    //  before each test).
    //
    void CSharpExtensionApiTests::SetUp()
    {
        SetupVariables();
    }

    // Name: CSharpExtensionApiTest::TearDown
    //
    // Description:
    //  Code here will be called immediately after each test
    //  (right before the destructor).
    //
    void CSharpExtensionApiTests::TearDown()
    {
        SessionCleanup();
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::SetUpPath
    //
    // Description:
    //  Sets the extension path variables.
    //
    void CSharpExtensionApiTests::SetUpPath()
    {
        char path[MAX_PATH+1] = {0};
        GetModuleFileName(NULL, path, MAX_PATH);
        fs::path exePath = path;
        fs::path buildOutputPath = exePath.parent_path().parent_path().parent_path().parent_path();
    #if defined(_DEBUG)
        sm_extensionPath = (buildOutputPath / "dotnet-core-CSharp-extension/windows/debug").string();
    #else
        sm_extensionPath = (buildOutputPath / "dotnet-core-CSharp-extension/windows/release").string();
    #endif
    }

    // Name: CSharpExtensionApiTest::SetupVariables
    //
    // Description:
    //  Sets up default, valid variables for use in tests
    //
    void CSharpExtensionApiTests::SetupVariables()
    {
        shared_ptr<SQLGUID> sessionId(new SQLGUID());
        m_sessionId = move(sessionId);
        m_taskId = 0;
        m_numTasks = 1;
        m_inputSchemaColumnsNumber = 1;
        m_parametersNumber = 0;
        m_scriptString = "DotNetLib";
        m_script = static_cast<SQLCHAR *>(static_cast<void *>(const_cast<char *>(m_scriptString.c_str())));
        m_scriptLength = m_scriptString.length();

        m_inputDataNameString = "InputDataSet";
        m_inputDataName = static_cast<SQLCHAR *>(
            static_cast<void *>(const_cast<char *>(m_inputDataNameString.c_str())));
        m_inputDataNameLength = m_inputDataNameString.length();

        m_outputDataNameString = "OutputDataSet";
        m_outputDataName = static_cast<SQLCHAR *>(
            static_cast<void *>(const_cast<char *>(m_outputDataNameString.c_str())));
        m_outputDataNameLength = m_outputDataNameString.length();
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::GetHandles
    //
    // Description:
    //  Loads library and gets handles to different functions
    //  Testing if CSharpExtension is successfully loaded dynamically
    //
    void CSharpExtensionApiTests::GetHandles()
    {
        sm_libHandle = LoadLibrary((sm_extensionPath+"\\nativecsharpextension.dll").c_str());
        EXPECT_TRUE(sm_libHandle != nullptr);

        sm_initFuncPtr = reinterpret_cast<FN_init*>(GetProcAddress(sm_libHandle, "Init"));
        EXPECT_TRUE(sm_initFuncPtr != nullptr);

        sm_initSessionFuncPtr = reinterpret_cast<FN_initSession*>(GetProcAddress(sm_libHandle, "InitSession"));
        EXPECT_TRUE(sm_initSessionFuncPtr != nullptr);

        sm_initColumnFuncPtr = reinterpret_cast<FN_initColumn*>(GetProcAddress(sm_libHandle, "InitColumn"));
        EXPECT_TRUE(sm_initColumnFuncPtr != nullptr);

        sm_initParamFuncPtr = reinterpret_cast<FN_initParam*>(GetProcAddress(sm_libHandle, "InitParam"));
        EXPECT_TRUE(sm_initParamFuncPtr != nullptr);

        sm_executeFuncPtr = reinterpret_cast<FN_execute*>(GetProcAddress(sm_libHandle, "Execute"));
        EXPECT_TRUE(sm_executeFuncPtr != nullptr);

        sm_getResultColumnFuncPtr = reinterpret_cast<FN_getResultColumn*>(GetProcAddress(sm_libHandle, "GetResultColumn"));
        EXPECT_TRUE(sm_getResultColumnFuncPtr != nullptr);

        sm_getResultsFuncPtr = reinterpret_cast<FN_getResults*>(GetProcAddress(sm_libHandle, "GetResults"));
        EXPECT_TRUE(sm_getResultsFuncPtr != nullptr);

        sm_getOutputParamFuncPtr = reinterpret_cast<FN_getOutputParam*>(GetProcAddress(sm_libHandle, "GetOutputParam"));
        EXPECT_TRUE(sm_getOutputParamFuncPtr != nullptr);

        sm_cleanupSessionFuncPtr = reinterpret_cast<FN_cleanupSession*>(GetProcAddress(sm_libHandle, "CleanupSession"));
        EXPECT_TRUE(sm_cleanupSessionFuncPtr != nullptr);

        sm_cleanupFuncPtr = reinterpret_cast<FN_cleanup*>(GetProcAddress(sm_libHandle, "Cleanup"));
        EXPECT_TRUE(sm_cleanupFuncPtr != nullptr);
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::DoInit
    //
    // Description:
    //  Does Init where embedded .NET is initialized - can be called only once in the test suite.
    //  Testing if Init api is implemented correctly.
    //
    void CSharpExtensionApiTests::DoInit()
    {
        SQLCHAR *unsignedExtensionPath = static_cast<SQLCHAR *>(
            static_cast<void *>(const_cast<char *>(sm_extensionPath.c_str())));

        SQLRETURN result = (*sm_initFuncPtr)(
            nullptr,                   // Extension Params
            0,                         // Extension Params Length
            unsignedExtensionPath,     // Extension Path
            sm_extensionPath.length(), // Extension Path Length
            unsignedExtensionPath,     // Public Library Path
            sm_extensionPath.length(), // Public Library Path Length
            nullptr,                   // Private Library Path
            0                          // Private Library Path Length
        );

        EXPECT_EQ(result, SQL_SUCCESS);
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::InitializeSession
    //
    // Description:
    //  Initializes a valid, default session for later tests
    //
    void CSharpExtensionApiTests::InitializeSession(
        SQLUSMALLINT inputSchemaColumnsNumber,
        SQLUSMALLINT parametersNumber,
        string       scriptString)
    {
        SQLCHAR *script = static_cast<SQLCHAR*>(
            static_cast<void*>(const_cast<char*>(scriptString.c_str())));

        SQLRETURN result = (*sm_initSessionFuncPtr)(
            *m_sessionId,
            m_taskId,
            m_numTasks,
            script,
            scriptString.length(),
            inputSchemaColumnsNumber,
            parametersNumber,
            m_inputDataName,
            m_inputDataNameString.length(),
            m_outputDataName,
            m_outputDataNameString.length());

        EXPECT_EQ(result, SQL_SUCCESS);
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::SessionCleanup
    //
    // Description:
    //  Cleans up a valid, default session for later tests
    //
    void CSharpExtensionApiTests::SessionCleanup()
    {
        SQLRETURN result = (*sm_cleanupSessionFuncPtr)(*m_sessionId, m_taskId);
        EXPECT_EQ(result, SQL_SUCCESS);
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::DoCleanup
    //
    // Description:
    // Calls Cleanup on the .NET Core CSharpExtension.
    // Testing if Cleanup is implemented correctly.
    //
    void CSharpExtensionApiTests::DoCleanup()
    {
        SQLRETURN result = (*sm_cleanupFuncPtr)();
        EXPECT_EQ(result, SQL_SUCCESS);
    }
}
