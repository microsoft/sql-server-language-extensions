//**************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server
// external language communication protocol.
// Copyright (C) 2020 Microsoft Corporation.
//
// This file is part of RExtension-test.
//
// RExtension-test is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RExtension-test is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RExtension-test.  If not, see <https://www.gnu.org/licenses/>.
//
// @File: RExtensionGetResultsTests.cpp
//
// Purpose:
//  Tests the RExtension's implementation of the external language GetResults API.
//
//**************************************************************************************************

#include "Common.h"

using namespace std;

namespace ExtensionApiTest
{
	//----------------------------------------------------------------------------------------------
	// Name: GetIntegerResultsTest
	//
	// Description:
	//  Tests GetResults with default script expecting an OutputDataSet of Integer column data.
	//
	TEST_F(RExtensionApiTests, GetIntegerResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			(*m_integerInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLINTEGER, SQL_C_SLONG>(m_integerInfo.get());

		Execute<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
			ColumnInfo<SQLINTEGER>::sm_rowsNumber,
			(*m_integerInfo).m_dataSet.data(),
			(*m_integerInfo).m_strLen_or_Ind.data(),
			(*m_integerInfo).m_columnNames,
			false);  // validate

		GetResults<SQLINTEGER, Rcpp::IntegerVector, SQLINTEGER, SQL_C_SLONG>(
			ColumnInfo<SQLINTEGER>::sm_rowsNumber,
			(*m_integerInfo).m_dataSet.data(),
			(*m_integerInfo).m_strLen_or_Ind.data(),
			(*m_integerInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetLogicalResultsTest
	//
	// Description:
	//  Tests GetResults with default script expecting an OutputDataSet of Logical column data.
	//
	TEST_F(RExtensionApiTests, GetLogicalResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			(*m_logicalInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLCHAR, SQL_C_BIT>(m_logicalInfo.get());

		Execute<SQLCHAR, Rcpp::LogicalVector, SQL_C_BIT>(
			ColumnInfo<SQLCHAR>::sm_rowsNumber,
			(*m_logicalInfo).m_dataSet.data(),
			(*m_logicalInfo).m_strLen_or_Ind.data(),
			(*m_logicalInfo).m_columnNames,
			false);  // validate

		GetResults<SQLCHAR, Rcpp::LogicalVector, SQLCHAR, SQL_C_BIT>(
			ColumnInfo<SQLCHAR>::sm_rowsNumber,
			(*m_logicalInfo).m_dataSet.data(),
			(*m_logicalInfo).m_strLen_or_Ind.data(),
			(*m_logicalInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetRealResultsTest
	//
	// Description:
	//  Tests GetResults with default script expecting an OutputDataSet of Real column data.
	//
	TEST_F(RExtensionApiTests, GetRealResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			(*m_realInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLREAL, SQL_C_FLOAT>(m_realInfo.get());

		Execute<SQLREAL, Rcpp::NumericVector, SQL_C_FLOAT>(
			ColumnInfo<SQLREAL>::sm_rowsNumber,
			(*m_realInfo).m_dataSet.data(),
			(*m_realInfo).m_strLen_or_Ind.data(),
			(*m_realInfo).m_columnNames,
			false);  // validate

		GetResults<SQLREAL, Rcpp::NumericVector, SQLDOUBLE, SQL_C_DOUBLE>(
			ColumnInfo<SQLREAL>::sm_rowsNumber,
			(*m_realInfo).m_dataSet.data(),
			(*m_realInfo).m_strLen_or_Ind.data(),
			(*m_realInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetDoubleResultsTest
	//
	// Description:
	//  Tests GetResults with default script expecting an OutputDataSet of Double column data.
	//
	TEST_F(RExtensionApiTests, GetDoubleResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			(*m_doubleInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLDOUBLE, SQL_C_DOUBLE>(m_doubleInfo.get());

		Execute<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
			ColumnInfo<SQLDOUBLE>::sm_rowsNumber,
			(*m_doubleInfo).m_dataSet.data(),
			(*m_doubleInfo).m_strLen_or_Ind.data(),
			(*m_doubleInfo).m_columnNames,
			false);  // validate

		GetResults<SQLDOUBLE, Rcpp::NumericVector, SQLDOUBLE, SQL_C_DOUBLE>(
			ColumnInfo<SQLDOUBLE>::sm_rowsNumber,
			(*m_doubleInfo).m_dataSet.data(),
			(*m_doubleInfo).m_strLen_or_Ind.data(),
			(*m_doubleInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetBigIntResultsTest
	//
	// Description:
	//  Tests GetResults with default script expecting an OutputDataSet of BigInteger columns.
	//
	TEST_F(RExtensionApiTests, GetBigIntResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			(*m_bigIntInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLBIGINT, SQL_C_SBIGINT>(m_bigIntInfo.get());

		Execute<SQLBIGINT, Rcpp::NumericVector, SQL_C_SBIGINT>(
			ColumnInfo<SQLBIGINT>::sm_rowsNumber,
			(*m_bigIntInfo).m_dataSet.data(),
			(*m_bigIntInfo).m_strLen_or_Ind.data(),
			(*m_bigIntInfo).m_columnNames,
			false); // validate

		GetResults<SQLBIGINT, Rcpp::NumericVector, SQLDOUBLE, SQL_C_DOUBLE>(
			ColumnInfo<SQLBIGINT>::sm_rowsNumber,
			(*m_bigIntInfo).m_dataSet.data(),
			(*m_bigIntInfo).m_strLen_or_Ind.data(),
			(*m_bigIntInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetSmallIntResultsTest
	//
	// Description:
	//  Tests GetResults with default script expecting an OutputDataSet of SmallInt columns.
	//
	TEST_F(RExtensionApiTests, GetSmallIntResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			(*m_smallIntInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLSMALLINT, SQL_C_SSHORT>(m_smallIntInfo.get());

		Execute<SQLSMALLINT, Rcpp::IntegerVector, SQL_C_SSHORT>(
			ColumnInfo<SQLSMALLINT>::sm_rowsNumber,
			(*m_smallIntInfo).m_dataSet.data(),
			(*m_smallIntInfo).m_strLen_or_Ind.data(),
			(*m_smallIntInfo).m_columnNames,
			false);  // validate

		GetResults<SQLSMALLINT, Rcpp::IntegerVector, SQLINTEGER, SQL_C_SLONG>(
			ColumnInfo<SQLSMALLINT>::sm_rowsNumber,
			(*m_smallIntInfo).m_dataSet.data(),
			(*m_smallIntInfo).m_strLen_or_Ind.data(),
			(*m_smallIntInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetTinyIntResultColumnsTest
	//
	// Description:
	//  Tests GetResultColumn with default script using an OutputDataSet of TinyInt columns.
	//
	TEST_F(RExtensionApiTests, GetTinyIntResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			(*m_tinyIntInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLCHAR, SQL_C_UTINYINT>(m_tinyIntInfo.get());

		Execute<SQLCHAR, Rcpp::IntegerVector, SQL_C_UTINYINT>(
			ColumnInfo<SQLCHAR>::sm_rowsNumber,
			(*m_tinyIntInfo).m_dataSet.data(),
			(*m_tinyIntInfo).m_strLen_or_Ind.data(),
			(*m_tinyIntInfo).m_columnNames,
			false);  // validate

		GetResults<SQLCHAR, Rcpp::IntegerVector, SQLINTEGER, SQL_C_SLONG>(
			ColumnInfo<SQLCHAR>::sm_rowsNumber,
			(*m_tinyIntInfo).m_dataSet.data(),
			(*m_tinyIntInfo).m_strLen_or_Ind.data(),
			(*m_tinyIntInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetCharResultsTest
	//
	// Description:
	//  Tests GetResults with default script expecting an OutputDataSet of Character columns.
	//
	TEST_F(RExtensionApiTests, GetCharResultsTest)
	{
		SQLUSMALLINT inputSchemaColumnsNumber = 3;

		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			inputSchemaColumnsNumber,
			m_scriptString);

		string charColumn1Name = "CharColumn1";
		InitializeColumn(0, // columnNumber
			charColumn1Name,
			SQL_C_CHAR,
			m_CharSize,
			SQL_NO_NULLS);

		string charColumn2Name = "CharColumn2";
		InitializeColumn(1, // columnNumber
			charColumn2Name,
			SQL_C_CHAR,
			m_CharSize,
			SQL_NULLABLE);

		string charColumn3Name = "CharColumn3";
		InitializeColumn(2, // columnNumber
			charColumn3Name,
			SQL_C_CHAR,
			m_CharSize,
			SQL_NULLABLE);

		SQLULEN rowsNumber = 5;
		vector<const char*> charCol1{ "Hello", "test", "data", "RExtension", "-123" };
		vector<const char*> charCol2{ "", 0, nullptr, "verify", "-1" };

		vector<SQLINTEGER> strLenOrIndCol1 =
			{ static_cast<SQLINTEGER>(strlen(charCol1[0])),
			  static_cast<SQLINTEGER>(strlen(charCol1[1])),
			  static_cast<SQLINTEGER>(strlen(charCol1[2])),
			  static_cast<SQLINTEGER>(strlen(charCol1[3])),
			  static_cast<SQLINTEGER>(strlen(charCol1[4])) };
		vector<SQLINTEGER> strLenOrIndCol2 =
			{ 0, SQL_NULL_DATA, SQL_NULL_DATA,
			  static_cast<SQLINTEGER>(strlen(charCol2[3])),
			  static_cast<SQLINTEGER>(strlen(charCol2[4])) };
		vector<SQLINTEGER> strLenOrIndCol3(rowsNumber, SQL_NULL_DATA);
		vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1.data(),
			strLenOrIndCol2.data(), strLenOrIndCol3.data() };

		// Coalesce the arrays of each row of each column
		// into a contiguous array for each column.
		//
		vector<char> charCol1Data = GenerateContiguousData<char>(charCol1, strLenOrIndCol1.data());
		vector<char> charCol2Data = GenerateContiguousData<char>(charCol2, strLenOrIndCol2.data());
		void* dataSet[inputSchemaColumnsNumber] = { charCol1Data.data(), charCol2Data.data(), nullptr};

		vector<string> columnNames{charColumn1Name, charColumn2Name, charColumn3Name};

		ExecuteChar<char>(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames,
			false); // validate

		GetCharResults(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetNCharResultsTest
	//
	// Description:
	//  Tests GetResults with default script expecting an OutputDataSet of NChar columns.
	//
	TEST_F(RExtensionApiTests, GetNCharResultsTest)
	{
		SQLUSMALLINT inputSchemaColumnsNumber = 3;

		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			inputSchemaColumnsNumber,
			m_scriptString);

		string ncharColumn1Name = "NCharColumn1";
		InitializeColumn(0, // columnNumber
			ncharColumn1Name,
			SQL_C_WCHAR,
			m_NCharSize,
			SQL_NO_NULLS);

		string ncharColumn2Name = "NCharColumn2";
		InitializeColumn(1, // columnNumber
			ncharColumn2Name,
			SQL_C_WCHAR,
			m_NCharSize,
			SQL_NULLABLE);

		string ncharColumn3Name = "NCharColumn3";
		InitializeColumn(2, // columnNumber
			ncharColumn3Name,
			SQL_C_WCHAR,
			m_NCharSize,
			SQL_NULLABLE);

		vector<const wchar_t *> ncharCol1{ L"Hello", L"test", L"data", L"World你好", L"你好" };
		vector<const wchar_t *> ncharCol2{ L"", 0, nullptr, L"verify", L"-1" };

		int rowsNumber = ncharCol1.size();

		vector<SQLINTEGER> strLenOrIndCol1 =
		{ static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(7 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(2 * sizeof(wchar_t)) };
		vector<SQLINTEGER> strLenOrIndCol2 =
		{ 0, SQL_NULL_DATA, SQL_NULL_DATA,
		  static_cast<SQLINTEGER>(6 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(2 * sizeof(wchar_t)) };
		vector<SQLINTEGER> strLenOrIndCol3(rowsNumber, SQL_NULL_DATA);

		vector<SQLINTEGER *> strLen_or_Ind{ strLenOrIndCol1.data(),
			strLenOrIndCol2.data(), strLenOrIndCol3.data() };

		// Coalesce the arrays of each row of each column
		// into a contiguous array for each column.
		//
		vector<wchar_t> ncharCol1Data =
			GenerateContiguousData<wchar_t>(ncharCol1, strLenOrIndCol1.data());

		vector<wchar_t> ncharCol2Data =
			GenerateContiguousData<wchar_t>(ncharCol2, strLenOrIndCol2.data());

		void *dataSet[] = { ncharCol1Data.data(),
							ncharCol2Data.data(),
							nullptr };

		vector<string> columnNames{ ncharColumn1Name, ncharColumn2Name, ncharColumn3Name };

		ExecuteChar<wchar_t>(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames,
			false); // validate

		// Because R is a UTF-8 language, we always return UTF-8 for the OutputDataSet.
		// Here we construct the expected output, which is the same as the input but normal char
		// instead of wchar_t.
		// Since we are retrieving UTF-8 strings, we also need to redo strLenOrInd.
		//

		// Construct the bytes that correspond to 你好
		//
		vector<char> chineseBytes = { -28, -67, -96, -27, -91, -67 };
		string chineseString = string(chineseBytes.data(), 6);

		vector<const char*> charCol1{ "Hello", "test", "data", ("World" + chineseString).c_str(),
			chineseString.c_str() };
		vector<const char*> charCol2{ "", 0, nullptr, "verify", "-1" };

		strLenOrIndCol1 =
		{ static_cast<SQLINTEGER>(strlen(charCol1[0])),
		  static_cast<SQLINTEGER>(strlen(charCol1[1])),
		  static_cast<SQLINTEGER>(strlen(charCol1[2])),
		  static_cast<SQLINTEGER>(strlen(charCol1[3])),
		  static_cast<SQLINTEGER>(strlen(charCol1[4])) };
		strLenOrIndCol2 =
		{ 0, SQL_NULL_DATA, SQL_NULL_DATA,
		  static_cast<SQLINTEGER>(strlen(charCol2[3])),
		  static_cast<SQLINTEGER>(strlen(charCol2[4])) };

		strLen_or_Ind = { strLenOrIndCol1.data(),
			strLenOrIndCol2.data(), strLenOrIndCol3.data() };

		// Coalesce the arrays of each row of each column
		// into a contiguous array for each column.
		//
		vector<char> charCol1Data =
			GenerateContiguousData<char>(charCol1, strLenOrIndCol1.data());

		vector<char> charCol2Data =
			GenerateContiguousData<char>(charCol2, strLenOrIndCol2.data());

		void* expectedDataSet[] = { charCol1Data.data(),
									charCol2Data.data(),
									nullptr };

		GetCharResults(
			rowsNumber,
			expectedDataSet,
			strLen_or_Ind.data(),
			columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetRawResultsTest
	//
	// Description:
	//  Tests GetResults with a script that returns OutputDataSet with a single raw column.
	//
	TEST_F(RExtensionApiTests, GetRawResultsTest)
	{
		string scriptString = "rawChars <- charToRaw(paste(letters[1:5], collapse=''));"
			"OutputDataSet <- data.frame(rawChars);"
			"message(OutputDataSet)";

		// Initialize with a Session that executes the above script
		// that creates an OutputDataSet with a single raw column.
		//
		InitializeSession(
			0,             // inputSchemaColumnsNumber
			scriptString);

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
			nullptr,
			nullptr,
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 1);

		SQLULEN    rowsNumber = 0;
		SQLPOINTER *data = nullptr;
		SQLINTEGER **strLen_or_Ind = nullptr;
		SQLULEN expectedRowsNumber = 1;
		vector<SQLCHAR> expectedData = {0x61, 0x62, 0x63, 0x64, 0x65};
		vector<SQLINTEGER> expectedStrLenOrInd =
			{ static_cast<SQLINTEGER>(expectedData.size()) };

		result = (*sm_getResultsFuncPtr)(
			*m_sessionId,
			m_taskId,
			&rowsNumber,
			&data,
			&strLen_or_Ind);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(rowsNumber, expectedRowsNumber);

		SQLCHAR *rawData = static_cast<SQLCHAR*>(data[0]);
		for(SQLINTEGER index = 0; index <
				static_cast<SQLINTEGER>(expectedData.size()); ++index)
		{
			EXPECT_EQ(rawData[index], expectedData[index]);
		}

		SQLINTEGER *strLenOrInd = strLen_or_Ind[0];
		EXPECT_EQ(strLenOrInd[0], expectedStrLenOrInd[0]);

		CleanupSession();

		scriptString = "rawChars <- raw(0);"
			"OutputDataSet <- data.frame(rawChars);"
			"message(OutputDataSet)";

		// Initialize with a Session that executes the above script
		// that creates an OutputDataSet with a null raw column.
		//
		InitializeSession(
			0,             // inputSchemaColumnsNumber
			scriptString);

		outputSchemaColumnsNumber = 0;
		result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
			nullptr,
			nullptr,
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 1);

		result = (*sm_getResultsFuncPtr)(
			*m_sessionId,
			m_taskId,
			&rowsNumber,
			&data,
			&strLen_or_Ind);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(rowsNumber, static_cast<SQLULEN>(0));
		EXPECT_EQ(data[0], nullptr);
		EXPECT_EQ(strLen_or_Ind[0], nullptr);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetDateResultsTest
	//
	// Description:
	//  Tests GetResults with default script expecting an OutputDataSet of Date column data.
	//
	TEST_F(RExtensionApiTests, GetDateResultsTest)
	{
		// Initialize with a default session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			(*m_dateInfo).GetColumnsNumber(),
			m_scriptString,
			0); // parametersNumber

		InitializeColumns<SQL_DATE_STRUCT, SQL_C_TYPE_DATE>(m_dateInfo.get());

		ExecuteDateTime<SQL_DATE_STRUCT, Rcpp::DateVector, Rcpp::Date>(
			ColumnInfo<SQL_DATE_STRUCT>::sm_rowsNumber,
			(*m_dateInfo).m_dataSet.data(),
			(*m_dateInfo).m_strLen_or_Ind.data(),
			(*m_dateInfo).m_columnNames,
			false);  // validate

		GetDateTimeResults<SQL_DATE_STRUCT, Rcpp::DateVector, Rcpp::Date>(
			ColumnInfo<SQL_DATE_STRUCT>::sm_rowsNumber,
			(*m_dateInfo).m_dataSet.data(),
			(*m_dateInfo).m_strLen_or_Ind.data(),
			(*m_dateInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetDateTimeResultsTest
	//
	// Description:
	//  Tests GetResults with default script expecting an OutputDataSet of DateTime column data.
	//
	TEST_F(RExtensionApiTests, GetDateTimeResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession((*m_dateTimeInfo).GetColumnsNumber(),
			m_scriptString,
			0); // parametersNumber

		InitializeColumns<SQL_TIMESTAMP_STRUCT, SQL_C_TYPE_TIMESTAMP>(m_dateTimeInfo.get());

		ExecuteDateTime<SQL_TIMESTAMP_STRUCT, Rcpp::DatetimeVector, Rcpp::Datetime>(
			ColumnInfo<SQL_TIMESTAMP_STRUCT>::sm_rowsNumber,
			(*m_dateTimeInfo).m_dataSet.data(),
			(*m_dateTimeInfo).m_strLen_or_Ind.data(),
			(*m_dateTimeInfo).m_columnNames,
			false); // validate

		GetDateTimeResults<SQL_TIMESTAMP_STRUCT, Rcpp::DatetimeVector, Rcpp::Datetime>(
			ColumnInfo<SQL_TIMESTAMP_STRUCT>::sm_rowsNumber,
			(*m_dateTimeInfo).m_dataSet.data(),
			(*m_dateTimeInfo).m_strLen_or_Ind.data(),
			(*m_dateTimeInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetDifferentColumnResultsTest
	//
	// Description:
	//  Tests GetResults with default script using an OutputDataSet of different column types.
	//
	TEST_F(RExtensionApiTests, GetDifferentColumnResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			3,               // inputSchemaColumnsNumber
			m_scriptString);

		string integerColumnName = "IntegerColumn";
		InitializeColumn(0, // columnNumber
			integerColumnName,
			SQL_C_SLONG,
			m_IntSize,
			SQL_NULLABLE);

		string doubleColumnName = "DoubleColumn";
		InitializeColumn(1, // columnNumber
			doubleColumnName,
			SQL_C_DOUBLE,
			m_DoubleSize,
			SQL_NO_NULLS);

		string charColumnName = "CharColumn";
		InitializeColumn(2, // columnNumber
			charColumnName,
			SQL_C_CHAR,
			m_CharSize,
			SQL_NULLABLE);

		SQLULEN expectedRowsNumber = 5;
		vector<SQLINTEGER> intColData{ m_MaxInt, m_MinInt, NA_INTEGER, NA_INTEGER, -1 };
		vector<SQLDOUBLE> doubleColData{ m_MinDouble, 1.33, 83.98, 72.45, m_MaxDouble };
		vector<const char*> charCol{ "Hello", "test", "data", "RExtension", "-123" };

		vector<SQLINTEGER> strLenOrIndCol1 = { m_IntSize, m_IntSize, SQL_NULL_DATA,
			SQL_NULL_DATA, m_IntSize };
		vector<SQLINTEGER> strLenOrIndCol2(expectedRowsNumber, m_DoubleSize);
		vector<SQLINTEGER> strLenOrIndCol3 =
			{ static_cast<SQLINTEGER>(strlen(charCol[0])),
			  static_cast<SQLINTEGER>(strlen(charCol[1])),
			  static_cast<SQLINTEGER>(strlen(charCol[2])),
			  static_cast<SQLINTEGER>(strlen(charCol[3])),
			  static_cast<SQLINTEGER>(strlen(charCol[4])) };
		vector<SQLINTEGER*> expectedStrLen_or_Ind{ strLenOrIndCol1.data(),
			strLenOrIndCol2.data(), strLenOrIndCol3.data()};

		vector<char> charColData = GenerateContiguousData<char>(charCol, strLenOrIndCol3.data());

		vector<void *> expectedData { intColData.data(), doubleColData.data(), charColData.data()};

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			expectedRowsNumber,
			expectedData.data(),
			expectedStrLen_or_Ind.data(),
			&outputschemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		SQLULEN    rowsNumber = 0;
		SQLPOINTER *data = nullptr;
		SQLINTEGER **strLen_or_Ind = nullptr;
		result = (*sm_getResultsFuncPtr)(
			*m_sessionId,
			m_taskId,
			&rowsNumber,
			&data,
			&strLen_or_Ind);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(rowsNumber, expectedRowsNumber);

		// Test the data returned.
		//
		CheckColumnDataEquality<SQLINTEGER, SQLINTEGER, SQL_C_SLONG>(
			rowsNumber,
			static_cast<SQLINTEGER*>(expectedData[0]),
			static_cast<SQLINTEGER*>(data[0]),
			expectedStrLen_or_Ind[0],
			strLen_or_Ind[0]);

		CheckColumnDataEquality<SQLDOUBLE, SQLDOUBLE, SQL_C_DOUBLE>(
			rowsNumber,
			static_cast<SQLDOUBLE*>(expectedData[1]),
			static_cast<SQLDOUBLE*>(data[1]),
			expectedStrLen_or_Ind[1],
			strLen_or_Ind[1]);

		CheckCharDataEquality(
			rowsNumber,
			static_cast<char*>(expectedData[2]),
			static_cast<char*>(data[2]),
			expectedStrLen_or_Ind[2],
			strLen_or_Ind[2]);

		// Test the data obtained is what is present in OutputDataSet.
		//
		Rcpp::DataFrame outputDataSet = m_globalEnvironment[m_outputDataNameString.c_str()];

		EXPECT_EQ(outputschemaColumnsNumber, outputDataSet.size());
		Rcpp::IntegerVector intColumn = outputDataSet[integerColumnName.c_str()];
		CheckRVectorColumnDataEquality<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
			rowsNumber,
			intColumn,
			data[0],
			strLen_or_Ind[0]);

		Rcpp::NumericVector numericColumn = outputDataSet[doubleColumnName.c_str()];
		CheckRVectorColumnDataEquality<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
			rowsNumber,
			numericColumn,
			data[1],
			strLen_or_Ind[1]);

		Rcpp::CharacterVector charColumn = outputDataSet[charColumnName.c_str()];
		CheckRCharacterVectorColumnDataEquality<char>(
			rowsNumber,
			charColumn,
			data[2],
			strLen_or_Ind[2]);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetEmptyResultsTest
	//
	// Description:
	//  Tests GetResults with a script that returns OutputDataSet with empty rows.
	//
	TEST_F(RExtensionApiTests, GetEmptyResultsTest)
	{
		string scriptString = ""
			"OutputDataSet <- data.frame(intCol = as.integer(c()))\n"
			"message(OutputDataSet)";

		// Initialize with a Session that executes the above script
		// that creates an empty row 1 column OutputDataSet.
		//
		InitializeSession(
			0,             // inputSchemaColumnsNumber
			scriptString);

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,  // input rowsNumber
			nullptr,
			nullptr,
			&outputschemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputschemaColumnsNumber, 1);

		GetResults<SQLINTEGER, Rcpp::IntegerVector, SQLINTEGER, SQL_C_SLONG>(
			0, // expected rowsNumber
			vector<void*>{nullptr}.data(),
			vector<SQLINTEGER*>{nullptr}.data(),
			vector<string>{"intCol"});
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::GetResults
	//
	// Description:
	//  Tests GetResults to verify the expected results are obtained.
	//  For numeric, logical and integer types.
	//
	template<class InputSQLType, class RVectorType, class OutputSQLType, SQLSMALLINT outputDataType>
	void RExtensionApiTests::GetResults(
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

		// Test data obtained is same as the expectedData and the OutputDataSet in R environment.
		//
		Rcpp::DataFrame outputDataSet = m_globalEnvironment[m_outputDataNameString.c_str()];

		for (SQLUSMALLINT columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
		{
			InputSQLType *expectedColumnData = static_cast<InputSQLType *>(expectedData[columnNumber]);
			OutputSQLType *columnData = static_cast<OutputSQLType *>(data[columnNumber]);

			SQLINTEGER *expectedColumnStrLenOrInd = expectedStrLen_or_Ind[columnNumber];
			SQLINTEGER *columnStrLenOrInd = strLen_or_Ind[columnNumber];

			CheckColumnDataEquality<InputSQLType, OutputSQLType, outputDataType>(
				rowsNumber,
				expectedColumnData,
				columnData,
				expectedColumnStrLenOrInd,
				columnStrLenOrInd);

			RVectorType column = outputDataSet[columnNames[columnNumber].c_str()];
			CheckRVectorColumnDataEquality<OutputSQLType, RVectorType, outputDataType>(
				rowsNumber,
				column,
				columnData,
				columnStrLenOrInd);
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::CheckColumnDataEquality
	//
	// Description:
	//  Templatized function to compare the given column data
	//  and nullMap with rowsNumber for equality.
	//
	template<class InputSQLType, class OutputSQLType, SQLSMALLINT OutputDataType>
	void RExtensionApiTests::CheckColumnDataEquality(
		SQLULEN        rowsNumber,
		InputSQLType   *expectedColumnData,
		OutputSQLType  *columnData,
		SQLINTEGER     *expectedColumnStrLenOrInd,
		SQLINTEGER     *columnStrLenOrInd)
	{
		if (rowsNumber == 0)
		{
			EXPECT_EQ(columnData, nullptr);
			EXPECT_EQ(columnStrLenOrInd, nullptr);
		}

		for (SQLULEN index = 0; index < rowsNumber; ++index)
		{
			EXPECT_EQ(columnStrLenOrInd[index], expectedColumnStrLenOrInd[index]);
			if (columnStrLenOrInd[index] == SQL_NULL_DATA)
			{
				if constexpr (is_same_v<OutputSQLType, SQLDOUBLE>)
				{
					EXPECT_TRUE(isnan(static_cast<SQLDOUBLE>(columnData[index])));
				}
				else
				{
					if constexpr (is_same_v<InputSQLType, OutputSQLType>)
					{
						EXPECT_EQ(columnData[index], expectedColumnData[index]);
					}
					else
					{
						// All the cases where InputSQLType is not same as OutputSQLType
						// and it is not a NAN are for SQLINTEGER so check for NA_INTEGER here.
						//
						EXPECT_EQ(columnData[index], NA_INTEGER);
					}
				}
			}
			else
			{
				if constexpr (OutputDataType == SQL_C_BIT)
				{
					if (expectedColumnData[index] == 0 ||
						expectedColumnData[index] == '0')
					{
						EXPECT_EQ(columnData[index], 0);
					}
					else
					{
						EXPECT_EQ(columnData[index], 1);
					}
				}
				else
				{
					EXPECT_EQ(columnData[index], expectedColumnData[index]);
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::GetCharResults
	//
	// Description:
	//  Tests GetResults to verify the expected results are obtained for character data.
	//
	void RExtensionApiTests::GetCharResults(
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

		// Test data obtained is same as the expectedData and the OutputDataSet in R environment.
		//
		Rcpp::DataFrame outputDataSet = m_globalEnvironment[m_outputDataNameString.c_str()];

		for (SQLUSMALLINT columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
		{
			char *expectedColumnData = static_cast<char *>(expectedData[columnNumber]);
			char *columnData = static_cast<char *>(data[columnNumber]);

			SQLINTEGER *expectedColumnStrLenOrInd = expectedStrLen_or_Ind[columnNumber];
			SQLINTEGER *columnStrLenOrInd = strLen_or_Ind[columnNumber];

			CheckCharDataEquality(
				rowsNumber,
				expectedColumnData,
				columnData,
				expectedColumnStrLenOrInd,
				columnStrLenOrInd);

			Rcpp::CharacterVector column = outputDataSet[columnNames[columnNumber].c_str()];
			CheckRCharacterVectorColumnDataEquality<char>(
				rowsNumber,
				column,
				columnData,
				columnStrLenOrInd);
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::CheckCharDataEquality
	//
	// Description:
	//  Compares the given character data & nullMap with rowsNumber for equality.
	//
	void RExtensionApiTests::CheckCharDataEquality(
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
			EXPECT_EQ(columnStrLenOrInd[index], expectedColumnStrLenOrInd[index]);
			if (columnStrLenOrInd[index] != SQL_NULL_DATA)
			{
				string expectedString(expectedColumnData + cumulativeLength,
					expectedColumnStrLenOrInd[index]);
				string testString(columnData + cumulativeLength,
					columnStrLenOrInd[index]);
				EXPECT_EQ(testString, expectedString);
				cumulativeLength += expectedColumnStrLenOrInd[index];
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::GetDateTimeResults
	//
	// Description:
	//  Tests GetResults to verify the expected results are obtained for date(time) data.
	//
	template<class SQLType, class RVectorType, class DateTimeTypeInR>
	void RExtensionApiTests::GetDateTimeResults(
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

		// Test data obtained is same as the expectedData and the OutputDataSet in R environment.
		//
		Rcpp::DataFrame outputDataSet = m_globalEnvironment[m_outputDataNameString.c_str()];

		for (SQLUSMALLINT columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
		{
			SQLType *expectedColumnData = static_cast<SQLType*>(expectedData[columnNumber]);
			SQLType *columnData = static_cast<SQLType *>(data[columnNumber]);

			SQLINTEGER *expectedColumnStrLenOrInd = expectedStrLen_or_Ind[columnNumber];
			SQLINTEGER *columnStrLenOrInd = strLen_or_Ind[columnNumber];

			CheckDateTimeDataEquality<SQLType>(
				rowsNumber,
				expectedColumnData,
				columnData,
				expectedColumnStrLenOrInd,
				columnStrLenOrInd);

			RVectorType column = outputDataSet[columnNames[columnNumber].c_str()];
			CheckRDateTimeVectorColumnDataEquality<SQLType, RVectorType, DateTimeTypeInR>(
				rowsNumber,
				column,
				columnData,
				columnStrLenOrInd);
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::CheckDateTimeDataEquality
	//
	// Description:
	//  Compares the given datetime data & nullMap for equality.
	//
	template<class SQLType>
	void RExtensionApiTests::CheckDateTimeDataEquality(
		SQLULEN    rowsNumber,
		SQLType    *expectedColumnData,
		SQLType    *columnData,
		SQLINTEGER *expectedColumnStrLenOrInd,
		SQLINTEGER *columnStrLenOrInd)
	{
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
					EXPECT_EQ(expectedColumnData->year, columnData->year);
					EXPECT_EQ(expectedColumnData->month, columnData->month);
					EXPECT_EQ(expectedColumnData->day, columnData->day);

					if constexpr (is_same_v<SQLType, SQL_TIMESTAMP_STRUCT>)
					{
						EXPECT_EQ(expectedColumnData->hour, columnData->hour);
						EXPECT_EQ(expectedColumnData->minute, columnData->minute);
						EXPECT_EQ(expectedColumnData->second, columnData->second);
						EXPECT_EQ(expectedColumnData->fraction, columnData->fraction);
					}
				}
			}
			else
			{
				EXPECT_EQ(columnStrLenOrInd[index], SQL_NULL_DATA);
			}
		}
	}
}
