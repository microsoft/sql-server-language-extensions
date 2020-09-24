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
// @File: RExtensionGetResultColumnTests.cpp
//
// Purpose:
//  Tests the RExtension's implementation of the external language GetResultColumn API.
//
//**************************************************************************************************

#include "Common.h"

using namespace std;

namespace ExtensionApiTest
{

	//----------------------------------------------------------------------------------------------
	// Name: GetIntegerResultColumnsTest
	//
	// Description:
	//  Tests GetResultColumn with default script expecting an OutputDataSet of Integer columns.
	//
	TEST_F(RExtensionApiTests, GetIntegerResultColumnsTest)
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

		GetResultColumn(0, // columnNumber
			SQL_C_SLONG,   // dataType
			m_IntSize,     // columnSize
			0,             // decimalDigits
			SQL_NO_NULLS); // nullable

		GetResultColumn(1, // columnNumber
			SQL_C_SLONG,   // dataType
			m_IntSize,     // columnSize
			0,             // decimalDigits
			SQL_NULLABLE); // nullable
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetLogicalResultColumnsTest
	//
	// Description:
	//  Tests GetResultColumn with default script using an OutputDataSet of Logical columns.
	//
	TEST_F(RExtensionApiTests, GetLogicalResultColumnsTest)
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

		GetResultColumn(0, // columnNumber
			SQL_C_BIT,     // dataType
			m_LogicalSize, // columnSize
			0,             // decimalDigits
			SQL_NO_NULLS); // nullable

		GetResultColumn(1, // columnNumber
			SQL_C_BIT,     // dataType
			m_LogicalSize, // columnSize
			0,             // decimalDigits
			SQL_NULLABLE); // nullable
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetRealResultColumnsTest
	//
	// Description:
	//  Tests GetResultColumn with default script using an OutputDataSet of Real columns.
	//
	TEST_F(RExtensionApiTests, GetRealResultColumnsTest)
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

		GetResultColumn(0, // columnNumber
			SQL_C_DOUBLE,  // dataType
			m_DoubleSize,  // columnSize
			0,             // decimalDigits
			SQL_NO_NULLS); // nullable

		GetResultColumn(1, // columnNumber
			SQL_C_DOUBLE,  // dataType
			m_DoubleSize,  // columnSize
			0,             // decimalDigits
			SQL_NULLABLE); // nullable
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetDoubleResultColumnsTest
	//
	// Description:
	//  Tests GetResultColumn with default script using an OutputDataSet of Double columns.
	//
	TEST_F(RExtensionApiTests, GetDoubleResultColumnsTest)
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

		GetResultColumn(0, // columnNumber
			SQL_C_DOUBLE,  // dataType
			m_DoubleSize,  // columnSize
			0,             // decimalDigits
			SQL_NO_NULLS); // nullable

		GetResultColumn(1, // columnNumber
			SQL_C_DOUBLE,  // dataType
			m_DoubleSize,  // columnSize
			0,             // decimalDigits
			SQL_NULLABLE); // nullable
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetBigIntResultColumnsTest
	//
	// Description:
	//  Tests GetResultColumn with default script using an OutputDataSet of BigInteger columns.
	//
	TEST_F(RExtensionApiTests, GetBigIntResultColumnsTest)
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

		GetResultColumn(0, // columnNumber
			SQL_C_DOUBLE,  // dataType
			m_DoubleSize,  // columnSize
			0,             // decimalDigits
			SQL_NO_NULLS); // nullable

		GetResultColumn(1, // columnNumber
			SQL_C_DOUBLE,  // dataType
			m_DoubleSize,  // columnSize
			0,             // decimalDigits
			SQL_NULLABLE); // nullable
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetSmallIntResultColumnsTest
	//
	// Description:
	//  Tests GetResultColumn with default script using an OutputDataSet of SmallInt columns.
	//
	TEST_F(RExtensionApiTests, GetSmallIntResultColumnsTest)
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

		GetResultColumn(0, // columnNumber
			SQL_C_SLONG,   // dataType
			m_IntSize,     // columnSize
			0,             // decimalDigits
			SQL_NO_NULLS); // nullable

		GetResultColumn(1, // columnNumber
			SQL_C_SLONG,   // dataType
			m_IntSize,     // columnSize
			0,             // decimalDigits
			SQL_NO_NULLS); // nullable
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetTinyIntResultColumnsTest
	//
	// Description:
	//  Tests GetResultColumn with default script using an OutputDataSet of TinyInt columns.
	//
	TEST_F(RExtensionApiTests, GetTinyIntResultColumnsTest)
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

		GetResultColumn(0, // columnNumber
			SQL_C_SLONG,   // dataType
			m_IntSize,     // columnSize
			0,             // decimalDigits
			SQL_NO_NULLS); // nullable

		GetResultColumn(1, // columnNumber
			SQL_C_SLONG,   // dataType
			m_IntSize,     // columnSize
			0,             // decimalDigits
			SQL_NULLABLE); // nullable
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetCharResultColumnsTest
	//
	// Description:
	//  Tests GetResultColumn with default script using an OutputDataSet of Character columns.
	//
	TEST_F(RExtensionApiTests, GetCharResultColumnsTest)
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
		vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1.data(), strLenOrIndCol2.data(), nullptr };

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

		SQLULEN maxCol1Len = Utilities::GetMaxLength(strLenOrIndCol1.data(), rowsNumber);
		SQLULEN maxCol2Len = Utilities::GetMaxLength(strLenOrIndCol2.data(), rowsNumber);

		GetResultColumn(0, // columnNumber
			SQL_C_CHAR,    // dataType
			maxCol1Len,    // columnSize
			0,             // decimalDigits
			SQL_NO_NULLS); // nullable

		GetResultColumn(1, // columnNumber
			SQL_C_CHAR,    // dataType
			maxCol2Len,    // columnSize
			0,             // decimalDigits
			SQL_NULLABLE); // nullable

		GetResultColumn(2, // columnNumber
			SQL_C_CHAR,    // dataType
			0,             // columnSize
			0,             // decimalDigits
			SQL_NULLABLE); // nullable
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetNCharResultColumnsTest
	//
	// Description:
	//  Tests GetResultColumn with default script using an OutputDataSet of nchar columns.
	//
	TEST_F(RExtensionApiTests, GetNCharResultColumnsTest)
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

		// Because R has default utf-8 encoding, we always return utf-8 strings.
		// Thus, we expect output result columns to be utf-8 strings, not wstrings.
		// Construct the bytes that correspond to 你好
		//
		vector<char> chineseBytes = { -28, -67, -96, -27, -91, -67 };
		string chineseString = string(chineseBytes.data(), 6);

		vector<const char*> charCol1{ "Hello", "test", "data", ("World" + chineseString).c_str(),
			chineseString.c_str() };
		vector<const char*> charCol2{ "", 0, nullptr, "verify", "-1" };

		// We need to recalculate the string length to get the correct max len and that would be the
		// expected column size.
		//
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

		SQLULEN maxCol1Len = Utilities::GetMaxLength(strLenOrIndCol1.data(), rowsNumber);
		SQLULEN maxCol2Len = Utilities::GetMaxLength(strLenOrIndCol2.data(), rowsNumber);

		GetResultColumn(0, // columnNumber
			SQL_C_CHAR,    // expectedDataType
			maxCol1Len,    // expectedColumnSize
			0,             // expectedDecimalDigits
			SQL_NO_NULLS); // expectedNullable

		GetResultColumn(1, // columnNumber
			SQL_C_CHAR,    // expectedDataType
			maxCol2Len,    // expectedColumnSize
			0,             // expectedDecimalDigits
			SQL_NULLABLE); // expectedNullable

		GetResultColumn(2, // columnNumber
			SQL_C_CHAR,    // expectedDataType
			0,             // expectedColumnSize
			0,             // expectedDecimalDigits
			SQL_NULLABLE); // expectedNullable
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetRawResultColumnTest
	//
	// Description:
	//  Tests GetResultColumn with a script that returns OutputDataSet with a single raw column.
	//
	TEST_F(RExtensionApiTests, GetRawResultColumnTest)
	{
		string scriptString = "rawChars <- charToRaw(paste(letters[1:26], collapse=''));"
			"OutputDataSet <- data.frame(rawChars);"
			"message(OutputDataSet)";

		// Initialize with a Session that executes the above script
		// that assigns creates an OutputDataSet with a single raw column.
		//
		InitializeSession(
			0,             // inputSchemaColumnsNumber
			scriptString);

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
			nullptr,
			nullptr,
			&outputschemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputschemaColumnsNumber, 1);

		GetResultColumn(0, // columnNumber
			SQL_C_BINARY,  // dataType
			26,            // columnSize
			0,             // decimalDigits
			SQL_NO_NULLS); // nullable

		CleanupSession();

		scriptString = "rawChars <- raw(0);"
			"OutputDataSet <- data.frame(rawChars);"
			"message(OutputDataSet)";

		// Initialize with a Session that executes the above script
		// that assigns creates an OutputDataSet with a null raw column.
		//
		InitializeSession(
			0,             // inputSchemaColumnsNumber
			scriptString);

		outputschemaColumnsNumber = 0;
		result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
			nullptr,
			nullptr,
			&outputschemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputschemaColumnsNumber, 1);

		GetResultColumn(0, // columnNumber
			SQL_C_BINARY,  // dataType
			0,             // columnSize
			0,             // decimalDigits
			SQL_NULLABLE); // nullable
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetDateResultColumnsTest
	//
	// Description:
	//  Tests GetResultColumn with default script expecting an OutputDataSet of Date columns.
	//
	TEST_F(RExtensionApiTests, GetDateResultColumnsTest)
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

		GetResultColumn(0,   // columnNumber
			SQL_C_TYPE_DATE, // dataType
			m_DateSize,      // columnSize
			0,               // decimalDigits
			SQL_NO_NULLS);   // nullable

		GetResultColumn(1,   // columnNumber
			SQL_C_TYPE_DATE, // dataType
			m_DateSize,      // columnSize
			0,               // decimalDigits
			SQL_NULLABLE);   // nullable
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetDateTimeResultColumnsTest
	//
	// Description:
	//  Tests GetResultColumn with default script expecting an OutputDataSet of DateTime columns.
	//
	TEST_F(RExtensionApiTests, GetDateTimeResultColumnsTest)
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

		GetResultColumn(0,        // columnNumber
			SQL_C_TYPE_TIMESTAMP, // dataType
			m_DateTimeSize,       // columnSize
			0,                    // decimalDigits
			SQL_NO_NULLS);        // nullable

		GetResultColumn(1,        // columnNumber
			SQL_C_TYPE_TIMESTAMP, // dataType
			m_DateTimeSize,       // columnSize
			0,                    // decimalDigits
			SQL_NULLABLE);        // nullable
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetDifferentResultColumnsTest
	//
	// Description:
	//  Tests GetResultColumn with default script using an OutputDataSet of different column types.
	//
	TEST_F(RExtensionApiTests, GetDifferentResultColumnsTest)
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

		SQLULEN rowsNumber = 5;
		vector<SQLINTEGER> intColData{ m_MaxInt, m_MinInt, NA_INTEGER, NA_INTEGER, -1 };
		vector<SQLDOUBLE> doubleColData{ m_MinDouble, 1.33, 83.98, 72.45, m_MaxDouble };
		vector<const char*> charCol{ "Hello", "test", "data", "RExtension", "-123" };

		vector<SQLINTEGER> strLenOrIndCol1 = { 0, 0, SQL_NULL_DATA, SQL_NULL_DATA, 0 };
		vector<SQLINTEGER> strLenOrIndCol3 =
			{ static_cast<SQLINTEGER>(strlen(charCol[0])),
			  static_cast<SQLINTEGER>(strlen(charCol[1])),
			  static_cast<SQLINTEGER>(strlen(charCol[2])),
			  static_cast<SQLINTEGER>(strlen(charCol[3])),
			  static_cast<SQLINTEGER>(strlen(charCol[4])) };
		vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1.data(), nullptr, strLenOrIndCol3.data()};

		vector<char> charColData = GenerateContiguousData<char>(charCol, strLenOrIndCol3.data());

		vector<void *> dataSet { intColData.data(), doubleColData.data(), charColData.data()};

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			rowsNumber,
			dataSet.data(),
			strLen_or_Ind.data(),
			&outputschemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		GetResultColumn(0, // columnNumber
			SQL_C_SLONG,   // dataType
			m_IntSize,     // columnSize
			0,             // decimalDigits
			SQL_NULLABLE); // nullable

		GetResultColumn(1, // columnNumber
			SQL_C_DOUBLE,  // dataType
			m_DoubleSize,  // columnSize
			0,             // decimalDigits
			SQL_NO_NULLS); // nullable

		SQLULEN maxLen = Utilities::GetMaxLength(strLenOrIndCol3.data(), rowsNumber);

		GetResultColumn(2, // columnNumber
			SQL_C_CHAR,    // dataType
			maxLen,        // columnSize
			0,             // decimalDigits
			SQL_NO_NULLS); // nullable
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetEmptyResultColumnTest
	//
	// Description:
	//  Tests GetResultColumn with a script that returns OutputDataSet with empty rows.
	//
	TEST_F(RExtensionApiTests, GetEmptyResultColumnTest)
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
			0,
			nullptr,
			nullptr,
			&outputschemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputschemaColumnsNumber, 1);

		GetResultColumn(0, // columnNumber
			SQL_C_SLONG,   // dataType
			m_IntSize,     // columnSize
			0,             // decimalDigits
			SQL_NO_NULLS); // nullable
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetResultColumn
	//
	// Description:
	//  Tests GetResultColumn to verify the expected result column information is obtained.
	//
	void RExtensionApiTests::GetResultColumn(
		SQLUSMALLINT columnNumber,
		SQLSMALLINT  expectedDataType,
		SQLULEN      expectedColumnSize,
		SQLSMALLINT  expectedDecimalDigits,
		SQLSMALLINT  expectedNullable)
	{
		SQLSMALLINT dataType = 0;
		SQLULEN columnSize = 0;
		SQLSMALLINT decimalDigits = 0;
		SQLSMALLINT nullable = 0;

		SQLRETURN result = (*sm_getResultColumnFuncPtr)(
			*m_sessionId,
			m_taskId,
			columnNumber,
			&dataType,
			&columnSize,
			&decimalDigits,
			&nullable);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(dataType, expectedDataType);
		EXPECT_EQ(columnSize, expectedColumnSize);
		EXPECT_EQ(decimalDigits, expectedDecimalDigits);
		EXPECT_EQ(nullable, expectedNullable);
	}
}
