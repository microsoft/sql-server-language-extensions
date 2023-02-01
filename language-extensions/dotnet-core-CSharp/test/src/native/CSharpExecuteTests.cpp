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
        // Unmatched library name with the dll file name.
        //
        string userLibName = "Microsoft.SqlServer.CSharpExtensionTest";
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
            EXPECT_TRUE(error.find("Error: Unable to find user class with full name:") != string::npos);
        }
    }
}
