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
// @File: RExtensionExecuteTests.cpp
//
// Purpose:
//  Tests the RExtension's implementation of the external language Execute API.
//
//*************************************************************************************************
#ifdef _WIN64
#include <windows.h>
#endif
#include <sqlext.h>
#include <sqltypes.h>
#include <sstream>
#include "gtest/gtest.h"
#include "RInside.h"
#include "RExtensionApiTest.h"

using namespace std;

namespace ExtensionApiTest
{
	// Name: ExecuteIntegerColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of Integer columns.
	//
	TEST_F(RExtensionApiTest, ExecuteIntegerColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(2);    // inputSchemaColumnsNumber

		string integerColumn1Name = "IntegerColumn1";
		InitializeColumn(0, integerColumn1Name, SQL_C_SLONG, sizeof(SQLINTEGER));

		string integerColumn2Name = "IntegerColumn2";
		InitializeColumn(1, integerColumn2Name, SQL_C_SLONG, sizeof(SQLINTEGER));

		SQLULEN rowsNumber = 5;
		vector<SQLINTEGER> intCol1Data{ 1, 2, 3, 4, 5 };
		vector<SQLINTEGER> intCol2Data{ 2'147'483'647, -2'147'483'647, 0, -2'147'483'648, -1 };
		vector<void*> dataSet = { intCol1Data.data(), intCol2Data.data() };

		SQLINTEGER strLenOrIndCol2[rowsNumber] = { 0, 0, SQL_NULL_DATA, SQL_NULL_DATA, 0 };
		vector<SQLINTEGER*> strLen_or_Ind{ nullptr, strLenOrIndCol2 };

		vector<string> columnNames{ integerColumn1Name, integerColumn2Name };

		TestExecute<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
			rowsNumber,
			dataSet.data(),
			strLen_or_Ind.data(),
			columnNames);
	}

	// Name: ExecuteLogicalColumnsTest
	//
	// Description:
	// Test Execute using an InputDataSet of Logical columns.
	//
	TEST_F(RExtensionApiTest, ExecuteLogicalColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(2);    // inputSchemaColumnsNumber

		string logicalColumn1Name = "LogicalColumn1";
		InitializeColumn(0, logicalColumn1Name, SQL_C_BIT, sizeof(SQLCHAR));

		string logicalColumn2Name = "LogicalColumn2";
		InitializeColumn(1, logicalColumn2Name, SQL_C_BIT, sizeof(SQLCHAR));

		SQLULEN rowsNumber = 5;
		vector<SQLCHAR> logicalCol1Data{ '1', '0', '1', '0', '1' };
		vector<SQLCHAR> logicalCol2Data{ '0', '2', '1', '0', '1' };
		vector<void*> dataSet = { logicalCol1Data.data(), logicalCol2Data.data() };

		SQLINTEGER strLenOrIndCol2[rowsNumber] = { SQL_NULL_DATA, 0, 0, 0, SQL_NULL_DATA };
		vector<SQLINTEGER*> strLen_or_Ind{ nullptr, strLenOrIndCol2 };

		vector<string> columnNames{ logicalColumn1Name, logicalColumn2Name };

		TestExecute<SQLCHAR, Rcpp::LogicalVector, SQL_C_BIT>(
			rowsNumber,
			dataSet.data(),
			strLen_or_Ind.data(),
			columnNames);
	}

	// Name: ExecuteRealColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of Real columns.
	//
	TEST_F(RExtensionApiTest, ExecuteRealColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(2);    // inputSchemaColumnsNumber

		string realColumn1Name = "RealColumn1";
		InitializeColumn(0, realColumn1Name, SQL_C_FLOAT, sizeof(SQLREAL));

		string realColumn2Name = "RealColumn2";
		InitializeColumn(1, realColumn2Name, SQL_C_FLOAT, sizeof(SQLREAL));

		SQLULEN rowsNumber = 5;
		vector<SQLREAL> realCol1Data{ 0.34, 1.33, 83.98, 72.45, 68e10 };
		vector<SQLREAL> realCol2Data{ 3.4e38F, 0, -3.4e38F, -1, 0 };
		vector<void*> dataSet = { realCol1Data.data(), realCol2Data.data() };

		SQLINTEGER strLenOrIndCol2[rowsNumber] = { 0, SQL_NULL_DATA, 0, SQL_NULL_DATA, 0 };
		vector<SQLINTEGER*> strLen_or_Ind{ nullptr, strLenOrIndCol2 };

		vector<string> columnNames{ realColumn1Name, realColumn2Name };

		TestExecute<SQLREAL, Rcpp::NumericVector, SQL_C_FLOAT>(
			rowsNumber,
			dataSet.data(),
			strLen_or_Ind.data(),
			columnNames);
	}

	// Name: ExecuteDoubleColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of Double columns.
	//
	TEST_F(RExtensionApiTest, ExecuteDoubleColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(2);    // inputSchemaColumnsNumber

		string doubleColumn1Name = "DoubleColumn1";
		InitializeColumn(0, doubleColumn1Name, SQL_C_DOUBLE, sizeof(SQLDOUBLE));

		string doubleColumn2Name = "DoubleColumn2";
		InitializeColumn(1, doubleColumn2Name, SQL_C_DOUBLE, sizeof(SQLDOUBLE));

		SQLULEN rowsNumber = 5;
		vector<SQLDOUBLE> doubleCol1Data{ -1.79e301, 1.33, 83.98, 72.45, 1.79e30 };
		vector<SQLDOUBLE> doubleCol2Data{ 0, 1.79e308, 0, -1.79e308, -1 };
		vector<void*> dataSet = { doubleCol1Data.data(), doubleCol2Data.data() };

		SQLINTEGER strLenOrIndCol2[rowsNumber] = { SQL_NULL_DATA, 0,
			SQL_NULL_DATA, 0, SQL_NULL_DATA };
		vector<SQLINTEGER*> strLen_or_Ind{ nullptr, strLenOrIndCol2 };

		vector<string> columnNames{ doubleColumn1Name, doubleColumn2Name };

		TestExecute<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
			rowsNumber,
			dataSet.data(),
			strLen_or_Ind.data(),
			columnNames);
	}

	// Name: ExecuteBigIntColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of BigInteger columns.
	//
	TEST_F(RExtensionApiTest, ExecuteBigIntColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(2);    // inputSchemaColumnsNumber

		string bigIntColumn1Name = "BigIntColumn1";
		InitializeColumn(0, bigIntColumn1Name, SQL_C_SBIGINT, sizeof(SQLBIGINT));

		string bigIntColumn2Name = "BigIntColumn2";
		InitializeColumn(1, bigIntColumn2Name, SQL_C_SBIGINT, sizeof(SQLBIGINT));

		SQLULEN rowsNumber = 5;
		vector<SQLBIGINT> bigIntCol1Data{ 9'223'372'036'854'775'807LL, 1,
			88883939, -9'223'372'036'854'775'807LL, -622280108 };
		vector<SQLBIGINT> bigIntCol2Data(rowsNumber, 0);
		vector<void*> dataSet = { bigIntCol1Data.data(), bigIntCol2Data.data() };

		SQLINTEGER strLenOrIndCol2[] = { SQL_NULL_DATA, SQL_NULL_DATA,
			SQL_NULL_DATA, SQL_NULL_DATA, SQL_NULL_DATA };
		vector<SQLINTEGER*> strLen_or_Ind{ nullptr, strLenOrIndCol2 };

		vector<string> columnNames{ bigIntColumn1Name, bigIntColumn2Name };

		TestExecute<SQLBIGINT, Rcpp::NumericVector, SQL_C_SBIGINT>(
			rowsNumber,
			dataSet.data(),
			strLen_or_Ind.data(),
			columnNames);
	}

	// Name: ExecuteSmallIntColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of SmallInt columns.
	//
	TEST_F(RExtensionApiTest, ExecuteSmallIntColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(2);    // inputSchemaColumnsNumber

		string smallIntColumn1Name = "SmallIntColumn1";
		InitializeColumn(0, smallIntColumn1Name, SQL_C_SSHORT, sizeof(SQLSMALLINT));

		string smallIntColumn2Name = "SmallIntColumn2";
		InitializeColumn(1, smallIntColumn2Name, SQL_C_SSHORT, sizeof(SQLSMALLINT));

		SQLULEN rowsNumber = 5;
		vector<SQLSMALLINT> smallIntCol1Data{ 223, 33, 9811, -725, 6810 };
		vector<SQLSMALLINT> smallIntCol2Data{ -1, 0, 32'767, -32'768, 3'276 };
		vector<void*> dataSet = { smallIntCol1Data.data(), smallIntCol2Data.data() };

		SQLINTEGER strLenOrIndCol2[rowsNumber] = { 0 };
		vector<SQLINTEGER*> strLen_or_Ind{ nullptr, strLenOrIndCol2 };

		vector<string> columnNames{ smallIntColumn1Name, smallIntColumn2Name };

		TestExecute<SQLSMALLINT, Rcpp::IntegerVector, SQL_C_SSHORT>(
			rowsNumber,
			dataSet.data(),
			strLen_or_Ind.data(),
			columnNames);
	}

	// Name: ExecuteTinyIntColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of TinyInt columns.
	//
	TEST_F(RExtensionApiTest, ExecuteTinyIntColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(2);    // inputSchemaColumnsNumber

		string tinyIntColumn1Name = "TinyIntColumn1";
		InitializeColumn(0, tinyIntColumn1Name, SQL_C_UTINYINT, sizeof(SQLCHAR));

		string tinyIntColumn2Name = "TinyIntColumn2";
		InitializeColumn(1, tinyIntColumn2Name, SQL_C_UTINYINT, sizeof(SQLCHAR));

		SQLULEN rowsNumber = 5;
		vector<SQLCHAR> tinyIntCol1Data{ 34, 133, 98, 72, 10 };
		vector<SQLCHAR> tinyIntCol2Data{ 255, 0, 1, 0, 128 };
		vector<void*> dataSet = { tinyIntCol1Data.data(), tinyIntCol2Data.data() };

		SQLINTEGER strLenOrIndCol2[rowsNumber] = { 0, SQL_NULL_DATA,
			SQL_NULL_DATA, SQL_NULL_DATA, 0 };
		vector<SQLINTEGER*> strLen_or_Ind{ nullptr, strLenOrIndCol2 };

		vector<string> columnNames{ tinyIntColumn1Name, tinyIntColumn2Name };

		TestExecute<SQLCHAR, Rcpp::IntegerVector, SQL_C_UTINYINT>(
			rowsNumber,
			dataSet.data(),
			strLen_or_Ind.data(),
			columnNames);
	}

	// Name: ExecuteCharColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of Character columns.
	//
	TEST_F(RExtensionApiTest, ExecuteCharColumnsTest)
	{
		SQLUSMALLINT inputSchemaColumnsNumber = 3;

		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(inputSchemaColumnsNumber);

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
		SQLINTEGER charCol1TotalLen = GetSumOfLengths(strLenOrIndCol1, rowsNumber);
		SQLINTEGER charCol2TotalLen = GetSumOfLengths(strLenOrIndCol2, rowsNumber);
		char charCol1Data[charCol1TotalLen] = { 0 };
		char charCol2Data[charCol2TotalLen] = { 0 };
		GenerateContiguousData<char>(charCol1Data, charCol1, strLenOrIndCol1);
		GenerateContiguousData<char>(charCol2Data, charCol2, strLenOrIndCol2);
		void* dataSet[inputSchemaColumnsNumber] = { charCol1Data, charCol2Data, nullptr};

		vector<string> columnNames{charColumn1Name, charColumn2Name, charColumn3Name};

		TestExecuteChar(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames);
	}

	// Name: ExecuteDifferentColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of different column types.
	//
	TEST_F(RExtensionApiTest, ExecuteDifferentColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(3);    // inputSchemaColumnsNumber

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

		SQLINTEGER charColTotalLen = GetSumOfLengths(strLenOrIndCol3, rowsNumber);
		char charColData[charColTotalLen] = { 0 };
		GenerateContiguousData<char>(charColData, charCol, strLenOrIndCol3);

		vector<void *> dataSet { intColData.data(), doubleColData.data(), charColData};

		testing::internal::CaptureStdout();

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			rowsNumber,
			dataSet.data(),
			strLen_or_Ind.data(),
			&outputschemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		// Test print message was printed correctly
		//
		string output = testing::internal::GetCapturedStdout();
		cout << output;
		ASSERT_TRUE(output.find("[1] \"" + m_printMessage + "\"") != string::npos);

		// Test InputDataSet
		//
		Rcpp::DataFrame inputDataSet = m_globalEnvironment[m_inputDataNameString.c_str()];

		Rcpp::IntegerVector intColumn = inputDataSet[integerColumnName.c_str()];
		CheckVectorEquality<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
			rowsNumber,
			intColumn,
			dataSet[0],
			strLen_or_Ind[0]);

		Rcpp::NumericVector numericColumn = inputDataSet[doubleColumnName.c_str()];
		CheckVectorEquality<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
			rowsNumber,
			numericColumn,
			dataSet[1],
			strLen_or_Ind[1]);

		Rcpp::CharacterVector charColumn = inputDataSet[charColumnName.c_str()];
		CheckCharacterVectorEquality(
			rowsNumber,
			charColumn,
			dataSet[2],
			strLen_or_Ind[2]);

		// Test OutputDataSet
		//
		Rcpp::DataFrame outputDataSet = m_globalEnvironment[m_outputDataNameString.c_str()];
		EXPECT_EQ(outputschemaColumnsNumber, outputDataSet.size());
		ASSERT_EQ(outputDataSet.size(), inputDataSet.size());
		CheckVectorEquality<Rcpp::IntegerVector>(
			outputDataSet[0],
			inputDataSet[0]);
	}

	// Name: GenerateContiguousData
	//
	// Description:
	// Fill a contiguous array columnData with members from the given columnVector
	// having lengths defined in strLenOrInd, unless it is SQL_NULL_DATA.
	//
	template<class SQLType>
	void RExtensionApiTest::GenerateContiguousData(
		SQLType                *columnData,
		vector<const SQLType*> columnVector,
		SQLINTEGER             *strLenOrInd)
	{
		SQLINTEGER baseIndex = 0;
		for (SQLULEN index = 0 ; index < columnVector.size(); index++)
		{
			if (strLenOrInd[index] != SQL_NULL_DATA)
			{
				memcpy(columnData + baseIndex, columnVector[index], strLenOrInd[index]);
				baseIndex += strLenOrInd[index];
			}
		}
	}

	// Name: GetSumOfLengths
	//
	// Description:
	// Get sum of lengths in strLenOrInd skipping SQL_NULL_DATA.
	//
	SQLINTEGER RExtensionApiTest::GetSumOfLengths(
		SQLINTEGER *strLenOrInd,
		SQLULEN    rowsNumber)
	{
		SQLINTEGER sumOfLengths = 0;
		for(SQLULEN index = 0 ; index < rowsNumber; index++)
		{
			if(strLenOrInd[index] != SQL_NULL_DATA)
			{
				sumOfLengths += strLenOrInd[index];
			}
		}

		return sumOfLengths;
	}

	// Name: InitializeColumn
	//
	// Description:
	// Call InitColumn for the given columnNumber, columnName, dataType and columnSize.
	//
	void RExtensionApiTest::InitializeColumn(
		SQLSMALLINT columnNumber,
		string      columnNameString,
		SQLSMALLINT dataType,
		SQLULEN     columnSize)
	{
		SQLCHAR *columnName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(columnNameString.c_str()))
			);

		SQLRETURN result = SQL_ERROR;

		result = (*m_initColumnFuncPtr)(
				*m_sessionId,
				m_taskId,
				columnNumber,
				columnName,
				columnNameString.length(),
				dataType,
				columnSize,
				0,         // decimalDigits
				1,         // nullable
				-1,        // partitionByNumber
				-1);       // orderByNumber
		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Name: TestExecute
	//
	// Description:
	// Templatized function to Test Execute with default script that assigns Input to Output.
	// for integer/numeric/logical data types.
	// It tests the correctness of the:
	//  1. Executed script,
	//  2. InputDataSet and
	//  3. OutputDataSet
	//
	template<class SQLType, class RType, SQLSMALLINT dataType>
	void RExtensionApiTest::TestExecute(
		SQLULEN        rowsNumber,
		void           **dataSet,
		SQLINTEGER     **strLen_or_Ind,
		vector<string> columnNames)
	{
		testing::internal::CaptureStdout();

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			rowsNumber,
			dataSet,
			strLen_or_Ind,
			&outputschemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		// Test print message was printed correctly
		//
		string output = testing::internal::GetCapturedStdout();
		cout << output;
		ASSERT_TRUE(output.find("[1] \"" + m_printMessage + "\"") != string::npos);

		// Test InputDataSet
		//
		Rcpp::DataFrame inputDataSet = m_globalEnvironment[m_inputDataNameString.c_str()];

		for (SQLUSMALLINT columnIndex = 0; columnIndex < columnNames.size(); columnIndex++)
		{
			RType column = inputDataSet[columnNames[columnIndex].c_str()];
			CheckVectorEquality<SQLType, RType, dataType>(
				rowsNumber,
				column,
				dataSet[columnIndex],
				strLen_or_Ind[columnIndex]);
		}

		// Test OutputDataSet
		//
		Rcpp::DataFrame outputDataSet = m_globalEnvironment[m_outputDataNameString.c_str()];
		EXPECT_EQ(outputschemaColumnsNumber, outputDataSet.size());
		CheckDataFrameEquality<RType>(
			outputDataSet,
			inputDataSet);
	}

	// Name: TestExecuteChar
	//
	// Description:
	// Test Execute with default script for Character columns.
	// It tests the correctness of the:
	//  1. Executed script,
	//  2. InputDataSet and
	//  3. OutputDataSet
	//
	void RExtensionApiTest::TestExecuteChar(
		SQLULEN        rowsNumber,
		void           **dataSet,
		SQLINTEGER     **strLen_or_Ind,
		vector<string> columnNames)
	{
		testing::internal::CaptureStdout();

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			rowsNumber,
			dataSet,
			strLen_or_Ind,
			&outputschemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		// Test print message was printed correctly
		//
		string output = testing::internal::GetCapturedStdout();
		cout << output;
		ASSERT_TRUE(output.find("[1] \"" + m_printMessage + "\"") != string::npos);

		// Test InputDataSet
		//
		Rcpp::DataFrame inputDataSet = m_globalEnvironment[m_inputDataNameString.c_str()];

		for (SQLUSMALLINT columnIndex = 0; columnIndex < columnNames.size(); columnIndex++)
		{
			Rcpp::CharacterVector column = inputDataSet[columnNames[columnIndex].c_str()];
			CheckCharacterVectorEquality(
				rowsNumber,
				column,
				dataSet[columnIndex],
				strLen_or_Ind[columnIndex]);
		}

		// Test OutputDataSet
		//
		Rcpp::DataFrame outputDataSet = m_globalEnvironment[m_outputDataNameString.c_str()];
		EXPECT_EQ(outputschemaColumnsNumber, outputDataSet.size());
		CheckDataFrameEquality<Rcpp::CharacterVector>(
			outputDataSet,
			inputDataSet);
	}

	// Name: CheckVectorEquality
	//
	// Description:
	// Templatized function to compare the given vector and data for equality.
	// for integer/numeric/logical data types.
	// The expectedData is input as a void*, hence we input the expectedRowsNumber as well.
	// Where strLen_or_Ind == SQL_NULL_DATA, check for is_na.
	//
	template<class SQLType, class RType, SQLSMALLINT dataType>
	void RExtensionApiTest::CheckVectorEquality(
		SQLULEN    expectedRowsNumber,
		RType      vectorToTest,
		void       *expectedData,
		SQLINTEGER *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<size_t>(vectorToTest.size()), expectedRowsNumber);
		for(SQLULEN index = 0 ; index < expectedRowsNumber; index++)
		{
			if (strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA)
			{
				EXPECT_TRUE(RType::is_na(vectorToTest[index]));
			}
			else
			{
				SQLType expectedValue = static_cast<SQLType*>(expectedData)[index];
				if (dataType == SQL_C_BIT)
				{
					EXPECT_EQ(vectorToTest[index],
						expectedValue != '0' ? true : false);
				}
				else
				{
					EXPECT_EQ(vectorToTest[index], expectedValue);
				}
			}
		}
	}

	// Name: CheckCharacterVectorEquality
	//
	// Description:
	// Compare character vector with the given data and corresponding strLen_or_Ind.
	// The expectedData is input as a void*, hence we input the expectedRowsNumber as well.
	// Where strLen_or_Ind == SQL_NULL_DATA, check for is_na.
	//
	void RExtensionApiTest::CheckCharacterVectorEquality(
		SQLULEN               expectedRowsNumber,
		Rcpp::CharacterVector vectorToTest,
		void                  *expectedData,
		SQLINTEGER            *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<size_t>(vectorToTest.size()), expectedRowsNumber);
		SQLINTEGER cumulativeLength = 0 ;
		for(SQLULEN index = 0 ; index < expectedRowsNumber; index++)
		{
			if (strLen_or_Ind == nullptr ||
			  (strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA))
			{
				EXPECT_TRUE(Rcpp::CharacterVector::is_na(vectorToTest[index]));
			}
			else
			{
				string expectedString = string(
					static_cast<char*>(expectedData) + cumulativeLength,
					strLen_or_Ind[index]);
				EXPECT_EQ(vectorToTest[index], expectedString);
				cumulativeLength += strLen_or_Ind[index];
			}
		}
	}

	// Name: CheckVectorEquality
	//
	// Description:
	// Templatized function to compare the given vectors for equality
	//
	template<class RType>
	void RExtensionApiTest::CheckVectorEquality(
		RType vectorToTest,
		RType expectedVector)
	{
		ASSERT_EQ(vectorToTest.size(), expectedVector.size());
		SQLULEN expectedRowsNumber = expectedVector.size();
		for(SQLULEN index = 0 ; index < expectedRowsNumber; index++)
		{
			if (RType::is_na(expectedVector[index]))
			{
				EXPECT_TRUE(RType::is_na(vectorToTest[index]));
			}
			else
			{
				EXPECT_EQ(vectorToTest[index], expectedVector[index]);
			}
		}
	}

	// Name: CheckDataFrameEquality
	//
	// Description:
	// Templatized function to compare the given dataframes for equality.
	//
	template<class RType>
	void RExtensionApiTest::CheckDataFrameEquality(
		Rcpp::DataFrame dataFrameToTest,
		Rcpp::DataFrame expectedDataFrame)
	{
		ASSERT_EQ(dataFrameToTest.size(), expectedDataFrame.size());
		SQLUSMALLINT expectedColumns = dataFrameToTest.size();
		for(SQLUSMALLINT index = 0 ; index < expectedColumns; index++)
		{
			CheckVectorEquality<RType>(
				dataFrameToTest[index],
				expectedDataFrame[index]);
		}
	}
}
