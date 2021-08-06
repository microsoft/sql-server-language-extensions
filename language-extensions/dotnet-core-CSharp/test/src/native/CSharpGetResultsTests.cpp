//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpGetResultsTests.cpp
//
// Purpose:
//  Test the .NET Core CSharp extension GetResults using the Extension API
//
//*********************************************************************
#include "CSharpExtensionApiTests.h"

using namespace std;

namespace ExtensionApiTest
{
    //----------------------------------------------------------------------------------------------
    // Name: GetIntegerResultsTest
    //
    // Description:
    //  Test GetResults with default script expecting an OutputDataSet of Integer columns.
    //
    TEST_F(CSharpExtensionApiTests, GetIntegerResultsTest)
    {
        InitializeSession(
            (*m_integerInfo).GetColumnsNumber(),
            0, // parametersNumber
            m_scriptString);

        InitializeColumns<SQLINTEGER, SQL_C_SLONG>(m_integerInfo.get());

        Execute<SQLINTEGER, SQL_C_SLONG>(
            ColumnInfo<SQLINTEGER>::sm_rowsNumber,
            (*m_integerInfo).m_dataSet.data(),
            (*m_integerInfo).m_strLen_or_Ind.data(),
            (*m_integerInfo).m_columnNames);

        GetResults<SQLINTEGER, SQLINTEGER, SQL_C_SLONG>(
            ColumnInfo<SQLINTEGER>::sm_rowsNumber,
            (*m_integerInfo).m_dataSet.data(),
            (*m_integerInfo).m_strLen_or_Ind.data(),
            (*m_integerInfo).m_columnNames,
            (*m_integerInfo).m_nullable);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetBigIntResultsTest
    //
    // Description:
    //  Test GetResults with default script expecting an OutputDataSet of BigInt columns.
    //
    TEST_F(CSharpExtensionApiTests, GetBigIntResultsTest)
    {
        InitializeSession(
            (*m_bigIntInfo).GetColumnsNumber(),
            0, // parametersNumber
            m_scriptString);

        InitializeColumns<SQLBIGINT, SQL_C_SBIGINT>(m_bigIntInfo.get());

        Execute<SQLBIGINT, SQL_C_SBIGINT>(
            ColumnInfo<SQLBIGINT>::sm_rowsNumber,
            (*m_bigIntInfo).m_dataSet.data(),
            (*m_bigIntInfo).m_strLen_or_Ind.data(),
            (*m_bigIntInfo).m_columnNames);

        GetResults<SQLBIGINT, SQLBIGINT, SQL_C_SBIGINT>(
            ColumnInfo<SQLBIGINT>::sm_rowsNumber,
            (*m_bigIntInfo).m_dataSet.data(),
            (*m_bigIntInfo).m_strLen_or_Ind.data(),
            (*m_bigIntInfo).m_columnNames,
            (*m_bigIntInfo).m_nullable);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetSmallIntResultsTest
    //
    // Description:
    //  Test GetResults with default script expecting an OutputDataSet of SmallInt columns.
    //
    TEST_F(CSharpExtensionApiTests, GetSmallIntResultsTest)
    {
        InitializeSession(
            (*m_smallIntInfo).GetColumnsNumber(),
            0, // parametersNumber
            m_scriptString);

        InitializeColumns<SQLSMALLINT, SQL_C_SSHORT>(m_smallIntInfo.get());

        Execute<SQLSMALLINT, SQL_C_SSHORT>(
            ColumnInfo<SQLSMALLINT>::sm_rowsNumber,
            (*m_smallIntInfo).m_dataSet.data(),
            (*m_smallIntInfo).m_strLen_or_Ind.data(),
            (*m_smallIntInfo).m_columnNames);

        GetResults<SQLSMALLINT, SQLSMALLINT, SQL_C_SSHORT>(
            ColumnInfo<SQLSMALLINT>::sm_rowsNumber,
            (*m_smallIntInfo).m_dataSet.data(),
            (*m_smallIntInfo).m_strLen_or_Ind.data(),
            (*m_smallIntInfo).m_columnNames,
            (*m_smallIntInfo).m_nullable);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetTinyIntResultsTest
    //
    // Description:
    //  Test GetResults with default script expecting an OutputDataSet of TinyInt columns.
    //
    TEST_F(CSharpExtensionApiTests, GetTinyIntResultsTest)
    {
        InitializeSession(
            (*m_tinyIntInfo).GetColumnsNumber(),
            0, // parametersNumber
            m_scriptString);

        InitializeColumns<SQLCHAR, SQL_C_UTINYINT>(m_tinyIntInfo.get());

        Execute<SQLCHAR, SQL_C_UTINYINT>(
            ColumnInfo<SQLCHAR>::sm_rowsNumber,
            (*m_tinyIntInfo).m_dataSet.data(),
            (*m_tinyIntInfo).m_strLen_or_Ind.data(),
            (*m_tinyIntInfo).m_columnNames);

        GetResults<SQLCHAR, SQLCHAR, SQL_C_UTINYINT>(
            ColumnInfo<SQLCHAR>::sm_rowsNumber,
            (*m_tinyIntInfo).m_dataSet.data(),
            (*m_tinyIntInfo).m_strLen_or_Ind.data(),
            (*m_tinyIntInfo).m_columnNames,
            (*m_tinyIntInfo).m_nullable);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetBooleanResultsTest
    //
    // Description:
    //  Test GetResults with default script expecting an OutputDataSet of Boolean columns.
    //
    TEST_F(CSharpExtensionApiTests, GetBooleanResultsTest)
    {
        InitializeSession(
            (*m_booleanInfo).GetColumnsNumber(),
            0, // parametersNumber
            m_scriptString);

        InitializeColumns<SQLCHAR, SQL_C_BIT>(m_booleanInfo.get());

        Execute<SQLCHAR, SQL_C_BIT>(
            ColumnInfo<SQLCHAR>::sm_rowsNumber,
            (*m_booleanInfo).m_dataSet.data(),
            (*m_booleanInfo).m_strLen_or_Ind.data(),
            (*m_booleanInfo).m_columnNames);

        GetResults<SQLCHAR, SQLCHAR, SQL_C_BIT>(
            ColumnInfo<SQLCHAR>::sm_rowsNumber,
            (*m_booleanInfo).m_dataSet.data(),
            (*m_booleanInfo).m_strLen_or_Ind.data(),
            (*m_booleanInfo).m_columnNames,
            (*m_booleanInfo).m_nullable);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetDoubleResultsTest
    //
    // Description:
    //  Test GetResults with default script expecting an OutputDataSet of Double columns.
    //
    TEST_F(CSharpExtensionApiTests, GetDoubleResultsTest)
    {
        InitializeSession(
            (*m_doubleInfo).GetColumnsNumber(),
            0, // parametersNumber
            m_scriptString);

        InitializeColumns<SQLDOUBLE, SQL_C_DOUBLE>(m_doubleInfo.get());

        Execute<SQLDOUBLE, SQL_C_DOUBLE>(
            ColumnInfo<SQLDOUBLE>::sm_rowsNumber,
            (*m_doubleInfo).m_dataSet.data(),
            (*m_doubleInfo).m_strLen_or_Ind.data(),
            (*m_doubleInfo).m_columnNames);

        GetResults<SQLDOUBLE, SQLDOUBLE, SQL_C_DOUBLE>(
            ColumnInfo<SQLDOUBLE>::sm_rowsNumber,
            (*m_doubleInfo).m_dataSet.data(),
            (*m_doubleInfo).m_strLen_or_Ind.data(),
            (*m_doubleInfo).m_columnNames,
            (*m_doubleInfo).m_nullable);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetRealResultsTest
    //
    // Description:
    //  Test GetResults with default script expecting an OutputDataSet of Real columns.
    //
    TEST_F(CSharpExtensionApiTests, GetRealResultsTest)
    {
        InitializeSession(
            (*m_realInfo).GetColumnsNumber(),
            0, // parametersNumber
            m_scriptString);

        InitializeColumns<SQLREAL, SQL_C_FLOAT>(m_realInfo.get());

        Execute<SQLREAL, SQL_C_FLOAT>(
            ColumnInfo<SQLREAL>::sm_rowsNumber,
            (*m_realInfo).m_dataSet.data(),
            (*m_realInfo).m_strLen_or_Ind.data(),
            (*m_realInfo).m_columnNames);

        GetResults<SQLREAL, SQLREAL, SQL_C_FLOAT>(
            ColumnInfo<SQLREAL>::sm_rowsNumber,
            (*m_realInfo).m_dataSet.data(),
            (*m_realInfo).m_strLen_or_Ind.data(),
            (*m_realInfo).m_columnNames,
            (*m_realInfo).m_nullable);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetStringResultsTest
    //
    // Description:
    //  Test GetResults with default script expecting an OutputDataSet of String columns.
    //
    TEST_F(CSharpExtensionApiTests, GetStringResultsTest)
    {
        SQLUSMALLINT inputSchemaColumnsNumber = 3;

        InitializeSession(
            inputSchemaColumnsNumber,
            0, // parametersNumber
            m_scriptString);

        string stringColumn1Name = "StringColumn1";
        InitializeColumn(0, stringColumn1Name, SQL_C_CHAR, m_CharSize);

        string stringColumn2Name = "StringColumn2";
        InitializeColumn(1, stringColumn2Name, SQL_C_CHAR, m_CharSize);

        string stringColumn3Name = "StringColumn3";
        InitializeColumn(2, stringColumn3Name, SQL_C_CHAR, m_CharSize);

        string goodUTF8 = string("a") + "\xE2" + "\x82" + "\xAC";

        vector<const char*> stringCol1{ "Hello", "test", "data", "World", "-123" };
        vector<const char*> stringCol2{ "", 0, nullptr, "verify", goodUTF8.c_str() };

        int rowsNumber = stringCol1.size();

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
        vector<SQLINTEGER> strLenOrIndCol3(rowsNumber, SQL_NULL_DATA);

        vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1.data(),
            strLenOrIndCol2.data(), strLenOrIndCol3.data() };

        // Coalesce the arrays of each row of each column
        // into a contiguous array for each column.
        //
        vector<char> stringCol1Data =
            GenerateContiguousData<char>(stringCol1, strLenOrIndCol1.data());

        vector<char> stringCol2Data =
            GenerateContiguousData<char>(stringCol2, strLenOrIndCol2.data());

        void* dataSet[] = { stringCol1Data.data(),
                            stringCol2Data.data(),
                            nullptr };

        vector<string> columnNames{ stringColumn1Name, stringColumn2Name, stringColumn3Name };

        Execute<SQLCHAR, SQL_C_CHAR>(
            rowsNumber,
            dataSet,
            strLen_or_Ind.data(),
            columnNames);

        GetStringResults(
            rowsNumber,
            dataSet,
            strLen_or_Ind.data(),
            columnNames);
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::GetResults
    //
    // Description:
    //  Tests GetResults to verify the expected results are obtained.
    //  For numeric, logical and integer types.
    //
    template<class InputSQLType, class OutputSQLType, SQLSMALLINT OutputDataType>
    void CSharpExtensionApiTests::GetResults(
        SQLULEN             expectedRowsNumber,
        SQLPOINTER          *expectedData,
        SQLINTEGER          **expectedStrLen_or_Ind,
        vector<string>      columnNames,
        vector<SQLSMALLINT> nullables)
    {
        SQLULEN    rowsNumber = 0;
        SQLPOINTER *data = nullptr;
        SQLINTEGER **strLen_or_Ind = nullptr;

        SQLRETURN  result = (*sm_getResultsFuncPtr)(
            *m_sessionId,
            m_taskId,
            &rowsNumber,
            &data,
            &strLen_or_Ind);
        ASSERT_EQ(result, SQL_SUCCESS);

        EXPECT_EQ(rowsNumber, expectedRowsNumber);

        // Test data obtained is same as the expectedData and the OutputDataSet.
        //
        for (SQLUSMALLINT columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
        {
            SQLINTEGER *expectedColumnStrLenOrInd = expectedStrLen_or_Ind[columnNumber];
            SQLINTEGER *columnStrLenOrInd = strLen_or_Ind[columnNumber];

            InputSQLType *expectedColumnData = static_cast<InputSQLType *>(expectedData[columnNumber]);
            InputSQLType *columnData = static_cast<InputSQLType *>(data[columnNumber]);

            CheckColumnDataEquality<InputSQLType, OutputSQLType, OutputDataType>(
                rowsNumber,
                expectedColumnData,
                columnData,
                expectedColumnStrLenOrInd,
                columnStrLenOrInd,
                nullables[columnNumber]);
        }
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::CheckColumnDataEquality
    //
    // Description:
    //  Templatized function to compare the given column data
    //  and nullMap with rowsNumber for equality.
    //
    template<class InputSQLType, class OutputSQLType, SQLSMALLINT OutputDataType>
    void CSharpExtensionApiTests::CheckColumnDataEquality(
        SQLULEN        rowsNumber,
        InputSQLType   *expectedColumnData,
        OutputSQLType  *columnData,
        SQLINTEGER     *expectedColumnStrLenOrInd,
        SQLINTEGER     *columnStrLenOrInd,
        SQLSMALLINT    nullable)
    {
        if (rowsNumber == 0)
        {
            EXPECT_EQ(columnData, nullptr);
            EXPECT_EQ(columnStrLenOrInd, nullptr);
        }

        for (SQLULEN index = 0; index < rowsNumber; ++index)
        {
            if (nullable == SQL_NULLABLE)
            {
                EXPECT_EQ(columnStrLenOrInd[index], expectedColumnStrLenOrInd[index]);
            }

            if (columnStrLenOrInd[index] == SQL_NULL_DATA)
            {
                if constexpr (is_same_v<OutputSQLType, SQLDOUBLE>)
                {
                    EXPECT_TRUE(isnan(static_cast<SQLDOUBLE>(columnData[index])));
                }
                else if constexpr (is_same_v<OutputSQLType, SQLREAL>)
                {
                    EXPECT_TRUE(isnan(static_cast<SQLREAL>(columnData[index])));
                }
                else
                {
                    if (nullable == SQL_NO_NULLS && is_same_v<InputSQLType, OutputSQLType>)
                    {
                        EXPECT_EQ(columnData[index], expectedColumnData[index]);
                    }
                }
            }
            else
            {
                EXPECT_EQ(columnData[index], expectedColumnData[index]);
            }
        }
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::GetStringResults
    //
    // Description:
    //  Test GetResults to verify the expected results are obtained for character data.
    //
    void CSharpExtensionApiTests::GetStringResults(
        SQLULEN        expectedRowsNumber,
        SQLPOINTER     *expectedData,
        SQLINTEGER     **expectedStrLen_or_Ind,
        vector<string> columnNames)
    {
        SQLULEN    rowsNumber = 0;
        SQLPOINTER *data = nullptr;
        SQLINTEGER **strLen_or_Ind = nullptr;
        SQLRETURN result = (*sm_getResultsFuncPtr)(
            *m_sessionId,
            m_taskId,
            &rowsNumber,
            &data,
            &strLen_or_Ind);
        ASSERT_EQ(result, SQL_SUCCESS);

        EXPECT_EQ(rowsNumber, expectedRowsNumber);

        for (SQLUSMALLINT columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
        {
            char *expectedColumnData = static_cast<char *>(expectedData[columnNumber]);
            char *columnData = static_cast<char *>(data[columnNumber]);

            SQLINTEGER *expectedColumnStrLenOrInd = expectedStrLen_or_Ind[columnNumber];
            SQLINTEGER *columnStrLenOrInd = strLen_or_Ind[columnNumber];

            CheckStringDataEquality(
                rowsNumber,
                expectedColumnData,
                columnData,
                expectedColumnStrLenOrInd,
                columnStrLenOrInd);
        }
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::CheckStringDataEquality
    //
    // Description:
    //  Compare the given character data & nullMap with rowsNumber for equality.
    //
    void CSharpExtensionApiTests::CheckStringDataEquality(
        SQLULEN    rowsNumber,
        char       *expectedColumnData,
        char       *columnData,
        SQLINTEGER *expectedColumnStrLenOrInd,
        SQLINTEGER *columnStrLenOrInd)
    {
        SQLINTEGER cumulativeLength = 0;
        if (rowsNumber == 0)
        {
            EXPECT_EQ(columnData, nullptr);
            EXPECT_EQ(columnStrLenOrInd, nullptr);
        }

        for (SQLULEN index = 0; index < rowsNumber; ++index)
        {
            if (expectedColumnStrLenOrInd != nullptr)
            {
                EXPECT_EQ(columnStrLenOrInd[index], expectedColumnStrLenOrInd[index]);

                if (columnStrLenOrInd[index] != SQL_NULL_DATA)
                {
                    // Compare the two strings byte by byte
                    // because encoded strings mess up EXPECT_EQ
                    //
                    for (SQLINTEGER strIndex = 0; strIndex < columnStrLenOrInd[index]; ++strIndex)
                    {
                        EXPECT_EQ((expectedColumnData + cumulativeLength)[strIndex],
                            (columnData + cumulativeLength)[strIndex]);
                    }

                    cumulativeLength += expectedColumnStrLenOrInd[index];
                }
            }
            else
            {
                EXPECT_EQ(columnStrLenOrInd[index], SQL_NULL_DATA);
            }
        }
    }
}
