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
    // Name: GetWStringResultsTest
    //
    // Description:
    //  Test GetResults with default script expecting an OutputDataSet of NVarChar/NChar columns.
    //  Input is provided as UTF-16 (SQL_C_WCHAR), output is verified as UTF-8 (SQL_C_CHAR)
    //  since the C# extension always outputs strings as varchar.
    //
    TEST_F(CSharpExtensionApiTests, GetWStringResultsTest)
    {
        SQLUSMALLINT inputSchemaColumnsNumber = 3;

        InitializeSession(
            inputSchemaColumnsNumber,
            0, // parametersNumber
            m_scriptString);

        string wstringColumn1Name = "WStringColumn1";
        InitializeColumn(0, wstringColumn1Name, SQL_C_WCHAR, m_CharSize);

        string wstringColumn2Name = "WStringColumn2";
        InitializeColumn(1, wstringColumn2Name, SQL_C_WCHAR, m_CharSize);

        string wstringColumn3Name = "WStringColumn3";
        InitializeColumn(2, wstringColumn3Name, SQL_C_WCHAR, m_CharSize);

        // Test data with Unicode characters: basic Latin, accented, CJK, Cyrillic
        //
        vector<const wchar_t*> wstringCol1{ L"Hello", L"test", L"data", L"World", L"ABC" };
        vector<const wchar_t*> wstringCol2{ L"", 0, nullptr, L"verify", L"123" };

        int rowsNumber = wstringCol1.size();

        vector<SQLINTEGER> strLenOrIndCol1 =
        { static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(3 * sizeof(wchar_t)) };
        vector<SQLINTEGER> strLenOrIndCol2 =
        { 0, SQL_NULL_DATA, SQL_NULL_DATA,
          static_cast<SQLINTEGER>(6 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(3 * sizeof(wchar_t)) };
        vector<SQLINTEGER> strLenOrIndCol3(rowsNumber, SQL_NULL_DATA);

        vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1.data(),
            strLenOrIndCol2.data(), strLenOrIndCol3.data() };

        // Coalesce the arrays of each row of each column
        // into a contiguous array for each column (UTF-16 input).
        //
        vector<wchar_t> wstringCol1Data =
            GenerateContiguousData<wchar_t>(wstringCol1, strLenOrIndCol1.data());

        vector<wchar_t> wstringCol2Data =
            GenerateContiguousData<wchar_t>(wstringCol2, strLenOrIndCol2.data());

        void* dataSet[] = { wstringCol1Data.data(),
                            wstringCol2Data.data(),
                            nullptr };

        vector<string> columnNames{ wstringColumn1Name, wstringColumn2Name, wstringColumn3Name };

        // Execute with UTF-16 input
        //
        Execute<wchar_t, SQL_C_WCHAR>(
            rowsNumber,
            dataSet,
            strLen_or_Ind.data(),
            columnNames);

        // The C# extension outputs UTF-8 for all string types,
        // so we construct expected output as UTF-8 strings with appropriate byte lengths.
        //
        vector<const char*> stringCol1{ "Hello", "test", "data", "World", "ABC" };
        vector<const char*> stringCol2{ "", 0, nullptr, "verify", "123" };

        vector<SQLINTEGER> expectedStrLenOrIndCol1 =
        { static_cast<SQLINTEGER>(strlen(stringCol1[0])),
          static_cast<SQLINTEGER>(strlen(stringCol1[1])),
          static_cast<SQLINTEGER>(strlen(stringCol1[2])),
          static_cast<SQLINTEGER>(strlen(stringCol1[3])),
          static_cast<SQLINTEGER>(strlen(stringCol1[4])) };
        vector<SQLINTEGER> expectedStrLenOrIndCol2 =
        { 0, SQL_NULL_DATA, SQL_NULL_DATA,
          static_cast<SQLINTEGER>(strlen(stringCol2[3])),
          static_cast<SQLINTEGER>(strlen(stringCol2[4])) };

        vector<SQLINTEGER*> expectedStrLen_or_Ind{ expectedStrLenOrIndCol1.data(),
            expectedStrLenOrIndCol2.data(), strLenOrIndCol3.data() };

        // Coalesce the expected UTF-8 output data
        //
        vector<char> stringCol1Data =
            GenerateContiguousData<char>(stringCol1, expectedStrLenOrIndCol1.data());

        vector<char> stringCol2Data =
            GenerateContiguousData<char>(stringCol2, expectedStrLenOrIndCol2.data());

        void* expectedDataSet[] = { stringCol1Data.data(),
                                    stringCol2Data.data(),
                                    nullptr };

        // Verify output is UTF-8 encoded
        //
        GetStringResults(
            rowsNumber,
            expectedDataSet,
            expectedStrLen_or_Ind.data(),
            columnNames);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetNVarcharOutputResultsTest
    //
    // Description:
    //  Test GetResults for an executor that opts a string column into NVARCHAR output via
    //  StringOutputColumnTypes["text"] = StringOutputType.NVarChar. Unlike the default path
    //  (which emits UTF-8), the output buffer must contain the original UTF-16 bytes. This
    //  guards against a regression where the column metadata reports WCHAR but the data path
    //  still writes UTF-8.
    //
    TEST_F(CSharpExtensionApiTests, GetNVarcharOutputResultsTest)
    {
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
        vector<const wchar_t*> wstringCol1{ L"Hello", L"World", L"Test", L"Unicode", L"\x4F60\x597D" };
        int rowsNumber = wstringCol1.size();

        vector<SQLINTEGER> strLenOrIndCol1 =
        { static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(7 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(2 * sizeof(wchar_t)) };

        vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1.data() };

        vector<wchar_t> wstringCol1Data =
            GenerateContiguousData<wchar_t>(wstringCol1, strLenOrIndCol1.data());

        void* dataSet[] = { wstringCol1Data.data() };

        vector<string> columnNames{ textColumnName };

        Execute<wchar_t, SQL_C_WCHAR>(
            rowsNumber,
            dataSet,
            strLen_or_Ind.data(),
            columnNames);

        // Output is NVARCHAR (UTF-16), so the output bytes must match the UTF-16 input exactly.
        //
        GetWStringResults(
            rowsNumber,
            dataSet,
            strLen_or_Ind.data(),
            columnNames);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetNVarcharOutputWithNullsResultsTest
    //
    // Description:
    //  Test GetResults for the NVARCHAR output path with interleaved NULL rows. This is the only
    //  test that drives NULL handling through the NVARCHAR (SQL_C_WCHAR) output writer
    //  (GetUnicodeStringArray + the WCHAR byte-length GetStrLenNullMap). It asserts both the
    //  preserved UTF-16 byte lengths for non-null rows and the SQL_NULL_DATA markers for null rows.
    //
    TEST_F(CSharpExtensionApiTests, GetNVarcharOutputWithNullsResultsTest)
    {
        string scriptNVarcharOutput = m_UserLibName + m_Separator +
            "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorNVarcharOutput";

        InitializeSession(
            1, // inputSchemaColumnsNumber
            0, // parametersNumber
            scriptNVarcharOutput);

        string textColumnName = "text";
        InitializeColumn(0, textColumnName, SQL_C_WCHAR, m_CharSize);

        // Interleaved NULL rows (rows 1 and 3) exercise SQL_NULL_DATA handling on the
        // NVARCHAR output writer alongside non-null UTF-16 values.
        //
        vector<const wchar_t*> wstringCol1{ L"Hi", nullptr, L"\x4F60\x597D", nullptr, L"End" };
        int rowsNumber = wstringCol1.size();

        vector<SQLINTEGER> strLenOrIndCol1 =
        { static_cast<SQLINTEGER>(2 * sizeof(wchar_t)),   // "Hi"
          SQL_NULL_DATA,                                  // NULL
          static_cast<SQLINTEGER>(2 * sizeof(wchar_t)),   // Chinese "Hello"
          SQL_NULL_DATA,                                  // NULL
          static_cast<SQLINTEGER>(3 * sizeof(wchar_t)) }; // "End"

        vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1.data() };

        vector<wchar_t> wstringCol1Data =
            GenerateContiguousData<wchar_t>(wstringCol1, strLenOrIndCol1.data());

        void* dataSet[] = { wstringCol1Data.data() };

        vector<string> columnNames{ textColumnName };

        Execute<wchar_t, SQL_C_WCHAR>(
            rowsNumber,
            dataSet,
            strLen_or_Ind.data(),
            columnNames);

        // Non-null rows must preserve the original UTF-16 bytes; null rows must report
        // SQL_NULL_DATA. CheckWStringDataEquality verifies both.
        //
        GetWStringResults(
            rowsNumber,
            dataSet,
            strLen_or_Ind.data(),
            columnNames);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetMixedStringOutputResultsTest
    //
    // Description:
    //  Test GetResults at the data level for a result set that mixes a default VARCHAR (UTF-8)
    //  column with an opted-in NVARCHAR (UTF-16) column, using CSharpTestExecutorMixedStringOutput
    //  (which sets StringOutputColumnTypes["unicode_col"] = StringOutputType.NVarChar and leaves
    //  "ascii_col" as the default VARCHAR). The single result set is retrieved once and each
    //  column is verified with the helper matching its output encoding.
    //
    TEST_F(CSharpExtensionApiTests, GetMixedStringOutputResultsTest)
    {
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

        // ascii_col is ASCII (UTF-8 byte length == char count); unicode_col includes a
        // non-ASCII value to ensure UTF-16 bytes are preserved rather than down-converted.
        //
        vector<const char*> stringCol{ "Hello", "World", "Test", "Data!", "Row05" };
        vector<const wchar_t*> wstringCol{ L"Alpha", L"Beta", L"Gamma", L"Delta", L"\x4F60\x597D" };
        int rowsNumber = stringCol.size();

        vector<SQLINTEGER> strLenOrIndCol1 =
        { 5, 5, 4, 5, 5 };

        vector<SQLINTEGER> strLenOrIndCol2 =
        { static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
          static_cast<SQLINTEGER>(2 * sizeof(wchar_t)) };

        vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1.data(), strLenOrIndCol2.data() };

        vector<char> stringColData = GenerateContiguousData<char>(stringCol, strLenOrIndCol1.data());
        vector<wchar_t> wstringColData = GenerateContiguousData<wchar_t>(wstringCol, strLenOrIndCol2.data());

        void* dataSet[] = { stringColData.data(), wstringColData.data() };

        vector<string> columnNames{ asciiColumnName, unicodeColumnName };

        // The template type params are inert here: Execute forwards dataSet and
        // strLen_or_Ind as opaque void**, so they do not have to match the per-column C types.
        //
        Execute<SQLCHAR, SQL_C_CHAR>(
            rowsNumber,
            dataSet,
            strLen_or_Ind.data(),
            columnNames);

        // Retrieve the single mixed result set once, then verify each column with the helper
        // matching its output encoding (UTF-8 for column 0, UTF-16 for column 1).
        //
        SQLULEN    resultRowsNumber = 0;
        SQLPOINTER *data = nullptr;
        SQLINTEGER **strLenOrIndResult = nullptr;
        SQLRETURN result = (*sm_getResultsFuncPtr)(
            *m_sessionId,
            m_taskId,
            &resultRowsNumber,
            &data,
            &strLenOrIndResult);
        ASSERT_EQ(result, SQL_SUCCESS);
        EXPECT_EQ(resultRowsNumber, static_cast<SQLULEN>(rowsNumber));

        // Column 0 (ascii_col): default VARCHAR/UTF-8 output. For ASCII, output bytes match input.
        //
        CheckStringDataEquality(
            resultRowsNumber,
            stringColData.data(),
            static_cast<char*>(data[0]),
            strLenOrIndCol1.data(),
            strLenOrIndResult[0]);

        // Column 1 (unicode_col): opted-in NVARCHAR/UTF-16 output, byte lengths preserved.
        //
        CheckWStringDataEquality(
            resultRowsNumber,
            wstringColData.data(),
            static_cast<wchar_t*>(data[1]),
            strLenOrIndCol2.data(),
            strLenOrIndResult[1]);
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
        for (size_t columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
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

        for (size_t columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
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

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::GetWStringResults
    //
    // Description:
    //  Test GetResults to verify the expected results are obtained for Unicode (UTF-16) data.
    //
    void CSharpExtensionApiTests::GetWStringResults(
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

        for (size_t columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
        {
            wchar_t *expectedColumnData = static_cast<wchar_t *>(expectedData[columnNumber]);
            wchar_t *columnData = static_cast<wchar_t *>(data[columnNumber]);

            SQLINTEGER *expectedColumnStrLenOrInd = expectedStrLen_or_Ind[columnNumber];
            SQLINTEGER *columnStrLenOrInd = strLen_or_Ind[columnNumber];

            CheckWStringDataEquality(
                rowsNumber,
                expectedColumnData,
                columnData,
                expectedColumnStrLenOrInd,
                columnStrLenOrInd);
        }
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::CheckWStringDataEquality
    //
    // Description:
    //  Compare the given Unicode (UTF-16) column data & nullMap with rowsNumber for equality.
    //  strLenOrInd values are byte lengths, so the contiguous buffer is walked in bytes while
    //  individual code units are compared as wchar_t.
    //
    void CSharpExtensionApiTests::CheckWStringDataEquality(
        SQLULEN    rowsNumber,
        wchar_t    *expectedColumnData,
        wchar_t    *columnData,
        SQLINTEGER *expectedColumnStrLenOrInd,
        SQLINTEGER *columnStrLenOrInd)
    {
        SQLINTEGER cumulativeByteLength = 0;
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
                    // strLenOrInd is in bytes; compare code unit by code unit.
                    //
                    SQLINTEGER codeUnits = columnStrLenOrInd[index] / static_cast<SQLINTEGER>(sizeof(wchar_t));
                    const wchar_t *expectedStart = reinterpret_cast<const wchar_t *>(
                        reinterpret_cast<const char *>(expectedColumnData) + cumulativeByteLength);
                    const wchar_t *actualStart = reinterpret_cast<const wchar_t *>(
                        reinterpret_cast<const char *>(columnData) + cumulativeByteLength);

                    for (SQLINTEGER unitIndex = 0; unitIndex < codeUnits; ++unitIndex)
                    {
                        EXPECT_EQ(expectedStart[unitIndex], actualStart[unitIndex]);
                    }

                    cumulativeByteLength += expectedColumnStrLenOrInd[index];
                }
            }
            else
            {
                EXPECT_EQ(columnStrLenOrInd[index], SQL_NULL_DATA);
            }
        }
    }
}
