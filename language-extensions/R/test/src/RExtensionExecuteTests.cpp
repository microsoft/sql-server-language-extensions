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
// @File: RExtensionExecuteTests.cpp
//
// Purpose:
//  Tests the RExtension's implementation of the external language Execute API.
//
//**************************************************************************************************

#include "Common.h"

using namespace std;

namespace ExtensionApiTest
{
	//----------------------------------------------------------------------------------------------
	// Name: ExecuteIntegerColumnsTest
	//
	// Description:
	//  Tests Execute with default script using an InputDataSet of Integer columns.
	//
	TEST_F(RExtensionApiTests, ExecuteIntegerColumnsTest)
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
			(*m_integerInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: ExecuteLogicalColumnsTest
	//
	// Description:
	//  Tests Execute using an InputDataSet of Logical columns.
	//
	TEST_F(RExtensionApiTests, ExecuteLogicalColumnsTest)
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
			(*m_logicalInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: ExecuteRealColumnsTest
	//
	// Description:
	//  Tests Execute with default script using an InputDataSet of Real columns.
	//
	TEST_F(RExtensionApiTests, ExecuteRealColumnsTest)
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
			(*m_realInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: ExecuteDoubleColumnsTest
	//
	// Description:
	//  Tests Execute with default script using an InputDataSet of Double columns.
	//
	TEST_F(RExtensionApiTests, ExecuteDoubleColumnsTest)
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
			(*m_doubleInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: ExecuteBigIntColumnsTest
	//
	// Description:
	//  Tests Execute with default script using an InputDataSet of BigInteger columns.
	//
	TEST_F(RExtensionApiTests, ExecuteBigIntColumnsTest)
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
			(*m_bigIntInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: ExecuteSmallIntColumnsTest
	//
	// Description:
	//  Tests Execute with default script using an InputDataSet of SmallInt columns.
	//
	TEST_F(RExtensionApiTests, ExecuteSmallIntColumnsTest)
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
			(*m_smallIntInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: ExecuteTinyIntColumnsTest
	//
	// Description:
	//  Tests Execute with default script using an InputDataSet of TinyInt columns.
	//
	TEST_F(RExtensionApiTests, ExecuteTinyIntColumnsTest)
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
			(*m_tinyIntInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: ExecuteCharColumnsTest
	//
	// Description:
	//  Tests Execute with default script using an InputDataSet of Character columns.
	//
	TEST_F(RExtensionApiTests, ExecuteCharColumnsTest)
	{
		SQLUSMALLINT inputSchemaColumnsNumber = 3;

		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(inputSchemaColumnsNumber, m_scriptString);

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

		vector<SQLINTEGER> strLenOrIndCol1=
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
			columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: ExecuteNCharColumnsTest
	//
	// Description:
	//  Tests Execute with default script using an InputDataSet of NCHAR/NVARCHAR columns.
	//
	TEST_F(RExtensionApiTests, ExecuteNCharColumnsTest)
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
			columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: ExecuteDifferentColumnsTest
	//
	// Description:
	//  Tests Execute with default script using an InputDataSet of different column types.
	//
	TEST_F(RExtensionApiTests, ExecuteDifferentColumnsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(3, // inputSchemaColumnsNumber
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
		vector<SQLINTEGER> intColData{ 2'147'483'647, -2'147'483'647, 0, -2'147'483'648, -1 };
		vector<SQLDOUBLE> doubleColData{ -1.79e301, 1.33, 83.98, 72.45, 1.79e30 };
		vector<const char*> charCol{ "Hello", "test", "data", "RExtension", "-123" };

		vector<SQLINTEGER> strLenOrIndCol1 = { 0, 0, SQL_NULL_DATA, SQL_NULL_DATA, 0 };
		vector<SQLINTEGER> strLenOrIndCol3 =
			{ static_cast<SQLINTEGER>(strlen(charCol[0])),
			  static_cast<SQLINTEGER>(strlen(charCol[1])),
			  static_cast<SQLINTEGER>(strlen(charCol[2])),
			  static_cast<SQLINTEGER>(strlen(charCol[3])),
			  static_cast<SQLINTEGER>(strlen(charCol[4])) };
		vector<SQLINTEGER*> strLen_or_Ind{
			strLenOrIndCol1.data(),
			// The second column doubleColData is not nullable,
			// so passing a non-null strLenOrInd should be ignored.
			//
			strLenOrIndCol1.data(),
			strLenOrIndCol3.data()};

		vector<char> charColData = GenerateContiguousData<char>(charCol, strLenOrIndCol3.data());
		vector<void *> dataSet { intColData.data(), doubleColData.data(), charColData.data()};

		testing::internal::CaptureStdout();

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
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
		ASSERT_TRUE(output.find(m_printMessage) != string::npos);

		// Test InputDataSet
		//
		Rcpp::DataFrame inputDataSet = m_globalEnvironment[m_inputDataNameString.c_str()];

		Rcpp::IntegerVector intColumn = inputDataSet[integerColumnName.c_str()];
		CheckRVectorColumnDataEquality<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
			rowsNumber,
			intColumn,
			dataSet[0],
			strLen_or_Ind[0]);

		Rcpp::NumericVector numericColumn = inputDataSet[doubleColumnName.c_str()];
		CheckRVectorColumnDataEquality<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
			rowsNumber,
			numericColumn,
			dataSet[1],
			strLen_or_Ind[1],
			SQL_NO_NULLS);

		Rcpp::CharacterVector charColumn = inputDataSet[charColumnName.c_str()];
		CheckRCharacterVectorColumnDataEquality<char>(
			rowsNumber,
			charColumn,
			dataSet[2],
			strLen_or_Ind[2]);

		// Test OutputDataSet
		//
		Rcpp::DataFrame outputDataSet = m_globalEnvironment[m_outputDataNameString.c_str()];
		EXPECT_EQ(outputschemaColumnsNumber, outputDataSet.size());
		ASSERT_EQ(outputDataSet.size(), inputDataSet.size());
		CheckRVectorEquality<Rcpp::IntegerVector>(
			outputDataSet[0],
			inputDataSet[0]);

		CheckRVectorEquality<Rcpp::NumericVector>(
			outputDataSet[1],
			inputDataSet[1]);

		CheckRVectorEquality<Rcpp::CharacterVector>(
			outputDataSet[2],
			inputDataSet[2]);
	}

	//----------------------------------------------------------------------------------------------
	// Name: ExecuteDateColumnsTest
	//
	// Description:
	//  Tests Execute with default script using an InputDataSet of Date columns.
	//
	TEST_F(RExtensionApiTests, ExecuteDateColumnsTest)
	{
		// Initialize with a default Session that prints Hello RExtension
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
			(*m_dateInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: ExecuteDateTimeColumnsTest
	//
	// Description:
	//  Tests Execute with default script using an InputDataSet of DateTime columns.
	//
	TEST_F(RExtensionApiTests, ExecuteDateTimeColumnsTest)
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
			(*m_dateTimeInfo).m_columnNames);
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::Execute
	//
	// Description:
	//  Templatized function to call Execute with default script that assigns Input to Output.
	//  for integer/numeric/logical data types.
	//  If validate is true (which is the default), it checks the correctness of the:
	//   1. Executed script,
	//   2. InputDataSet and
	//   3. OutputDataSet
	//
	template<class SQLType, class RVectorType, SQLSMALLINT dataType>
	void RExtensionApiTests::Execute(
		SQLULEN        rowsNumber,
		void           **dataSet,
		SQLINTEGER     **strLen_or_Ind,
		vector<string> columnNames,
		bool           validate)
	{
		testing::internal::CaptureStdout();

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
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
			ASSERT_TRUE(output.find(m_printMessage) != string::npos);

			// Test InputDataSet
			//
			Rcpp::DataFrame inputDataSet = m_globalEnvironment[m_inputDataNameString.c_str()];

			for (SQLUSMALLINT columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
			{
				RVectorType column = inputDataSet[columnNames[columnNumber].c_str()];
				CheckRVectorColumnDataEquality<SQLType, RVectorType, dataType>(
					rowsNumber,
					column,
					dataSet[columnNumber],
					strLen_or_Ind[columnNumber]);
			}

			// Test OutputDataSet
			//
			Rcpp::DataFrame outputDataSet = m_globalEnvironment[m_outputDataNameString.c_str()];
			EXPECT_EQ(outputschemaColumnsNumber, outputDataSet.size());
			CheckDataFrameEquality<RVectorType>(
				outputDataSet,
				inputDataSet);
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::ExecuteChar
	//
	// Description:
	//  Calls Execute with default script for Character columns.
	//  If validate is true (which is the default), it checks the correctness of the:
	//   1. Executed script,
	//   2. InputDataSet and
	//   3. OutputDataSet
	//
	template<class CharType>
	void RExtensionApiTests::ExecuteChar(
		SQLULEN        rowsNumber,
		void           **dataSet,
		SQLINTEGER     **strLen_or_Ind,
		vector<string> columnNames,
		bool           validate)
	{
		testing::internal::CaptureStdout();

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
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
			ASSERT_TRUE(output.find(m_printMessage) != string::npos);

			// Test InputDataSet
			//
			Rcpp::DataFrame inputDataSet = m_globalEnvironment[m_inputDataNameString.c_str()];

			for (SQLUSMALLINT columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
			{
				Rcpp::CharacterVector column = inputDataSet[columnNames[columnNumber].c_str()];
				CheckRCharacterVectorColumnDataEquality<CharType>(
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

	// Template instantiations
	//
	template void RExtensionApiTests::ExecuteChar<char>(
		SQLULEN        rowsNumber,
		void           **dataSet,
		SQLINTEGER     **strLen_or_Ind,
		vector<string> columnNames,
		bool           validate);

	template void RExtensionApiTests::ExecuteChar<wchar_t>(
		SQLULEN        rowsNumber,
		void           **dataSet,
		SQLINTEGER     **strLen_or_Ind,
		vector<string> columnNames,
		bool           validate);

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::ExecuteDateTime
	//
	// Description:
	//  Templatized function to call Execute with default script that assigns Input to Output.
	//  for date/datetime types.
	//  If validate is true (which is the default), it checks the correctness of the:
	//   1. Executed script,
	//   2. InputDataSet and
	//   3. OutputDataSet
	//
	template<class SQLType, class RVectorType, class DateTimeTypeInR>
	void RExtensionApiTests::ExecuteDateTime(
		SQLULEN        rowsNumber,
		void           **dataSet,
		SQLINTEGER     **strLen_or_Ind,
		vector<string> columnNames,
		bool           validate)
	{
		testing::internal::CaptureStdout();

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
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
			ASSERT_TRUE(output.find(m_printMessage) != string::npos);

			// Test InputDataSet
			//
			Rcpp::DataFrame inputDataSet = m_globalEnvironment[m_inputDataNameString.c_str()];

			for (SQLUSMALLINT columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
			{
				RVectorType column = inputDataSet[columnNames[columnNumber].c_str()];
				CheckRDateTimeVectorColumnDataEquality<SQLType, RVectorType, DateTimeTypeInR>(
					rowsNumber,
					column,
					dataSet[columnNumber],
					strLen_or_Ind[columnNumber]);
			}

			// Test OutputDataSet
			//
			Rcpp::DataFrame outputDataSet = m_globalEnvironment[m_outputDataNameString.c_str()];
			EXPECT_EQ(outputschemaColumnsNumber, outputDataSet.size());
			CheckDataFrameEquality<RVectorType>(
				outputDataSet,
				inputDataSet);
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::CheckRVectorEquality
	//
	// Description:
	//  Templatized function to compare the given vectors for equality.
	//
	template<class RVectorType>
	void RExtensionApiTests::CheckRVectorEquality(
		RVectorType vectorToTest,
		RVectorType expectedVector)
	{
		ASSERT_EQ(vectorToTest.size(), expectedVector.size());
		SQLULEN expectedRowsNumber = expectedVector.size();
		for(SQLULEN index = 0 ; index < expectedRowsNumber; ++index)
		{
			if (RVectorType::is_na(expectedVector[index]))
			{
				EXPECT_TRUE(RVectorType::is_na(vectorToTest[index]));
			}
			else
			{
				EXPECT_EQ(vectorToTest[index], expectedVector[index]);
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::CheckDataFrameEquality
	//
	// Description:
	//  Templatized function to compare the given dataframes for equality.
	//
	template<class RVectorType>
	void RExtensionApiTests::CheckDataFrameEquality(
		Rcpp::DataFrame dataFrameToTest,
		Rcpp::DataFrame expectedDataFrame)
	{
		ASSERT_EQ(dataFrameToTest.size(), expectedDataFrame.size());
		SQLUSMALLINT expectedColumns = dataFrameToTest.size();
		for(SQLUSMALLINT index = 0 ; index < expectedColumns; ++index)
		{
			CheckRVectorEquality<RVectorType>(
				dataFrameToTest[index],
				expectedDataFrame[index]);
		}
	}
}
