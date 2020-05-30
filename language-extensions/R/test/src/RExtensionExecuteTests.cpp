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
			(*m_integerInfo).m_columnNames);
	}

	// Name: ExecuteLogicalColumnsTest
	//
	// Description:
	// Test Execute using an InputDataSet of Logical columns.
	//
	TEST_F(RExtensionApiTest, ExecuteLogicalColumnsTest)
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
			(*m_logicalInfo).m_columnNames);
	}

	// Name: ExecuteRealColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of Real columns.
	//
	TEST_F(RExtensionApiTest, ExecuteRealColumnsTest)
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
			(*m_realInfo).m_columnNames);
	}

	// Name: ExecuteDoubleColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of Double columns.
	//
	TEST_F(RExtensionApiTest, ExecuteDoubleColumnsTest)
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
			(*m_doubleInfo).m_columnNames);
	}

	// Name: ExecuteBigIntColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of BigInteger columns.
	//
	TEST_F(RExtensionApiTest, ExecuteBigIntColumnsTest)
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
			(*m_bigIntInfo).m_columnNames);
	}

	// Name: ExecuteSmallIntColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of SmallInt columns.
	//
	TEST_F(RExtensionApiTest, ExecuteSmallIntColumnsTest)
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
			(*m_smallIntInfo).m_columnNames);
	}

	// Name: ExecuteTinyIntColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of TinyInt columns.
	//
	TEST_F(RExtensionApiTest, ExecuteTinyIntColumnsTest)
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
			(*m_tinyIntInfo).m_columnNames);
	}

	// Name: ExecuteCharColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of Character columns.
	//
	TEST_F(RExtensionApiTest, ExecuteCharColumnsTest)
	{
		SQLUSMALLINT inputSchemaColumnsNumber = 3;

		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(inputSchemaColumnsNumber, m_scriptString);

		string charColumn1Name = "CharColumn1";
		InitializeColumn(0, charColumn1Name, SQL_C_CHAR, m_CharSize);

		string charColumn2Name = "CharColumn2";
		InitializeColumn(1, charColumn2Name, SQL_C_CHAR, m_CharSize);

		string charColumn3Name = "CharColumn3";
		InitializeColumn(2, charColumn3Name, SQL_C_CHAR, m_CharSize);

		SQLULEN rowsNumber = 5;
		vector<const char*> charCol1{ "Hello", "test", "data", "RExtension", "-123" };
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
		SQLINTEGER charCol1TotalLen = GetSumOfLengths(strLenOrIndCol1, rowsNumber, nullptr);
		SQLINTEGER charCol2TotalLen = GetSumOfLengths(strLenOrIndCol2, rowsNumber, nullptr);
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
			columnNames);
	}

	// Name: ExecuteDifferentColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of different column types.
	//
	TEST_F(RExtensionApiTest, ExecuteDifferentColumnsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(3,    // inputSchemaColumnsNumber
			m_scriptString);

		string integerColumnName = "IntegerColumn";
		InitializeColumn(0, integerColumnName, SQL_C_SLONG, m_IntSize);

		string doubleColumnName = "DoubleColumn";
		InitializeColumn(1, doubleColumnName, SQL_C_DOUBLE, m_DoubleSize);

		string charColumnName = "CharColumn";
		InitializeColumn(2, charColumnName, SQL_C_CHAR, m_CharSize);

		SQLULEN rowsNumber = 5;
		vector<SQLINTEGER> intColData{ 2'147'483'647, -2'147'483'647, 0, -2'147'483'648, -1 };
		vector<SQLDOUBLE> doubleColData{ -1.79e301, 1.33, 83.98, 72.45, 1.79e30 };
		vector<const char*> charCol{ "Hello", "test", "data", "RExtension", "-123" };

		SQLINTEGER strLenOrIndCol1[rowsNumber] = { 0, 0, SQL_NULL_DATA, SQL_NULL_DATA, 0 };
		SQLINTEGER strLenOrIndCol3[rowsNumber] =
			{ static_cast<SQLINTEGER>(strlen(charCol[0])),
			  static_cast<SQLINTEGER>(strlen(charCol[1])),
			  static_cast<SQLINTEGER>(strlen(charCol[2])),
			  static_cast<SQLINTEGER>(strlen(charCol[3])),
			  static_cast<SQLINTEGER>(strlen(charCol[4])) };
		vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1, nullptr, strLenOrIndCol3};

		SQLINTEGER charColTotalLen = GetSumOfLengths(strLenOrIndCol3, rowsNumber, nullptr);
		char charColData[charColTotalLen] = { 0 };
		GenerateContiguousData(charColData, charCol, strLenOrIndCol3);

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

	// Name: Execute
	//
	// Description:
	// Templatized function to call Execute with default script that assigns Input to Output.
	// for integer/numeric/logical data types.
	// If validate is true (which is the default), it checks the correctness of the:
	//  1. Executed script,
	//  2. InputDataSet and
	//  3. OutputDataSet
	//
	template<class SQLType, class RType, SQLSMALLINT dataType>
	void RExtensionApiTest::Execute(
		SQLULEN        rowsNumber,
		void           **dataSet,
		SQLINTEGER     **strLen_or_Ind,
		vector<string> columnNames,
		bool           validate)
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

		string output = testing::internal::GetCapturedStdout();
		cout << output;

		if (validate)
		{
			// Test print message was printed correctly
			//
			ASSERT_TRUE(output.find("[1] \"" + m_printMessage + "\"") != string::npos);

			// Test InputDataSet
			//
			Rcpp::DataFrame inputDataSet = m_globalEnvironment[m_inputDataNameString.c_str()];

			for (SQLUSMALLINT columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
			{
				RType column = inputDataSet[columnNames[columnNumber].c_str()];
				CheckVectorEquality<SQLType, RType, dataType>(
					rowsNumber,
					column,
					dataSet[columnNumber],
					strLen_or_Ind[columnNumber]);
			}

			// Test OutputDataSet
			//
			Rcpp::DataFrame outputDataSet = m_globalEnvironment[m_outputDataNameString.c_str()];
			EXPECT_EQ(outputschemaColumnsNumber, outputDataSet.size());
			CheckDataFrameEquality<RType>(
				outputDataSet,
				inputDataSet);
		}
	}

	// Name: ExecuteChar
	//
	// Description:
	// Call Execute with default script for Character columns.
	// If validate is true (which is the default), it checks the correctness of the:
	//  1. Executed script,
	//  2. InputDataSet and
	//  3. OutputDataSet
	//
	void RExtensionApiTest::ExecuteChar(
		SQLULEN        rowsNumber,
		void           **dataSet,
		SQLINTEGER     **strLen_or_Ind,
		vector<string> columnNames,
		bool           validate)
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

		string output = testing::internal::GetCapturedStdout();
		cout << output;

		if (validate)
		{
			// Test print message was printed correctly
			//
			ASSERT_TRUE(output.find("[1] \"" + m_printMessage + "\"") != string::npos);

			// Test InputDataSet
			//
			Rcpp::DataFrame inputDataSet = m_globalEnvironment[m_inputDataNameString.c_str()];

			for (SQLUSMALLINT columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
			{
				Rcpp::CharacterVector column = inputDataSet[columnNames[columnNumber].c_str()];
				CheckCharacterVectorEquality(
					rowsNumber,
					column,
					dataSet[columnNumber],
					strLen_or_Ind[columnNumber]);
			}

			// Test OutputDataSet
			//
			Rcpp::DataFrame outputDataSet = m_globalEnvironment[m_outputDataNameString.c_str()];
			EXPECT_EQ(outputschemaColumnsNumber, outputDataSet.size());
			CheckDataFrameEquality<Rcpp::CharacterVector>(
				outputDataSet,
				inputDataSet);
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
		for(SQLULEN index = 0 ; index < expectedRowsNumber; ++index)
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
		for(SQLUSMALLINT index = 0 ; index < expectedColumns; ++index)
		{
			CheckVectorEquality<RType>(
				dataFrameToTest[index],
				expectedDataFrame[index]);
		}
	}
}
