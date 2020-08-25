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
// @File: RExtensionGetResultsTests.cpp
//
// Purpose:
//  Tests the RExtension's implementation of the external language GetResults API.
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
	// Name: GetIntegerResultsTest
	//
	// Description:
	// Test GetResults with default script expecting an OutputDataSet of Integer column data.
	//
	TEST_F(RExtensionApiTest, GetIntegerResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			(*m_integerInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLINTEGER, SQL_C_SLONG>(m_integerInfo.get());

		Execute<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
			ColumnInfo<SQLINTEGER>::m_rowsNumber,
			(*m_integerInfo).m_dataSet.data(),
			(*m_integerInfo).m_strLen_or_Ind.data(),
			(*m_integerInfo).m_columnNames,
			false);  // validate

		GetResults<SQLINTEGER, Rcpp::IntegerVector, SQLINTEGER, SQL_C_SLONG>(
			ColumnInfo<SQLINTEGER>::m_rowsNumber,
			(*m_integerInfo).m_dataSet.data(),
			(*m_integerInfo).m_strLen_or_Ind.data(),
			(*m_integerInfo).m_columnNames);
	}

	// Name: GetLogicalResultsTest
	//
	// Description:
	// Test GetResults with default script expecting an OutputDataSet of Logical column data.
	//
	TEST_F(RExtensionApiTest, GetLogicalResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			(*m_logicalInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLCHAR, SQL_C_BIT>(m_logicalInfo.get());

		Execute<SQLCHAR, Rcpp::LogicalVector, SQL_C_BIT>(
			ColumnInfo<SQLCHAR>::m_rowsNumber,
			(*m_logicalInfo).m_dataSet.data(),
			(*m_logicalInfo).m_strLen_or_Ind.data(),
			(*m_logicalInfo).m_columnNames,
			false);  // validate

		GetResults<SQLCHAR, Rcpp::LogicalVector, SQLCHAR, SQL_C_BIT>(
			ColumnInfo<SQLCHAR>::m_rowsNumber,
			(*m_logicalInfo).m_dataSet.data(),
			(*m_logicalInfo).m_strLen_or_Ind.data(),
			(*m_logicalInfo).m_columnNames);
	}

	// Name: GetRealResultsTest
	//
	// Description:
	// Test GetResults with default script expecting an OutputDataSet of Real column data.
	//
	TEST_F(RExtensionApiTest, GetRealResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			(*m_realInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLREAL, SQL_C_FLOAT>(m_realInfo.get());

		Execute<SQLREAL, Rcpp::NumericVector, SQL_C_FLOAT>(
			ColumnInfo<SQLREAL>::m_rowsNumber,
			(*m_realInfo).m_dataSet.data(),
			(*m_realInfo).m_strLen_or_Ind.data(),
			(*m_realInfo).m_columnNames,
			false);  // validate

		GetResults<SQLREAL, Rcpp::NumericVector, SQLDOUBLE, SQL_C_DOUBLE>(
			ColumnInfo<SQLREAL>::m_rowsNumber,
			(*m_realInfo).m_dataSet.data(),
			(*m_realInfo).m_strLen_or_Ind.data(),
			(*m_realInfo).m_columnNames);
	}

	// Name: GetDoubleResultsTest
	//
	// Description:
	// Test GetResults with default script expecting an OutputDataSet of Double column data.
	//
	TEST_F(RExtensionApiTest, GetDoubleResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			(*m_doubleInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLDOUBLE, SQL_C_DOUBLE>(m_doubleInfo.get());

		Execute<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
			ColumnInfo<SQLDOUBLE>::m_rowsNumber,
			(*m_doubleInfo).m_dataSet.data(),
			(*m_doubleInfo).m_strLen_or_Ind.data(),
			(*m_doubleInfo).m_columnNames,
			false);  // validate

		GetResults<SQLDOUBLE, Rcpp::NumericVector, SQLDOUBLE, SQL_C_DOUBLE>(
			ColumnInfo<SQLDOUBLE>::m_rowsNumber,
			(*m_doubleInfo).m_dataSet.data(),
			(*m_doubleInfo).m_strLen_or_Ind.data(),
			(*m_doubleInfo).m_columnNames);
	}

	// Name: GetBigIntResultsTest
	//
	// Description:
	// Test GetResults with default script expecting an OutputDataSet of BigInteger columns.
	//
	TEST_F(RExtensionApiTest, GetBigIntResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			(*m_bigIntInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLBIGINT, SQL_C_SBIGINT>(m_bigIntInfo.get());

		Execute<SQLBIGINT, Rcpp::NumericVector, SQL_C_SBIGINT>(
			ColumnInfo<SQLBIGINT>::m_rowsNumber,
			(*m_bigIntInfo).m_dataSet.data(),
			(*m_bigIntInfo).m_strLen_or_Ind.data(),
			(*m_bigIntInfo).m_columnNames,
			false); // validate

		GetResults<SQLBIGINT, Rcpp::NumericVector, SQLDOUBLE, SQL_C_DOUBLE>(
			ColumnInfo<SQLBIGINT>::m_rowsNumber,
			(*m_bigIntInfo).m_dataSet.data(),
			(*m_bigIntInfo).m_strLen_or_Ind.data(),
			(*m_bigIntInfo).m_columnNames);
	}

	// Name: GetSmallIntResultsTest
	//
	// Description:
	// Test GetResults with default script expecting an OutputDataSet of SmallInt columns.
	//
	TEST_F(RExtensionApiTest, GetSmallIntResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			(*m_smallIntInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLSMALLINT, SQL_C_SSHORT>(m_smallIntInfo.get());

		Execute<SQLSMALLINT, Rcpp::IntegerVector, SQL_C_SSHORT>(
			ColumnInfo<SQLSMALLINT>::m_rowsNumber,
			(*m_smallIntInfo).m_dataSet.data(),
			(*m_smallIntInfo).m_strLen_or_Ind.data(),
			(*m_smallIntInfo).m_columnNames,
			false);  // validate

		GetResults<SQLSMALLINT, Rcpp::IntegerVector, SQLINTEGER, SQL_C_SLONG>(
			ColumnInfo<SQLSMALLINT>::m_rowsNumber,
			(*m_smallIntInfo).m_dataSet.data(),
			(*m_smallIntInfo).m_strLen_or_Ind.data(),
			(*m_smallIntInfo).m_columnNames);
	}

	// Name: GetTinyIntResultColumnsTest
	//
	// Description:
	// Test GetResultColumn with default script using an OutputDataSet of TinyInt columns.
	//
	TEST_F(RExtensionApiTest, GetTinyIntResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			(*m_tinyIntInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLCHAR, SQL_C_UTINYINT>(m_tinyIntInfo.get());

		Execute<SQLCHAR, Rcpp::IntegerVector, SQL_C_UTINYINT>(
			ColumnInfo<SQLCHAR>::m_rowsNumber,
			(*m_tinyIntInfo).m_dataSet.data(),
			(*m_tinyIntInfo).m_strLen_or_Ind.data(),
			(*m_tinyIntInfo).m_columnNames,
			false);  // validate

		GetResults<SQLCHAR, Rcpp::IntegerVector, SQLINTEGER, SQL_C_SLONG>(
			ColumnInfo<SQLCHAR>::m_rowsNumber,
			(*m_tinyIntInfo).m_dataSet.data(),
			(*m_tinyIntInfo).m_strLen_or_Ind.data(),
			(*m_tinyIntInfo).m_columnNames);
	}

	// Name: GetCharResultsTest
	//
	// Description:
	// Test GetResults with default script expecting an OutputDataSet of Character columns.
	//
	TEST_F(RExtensionApiTest, GetCharResultsTest)
	{
		SQLUSMALLINT inputSchemaColumnsNumber = 3;

		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			inputSchemaColumnsNumber,
			m_scriptString);

		string charColumn1Name = "CharColumn1";
		InitializeColumn(0, charColumn1Name, SQL_C_CHAR, m_CharSize);

		string charColumn2Name = "CharColumn2";
		InitializeColumn(1, charColumn2Name, SQL_C_CHAR, m_CharSize);

		string charColumn3Name = "CharColumn3";
		InitializeColumn(2, charColumn3Name, SQL_C_CHAR, m_CharSize);

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
		SQLINTEGER maxCol1Len = 0, maxCol2Len = 0;
		SQLINTEGER charCol1TotalLen = GetSumOfLengths(strLenOrIndCol1.data(), rowsNumber, &maxCol1Len);
		SQLINTEGER charCol2TotalLen = GetSumOfLengths(strLenOrIndCol2.data(), rowsNumber, &maxCol2Len);
		char charCol1Data[charCol1TotalLen] = { 0 };
		char charCol2Data[charCol2TotalLen] = { 0 };
		GenerateContiguousData(charCol1Data, charCol1, strLenOrIndCol1.data());
		GenerateContiguousData(charCol2Data, charCol2, strLenOrIndCol2.data());
		void* dataSet[inputSchemaColumnsNumber] = { charCol1Data, charCol2Data, nullptr};

		vector<string> columnNames{charColumn1Name, charColumn2Name, charColumn3Name};

		ExecuteChar(
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

	// Name: GetRawResultsTest
	//
	// Description:
	// Test GetResults with a script that returns OutputDataSet with a single raw column.
	//
	TEST_F(RExtensionApiTest, GetRawResultsTest)
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
		SQLRETURN result = (*m_executeFuncPtr)(
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

		result = (*m_getResultsFuncPtr)(
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
		result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
			nullptr,
			nullptr,
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 1);

		result = (*m_getResultsFuncPtr)(
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

	// Name: GetDifferentColumnResultsTest
	//
	// Description:
	// Test GetResults with default script using an OutputDataSet of different column types.
	//
	TEST_F(RExtensionApiTest, GetDifferentColumnResultsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(
			3,               // inputSchemaColumnsNumber
			m_scriptString);

		string integerColumnName = "IntegerColumn";
		InitializeColumn(0, integerColumnName, SQL_C_SLONG, m_IntSize);

		string doubleColumnName = "DoubleColumn";
		InitializeColumn(1, doubleColumnName, SQL_C_DOUBLE, m_DoubleSize);

		string charColumnName = "CharColumn";
		InitializeColumn(2, charColumnName, SQL_C_CHAR, m_CharSize);

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

		SQLINTEGER maxLen = 0;
		SQLINTEGER charColTotalLen = GetSumOfLengths(strLenOrIndCol3.data(), expectedRowsNumber, &maxLen);
		char charColData[charColTotalLen] = { 0 };
		GenerateContiguousData(charColData, charCol, strLenOrIndCol3.data());

		vector<void *> expectedData { intColData.data(), doubleColData.data(), charColData};

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = (*m_executeFuncPtr)(
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
		result = (*m_getResultsFuncPtr)(
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
		CheckVectorEquality<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
			rowsNumber,
			intColumn,
			data[0],
			strLen_or_Ind[0]);

		Rcpp::NumericVector numericColumn = outputDataSet[doubleColumnName.c_str()];
		CheckVectorEquality<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
			rowsNumber,
			numericColumn,
			data[1],
			strLen_or_Ind[1]);

		Rcpp::CharacterVector charColumn = outputDataSet[charColumnName.c_str()];
		CheckCharacterVectorEquality(
			rowsNumber,
			charColumn,
			data[2],
			strLen_or_Ind[2]);
	}

	// Name: GetEmptyResultsTest
	//
	// Description:
	// Test GetResults with a script that returns OutputDataSet with empty rows.
	//
	TEST_F(RExtensionApiTest, GetEmptyResultsTest)
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
		SQLRETURN result = (*m_executeFuncPtr)(
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

	// Name: GetResults
	//
	// Description:
	// Test GetResults to verify the expected results are obtained.
	// For numeric, logical and integer types.
	//
	template<class InputSQLType, class RType, class OutputSQLType, SQLSMALLINT outputDataType>
	void RExtensionApiTest::GetResults(
		SQLULEN        expectedRowsNumber,
		SQLPOINTER     *expectedData,
		SQLINTEGER     **expectedStrLen_or_Ind,
		vector<string> columnNames)
	{
		SQLULEN    rowsNumber = 0;
		SQLPOINTER *data = nullptr;
		SQLINTEGER **strLen_or_Ind = nullptr;
		SQLRETURN result = (*m_getResultsFuncPtr)(
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

			RType column = outputDataSet[columnNames[columnNumber].c_str()];
			CheckVectorEquality<OutputSQLType, RType, outputDataType>(
				rowsNumber,
				column,
				columnData,
				columnStrLenOrInd);
		}
	}

	// Name: CheckColumnDataEquality
	//
	// Description:
	// Templatized function to compare the given column data
	// and nullMap with rowsNumber for equality.
	//
	template<class InputSQLType, class OutputSQLType, SQLSMALLINT outputDataType>
	void RExtensionApiTest::CheckColumnDataEquality(
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
				if (is_same<OutputSQLType, SQLDOUBLE>::value)
				{
					EXPECT_TRUE(isnan(static_cast<SQLDOUBLE>(columnData[index])));
				}
				else
				{
					if (is_same<InputSQLType, OutputSQLType>::value)
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
				if (outputDataType == SQL_C_BIT && expectedColumnData[index] != '0')
				{
					EXPECT_EQ(columnData[index], '1');
				}
				else
				{
					EXPECT_EQ(columnData[index], expectedColumnData[index]);
				}
			}
		}
	}

	// Name: GetCharResults
	//
	// Description:
	// Test GetResults to verify the expected results are obtained for character data.
	//
	void RExtensionApiTest::GetCharResults(
		SQLULEN        expectedRowsNumber,
		SQLPOINTER     *expectedData,
		SQLINTEGER     **expectedStrLen_or_Ind,
		vector<string> columnNames)
	{
		SQLULEN    rowsNumber = 0;
		SQLPOINTER *data = nullptr;
		SQLINTEGER **strLen_or_Ind = nullptr;
		SQLRETURN result = (*m_getResultsFuncPtr)(
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
			CheckCharacterVectorEquality(
				rowsNumber,
				column,
				columnData,
				columnStrLenOrInd);
		}
	}

	// Name: CheckCharDataEquality
	//
	// Description:
	// Compare the given character data & nullMap with rowsNumber for equality.
	//
	void RExtensionApiTest::CheckCharDataEquality(
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
}
