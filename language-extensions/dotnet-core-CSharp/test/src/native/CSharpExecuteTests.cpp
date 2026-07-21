//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpExecuteTests.cpp
//
// Purpose:
//  Test the .NET Core CSharp Execute using the Extension API
//
//*********************************************************************
#include "CSharpExtensionApiTests.h"
#include "LogXEventTestHarness.h"

#include <cstring>

using namespace std;

namespace ExtensionApiTest
{
    //----------------------------------------------------------------------------------------------
    // Name: ExecuteEmptyDllNameTest
    //
    // Description:
    //  Test Execute with empty dll file name.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteEmptyDllNameTest)
    {
        InitializeSession(
            0, // inputSchemaColumnsNumber
            0, // parametersNumber
            m_UserClassFullName);

        SQLUSMALLINT outputschemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            0,
            nullptr, // dataSet
            nullptr, // strLenOrInd
            &outputschemaColumnsNumber);

        EXPECT_EQ(result, SQL_SUCCESS);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteEmptyScriptTest
    //
    // Description:
    //  Test Execute with null script.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteEmptyScriptTest)
    {
        InitializeSession(
            0,   // inputSchemaColumnsNumber
            0,   // parametersNumber
            ""); // scriptStr

        SQLUSMALLINT outputschemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            0,
            nullptr, // dataSet
            nullptr, // strLenOrInd
            &outputschemaColumnsNumber);

        EXPECT_EQ(result, SQL_SUCCESS);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteInvalidScriptTest
    //
    // Description:
    //  Test Execute with invalid script format.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteInvalidScriptTest)
    {
        // Script in invalid format
        //
        string scriptString = m_UserLibName + "." + m_UserClassFullName;
        InitializeSession(
            0,   // inputSchemaColumnsNumber
            0,   // parametersNumber
            scriptString);

        testing::internal::CaptureStderr();
        SQLUSMALLINT outputschemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            0,
            nullptr, // dataSet
            nullptr, // strLenOrInd
            &outputschemaColumnsNumber);

        EXPECT_EQ(result, SQL_ERROR);
        string error = testing::internal::GetCapturedStderr();
        EXPECT_TRUE(error.find("Please provide user class in the form of LibraryName;Namespace.Classname or Namespace.Classname") != string::npos);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteInvalidLibraryNameScriptTest
    //
    // Description:
    //  Test Execute with invalid library name.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteInvalidLibraryNameScriptTest)
    {
        // Use a library name that cannot resolve to any DLL on the search path.
        // The pre-PR literal "Microsoft.SqlServer.CSharpExtensionTest" is the basename of
        // m_UserLibName ("Microsoft.SqlServer.CSharpExtensionTest.dll"), so the loader now
        // resolves it successfully and the test would fail to observe the expected error.
        //
        string userLibName = "NonExistentLibrary";
        string scriptString = userLibName + m_Separator + m_UserClassFullName;
        InitializeSession(
            0,   // inputSchemaColumnsNumber
            0,   // parametersNumber
            scriptString);

        testing::internal::CaptureStderr();
        SQLUSMALLINT outputschemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            0,
            nullptr, // dataSet
            nullptr, // strLenOrInd
            &outputschemaColumnsNumber);

        EXPECT_EQ(result, SQL_ERROR);
        string error = testing::internal::GetCapturedStderr();
        EXPECT_TRUE(error.find("Unable to find user dll under") != string::npos);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteEmptyColumnsTest
    //
    // Description:
    //  Test Execute with default script using an InputDataSet of Empty columns.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteEmptyColumnsTest)
    {
        InitializeSession(
            (*m_emptyIntegerInfo).GetColumnsNumber(), // inputSchemaColumnsNumber
            0,                                        // parametersNumber
            m_scriptString);                          // scriptString

        InitializeColumns<SQLINTEGER, SQL_C_SLONG>(m_emptyIntegerInfo.get());

        Execute<SQLINTEGER, SQL_C_SLONG>(
            0,
            nullptr,
            nullptr,
            (*m_emptyIntegerInfo).m_columnNames);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteIntegerColumnsTest
    //
    // Description:
    //  Test Execute with default script using an InputDataSet of Integer columns.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteIntegerColumnsTest)
    {
        InitializeSession(
            (*m_integerInfo).GetColumnsNumber(), // inputSchemaColumnsNumber
            0,                                   // parametersNumber
            m_scriptString);                     // scriptString

        InitializeColumns<SQLINTEGER, SQL_C_SLONG>(m_integerInfo.get());

        Execute<SQLINTEGER, SQL_C_SLONG>(
            ColumnInfo<SQLINTEGER>::sm_rowsNumber,
            (*m_integerInfo).m_dataSet.data(),
            (*m_integerInfo).m_strLen_or_Ind.data(),
            (*m_integerInfo).m_columnNames);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteInvalidIntegerColumnsTest
    //
    // Description:
    //  Test Execute with invalid script using an InputDataSet of Integer columns.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteInvalidIntegerColumnsTest)
    {
        string scriptString = m_UserLibName + m_Separator + "UserExecutor.CSharpTestInValidExecutor";
        InitializeSession(
            (*m_integerInfo).GetColumnsNumber(),       // inputSchemaColumnsNumber
            0,                                         // parametersNumber
            scriptString);                             // scriptString (undefined user class name)

        InitializeColumns<SQLINTEGER, SQL_C_SLONG>(m_integerInfo.get());

        Execute<SQLINTEGER, SQL_C_SLONG>(
            ColumnInfo<SQLINTEGER>::sm_rowsNumber,
            (*m_integerInfo).m_dataSet.data(),
            (*m_integerInfo).m_strLen_or_Ind.data(),
            (*m_integerInfo).m_columnNames,
            SQL_ERROR);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteBooleanColumnsTest
    //
    // Description:
    //  Test Execute using an InputDataSet of Boolean columns.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteBooleanColumnsTest)
    {
        InitializeSession(
            (*m_booleanInfo).GetColumnsNumber(), // inputSchemaColumnsNumber
            0,                                   // parametersNumber
            m_scriptString);                     // scriptString

        InitializeColumns<SQLCHAR, SQL_C_BIT>(m_booleanInfo.get());

        Execute<SQLCHAR, SQL_C_BIT>(
            ColumnInfo<SQLCHAR>::sm_rowsNumber,
            (*m_booleanInfo).m_dataSet.data(),
            (*m_booleanInfo).m_strLen_or_Ind.data(),
            (*m_booleanInfo).m_columnNames);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteRealColumnsTest
    //
    // Description:
    //  Test Execute with default script using an InputDataSet of Real columns.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteRealColumnsTest)
    {
        InitializeSession(
            (*m_realInfo).GetColumnsNumber(), // inputSchemaColumnsNumber
            0,                                   // parametersNumber
            m_scriptString);                     // scriptString

        InitializeColumns<SQLREAL, SQL_C_FLOAT>(m_realInfo.get());

        Execute<SQLREAL, SQL_C_FLOAT>(
            ColumnInfo<SQLREAL>::sm_rowsNumber,
            (*m_realInfo).m_dataSet.data(),
            (*m_realInfo).m_strLen_or_Ind.data(),
            (*m_realInfo).m_columnNames);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteDoubleColumnsTest
    //
    // Description:
    //  Test Execute with default script using an InputDataSet of Double columns.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteDoubleColumnsTest)
    {
        InitializeSession(
            (*m_doubleInfo).GetColumnsNumber(), // inputSchemaColumnsNumber
            0,                                   // parametersNumber
            m_scriptString);                     // scriptString

        InitializeColumns<SQLDOUBLE, SQL_C_DOUBLE>(m_doubleInfo.get());

        Execute<SQLDOUBLE, SQL_C_DOUBLE>(
            ColumnInfo<SQLDOUBLE>::sm_rowsNumber,
            (*m_doubleInfo).m_dataSet.data(),
            (*m_doubleInfo).m_strLen_or_Ind.data(),
            (*m_doubleInfo).m_columnNames);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteBigIntColumnsTest
    //
    // Description:
    //  Test Execute with default script using an InputDataSet of BigInteger columns.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteBigIntColumnsTest)
    {
        InitializeSession(
            (*m_bigIntInfo).GetColumnsNumber(), // inputSchemaColumnsNumber
            0,                                   // parametersNumber
            m_scriptString);                     // scriptString

        InitializeColumns<SQLBIGINT, SQL_C_SBIGINT>(m_bigIntInfo.get());

        Execute<SQLBIGINT, SQL_C_SBIGINT>(
            ColumnInfo<SQLBIGINT>::sm_rowsNumber,
            (*m_bigIntInfo).m_dataSet.data(),
            (*m_bigIntInfo).m_strLen_or_Ind.data(),
            (*m_bigIntInfo).m_columnNames);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteSmallIntColumnsTest
    //
    // Description:
    //  Test Execute with default script using an InputDataSet of SmallInt columns.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteSmallIntColumnsTest)
    {
        InitializeSession(
            (*m_smallIntInfo).GetColumnsNumber(), // inputSchemaColumnsNumber
            0,                                   // parametersNumber
            m_scriptString);                     // scriptString

        InitializeColumns<SQLSMALLINT, SQL_C_SSHORT>(m_smallIntInfo.get());

        Execute<SQLSMALLINT, SQL_C_SSHORT>(
            ColumnInfo<SQLSMALLINT>::sm_rowsNumber,
            (*m_smallIntInfo).m_dataSet.data(),
            (*m_smallIntInfo).m_strLen_or_Ind.data(),
            (*m_smallIntInfo).m_columnNames);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteTinyIntColumnsTest
    //
    // Description:
    //  Test Execute with default script using an InputDataSet of TinyInt columns.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteTinyIntColumnsTest)
    {
        InitializeSession(
            (*m_tinyIntInfo).GetColumnsNumber(), // inputSchemaColumnsNumber
            0,                                   // parametersNumber
            m_scriptString);                     // scriptString

        InitializeColumns<SQLCHAR, SQL_C_UTINYINT>(m_tinyIntInfo.get());

        Execute<SQLCHAR, SQL_C_UTINYINT>(
            ColumnInfo<SQLCHAR>::sm_rowsNumber,
            (*m_tinyIntInfo).m_dataSet.data(),
            (*m_tinyIntInfo).m_strLen_or_Ind.data(),
            (*m_tinyIntInfo).m_columnNames);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteStringColumnsTest
    //
    // Description:
    //  Test Execute with default script using an InputDataSet of string columns.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteStringColumnsTest)
    {
        InitializeSession(
            3,               // inputSchemaColumnsNumber
            0,               // parametersNumber
            m_scriptString); // scriptString

        string stringColumn1Name = "StringColumn1";
        InitializeColumn(0, stringColumn1Name, SQL_C_CHAR, m_CharSize);

        string stringColumn2Name = "StringColumn2";
        InitializeColumn(1, stringColumn2Name, SQL_C_CHAR, m_CharSize);

        string stringColumn3Name = "StringColumn3";
        InitializeColumn(2, stringColumn3Name, SQL_C_CHAR, m_CharSize);

        vector<const char*> stringCol1{ "Hello", "test", "data", "World", "-123" };
        vector<const char*> stringCol2{ "", 0, nullptr, "verify", "-1" };

        vector<SQLINTEGER> strLenOrIndCol1 =
        { static_cast<SQLINTEGER>(strlen(stringCol1[0])),
          static_cast<SQLINTEGER>(strlen(stringCol1[1])),
          static_cast<SQLINTEGER>(strlen(stringCol1[2])),
          static_cast<SQLINTEGER>(strlen(stringCol1[3])),
          static_cast<SQLINTEGER>(strlen(stringCol1[4])) };
        vector<SQLINTEGER> strLenOrIndCol2 =
        { 0, SQL_NULL_DATA, SQL_NULL_DATA,
          static_cast<SQLINTEGER>(strlen(stringCol2[3])),
          static_cast<SQLINTEGER>(strlen(stringCol2[4])) };

        vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1.data(),
            strLenOrIndCol2.data(), nullptr };

        // Coalesce the arrays of each row of each column
        // into a contiguous array for each column.
        //
        vector<char> stringCol1Data = GenerateContiguousData<char>(stringCol1, strLenOrIndCol1.data());
        vector<char> stringCol2Data = GenerateContiguousData<char>(stringCol2, strLenOrIndCol2.data());

        void* dataSet[] = { stringCol1Data.data(),
                            stringCol2Data.data(),
                            nullptr };

        int rowsNumber = stringCol1.size();

        vector<string> columnNames{ stringColumn1Name, stringColumn2Name, stringColumn3Name };

        Execute<SQLCHAR, SQL_C_CHAR>(
            rowsNumber,
            dataSet,
            strLen_or_Ind.data(),
            columnNames);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteNullColumnsTest
    //
    // Description:
    //  Test Execute with default script using a null dataset.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteNullColumnsTest)
    {
        InitializeSession(
            0,               // inputSchemaColumnsNumber
            0,               // parametersNumber
            m_scriptString); // scriptString

        SQLUSMALLINT outputschemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            0,
            nullptr, // dataSet
            nullptr, // strLenOrInd
            &outputschemaColumnsNumber);

        EXPECT_EQ(result, SQL_SUCCESS);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteWStringColumnsTest
    //
    // Description:
    //  Test Execute with default script using an InputDataSet of nvarchar/nchar (Unicode) columns.
    //  Tests nullptr, empty strings, accented characters, emojis, and mixed scripts.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteWStringColumnsTest)
    {
        InitializeSession(
            3,               // inputSchemaColumnsNumber
            0,               // parametersNumber
            m_scriptString); // scriptString

        string wstringColumn1Name = "WStringColumn1";
        InitializeColumn(0, wstringColumn1Name, SQL_C_WCHAR, m_CharSize);

        string wstringColumn2Name = "WStringColumn2";
        InitializeColumn(1, wstringColumn2Name, SQL_C_WCHAR, m_CharSize);

        string wstringColumn3Name = "WStringColumn3";
        InitializeColumn(2, wstringColumn3Name, SQL_C_WCHAR, m_CharSize);

        // Test data including: basic strings, empty, null, accented chars, emojis, mixed scripts
        //
        vector<const wchar_t*> wstringCol1{
            L"Hello",
            L"Café",           // Accented characters
            L"Hello世界",       // Mixed Latin and Chinese
            L"\U0001F600Test", // Emoji (surrogate pair) + text
            L"naïve"           // More accented chars
        };
        vector<const wchar_t*> wstringCol2{
            L"",               // Empty string
            nullptr,           // Null value
            L"€100£50¥",       // Currency symbols
            L"verify",
            L"HELLO"           // Uppercase (for case sensitivity check)
        };

        int rowsNumber = wstringCol1.size();

        vector<SQLINTEGER> strLenOrIndCol1 =
        { static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),   // "Hello"
          static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),   // "Café"
          static_cast<SQLINTEGER>(7 * sizeof(wchar_t)),   // "Hello世界"
          static_cast<SQLINTEGER>(6 * sizeof(wchar_t)),   // emoji(2) + "Test"(4)
          static_cast<SQLINTEGER>(5 * sizeof(wchar_t)) }; // "naïve"

        vector<SQLINTEGER> strLenOrIndCol2 =
        { 0,
          SQL_NULL_DATA,
          static_cast<SQLINTEGER>(8 * sizeof(wchar_t)),   // "€100£50¥"
          static_cast<SQLINTEGER>(6 * sizeof(wchar_t)),   // "verify"
          static_cast<SQLINTEGER>(5 * sizeof(wchar_t)) }; // "HELLO"

        vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1.data(),
            strLenOrIndCol2.data(), nullptr };

        // Coalesce the arrays of each row of each column
        // into a contiguous array for each column.
        //
        vector<wchar_t> wstringCol1Data = GenerateContiguousData<wchar_t>(wstringCol1, strLenOrIndCol1.data());
        vector<wchar_t> wstringCol2Data = GenerateContiguousData<wchar_t>(wstringCol2, strLenOrIndCol2.data());

        void* dataSet[] = { wstringCol1Data.data(),
                            wstringCol2Data.data(),
                            nullptr };

        vector<string> columnNames{ wstringColumn1Name, wstringColumn2Name, wstringColumn3Name };

        Execute<wchar_t, SQL_C_WCHAR>(
            rowsNumber,
            dataSet,
            strLen_or_Ind.data(),
            columnNames);
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteForwardsSessionTaggedLogEvent
    //
    // Description:
    //  Drives a session with a known session ID and task ID whose executor logs through the SDK
    //  ExtensionEventLogger facade, and verifies the forwarded XEvent carries that
    //  session's ID and task ID. This exercises the AsyncLocal session tagging in
    //  Logging/CSharpSession.Execute end to end.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteForwardsSessionTaggedLogEvent)
    {
        ASSERT_EQ(RegisterTestLogXEventCallback(sm_libHandle), SQL_SUCCESS);

        // Known, non-zero identity so it can't be confused with the empty GUID used
        // by the registration-time "extension loaded" event.
        // Equivalent to GUID 0A0B0C0D-1011-1213-1415-161718191A1B.
        //
        const SQLGUID sessionId = {
            0x0A0B0C0D, 0x1011, 0x1213,
            { 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B } };
        const SQLUSMALLINT taskId = 1;

        *m_sessionId = sessionId;
        m_taskId     = taskId;

        string scriptString = m_UserLibName + m_Separator + c_logInformationExecutor;
        InitializeSession(0, 0, scriptString);

        // Observe only the events emitted by the execution below.
        //
        g_capturedLogEvents.clear();

        SQLUSMALLINT outputSchemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            0,       // rowsNumber
            nullptr, // dataSet
            nullptr, // strLenOrInd
            &outputSchemaColumnsNumber);
        ASSERT_EQ(result, SQL_SUCCESS);

        const CapturedLogEvent *tagged = FindLogInformationEvent();
        ASSERT_NE(tagged, nullptr)
            << "Executor's ExtensionEventLogger event was not forwarded to the host callback";
        EXPECT_TRUE(SqlGuidEquals(tagged->sessionId, sessionId))
            << "Forwarded event did not carry the executing session's ID";
        EXPECT_EQ(tagged->taskId, taskId);
        EXPECT_EQ(tagged->traceLevel, static_cast<SQLUSMALLINT>(Extension_Information));
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteForwardsNamedExtensionLogEvent
    //
    // Description:
    //  Drives an executor that logs through the ExtensionEventLogger.Log extensionName
    //  overload and verifies the forwarded XEvent carries the caller-supplied extension
    //  name rather than the extension's default. Covers the SDK facade path an in-process
    //  library (e.g. MssqlAI) uses to attribute its own activity events.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteForwardsNamedExtensionLogEvent)
    {
        ASSERT_EQ(RegisterTestLogXEventCallback(sm_libHandle), SQL_SUCCESS);

        const string namedExecutor =
            "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorLogNamedExtension";
        const string namedMessage = "CSharpTestExecutorLogNamedExtension emitted event";
        const string expectedExtensionName = "TestExtension";

        string scriptString = m_UserLibName + m_Separator + namedExecutor;
        InitializeSession(0, 0, scriptString);

        g_capturedLogEvents.clear();

        SQLUSMALLINT outputSchemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            0,       // rowsNumber
            nullptr, // dataSet
            nullptr, // strLenOrInd
            &outputSchemaColumnsNumber);
        ASSERT_EQ(result, SQL_SUCCESS);

        const CapturedLogEvent *named = nullptr;
        for (const CapturedLogEvent &ev : g_capturedLogEvents)
        {
            if (ev.message.find(namedMessage) != string::npos)
            {
                named = &ev;
                break;
            }
        }

        ASSERT_NE(named, nullptr)
            << "Named-extension ExtensionEventLogger event was not forwarded to the host callback";
        EXPECT_EQ(named->extensionName, expectedExtensionName)
            << "Forwarded event did not carry the caller-supplied extension name";
        EXPECT_EQ(named->traceLevel, static_cast<SQLUSMALLINT>(Extension_Information));
    }

    //----------------------------------------------------------------------------------------------
    // Name: ExecuteIsolatesSessionTaggingBetweenSessions
    //
    // Description:
    //  Runs two sessions with distinct session IDs and task IDs through the same logging executor
    //  and verifies each forwarded event carries its own session's ID and task ID, proving
    //  the AsyncLocal session context is isolated per execution rather than leaking
    //  from one session into the next.
    //
    TEST_F(CSharpExtensionApiTests, ExecuteIsolatesSessionTaggingBetweenSessions)
    {
        ASSERT_EQ(RegisterTestLogXEventCallback(sm_libHandle), SQL_SUCCESS);

        const string scriptString = m_UserLibName + m_Separator + c_logInformationExecutor;
        SQLUSMALLINT outputSchemaColumnsNumber = 0;

        // First session's ID and task ID.
        // Equivalent to GUID 0A0B0C0D-1011-1213-1415-161718191A1B.
        //
        const SQLGUID sessionIdA = {
            0x0A0B0C0D, 0x1011, 0x1213,
            { 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B } };
        const SQLUSMALLINT taskIdA = 1;

        *m_sessionId = sessionIdA;
        m_taskId     = taskIdA;
        InitializeSession(0, 0, scriptString);

        g_capturedLogEvents.clear();
        ASSERT_EQ(
            (*sm_executeFuncPtr)(
                *m_sessionId, m_taskId, 0, nullptr, nullptr, &outputSchemaColumnsNumber),
            SQL_SUCCESS);

        const CapturedLogEvent *eventA = FindLogInformationEvent();
        ASSERT_NE(eventA, nullptr);
        EXPECT_TRUE(SqlGuidEquals(eventA->sessionId, sessionIdA));
        EXPECT_EQ(eventA->taskId, taskIdA);

        // Tear down the first session before starting the second.
        //
        SessionCleanup();

        // Second session's ID and task ID.
        // Equivalent to GUID F0E1D2C3-B4A5-9687-7869-5A4B3C2D1E0F.
        //
        const SQLGUID sessionIdB = {
            0xF0E1D2C3, 0xB4A5, 0x9687,
            { 0x78, 0x69, 0x5A, 0x4B, 0x3C, 0x2D, 0x1E, 0x0F } };
        const SQLUSMALLINT taskIdB = 2;

        *m_sessionId = sessionIdB;
        m_taskId     = taskIdB;
        InitializeSession(0, 0, scriptString);

        g_capturedLogEvents.clear();
        ASSERT_EQ(
            (*sm_executeFuncPtr)(
                *m_sessionId, m_taskId, 0, nullptr, nullptr, &outputSchemaColumnsNumber),
            SQL_SUCCESS);

        const CapturedLogEvent *eventB = FindLogInformationEvent();
        ASSERT_NE(eventB, nullptr);
        EXPECT_TRUE(SqlGuidEquals(eventB->sessionId, sessionIdB));
        EXPECT_EQ(eventB->taskId, taskIdB);

        // The two sessions must have produced distinct tags.
        //
        EXPECT_FALSE(SqlGuidEquals(sessionIdA, sessionIdB));
        EXPECT_NE(taskIdA, taskIdB);

        // TearDown cleans up the second session.
    }

    //----------------------------------------------------------------------------------------------
    // Name: Execute
    //
    // Description:
    //  Template function to Test Execute with script that contains user executor class full name
    //
    template<class SQLType, SQLSMALLINT dataType>
    void CSharpExtensionApiTests::Execute(
        SQLULEN        rowsNumber,
        void           **dataSet,
        SQLINTEGER     **strLen_or_Ind,
        vector<string> columnNames,
        SQLRETURN      SQLResult)
    {
        testing::internal::CaptureStdout();
        testing::internal::CaptureStderr();
        SQLUSMALLINT outputschemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            rowsNumber,
            dataSet,
            strLen_or_Ind,
            &outputschemaColumnsNumber);

        EXPECT_EQ(result, SQLResult);

        string output = testing::internal::GetCapturedStdout();
        string error = testing::internal::GetCapturedStderr();
        if(result == SQL_SUCCESS)
        {
            EXPECT_TRUE(output.find("Hello .NET Core CSharpExtension!") != string::npos);
        }
        else
        {
            // On failure, surface the captured stdout/stderr so logs show the actual extension output.
            //
            bool matched = error.find("Unable to find user class with full name:") != string::npos;
            EXPECT_TRUE(matched)
                << "Expected stderr to contain 'Unable to find user class with full name:'.\n"
                << "----- Captured stdout -----\n" << output << "\n"
                << "----- Captured stderr -----\n" << error << "\n"
                << "---------------------------";
        }
    }

    //----------------------------------------------------------------------------------------------
    // Name: Execute<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC> (Explicit Template Instantiation)
    //
    // Description:
    //  Explicit template instantiation for Execute function with SQL_NUMERIC_STRUCT type.
    //  Required for linking decimal/numeric column tests that use SQL_C_NUMERIC data type.
    //
    template void CSharpExtensionApiTests::Execute<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(
        SQLULEN        rowsNumber,
        void           **dataSet,
        SQLINTEGER     **strLen_or_Ind,
        vector<string> columnNames,
        SQLRETURN      SQLResult);
}
