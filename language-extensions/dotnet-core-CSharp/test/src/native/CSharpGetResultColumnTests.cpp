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
    //  Note: Without explicit OutputColumnDataTypes configuration, string columns
    //  default to SQL_C_CHAR (VARCHAR) output, regardless of input type.
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

        // NVARCHAR input columns are converted to SQL_C_CHAR output by default.
        // To preserve NVARCHAR, use OutputColumnDataTypes["columnName"] = SqlDataType.DotNetWChar.
        // For ASCII strings, UTF-8 byte length == character count.
        //
        SQLULEN maxCol1CharCount = GetMaxLength(strLenOrIndCol1.data(), rowsNumber) / sizeof(wchar_t);
        SQLULEN maxCol2CharCount = GetMaxLength(strLenOrIndCol2.data(), rowsNumber) / sizeof(wchar_t);

        // Output is SQL_C_CHAR (VARCHAR default), column size is max character count
        //
        GetResultColumn(
            0,                                           // columnNumber
            SQL_C_CHAR,                                  // dataType (default VARCHAR)
            maxCol1CharCount,                            // columnSize (character count)
            0,                                           // decimalDigits
            SQL_NO_NULLS);                               // nullable

        GetResultColumn(
            1,                                           // columnNumber
            SQL_C_CHAR,                                  // dataType (default VARCHAR)
            maxCol2CharCount,                            // columnSize (character count)
            0,                                           // decimalDigits
            SQL_NULLABLE);                               // nullable

        GetResultColumn(
            2,                                           // columnNumber
            SQL_C_CHAR,                                  // dataType (default VARCHAR)
            1,                                           // columnSize (minimum 1 for null column)
            0,                                           // decimalDigits
            SQL_NULLABLE);                               // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetNVarcharOutputResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with an InputDataSet of nvarchar columns where the executor
    //  explicitly specifies NVARCHAR output using OutputColumnDataTypes["text"] = SqlDataType.DotNetWChar.
    //  This verifies that user-specified column metadata is correctly applied.
    //
    TEST_F(CSharpExtensionApiTests, GetNVarcharOutputResultColumnsTest)
    {
        // Use the executor that explicitly sets OutputColumnDataTypes["text"] = SqlDataType.DotNetWChar
        //
        string scriptNVarcharOutput = m_UserLibName + m_Separator +
            "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorNVarcharOutput";

        InitializeSession(
            1, // inputSchemaColumnsNumber
            0, // parametersNumber
            scriptNVarcharOutput);

        string textColumnName = "text";
        InitializeColumn(0, textColumnName, SQL_C_WCHAR, m_CharSize);

        // Input data as UTF-16 (nvarchar)
        //
        vector<const wchar_t*> wstringCol1{ L"Hello", L"World", L"Test", L"Unicode", L"\x4F60\x597D" };  // Last one is Chinese for "Hello"
        int rowsNumber = wstringCol1.size();

        vector<SQLINTEGER> strLenOrIndCol1 =
        { static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),   // "Hello"
          static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),   // "World"
          static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),   // "Test"
          static_cast<SQLINTEGER>(7 * sizeof(wchar_t)),   // "Unicode"
          static_cast<SQLINTEGER>(2 * sizeof(wchar_t)) }; // Chinese chars

        vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1.data() };

        vector<wchar_t> wstringCol1Data = GenerateContiguousData<wchar_t>(wstringCol1, strLenOrIndCol1.data());

        void* dataSet[] = { wstringCol1Data.data() };

        vector<string> columnNames{ textColumnName };

        Execute<wchar_t, SQL_C_WCHAR>(
            rowsNumber,
            dataSet,
            strLen_or_Ind.data(),
            columnNames);

        // With OutputColumnDataTypes["text"] = SqlDataType.DotNetWChar, output should be SQL_C_WCHAR
        // Column size is max byte count (matching extension host expectations)
        //
        SQLULEN maxCol1ByteLen = 7 * sizeof(wchar_t);  // "Unicode" is the longest at 7 characters * 2 bytes

        GetResultColumn(
            0,                                           // columnNumber
            SQL_C_WCHAR,                                 // dataType (UTF-16 output due to NVarchar setting)
            maxCol1ByteLen,                              // columnSize (byte count)
            0,                                           // decimalDigits
            SQL_NO_NULLS);                               // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetPreserveNVarcharTypeResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn where input columns are NVARCHAR but output defaults to VARCHAR
    //  since no explicit OutputColumnDataTypes configuration is provided.
    //  Uses CSharpTestExecutorPreserveInputTypes which doesn't set explicit column metadata.
    //
    TEST_F(CSharpExtensionApiTests, GetPreserveNVarcharTypeResultColumnsTest)
    {
        // Use the executor that just returns input unchanged without explicit OutputColumnDataTypes config
        //
        string scriptPreserve = m_UserLibName + m_Separator +
            "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorPreserveInputTypes";

        InitializeSession(
            2, // inputSchemaColumnsNumber
            0, // parametersNumber
            scriptPreserve);

        string nvarcharColumnName = "nvarchar_col";
        string varcharColumnName = "varchar_col";
        InitializeColumn(0, nvarcharColumnName, SQL_C_WCHAR, m_CharSize);  // NVARCHAR input
        InitializeColumn(1, varcharColumnName, SQL_C_CHAR, m_CharSize);    // VARCHAR input

        // Input data
        //
        vector<const wchar_t*> wstringCol{ L"Hello", L"World", L"Test", L"Data", L"Row5" };
        vector<const char*> stringCol{ "ASCII1", "ASCII2", "ASCII3", "ASCII4", "ASCII5" };
        int rowsNumber = wstringCol.size();

        vector<SQLINTEGER> strLenOrIndCol1 =
        { static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(4 * sizeof(wchar_t)) };

        vector<SQLINTEGER> strLenOrIndCol2 =
        { 6, 6, 6, 6, 6 };  // All strings are 6 bytes

        vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1.data(), strLenOrIndCol2.data() };

        vector<wchar_t> wstringColData = GenerateContiguousData<wchar_t>(wstringCol, strLenOrIndCol1.data());
        vector<char> stringColData = GenerateContiguousData<char>(stringCol, strLenOrIndCol2.data());

        void* dataSet[] = { wstringColData.data(), stringColData.data() };

        vector<string> columnNames{ nvarcharColumnName, varcharColumnName };

        Execute<wchar_t, SQL_C_WCHAR>(
            rowsNumber,
            dataSet,
            strLen_or_Ind.data(),
            columnNames);

        // Column 0: Input was NVARCHAR, but output defaults to SQL_C_CHAR (VARCHAR)
        // since no OutputColumnDataTypes configuration is provided.
        //
        GetResultColumn(
            0,                                           // columnNumber
            SQL_C_CHAR,                                  // dataType (default VARCHAR)
            5,                                           // columnSize (max char count)
            0,                                           // decimalDigits
            SQL_NO_NULLS);                               // nullable

        // Column 1: Input was VARCHAR, output should be SQL_C_CHAR
        //
        GetResultColumn(
            1,                                           // columnNumber
            SQL_C_CHAR,                                  // dataType (preserved from input)
            6,                                           // columnSize (max byte length)
            0,                                           // decimalDigits
            SQL_NO_NULLS);                               // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetMixedStringOutputResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with mixed VARCHAR and NVARCHAR output columns.
    //  Uses CSharpTestExecutorMixedStringOutput which sets OutputColumnDataTypes
    //  for "unicode_col" only, leaving "ascii_col" as default VARCHAR.
    //
    TEST_F(CSharpExtensionApiTests, GetMixedStringOutputResultColumnsTest)
    {
        // Use the executor that sets OutputColumnDataTypes["unicode_col"] = SqlDataType.DotNetWChar
        //
        string scriptMixed = m_UserLibName + m_Separator +
            "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorMixedStringOutput";

        InitializeSession(
            2, // inputSchemaColumnsNumber
            0, // parametersNumber
            scriptMixed);

        string asciiColumnName = "ascii_col";
        string unicodeColumnName = "unicode_col";
        InitializeColumn(0, asciiColumnName, SQL_C_CHAR, m_CharSize);
        InitializeColumn(1, unicodeColumnName, SQL_C_WCHAR, m_CharSize);

        // Input data
        //
        vector<const char*> stringCol{ "Hello", "World", "Test", "Data!", "Row05" };
        vector<const wchar_t*> wstringCol{ L"Alpha", L"Beta", L"Gamma", L"Delta", L"Omega" };
        int rowsNumber = stringCol.size();

        vector<SQLINTEGER> strLenOrIndCol1 =
        { 5, 5, 4, 5, 5 };

        vector<SQLINTEGER> strLenOrIndCol2 =
        { static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(5 * sizeof(wchar_t)) };

        vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1.data(), strLenOrIndCol2.data() };

        vector<char> stringColData = GenerateContiguousData<char>(stringCol, strLenOrIndCol1.data());
        vector<wchar_t> wstringColData = GenerateContiguousData<wchar_t>(wstringCol, strLenOrIndCol2.data());

        void* dataSet[] = { stringColData.data(), wstringColData.data() };

        vector<string> columnNames{ asciiColumnName, unicodeColumnName };

        Execute<SQLCHAR, SQL_C_CHAR>(
            rowsNumber,
            dataSet,
            strLen_or_Ind.data(),
            columnNames);

        // Column 0 (ascii_col): No OutputColumnDataTypes config -> default SQL_C_CHAR (VARCHAR)
        //
        GetResultColumn(
            0,                                           // columnNumber
            SQL_C_CHAR,                                  // dataType (default VARCHAR)
            5,                                           // columnSize (max byte length)
            0,                                           // decimalDigits
            SQL_NO_NULLS);                               // nullable

        // Column 1 (unicode_col): Explicitly set to DotNetWChar -> SQL_C_WCHAR (NVARCHAR)
        //
        SQLULEN maxCol2ByteLen = 5 * sizeof(wchar_t);   // "Alpha"/"Gamma"/"Delta"/"Omega" are 5 chars
        GetResultColumn(
            1,                                           // columnNumber
            SQL_C_WCHAR,                                 // dataType (NVARCHAR due to OutputColumnDataTypes)
            maxCol2ByteLen,                              // columnSize (byte count)
            0,                                           // decimalDigits
            SQL_NO_NULLS);                               // nullable
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
