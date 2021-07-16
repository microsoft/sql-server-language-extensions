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

    // Name: CSharpExtensionApiTest::SetUp
    //
    // Description:
    //  Code here will be called immediately after the constructor (right
    //  before each test).
    //
    void CSharpExtensionApiTests::SetUp()
    {
        SetupVariables();
        GetHandles();
    }

    // Name: CSharpExtensionApiTest::TearDown
    //
    // Description:
    //  Code here will be called immediately after each test
    //  (right before the destructor).
    //
    void CSharpExtensionApiTests::TearDown()
    {
        DoCleanup();
    }

    // Name: CSharpExtensionApiTest::SetupVariables
    //
    // Description:
    //  Set up default, valid variables for use in tests
    //
    void CSharpExtensionApiTests::SetupVariables()
    {
        char path[MAX_PATH+1] = {0};
        GetModuleFileName(NULL, path, MAX_PATH);
        fs::path exePath = path;
        fs::path buildOutputPath = exePath.parent_path().parent_path().parent_path().parent_path();
        sm_extensionPath = (buildOutputPath / "dotnet-core-CSharp-extension/windows/debug").string();
        m_extensionPathLength = sm_extensionPath.length();

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
    // Name: CSharpExtensionApiTest::DoCleanup
    //
    // Description:
    // Calls Cleanup on the .NET Core CSharpExtension.
    // Testing if Cleanup is implemented correctly.
    //
    void CSharpExtensionApiTests::DoCleanup()
    {
        SQLRETURN result = SQL_ERROR;

        result = (*sm_cleanupSessionFuncPtr)(*m_sessionId, m_taskId);
        EXPECT_EQ(result, SQL_SUCCESS);

        result = (*sm_cleanupFuncPtr)();
        EXPECT_EQ(result, SQL_SUCCESS);
    }
}
