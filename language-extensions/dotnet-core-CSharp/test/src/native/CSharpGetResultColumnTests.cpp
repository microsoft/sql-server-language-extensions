//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpGetResultColumnTests.cpp
//
// Purpose:
//  Tests the CSharp Extension's implementation of the external language GetResultColumn API.
//
//*********************************************************************
#include "CSharpExtensionApiTests.h"

using namespace std;

namespace ExtensionApiTest
{
    //----------------------------------------------------------------------------------------------
    // Name: GetIntegerResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with default script expecting an OutputDataSet of Integer columns.
    //
    TEST_F(CSharpExtensionApiTests, GetIntegerResultColumnsTest)
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

        GetResultColumn(
            0,                  // columnNumber
            SQL_C_SLONG,        // dataType
            m_IntSize,          // columnSize
            0,                  // decimalDigits
            SQL_NO_NULLS);      // nullable

        GetResultColumn(
            1,                  // columnNumber
            SQL_C_SLONG,        // dataType
            m_IntSize,          // columnSize
            0,                  // decimalDigits
            SQL_NULLABLE);      // nullable

        GetResultColumn(
            2,                  // columnNumber
            SQL_C_SLONG,        // dataType
            m_IntSize,          // columnSize
            0,                  // decimalDigits
            SQL_NULLABLE,       // nullable
            SQL_ERROR);         // Invalid Column Number

        GetResultColumn(
            -1,                 // columnNumber
            SQL_C_SLONG,        // dataType
            m_IntSize,          // columnSize
            0,                  // decimalDigits
            SQL_NULLABLE,       // nullable
            SQL_ERROR);         // Negative Column Number
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetBooleanResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with default script expecting an OutputDataSet of Boolean columns.
    //
    TEST_F(CSharpExtensionApiTests, GetBooleanResultColumnsTest)
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

        GetResultColumn(
            0,             // columnNumber
            SQL_C_BIT,     // dataType
            m_BooleanSize, // columnSize
            0,             // decimalDigits
            SQL_NO_NULLS); // nullable

        GetResultColumn(
            1,             // columnNumber
            SQL_C_BIT,     // dataType
            m_BooleanSize, // columnSize
            0,             // decimalDigits
            SQL_NULLABLE); // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetBigIntResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with default script expecting an OutputDataSet of BigInt columns.
    //
    TEST_F(CSharpExtensionApiTests, GetBigIntResultColumnsTest)
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

        GetResultColumn(
            0,             // columnNumber
            SQL_C_SBIGINT, // dataType
            m_BigIntSize,  // columnSize
            0,             // decimalDigits
            SQL_NO_NULLS); // nullable

        GetResultColumn(
            1,             // columnNumber
            SQL_C_SBIGINT, // dataType
            m_BigIntSize,  // columnSize
            0,             // decimalDigits
            SQL_NULLABLE); // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetSmallIntResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with default script expecting an OutputDataSet of SmallInt columns.
    //
    TEST_F(CSharpExtensionApiTests, GetSmallIntResultColumnsTest)
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

        GetResultColumn(
            0,              // columnNumber
            SQL_C_SSHORT,   // dataType
            m_SmallIntSize, // columnSize
            0,              // decimalDigits
            SQL_NO_NULLS);  // nullable

        GetResultColumn(
            1,              // columnNumber
            SQL_C_SSHORT,   // dataType
            m_SmallIntSize, // columnSize
            0,              // decimalDigits
            SQL_NULLABLE);  // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetTinyIntResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with default script expecting an OutputDataSet of TinyInt columns.
    //
    TEST_F(CSharpExtensionApiTests, GetTinyIntResultColumnsTest)
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

        GetResultColumn(
            0,              // columnNumber
            SQL_C_UTINYINT, // dataType
            m_TinyIntSize,  // columnSize
            0,              // decimalDigits
            SQL_NO_NULLS);  // nullable

        GetResultColumn(
            1,              // columnNumber
            SQL_C_UTINYINT, // dataType
            m_TinyIntSize,  // columnSize
            0,              // decimalDigits
            SQL_NULLABLE);  // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetRealResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with default script expecting an OutputDataSet of Real columns.
    //
    TEST_F(CSharpExtensionApiTests, GetRealResultColumnsTest)
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

        GetResultColumn(
            0,             // columnNumber
            SQL_C_FLOAT,   // dataType
            m_RealSize,    // columnSize
            0,             // decimalDigits
            SQL_NO_NULLS); // nullable

        GetResultColumn(
            1,             // columnNumber
            SQL_C_FLOAT,   // dataType
            m_RealSize,    // columnSize
            0,             // decimalDigits
            SQL_NULLABLE); // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetDoubleResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with default script expecting an OutputDataSet of Double columns.
    //
    TEST_F(CSharpExtensionApiTests, GetDoubleResultColumnsTest)
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

        GetResultColumn(
            0,             // columnNumber
            SQL_C_DOUBLE,  // dataType
            m_DoubleSize,  // columnSize
            0,             // decimalDigits
            SQL_NO_NULLS); // nullable

        GetResultColumn(
            1,             // columnNumber
            SQL_C_DOUBLE,  // dataType
            m_DoubleSize,  // columnSize
            0,             // decimalDigits
            SQL_NULLABLE); // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetStringResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with default script expecting an OutputDataSet of String columns.
    //
    TEST_F(CSharpExtensionApiTests, GetStringResultColumnsTest)
    {
        InitializeSession(
            3, // inputSchemaColumnsNumber
            0, // parametersNumber
            m_scriptString);

        string stringColumn1Name = "StringColumn1";
        InitializeColumn(0, stringColumn1Name, SQL_C_CHAR, m_CharSize);

        string stringColumn2Name = "StringColumn2";
        InitializeColumn(1, stringColumn2Name, SQL_C_CHAR, m_CharSize);

        string stringColumn3Name = "StringColumn3";
        InitializeColumn(2, stringColumn3Name, SQL_C_CHAR, m_CharSize);

        vector<const char *> stringCol1{ "Hello", "test", "data", "World", "-123" };
        vector<const char *> stringCol2{ "", 0, nullptr, "verify", "-1" };

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

        vector<SQLINTEGER *> strLen_or_Ind{ strLenOrIndCol1.data(),
            strLenOrIndCol2.data(), nullptr };

        // Coalesce the arrays of each row of each column
        // into a contiguous array for each column.
        //
        vector<char> stringCol1Data = GenerateContiguousData<char>(stringCol1, strLenOrIndCol1.data());
        vector<char> stringCol2Data = GenerateContiguousData<char>(stringCol2, strLenOrIndCol2.data());

        void *dataSet[] = { stringCol1Data.data(),
                            stringCol2Data.data(),
                            nullptr };

        int rowsNumber = stringCol1.size();

        vector<string> columnNames{ stringColumn1Name, stringColumn2Name, stringColumn3Name };

        Execute<SQLCHAR, SQL_C_CHAR>(
            rowsNumber,
            dataSet,
            strLen_or_Ind.data(),
            columnNames);

        SQLULEN maxCol1Len = GetMaxLength(strLenOrIndCol1.data(), rowsNumber);
        SQLULEN maxCol2Len = GetMaxLength(strLenOrIndCol2.data(), rowsNumber);

        GetResultColumn(
            0,                 // columnNumber
            SQL_C_CHAR,        // dataType
            maxCol1Len,        // columnSize
            0,                 // decimalDigits
            SQL_NO_NULLS);     // nullable

        GetResultColumn(
            1,                 // columnNumber
            SQL_C_CHAR,        // dataType
            maxCol2Len,        // columnSize
            0,                 // decimalDigits
            SQL_NULLABLE);     // nullable

        GetResultColumn(
            2,                 // columnNumber
            SQL_C_CHAR,        // dataType
            sizeof(SQLCHAR),   // columnSize
            0,                 // decimalDigits
            SQL_NULLABLE);     // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetWStringResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with an InputDataSet of nvarchar/nchar (Unicode) columns.
    //  Tests nullptr, empty strings, and basic Unicode characters.
    //  Note: Output columns are returned as SQL_C_CHAR (UTF-8 encoded) regardless
    //  of input type, since C# DataFrame internally stores strings as System.String.
    //
    TEST_F(CSharpExtensionApiTests, GetWStringResultColumnsTest)
    {
        InitializeSession(
            3, // inputSchemaColumnsNumber
            0, // parametersNumber
            m_scriptString);

        string wstringColumn1Name = "WStringColumn1";
        InitializeColumn(0, wstringColumn1Name, SQL_C_WCHAR, m_CharSize);

        string wstringColumn2Name = "WStringColumn2";
        InitializeColumn(1, wstringColumn2Name, SQL_C_WCHAR, m_CharSize);

        string wstringColumn3Name = "WStringColumn3";
        InitializeColumn(2, wstringColumn3Name, SQL_C_WCHAR, m_CharSize);

        // Use simple ASCII strings to make UTF-8 size calculation straightforward.
        // For ASCII characters, UTF-8 byte length == character count.
        //
        vector<const wchar_t*> wstringCol1{ L"Hello", L"test", L"data", L"World", L"abc" };
        vector<const wchar_t*> wstringCol2{ L"", nullptr, nullptr, L"verify", L"-1" };

        int rowsNumber = wstringCol1.size();

        vector<SQLINTEGER> strLenOrIndCol1 =
        { static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),   // "Hello"
          static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),   // "test"
          static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),   // "data"
          static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),   // "World"
          static_cast<SQLINTEGER>(3 * sizeof(wchar_t)) }; // "abc"

        vector<SQLINTEGER> strLenOrIndCol2 =
        { 0,
          SQL_NULL_DATA,
          SQL_NULL_DATA,
          static_cast<SQLINTEGER>(6 * sizeof(wchar_t)),   // "verify"
          static_cast<SQLINTEGER>(2 * sizeof(wchar_t)) }; // "-1"

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

        // C# outputs all string columns as SQL_C_CHAR (UTF-8).
        // For ASCII strings, UTF-8 byte length == character count.
        // We divide by sizeof(wchar_t) to get character count from UTF-16 byte length.
        //
        SQLULEN maxCol1Len = GetMaxLength(strLenOrIndCol1.data(), rowsNumber) / sizeof(wchar_t);
        SQLULEN maxCol2Len = GetMaxLength(strLenOrIndCol2.data(), rowsNumber) / sizeof(wchar_t);

        // Output is SQL_C_CHAR (UTF-8), column size is max UTF-8 byte length
        // For ASCII strings, this equals the character count
        //
        GetResultColumn(
            0,                                           // columnNumber
            SQL_C_CHAR,                                  // dataType (UTF-8 output)
            maxCol1Len,                                  // columnSize
            0,                                           // decimalDigits
            SQL_NO_NULLS);                               // nullable

        GetResultColumn(
            1,                                           // columnNumber
            SQL_C_CHAR,                                  // dataType (UTF-8 output)
            maxCol2Len,                                  // columnSize
            0,                                           // decimalDigits
            SQL_NULLABLE);                               // nullable

        GetResultColumn(
            2,                                           // columnNumber
            SQL_C_CHAR,                                  // dataType (UTF-8 output)
            sizeof(SQLCHAR),                             // columnSize (1 for null column)
            0,                                           // decimalDigits
            SQL_NULLABLE);                               // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetResultColumn
    //
    // Description:
    //  Test GetResultColumn to verify the expected result column information is obtained.
    //
    void CSharpExtensionApiTests::GetResultColumn(
        SQLUSMALLINT columnNumber,
        SQLSMALLINT  expectedDataType,
        SQLULEN      expectedColumnSize,
        SQLSMALLINT  expectedDecimalDigits,
        SQLSMALLINT  expectedNullable,
        SQLRETURN    SQLResult)
    {
        SQLSMALLINT dataType = 0;
        SQLULEN columnSize = 0;
        SQLSMALLINT decimalDigits = 0;
        SQLSMALLINT nullable = 0;

        SQLRETURN result = (sm_getResultColumnFuncPtr)(
            *m_sessionId,
            m_taskId,
            columnNumber,
            &dataType,
            &columnSize,
            &decimalDigits,
            &nullable);
        EXPECT_EQ(result, SQLResult);

        if(SQLResult == SQL_SUCCESS)
        {
            EXPECT_EQ(dataType, expectedDataType);
            EXPECT_EQ(columnSize, expectedColumnSize);
            EXPECT_EQ(decimalDigits, expectedDecimalDigits);
            EXPECT_EQ(nullable, expectedNullable);
        }
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetMaxLength
    //
    // Description:
    //  Get max length of all strings from strLenOrInd.
    //
    SQLINTEGER CSharpExtensionApiTests::GetMaxLength(
        SQLINTEGER *strLenOrInd,
        SQLULEN    rowsNumber)
    {
        SQLINTEGER maxLen = 0;
        for (SQLULEN index = 0; index < rowsNumber; ++index)
        {
            if (strLenOrInd[index] != SQL_NULL_DATA && maxLen < strLenOrInd[index])
            {
                maxLen = strLenOrInd[index];
            }
        }

        return maxLen;
    }
}
