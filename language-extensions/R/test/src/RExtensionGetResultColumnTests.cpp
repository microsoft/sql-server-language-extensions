//*************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server
// external language communication protocol.
// Copyright (C) 2019 Microsoft Corporation.
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
//*************************************************************************************************
#ifdef _WIN64
#include <windows.h>
#endif
#include <sqlext.h>
#include <sqltypes.h>
#include "gtest/gtest.h"
#include "RInside.h"
#include "RExtensionApiTest.h"

using namespace std;

namespace ExtensionApiTest
{
	// Name: GetIntegerResultColumnsTest
	//
	// Description:
	// Test GetResultColumn with default script expecting an OutputDataSet of Integer columns.
	//
	TEST_F(RExtensionApiTest, GetIntegerResultColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession((*m_integerInfo).GetColumnsNumber(),
			m_script,
			m_scriptString.length());

		InitializeColumns<SQLINTEGER, SQL_C_SLONG>(m_integerInfo.get());

		Execute<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
			ColumnInfo<SQLINTEGER>::m_rowsNumber,
			(*m_integerInfo).m_dataSet.data(),
			(*m_integerInfo).m_strLen_or_Ind.data(),
			(*m_integerInfo).m_columnNames,
			false);  // test

		TestGetResultColumn(0, // columnNumber
			SQL_C_SLONG,       // dataType
			sizeof(SQLINTEGER),// columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		TestGetResultColumn(1, // columnNumber
			SQL_C_SLONG,       // dataType
			sizeof(SQLINTEGER),// columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetLogicalResultColumnsTest
	//
	// Description:
	// Test GetResultColumn with default script using an OutputDataSet of Logical columns.
	//
	TEST_F(RExtensionApiTest, GetLogicalResultColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession((*m_logicalInfo).GetColumnsNumber(),
			m_script,
			m_scriptString.length());

		InitializeColumns<SQLCHAR, SQL_C_BIT>(m_logicalInfo.get());

		Execute<SQLCHAR, Rcpp::LogicalVector, SQL_C_BIT>(
			ColumnInfo<SQLCHAR>::m_rowsNumber,
			(*m_logicalInfo).m_dataSet.data(),
			(*m_logicalInfo).m_strLen_or_Ind.data(),
			(*m_logicalInfo).m_columnNames,
			false);  // test

		TestGetResultColumn(0, // columnNumber
			SQL_C_BIT,         // dataType
			sizeof(SQLCHAR),   // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		TestGetResultColumn(1, // columnNumber
			SQL_C_BIT,         // dataType
			sizeof(SQLCHAR),   // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetRealResultColumnsTest
	//
	// Description:
	// Test GetResultColumn with default script using an OutputDataSet of Real columns.
	//
	TEST_F(RExtensionApiTest, GetRealResultColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession((*m_realInfo).GetColumnsNumber(),
			m_script,
			m_scriptString.length());

		InitializeColumns<SQLREAL, SQL_C_FLOAT>(m_realInfo.get());

		Execute<SQLREAL, Rcpp::NumericVector, SQL_C_FLOAT>(
			ColumnInfo<SQLREAL>::m_rowsNumber,
			(*m_realInfo).m_dataSet.data(),
			(*m_realInfo).m_strLen_or_Ind.data(),
			(*m_realInfo).m_columnNames,
			false);  // test

		TestGetResultColumn(0, // columnNumber
			SQL_C_DOUBLE,      // dataType
			sizeof(SQLDOUBLE), // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		TestGetResultColumn(1, // columnNumber
			SQL_C_DOUBLE,      // dataType
			sizeof(SQLDOUBLE), // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetDoubleResultColumnsTest
	//
	// Description:
	// Test GetResultColumn with default script using an OutputDataSet of Double columns.
	//
	TEST_F(RExtensionApiTest, GetDoubleResultColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession((*m_doubleInfo).GetColumnsNumber(),
			m_script,
			m_scriptString.length());

		InitializeColumns<SQLDOUBLE, SQL_C_DOUBLE>(m_doubleInfo.get());

		Execute<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
			ColumnInfo<SQLDOUBLE>::m_rowsNumber,
			(*m_doubleInfo).m_dataSet.data(),
			(*m_doubleInfo).m_strLen_or_Ind.data(),
			(*m_doubleInfo).m_columnNames,
			false);  // test

		TestGetResultColumn(0, // columnNumber
			SQL_C_DOUBLE,      // dataType
			sizeof(SQLDOUBLE), // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		TestGetResultColumn(1, // columnNumber
			SQL_C_DOUBLE,      // dataType
			sizeof(SQLDOUBLE), // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetBigIntResultColumnsTest
	//
	// Description:
	// Test GetResultColumn with default script using an OutputDataSet of BigInteger columns.
	//
	TEST_F(RExtensionApiTest, GetBigIntResultColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession((*m_bigIntInfo).GetColumnsNumber(),
			m_script,
			m_scriptString.length());

		InitializeColumns<SQLBIGINT, SQL_C_SBIGINT>(m_bigIntInfo.get());

		Execute<SQLBIGINT, Rcpp::NumericVector, SQL_C_SBIGINT>(
			ColumnInfo<SQLBIGINT>::m_rowsNumber,
			(*m_bigIntInfo).m_dataSet.data(),
			(*m_bigIntInfo).m_strLen_or_Ind.data(),
			(*m_bigIntInfo).m_columnNames,
			false); // test

		TestGetResultColumn(0, // columnNumber
			SQL_C_DOUBLE,      // dataType
			sizeof(SQLDOUBLE), // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		TestGetResultColumn(1, // columnNumber
			SQL_C_DOUBLE,      // dataType
			sizeof(SQLDOUBLE), // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetSmallIntResultColumnsTest
	//
	// Description:
	// Test GetResultColumn with default script using an OutputDataSet of SmallInt columns.
	//
	TEST_F(RExtensionApiTest, GetSmallIntResultColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession((*m_smallIntInfo).GetColumnsNumber(),
			m_script,
			m_scriptString.length());

		InitializeColumns<SQLSMALLINT, SQL_C_SSHORT>(m_smallIntInfo.get());

		Execute<SQLSMALLINT, Rcpp::IntegerVector, SQL_C_SSHORT>(
			ColumnInfo<SQLSMALLINT>::m_rowsNumber,
			(*m_smallIntInfo).m_dataSet.data(),
			(*m_smallIntInfo).m_strLen_or_Ind.data(),
			(*m_smallIntInfo).m_columnNames,
			false);  // test

		TestGetResultColumn(0,  // columnNumber
			SQL_C_SLONG,        // dataType
			sizeof(SQLINTEGER), // columnSize
			0,                  // decimalDigits
			SQL_NO_NULLS);      // nullable

		TestGetResultColumn(1,   // columnNumber
			SQL_C_SLONG,         // dataType
			sizeof(SQLINTEGER),  // columnSize
			0,                   // decimalDigits
			SQL_NO_NULLS);       // nullable
	}

	// Name: GetTinyIntResultColumnsTest
	//
	// Description:
	// Test GetResultColumn with default script using an OutputDataSet of TinyInt columns.
	//
	TEST_F(RExtensionApiTest, GetTinyIntResultColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession((*m_tinyIntInfo).GetColumnsNumber(),
			m_script,
			m_scriptString.length());

		InitializeColumns<SQLCHAR, SQL_C_UTINYINT>(m_tinyIntInfo.get());

		Execute<SQLCHAR, Rcpp::IntegerVector, SQL_C_UTINYINT>(
			ColumnInfo<SQLCHAR>::m_rowsNumber,
			(*m_tinyIntInfo).m_dataSet.data(),
			(*m_tinyIntInfo).m_strLen_or_Ind.data(),
			(*m_tinyIntInfo).m_columnNames,
			false);  // test

		TestGetResultColumn(0, // columnNumber
			SQL_C_SLONG,       // dataType
			sizeof(SQLINTEGER),// columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		TestGetResultColumn(1, // columnNumber
			SQL_C_SLONG,       // dataType
			sizeof(SQLINTEGER),// columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetCharResultColumnsTest
	//
	// Description:
	// Test GetResultColumn with default script using an OutputDataSet of Character columns.
	//
	TEST_F(RExtensionApiTest, GetCharResultColumnsTest)
	{
		SQLUSMALLINT inputSchemaColumnsNumber = 3;

		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(inputSchemaColumnsNumber,
			m_script,
			m_scriptString.length());

		string charColumn1Name = "CharColumn1";
		InitializeColumn(0, charColumn1Name, SQL_C_CHAR, sizeof(SQLCHAR));

		string charColumn2Name = "CharColumn2";
		InitializeColumn(1, charColumn2Name, SQL_C_CHAR, sizeof(SQLCHAR));

		string charColumn3Name = "CharColumn3";
		InitializeColumn(2, charColumn3Name, SQL_C_CHAR, sizeof(SQLCHAR));

		SQLULEN rowsNumber = 5;
		vector<const char*> charCol1{ "Hello", "test", "data", "World", "-123" };
		vector<const char*> charCol2{ "", 0, nullptr, "verify", "-1" };

		SQLINTEGER strLenOrIndCol1[rowsNumber] =
			{ static_cast<SQLINTEGER>(strlen(charCol1[0])),
			  static_cast<SQLINTEGER>(strlen(charCol1[1])),
			  static_cast<SQLINTEGER>(strlen(charCol1[2])),
			  static_cast<SQLINTEGER>(strlen(charCol1[3])),
			  static_cast<SQLINTEGER>(strlen(charCol1[4])) };
		SQLINTEGER strLenOrIndCol2[rowsNumber] =
			{ 0, SQL_NULL_DATA, SQL_NULL_DATA,
			  static_cast<SQLINTEGER>(strlen(charCol2[3])),
			  static_cast<SQLINTEGER>(strlen(charCol2[4])) };
		vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1, strLenOrIndCol2, nullptr };

		// Coalesce the arrays of each row of each column
		// into a contiguous array for each column.
		//
		SQLINTEGER maxCol1Len = 0, maxCol2Len = 0;
		SQLINTEGER charCol1TotalLen = GetSumOfLengths(strLenOrIndCol1, rowsNumber, &maxCol1Len);
		SQLINTEGER charCol2TotalLen = GetSumOfLengths(strLenOrIndCol2, rowsNumber, &maxCol2Len);
		char charCol1Data[charCol1TotalLen] = { 0 };
		char charCol2Data[charCol2TotalLen] = { 0 };
		GenerateContiguousData(charCol1Data, charCol1, strLenOrIndCol1);
		GenerateContiguousData(charCol2Data, charCol2, strLenOrIndCol2);
		void* dataSet[inputSchemaColumnsNumber] = { charCol1Data, charCol2Data, nullptr};

		vector<string> columnNames{charColumn1Name, charColumn2Name, charColumn3Name};

		ExecuteChar(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames,
			false); // test

		TestGetResultColumn(0, // columnNumber
			SQL_C_CHAR,        // dataType
			maxCol1Len,        // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		TestGetResultColumn(1, // columnNumber
			SQL_C_CHAR,        // dataType
			maxCol2Len,        // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable

		TestGetResultColumn(2, // columnNumber
			SQL_C_CHAR,        // dataType
			0,                 // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetRawResultColumnTest
	//
	// Description:
	// Test GetResultColumn with a script that returns OutputDataSet with a single raw column.
	//
	TEST_F(RExtensionApiTest, GetRawResultColumnTest)
	{
		string scriptString = "rawChars <- charToRaw(paste(letters[1:26], collapse=''));"
			"OutputDataSet <- data.frame(rawChars);"
			"print(OutputDataSet)";
		SQLCHAR *script = static_cast<SQLCHAR*>(
			static_cast<void*>(const_cast<char*>(scriptString.c_str())));

		// Initialize with a Session that executes the above script
		// that assigns creates an OutputDataSet with a single raw column.
		//
		InitializeSession(0,
			script,
			scriptString.length());

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
			nullptr,
			nullptr,
			&outputschemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputschemaColumnsNumber, 1);

		TestGetResultColumn(0, // columnNumber
			SQL_C_BINARY,      // dataType
			26,                // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		CleanupSession();

		scriptString = "rawChars <- raw(0);"
			"OutputDataSet <- data.frame(rawChars);"
			"print(OutputDataSet)";
		script = static_cast<SQLCHAR*>(
			static_cast<void*>(const_cast<char*>(scriptString.c_str())));

		// Initialize with a Session that executes the above script
		// that assigns creates an OutputDataSet with a null raw column.
		//
		InitializeSession(0,
			script,
			scriptString.length());

		outputschemaColumnsNumber = 0;
		result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
			nullptr,
			nullptr,
			&outputschemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputschemaColumnsNumber, 1);

		TestGetResultColumn(0, // columnNumber
			SQL_C_BINARY,      // dataType
			0,                 // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetDifferentResultColumnsTest
	//
	// Description:
	// Test GetResultColumn with default script using an OutputDataSet of different column types.
	//
	TEST_F(RExtensionApiTest, GetDifferentResultColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(3,
			m_script,
			m_scriptString.length());

		string integerColumnName = "IntegerColumn";
		InitializeColumn(0, integerColumnName, SQL_C_SLONG, sizeof(SQLINTEGER));

		string doubleColumnName = "DoubleColumn";
		InitializeColumn(1, doubleColumnName, SQL_C_DOUBLE, sizeof(SQLDOUBLE));

		string charColumnName = "CharColumn";
		InitializeColumn(2, charColumnName, SQL_C_CHAR, sizeof(SQLCHAR));

		SQLULEN rowsNumber = 5;
		vector<SQLINTEGER> intColData{ 2'147'483'647, -2'147'483'647, 0, -2'147'483'648, -1 };
		vector<SQLDOUBLE> doubleColData{ -1.79e301, 1.33, 83.98, 72.45, 1.79e30 };
		vector<const char*> charCol{ "Hello", "test", "data", "World", "-123" };

		SQLINTEGER strLenOrIndCol1[rowsNumber] = { 0, 0, SQL_NULL_DATA, SQL_NULL_DATA, 0 };
		SQLINTEGER strLenOrIndCol3[rowsNumber] =
			{ static_cast<SQLINTEGER>(strlen(charCol[0])),
			  static_cast<SQLINTEGER>(strlen(charCol[1])),
			  static_cast<SQLINTEGER>(strlen(charCol[2])),
			  static_cast<SQLINTEGER>(strlen(charCol[3])),
			  static_cast<SQLINTEGER>(strlen(charCol[4])) };
		vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1, nullptr, strLenOrIndCol3};

		SQLINTEGER maxLen = 0;
		SQLINTEGER charColTotalLen = GetSumOfLengths(strLenOrIndCol3, rowsNumber, &maxLen);
		char charColData[charColTotalLen] = { 0 };
		GenerateContiguousData(charColData, charCol, strLenOrIndCol3);

		vector<void *> dataSet { intColData.data(), doubleColData.data(), charColData};

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			rowsNumber,
			dataSet.data(),
			strLen_or_Ind.data(),
			&outputschemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		TestGetResultColumn(0, // columnNumber
			SQL_C_SLONG,       // dataType
			sizeof(SQLINTEGER),// columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable

		TestGetResultColumn(1, // columnNumber
			SQL_C_DOUBLE,      // dataType
			sizeof(SQLDOUBLE), // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		TestGetResultColumn(2, // columnNumber
			SQL_C_CHAR,        // dataType
			maxLen,            // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable
	}

	// Name: GetEmptyResultColumnTest
	//
	// Description:
	// Test GetResultColumn with a script that returns OutputDataSet with empty rows.
	//
	TEST_F(RExtensionApiTest, GetEmptyResultColumnTest)
	{
		string scriptString = ""
			"OutputDataSet <- data.frame(intCol = as.integer(c()))\n"
			"print(OutputDataSet)";
		SQLCHAR *script = static_cast<SQLCHAR*>(
			static_cast<void*>(const_cast<char*>(scriptString.c_str())));

		// Initialize with a Session that executes the above script
		// that creates an empty row 1 column OutputDataSet.
		//
		InitializeSession(0,
			script,
			scriptString.length());

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
			nullptr,
			nullptr,
			&outputschemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputschemaColumnsNumber, 1);

		TestGetResultColumn(0,  // columnNumber
			SQL_C_SLONG,        // dataType
			sizeof(SQLINTEGER), // columnSize
			0,                  // decimalDigits
			SQL_NO_NULLS);       // nullable
	}

	// Name: TestGetResultColumn
	//
	// Description:
	// Test GetResultColumn to verify the expected result column information is obtained.
	//
	void RExtensionApiTest::TestGetResultColumn(
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

		SQLRETURN result = (*m_getResultColumnFuncPtr)(
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
