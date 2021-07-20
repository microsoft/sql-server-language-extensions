//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpExtensionApiTests.h
//
// Purpose:
//  Specifies some typedefinitions to be used in the test.
//  And declares the base test fixture class .NET Core CSharpExtensionApiTests.
//
//*********************************************************************
#pragma once
#include "Common.h"
#include <unordered_map>

typedef SQLRETURN FN_init(
    SQLCHAR *, // extensionParams
    SQLULEN,   // extensionParamsLength
    SQLCHAR *, // extensionPath
    SQLULEN,   // extensionPathLength
    SQLCHAR *, // publicLibraryPath
    SQLULEN,   // publicLibraryPathLength
    SQLCHAR *, // privateLibraryPath
    SQLULEN);  // privateLibraryPathLength

typedef SQLRETURN FN_initSession(
    SQLGUID,       // sessionId
    SQLUSMALLINT,  // taskId
    SQLUSMALLINT,  // numTasks
    SQLCHAR *,     // script
    SQLULEN,       // scriptLength
    SQLUSMALLINT,  // inputSchemaColumnsNumber
    SQLUSMALLINT,  // parametersNumber
    SQLCHAR*,      // inputDataName
    SQLUSMALLINT,  // inputDataNameLength
    SQLCHAR*,      // outputDataName
    SQLUSMALLINT); // outputDataNameLength

typedef SQLRETURN FN_initColumn(
    SQLGUID,      // sessionId
    SQLUSMALLINT, // taskId
    SQLUSMALLINT, // columnNumber
    SQLCHAR *,    // columnName
    SQLSMALLINT,  // columnNameLenth
    SQLSMALLINT,  // dataType
    SQLULEN,      // columnSize,
    SQLSMALLINT,  // decimalDigits
    SQLSMALLINT,  // nullable
    SQLSMALLINT,  // partitionByNumber
    SQLSMALLINT); // orderByNumber

typedef SQLRETURN FN_initParam(
    SQLGUID,      // sessionId
    SQLUSMALLINT, // taskId
    SQLUSMALLINT, // paramNumber
    SQLCHAR*,     // paramName,
    SQLSMALLINT,  // paramNameLength
    SQLSMALLINT,  // dataType
    SQLULEN,      // argSize
    SQLSMALLINT,  // decimalDigits
    SQLPOINTER,   // paramValue
    SQLINTEGER,   // strLen_or_Ind
    SQLSMALLINT); // inputOutputType

typedef SQLRETURN FN_execute(
    SQLGUID,        // sessionId
    SQLUSMALLINT,   // taskId
    SQLULEN,        // rowsNumber
    SQLPOINTER *,   // data
    SQLINTEGER **,  // strLenOrInd
    SQLUSMALLINT *);// outputSchemaColumnsNumber

typedef SQLRETURN FN_getResultColumn(
    SQLGUID,        // sessionId
    SQLUSMALLINT,   // taskId
    SQLUSMALLINT,   // columnNumber
    SQLSMALLINT *,  // dataType,
    SQLULEN *,      // columnSize,
    SQLSMALLINT *,  // decimalDigits,
    SQLSMALLINT *); // nullable

typedef SQLRETURN FN_getResults(
    SQLGUID,         // sessionId
    SQLUSMALLINT,    // taskId
    SQLULEN *,       // rowsNumber
    SQLPOINTER **,   // data
    SQLINTEGER ***); // strLen_or_Ind

typedef SQLRETURN FN_getOutputParam(
    SQLGUID,       // sessionId
    SQLUSMALLINT,  // taskId
    SQLUSMALLINT,  // paramNumber
    SQLPOINTER *,  // paramValue
    SQLINTEGER *); // strLen_or_Ind

typedef SQLRETURN FN_cleanupSession(
    SQLGUID,       // sessionId
    SQLUSMALLINT); // taskId

typedef SQLRETURN FN_cleanup();

namespace ExtensionApiTest
{
    class CSharpExtensionApiTests : public ::testing::Test
    {
    protected:

        // Per-test-suite set-up.
        // Called before the first test in this test suite.
        //
        static void SetUpTestCase();

        // Per-test-suite tear-down.
        // Called after the last test in this test suite.
        //
        static void TearDownTestCase();

        // Code here will be called immediately after the constructor (right
        // before each test).
        //
        virtual void SetUp() override;

        // Code here will be called immediately after each test (right
        // before the destructor).
        //
        virtual void TearDown() override;

        // Set up default, valid variables for use in tests.
        //
        virtual void SetupVariables();

        // Load library and get handles to different functions
        // Testing if CSharpExtension is successfully loaded dynamically
        //
        static void GetHandles();

        // Do Init where .NET runtime is initialized - can be called only once in the validate suite.
        // Testing if Init is implemented correctly.
        //
        static void DoInit();

        // Get length of a wstring
        //
        SQLINTEGER GetWStringLength(const wchar_t *str);

        // Cleanup a valid session.
        //
        void SessionCleanup();

        // Set the extension path variables.
        //
        static void SetUpPath();

        // Call Cleanup on the CSharpExtension.
        // Testing if Cleanup is implemented correctly.
        //
        static void DoCleanup();

        // Initialize a valid session.
        //
        void InitializeSession(
            SQLUSMALLINT inputSchemaColumnsNumber = 0,
            SQLUSMALLINT parametersNumber = 0,
            std::string  scriptString = "");

        // Template to test all input parameter data types
        //
        template<class SQLType, SQLSMALLINT dataType>
        void TestParameter(
            int         paramNumber,
            SQLType     paramValue,
            bool        isNull = false,
            SQLSMALLINT inputOutputType = SQL_PARAM_INPUT_OUTPUT,
            SQLRETURN   SQLResult = 0);

        // Test a string parameter
        //
        void TestStringParameter(
            int         paramNumber,
            const char  *paramValue,
            SQLULEN     paramSize,
            bool        isFixedType,
            SQLSMALLINT inputOutputType = SQL_PARAM_INPUT_OUTPUT,
            SQLRETURN   SQLResult = 0);

        // Test a string parameter
        //
        void TestWStringParameter(
            int           paramNumber,
            const wchar_t *paramValue,
            SQLINTEGER    paramSize,
            bool          isFixedType,
            SQLSMALLINT   inputOutputType = SQL_PARAM_INPUT_OUTPUT,
            SQLRETURN     SQLResult = 0);

        // Objects declared here can be used by all tests in the test suite.
        //
        std::shared_ptr<SQLGUID> m_sessionId;
        SQLUSMALLINT m_taskId;
        SQLUSMALLINT m_numTasks;

        SQLCHAR *m_script = nullptr;
        std::string m_scriptString;
        SQLSMALLINT m_scriptLength;

        SQLCHAR *m_columnName = nullptr;
        std::string m_columnNameString;

        SQLCHAR *m_inputDataName = nullptr;
        std::string m_inputDataNameString;
        SQLSMALLINT m_inputDataNameLength;

        SQLCHAR *m_outputDataName = nullptr;
        std::string m_outputDataNameString;
        SQLSMALLINT m_outputDataNameLength;

        SQLUSMALLINT m_inputSchemaColumnsNumber;
        SQLUSMALLINT m_parametersNumber;

        const SQLINTEGER m_IntSize = sizeof(SQLINTEGER);

        // Path of .NET Core CSharp Extension
        //
        static std::string sm_extensionPath;

        // Pointer handle to the library nativecsharpextension
        //
        static HINSTANCE sm_libHandle;

        // Pointer to the Init function
        //
        static FN_init *sm_initFuncPtr;

        // Pointer to the InitSession function
        //
        static FN_initSession *sm_initSessionFuncPtr;

        // Pointer to the InitColumn function
        //
        static FN_initColumn *sm_initColumnFuncPtr;

        // Pointer to the InitParam function
        //
        static FN_initParam *sm_initParamFuncPtr;

        // Pointer to the Execute function
        //
        static FN_execute *sm_executeFuncPtr;

        // Pointer to the GetResultColumn function
        //
        static FN_getResultColumn *sm_getResultColumnFuncPtr;

        // Pointer to the GetResults function
        //
        static FN_getResults *sm_getResultsFuncPtr;

        // Pointer to the GetOutputParam function
        //
        static FN_getOutputParam *sm_getOutputParamFuncPtr;

        // Pointer to the CleanupSession function
        //
        static FN_cleanupSession *sm_cleanupSessionFuncPtr;

        // Pointer to the Cleanup function
        //
        static FN_cleanup *sm_cleanupFuncPtr;
    };
}
