//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonGetResultsTests.cpp
//
// Purpose:
//  Tests the Python Extension's implementation of the external language GetResults API.
//
//*************************************************************************************************
#include "PythonExtensionApiTests.h"

using namespace std;
namespace py = boost::python;

namespace ExtensionApiTest
{
	// Name: GetIntegerResultsTest
	//
	// Description:
	//  Test GetResults with default script expecting an OutputDataSet of Integer column data.
	//
	TEST_F(PythonExtensionApiTests, GetIntegerResultsTest)
	{
		// Initialize with a default Session that prints Hello PythonExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession((*m_integerInfo).GetColumnsNumber(),
			m_script,
			m_scriptString.length());

		InitializeColumns<SQLINTEGER, SQL_C_SLONG>(m_integerInfo.get());

		TestExecute<SQLINTEGER, SQL_C_SLONG>(
			ColumnInfo<SQLINTEGER>::m_rowsNumber,
			(*m_integerInfo).m_dataSet.data(),
			(*m_integerInfo).m_strLen_or_Ind.data(),
			(*m_integerInfo).m_columnNames,
			false);  // validate

		vector<SQLINTEGER> defaultStrLen_Or_Ind =
			vector<SQLINTEGER>(ColumnInfo<SQLINTEGER>::m_rowsNumber, sizeof(SQLINTEGER));

		SQLINTEGER *expectedStrLen_or_Ind[] =
		{ defaultStrLen_Or_Ind.data(), defaultStrLen_Or_Ind.data() };

		TestGetResults<SQLINTEGER, SQL_C_SLONG>(
			ColumnInfo<SQLINTEGER>::m_rowsNumber,
			(*m_integerInfo).m_dataSet.data(),
			expectedStrLen_or_Ind, // int columns strLen_or_Ind should be 0, there are no nulls
			(*m_integerInfo).m_columnNames);
	}

	// Name: GetBooleanResultsTest
	//
	// Description:
	//  Test GetResults with default script expecting an OutputDataSet of Boolean column data.
	//
	TEST_F(PythonExtensionApiTests, GetBooleanResultsTest)
	{
		// Initialize with a default Session that prints Hello PythonExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession((*m_booleanInfo).GetColumnsNumber(),
			m_script,
			m_scriptString.length());

		InitializeColumns<SQLCHAR, SQL_C_BIT>(m_booleanInfo.get());

		TestExecute<SQLCHAR, SQL_C_BIT>(
			ColumnInfo<SQLCHAR>::m_rowsNumber,
			(*m_booleanInfo).m_dataSet.data(),
			(*m_booleanInfo).m_strLen_or_Ind.data(),
			(*m_booleanInfo).m_columnNames,
			false);  // validate

		vector<SQLINTEGER> defaultStrLen_Or_Ind =
			vector<SQLINTEGER>(ColumnInfo<SQLINTEGER>::m_rowsNumber, sizeof(SQLCHAR));

		SQLINTEGER *expectedStrLen_or_Ind[] =
		{ defaultStrLen_Or_Ind.data(), defaultStrLen_Or_Ind.data() };

		TestGetResults<SQLCHAR, SQL_C_BIT>(
			ColumnInfo<SQLCHAR>::m_rowsNumber,
			(*m_booleanInfo).m_dataSet.data(),
			expectedStrLen_or_Ind, // boolean columns strLen_or_Ind should be 0, there are no nulls
			(*m_booleanInfo).m_columnNames);
	}

	// Name: GetRealResultsTest
	//
	// Description:
	//  Test GetResults with default script expecting an OutputDataSet of Real column data.
	//
	TEST_F(PythonExtensionApiTests, GetRealResultsTest)
	{
		// Initialize with a default Session that prints Hello PythonExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession((*m_realInfo).GetColumnsNumber(),
			m_script,
			m_scriptString.length());

		InitializeColumns<SQLREAL, SQL_C_FLOAT>(m_realInfo.get());

		TestExecute<SQLREAL, SQL_C_FLOAT>(
			ColumnInfo<SQLREAL>::m_rowsNumber,
			(*m_realInfo).m_dataSet.data(),
			(*m_realInfo).m_strLen_or_Ind.data(),
			(*m_realInfo).m_columnNames,
			false);  // validate

		TestGetResults<SQLREAL, SQL_C_FLOAT>(
			ColumnInfo<SQLREAL>::m_rowsNumber,
			(*m_realInfo).m_dataSet.data(),
			(*m_realInfo).m_strLen_or_Ind.data(),
			(*m_realInfo).m_columnNames);
	}

	// Name: GetDoubleResultsTest
	//
	// Description:
	//  Test GetResults with default script expecting an OutputDataSet of Double column data.
	//
	TEST_F(PythonExtensionApiTests, GetDoubleResultsTest)
	{
		// Initialize with a default Session that prints Hello PythonExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession((*m_doubleInfo).GetColumnsNumber(),
			m_script,
			m_scriptString.length());

		InitializeColumns<SQLDOUBLE, SQL_C_DOUBLE>(m_doubleInfo.get());

		TestExecute<SQLDOUBLE, SQL_C_DOUBLE>(
			ColumnInfo<SQLDOUBLE>::m_rowsNumber,
			(*m_doubleInfo).m_dataSet.data(),
			(*m_doubleInfo).m_strLen_or_Ind.data(),
			(*m_doubleInfo).m_columnNames,
			false);  // validate

		TestGetResults<SQLDOUBLE, SQL_C_DOUBLE>(
			ColumnInfo<SQLDOUBLE>::m_rowsNumber,
			(*m_doubleInfo).m_dataSet.data(),
			(*m_doubleInfo).m_strLen_or_Ind.data(),
			(*m_doubleInfo).m_columnNames);
	}

	// Name: GetBigIntResultsTest
	//
	// Description:
	//  Test GetResults with default script expecting an OutputDataSet of BigInteger columns.
	//
	TEST_F(PythonExtensionApiTests, GetBigIntResultsTest)
	{
		// Initialize with a default Session that prints Hello PythonExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession((*m_bigIntInfo).GetColumnsNumber(),
			m_script,
			m_scriptString.length());

		InitializeColumns<SQLBIGINT, SQL_C_SBIGINT>(m_bigIntInfo.get());

		TestExecute<SQLBIGINT, SQL_C_SBIGINT>(
			ColumnInfo<SQLBIGINT>::m_rowsNumber,
			(*m_bigIntInfo).m_dataSet.data(),
			(*m_bigIntInfo).m_strLen_or_Ind.data(),
			(*m_bigIntInfo).m_columnNames,
			false); // validate

		vector<SQLINTEGER> defaultStrLen_Or_Ind =
			vector<SQLINTEGER>(ColumnInfo<SQLINTEGER>::m_rowsNumber, sizeof(SQLBIGINT));

		SQLINTEGER *expectedStrLen_or_Ind[] =
		{ defaultStrLen_Or_Ind.data(), defaultStrLen_Or_Ind.data() };

		TestGetResults<SQLBIGINT, SQL_C_SBIGINT>(
			ColumnInfo<SQLBIGINT>::m_rowsNumber,
			(*m_bigIntInfo).m_dataSet.data(),
			expectedStrLen_or_Ind, // int columns strLen_or_Ind should be 0, there are no nulls
			(*m_bigIntInfo).m_columnNames);
	}

	// Name: GetSmallIntResultsTest
	//
	// Description:
	//  Test GetResults with default script expecting an OutputDataSet of SmallInt columns.
	//
	TEST_F(PythonExtensionApiTests, GetSmallIntResultsTest)
	{
		// Initialize with a default Session that prints Hello PythonExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession((*m_smallIntInfo).GetColumnsNumber(),
			m_script,
			m_scriptString.length());

		InitializeColumns<SQLSMALLINT, SQL_C_SSHORT>(m_smallIntInfo.get());

		TestExecute<SQLSMALLINT, SQL_C_SSHORT>(
			ColumnInfo<SQLSMALLINT>::m_rowsNumber,
			(*m_smallIntInfo).m_dataSet.data(),
			(*m_smallIntInfo).m_strLen_or_Ind.data(),
			(*m_smallIntInfo).m_columnNames,
			false);  // validate

		vector<SQLINTEGER> defaultStrLen_Or_Ind =
			vector<SQLINTEGER>(ColumnInfo<SQLINTEGER>::m_rowsNumber, sizeof(SQLSMALLINT));

		SQLINTEGER *expectedStrLen_or_Ind[] =
		{ defaultStrLen_Or_Ind.data(), defaultStrLen_Or_Ind.data() };

		TestGetResults<SQLSMALLINT, SQL_C_SSHORT>(
			ColumnInfo<SQLSMALLINT>::m_rowsNumber,
			(*m_smallIntInfo).m_dataSet.data(),
			expectedStrLen_or_Ind, // int columns strLen_or_Ind should be 0, there are no nulls
			(*m_smallIntInfo).m_columnNames);
	}

	// Name: GetTinyIntResultsTest
	//
	// Description:
	//  Test GetResults with default script using an OutputDataSet of TinyInt columns.
	//
	TEST_F(PythonExtensionApiTests, GetTinyIntResultsTest)
	{
		// Initialize with a default Session that prints Hello PythonExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession((*m_tinyIntInfo).GetColumnsNumber(),
			m_script,
			m_scriptString.length());

		InitializeColumns<SQLCHAR, SQL_C_UTINYINT>(m_tinyIntInfo.get());

		TestExecute<SQLCHAR,SQL_C_UTINYINT>(
			ColumnInfo<SQLCHAR>::m_rowsNumber,
			(*m_tinyIntInfo).m_dataSet.data(),
			(*m_tinyIntInfo).m_strLen_or_Ind.data(),
			(*m_tinyIntInfo).m_columnNames,
			false);  // validate

		vector<SQLINTEGER> defaultStrLen_Or_Ind =
			vector<SQLINTEGER>(ColumnInfo<SQLINTEGER>::m_rowsNumber, sizeof(SQLCHAR));

		SQLINTEGER *expectedStrLen_or_Ind[] =
		{ defaultStrLen_Or_Ind.data(), defaultStrLen_Or_Ind.data() };

		TestGetResults<SQLCHAR, SQL_C_UTINYINT>(
			ColumnInfo<SQLCHAR>::m_rowsNumber,
			(*m_tinyIntInfo).m_dataSet.data(),
			expectedStrLen_or_Ind, // int columns strLen_or_Ind should be 0, there are no nulls
			(*m_tinyIntInfo).m_columnNames);
	}

	// Name: GetStringResultsTest
	//
	// Description:
	//  Test GetResults with default script expecting an OutputDataSet of Character columns.
	//
	TEST_F(PythonExtensionApiTests, GetStringResultsTest)
	{
		SQLUSMALLINT inputSchemaColumnsNumber = 3;

		// Initialize with a default Session that prints Hello PythonExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(inputSchemaColumnsNumber,
			m_script,
			m_scriptString.length());

		string stringColumn1Name = "StringColumn1";
		InitializeColumn(0, stringColumn1Name, SQL_C_CHAR, sizeof(SQLCHAR));

		string stringColumn2Name = "StringColumn2";
		InitializeColumn(1, stringColumn2Name, SQL_C_CHAR, sizeof(SQLCHAR));

		string stringColumn3Name = "StringColumn3";
		InitializeColumn(2, stringColumn3Name, SQL_C_CHAR, sizeof(SQLCHAR));

		vector<const char*> stringCol1{ "Hello", "test", "data", "World", "-123" };
		vector<const char*> stringCol2{ "", 0, nullptr, "verify", "-1" };

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

		TestExecute<SQLCHAR, SQL_C_CHAR>(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames,
			false); // validate

		TestGetStringResults(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames);
	}

	// Name: GetRawResultsTest
	// Description:
	//  Test GetResults with a script that returns OutputDataSet with raw columns.
	//
	TEST_F(PythonExtensionApiTests, GetRawResultsTest)
	{
		SQLUSMALLINT inputSchemaColumnsNumber = 3;

		// Initialize with a default Session that prints Hello PythonExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(inputSchemaColumnsNumber,
			m_script,
			m_scriptString.length());

		const SQLCHAR BinaryValue1[] = { 0x01, 0x01, 0xe2, 0x40 };
		const SQLCHAR BinaryValue2[] = { 0x04, 0x05, 0xe1 };
		const SQLCHAR BinaryValue3[] = { 0x00, 0x00, 0x00, 0x01 };
		const SQLCHAR BinaryValue4[] = { 0xff };

		const SQLCHAR BinaryValue5[] = { 0x00 };
		const SQLCHAR BinaryValue6[] = { 0xff, 0xff, 0xff, 0xff };
		const SQLCHAR BinaryValue7[] = { 0x00, 0x12, 0xd2, 0xff, 0x00, 0x12, 0xd2, 0xff, 0x00, 0x12, 0xd2, 0xff };

		string binaryColumn1Name = "BinaryColumn1";
		InitializeColumn(0, binaryColumn1Name, SQL_C_BINARY, sizeof(SQLCHAR));

		string binaryColumn2Name = "BinaryColumn2";
		InitializeColumn(1, binaryColumn2Name, SQL_C_BINARY, sizeof(SQLCHAR));

		string binaryColumn3Name = "BinaryColumn3";
		InitializeColumn(2, binaryColumn3Name, SQL_C_BINARY, sizeof(SQLCHAR));

		vector<const SQLCHAR*> binaryCol1{ BinaryValue1, BinaryValue2, BinaryValue3, BinaryValue4 };
		vector<const SQLCHAR*> binaryCol2{ BinaryValue5, BinaryValue6, nullptr, BinaryValue7 };

		SQLINTEGER strLenOrIndCol1[] =
		{
			static_cast<SQLINTEGER>(sizeof(BinaryValue1) / sizeof(BinaryValue1[0])),
			static_cast<SQLINTEGER>(sizeof(BinaryValue2) / sizeof(BinaryValue2[0])),
			static_cast<SQLINTEGER>(sizeof(BinaryValue3) / sizeof(BinaryValue3[0])),
			static_cast<SQLINTEGER>(sizeof(BinaryValue4) / sizeof(BinaryValue4[0]))
		};

		SQLINTEGER strLenOrIndCol2[] =
		{
			SQL_NULL_DATA,
			static_cast<SQLINTEGER>(sizeof(BinaryValue6) / sizeof(BinaryValue6[0])),
			SQL_NULL_DATA,
			static_cast<SQLINTEGER>(sizeof(BinaryValue7) / sizeof(BinaryValue7[0]))
		};

		vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1, strLenOrIndCol2, nullptr };

		// Coalesce the arrays of each row of each column
		// into a contiguous array for each column.
		//
		int rowsNumber = binaryCol1.size();

		vector<SQLCHAR> binaryCol1Data = GenerateContiguousData<SQLCHAR>(binaryCol1, strLenOrIndCol1);
		vector<SQLCHAR> binaryCol2Data = GenerateContiguousData<SQLCHAR>(binaryCol2, strLenOrIndCol2);

		void* dataSet[] = { binaryCol1Data.data(),
							binaryCol2Data.data(),
							nullptr };

		vector<string> columnNames{ binaryColumn1Name, binaryColumn2Name, binaryColumn3Name };

		TestExecute<SQLCHAR, SQL_C_BINARY>(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames,
			false); // validate

		TestGetRawResults(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames);
	}

	// Name: GetDifferentColumnResultsTest
	//
	// Description:
	//  Test GetResults with default script using an OutputDataSet of different column types.
	//
	TEST_F(PythonExtensionApiTests, GetDifferentColumnResultsTest)
	{
		// Initialize with a default Session that prints Hello PythonExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(3,
			m_script,
			m_scriptString.length());

		const SQLINTEGER intSize = sizeof(SQLINTEGER);
		const SQLINTEGER doubleSize = sizeof(SQLDOUBLE);

		string integerColumnName = "IntegerColumn";
		InitializeColumn(0, integerColumnName, SQL_C_SLONG, intSize);

		string doubleColumnName = "DoubleColumn";
		InitializeColumn(1, doubleColumnName, SQL_C_DOUBLE, doubleSize);

		string stringColumnName = "CharColumn";
		InitializeColumn(2, stringColumnName, SQL_C_CHAR, sizeof(SQLCHAR));

		SQLULEN expectedRowsNumber = 5;
		vector<SQLINTEGER> intColData{ 2'147'483'647, -2'147'483'647, 0, 0, -1 };
		vector<SQLDOUBLE> doubleColData{ -1.79e301, INFINITY, -INFINITY, 72.45, 1.79e30 };
		vector<const char*> stringCol{ "Hello", "test", "data", "World", "-123" };

		vector<SQLINTEGER> strLenOrIndCol1 = { intSize, intSize, SQL_NULL_DATA,
			SQL_NULL_DATA, intSize };
		vector<SQLINTEGER> strLenOrIndCol2(expectedRowsNumber, doubleSize);
		vector<SQLINTEGER> strLenOrIndCol3 =
		{ static_cast<SQLINTEGER>(strlen(stringCol[0])),
		  static_cast<SQLINTEGER>(strlen(stringCol[1])),
		  static_cast<SQLINTEGER>(strlen(stringCol[2])),
		  static_cast<SQLINTEGER>(strlen(stringCol[3])),
		  static_cast<SQLINTEGER>(strlen(stringCol[4])) };
		vector<SQLINTEGER*> expectedStrLen_or_Ind{ strLenOrIndCol1.data(),
			strLenOrIndCol2.data(), strLenOrIndCol3.data() };

		vector<char> stringColData = GenerateContiguousData( stringCol, strLenOrIndCol3.data());

		vector<void *> expectedData{ intColData.data(), doubleColData.data(), stringColData.data() };

		// int column strLen will always be sizeof(SQLINTEGER), can't be null
		//
		vector<SQLINTEGER> expected_Int_StrLen_or_Ind =
			vector<SQLINTEGER>(strLenOrIndCol1.size(), sizeof(SQLINTEGER));

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = Execute(
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
		result = GetResults(
			*m_sessionId,
			m_taskId,
			&rowsNumber,
			&data,
			&strLen_or_Ind);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(rowsNumber, expectedRowsNumber);

		// Test the data returned.
		//
		CheckColumnDataEquality<SQLINTEGER>(
			rowsNumber,
			static_cast<SQLINTEGER*>(expectedData[0]),
			static_cast<SQLINTEGER*>(data[0]),
			expected_Int_StrLen_or_Ind.data(),
			strLen_or_Ind[0]);

		CheckColumnDataEquality<SQLDOUBLE>(
			rowsNumber,
			static_cast<SQLDOUBLE*>(expectedData[1]),
			static_cast<SQLDOUBLE*>(data[1]),
			expectedStrLen_or_Ind[1],
			strLen_or_Ind[1]);

		CheckStringDataEquality(
			rowsNumber,
			static_cast<char*>(expectedData[2]),
			static_cast<char*>(data[2]),
			expectedStrLen_or_Ind[2],
			strLen_or_Ind[2]);

		// Test the data obtained is what is present in OutputDataSet.
		//
		string createDictScript = m_outputDataNameString + ".to_dict()";
		py::dict outputDataSet = py::extract<py::dict>(
			py::eval(createDictScript.c_str(), m_mainNamespace));

		EXPECT_EQ(outputschemaColumnsNumber, py::len(outputDataSet.keys()));

		py::dict intColumn = py::extract<py::dict>(outputDataSet[integerColumnName]);
		CheckIntColumnEquality<SQLINTEGER>(
			rowsNumber,
			intColumn,
			data[0],
			expected_Int_StrLen_or_Ind.data());

		py::dict doubleColumn = py::extract<py::dict>(outputDataSet[doubleColumnName]);
		CheckFloatColumnEquality<SQLDOUBLE>(
			rowsNumber,
			doubleColumn,
			data[1],
			strLen_or_Ind[1]);

		py::dict stringColumn = py::extract<py::dict>(outputDataSet[stringColumnName]);
		CheckStringColumnEquality(
			rowsNumber,
			stringColumn,
			data[2],
			strLen_or_Ind[2]);
	}

	// Name: GetEmptyResultsTest
	//
	// Description:
	//  Test GetResults with a script that returns OutputDataSet with empty rows.
	//
	TEST_F(PythonExtensionApiTests, GetEmptyResultsTest)
	{
		string scriptString = "from pandas import DataFrame; import numpy as np;"
			"OutputDataSet = DataFrame({'intCol' : np.array([], dtype=np.int32)});"
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
		SQLRETURN result = Execute(
			*m_sessionId,
			m_taskId,
			0,
			nullptr,
			nullptr,
			&outputschemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputschemaColumnsNumber, 1);

		TestGetResults<SQLINTEGER, SQL_C_SLONG>(
			0, // expected rowsNumber
			vector<void*>{nullptr}.data(),
			vector<SQLINTEGER*>{nullptr}.data(),
			vector<string>{"intCol"});
	}

	// Name: TestGetResults
	//
	// Description:
	//  Test GetResults to verify the expected results are obtained.
	//  For numeric, boolean and integer types.
	//
	template<class SQLType, SQLSMALLINT dataType>
	void PythonExtensionApiTests::TestGetResults(
		SQLULEN        expectedRowsNumber,
		SQLPOINTER     *expectedData,
		SQLINTEGER     **expectedStrLen_or_Ind,
		vector<string> columnNames)
	{
		SQLULEN    rowsNumber = 0;
		SQLPOINTER *data = nullptr;
		SQLINTEGER **strLen_or_Ind = nullptr;
		SQLRETURN result = GetResults(
			*m_sessionId,
			m_taskId,
			&rowsNumber,
			&data,
			&strLen_or_Ind);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(rowsNumber, expectedRowsNumber);

		// Test data obtained is same as the expectedData and the OutputDataSet in python namespace.
		//
		string createDictScript = m_outputDataNameString + ".to_dict()";
		py::dict outputDataSet = py::extract<py::dict>(
			py::eval(createDictScript.c_str(), m_mainNamespace));

		for (SQLUSMALLINT columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
		{
			SQLType *expectedColumnData = static_cast<SQLType *>(expectedData[columnNumber]);
			SQLType *columnData = static_cast<SQLType *>(data[columnNumber]);

			SQLINTEGER *expectedColumnStrLenOrInd = expectedStrLen_or_Ind[columnNumber];
			SQLINTEGER *columnStrLenOrInd = strLen_or_Ind[columnNumber];

			CheckColumnDataEquality<SQLType>(
				rowsNumber,
				expectedColumnData,
				columnData,
				expectedColumnStrLenOrInd,
				columnStrLenOrInd);

			py::dict column = py::extract<py::dict>(outputDataSet[columnNames[columnNumber]]);

			CheckColumnEqualityFnMap::const_iterator it = m_fnCheckColumnEqualityMap.find(dataType);

			if (it == m_fnCheckColumnEqualityMap.end())
			{
				throw runtime_error("Unsupported column type " + to_string(dataType) +
					" encountered when testing column equality");
			}

			(this->*it->second)(
				rowsNumber,
				column,
				columnData,
				columnStrLenOrInd);
		}
	}

	// Name: CheckColumnDataEquality
	//
	// Description:
	//  Template function to compare the given column data
	//  and nullMap with rowsNumber for equality.
	//
	template<class SQLType>
	void PythonExtensionApiTests::CheckColumnDataEquality(
		SQLULEN    rowsNumber,
		SQLType    *expectedColumnData,
		SQLType    *columnData,
		SQLINTEGER *expectedColumnStrLenOrInd,
		SQLINTEGER *columnStrLenOrInd)
	{
		if (rowsNumber == 0 || columnStrLenOrInd == nullptr)
		{
			EXPECT_EQ(columnData, nullptr);
			EXPECT_EQ(columnStrLenOrInd, nullptr);
		}

		for (SQLULEN index = 0; index < rowsNumber; ++index)
		{
			EXPECT_EQ(columnStrLenOrInd[index], expectedColumnStrLenOrInd[index]);

			if (columnStrLenOrInd[index] == SQL_NULL_DATA)
			{
				if (is_same<SQLType, float>::value || is_same<SQLType, double>::value)
				{
					EXPECT_TRUE(isnan(static_cast<double>(columnData[index])));
				}
				else
				{
					EXPECT_EQ(columnData[index], 0);
				}
			}
			else
			{
				EXPECT_EQ(columnData[index], expectedColumnData[index]);
			}
		}
	}


	// Name: TestGetStringResults
	//
	// Description:
	//  Test GetResults to verify the expected results are obtained for character data.
	//
	void PythonExtensionApiTests::TestGetStringResults(
		SQLULEN        expectedRowsNumber,
		SQLPOINTER     *expectedData,
		SQLINTEGER     **expectedStrLen_or_Ind,
		vector<string> columnNames)
	{
		SQLULEN    rowsNumber = 0;
		SQLPOINTER *data = nullptr;
		SQLINTEGER **strLen_or_Ind = nullptr;
		SQLRETURN result = GetResults(
			*m_sessionId,
			m_taskId,
			&rowsNumber,
			&data,
			&strLen_or_Ind);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(rowsNumber, expectedRowsNumber);

		// Test data obtained is same as the expectedData and the OutputDataSet in python namespace.
		//
		string createDictScript = m_outputDataNameString + ".to_dict()";
		py::dict outputDataSet = py::extract<py::dict>(
			py::eval(createDictScript.c_str(), m_mainNamespace));

		for (SQLUSMALLINT columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
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

			py::dict column = py::extract<py::dict>(outputDataSet[columnNames[columnNumber]]);
			CheckStringColumnEquality(
				rowsNumber,
				column,
				columnData,
				columnStrLenOrInd);
		}
	}

	// Name: CheckStringDataEquality
	//
	// Description:
	//  Compare the given character data & nullMap with rowsNumber for equality.
	//
	void PythonExtensionApiTests::CheckStringDataEquality(
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
					string expectedString(expectedColumnData + cumulativeLength,
						expectedColumnStrLenOrInd[index]);
					string testString(columnData + cumulativeLength,
						columnStrLenOrInd[index]);
					EXPECT_EQ(testString, expectedString);
					cumulativeLength += expectedColumnStrLenOrInd[index];
				}
			}
			else
			{
				EXPECT_EQ(columnStrLenOrInd[index], SQL_NULL_DATA);
			}
		}
	}


	// Name: TestGetRawResults
	//
	// Description:
	//  Test GetResults to verify the expected results are obtained for raw data.
	//
	void PythonExtensionApiTests::TestGetRawResults(
		SQLULEN        expectedRowsNumber,
		SQLPOINTER     *expectedData,
		SQLINTEGER     **expectedStrLen_or_Ind,
		vector<string> columnNames)
	{
		SQLULEN    rowsNumber = 0;
		SQLPOINTER *data = nullptr;
		SQLINTEGER **strLen_or_Ind = nullptr;
		SQLRETURN result = GetResults(
			*m_sessionId,
			m_taskId,
			&rowsNumber,
			&data,
			&strLen_or_Ind);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(rowsNumber, expectedRowsNumber);

		// Test data obtained is same as the expectedData and the OutputDataSet in python namespace.
		//
		string createDictScript = m_outputDataNameString + ".to_dict()";
		py::dict outputDataSet = py::extract<py::dict>(
			py::eval(createDictScript.c_str(), m_mainNamespace));

		for (SQLUSMALLINT columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
		{
			SQLCHAR *expectedColumnData = static_cast<SQLCHAR *>(expectedData[columnNumber]);
			SQLCHAR *columnData = static_cast<SQLCHAR *>(data[columnNumber]);

			SQLINTEGER *expectedColumnStrLenOrInd = expectedStrLen_or_Ind[columnNumber];
			SQLINTEGER *columnStrLenOrInd = strLen_or_Ind[columnNumber];

			CheckRawDataEquality(
				rowsNumber,
				expectedColumnData,
				columnData,
				expectedColumnStrLenOrInd,
				columnStrLenOrInd);

			py::dict column = py::extract<py::dict>(outputDataSet[columnNames[columnNumber]]);
			CheckRawColumnEquality(
				rowsNumber,
				column,
				columnData,
				columnStrLenOrInd);
		}
	}

	// Name: CheckRawDataEquality
	//
	// Description:
	//  Compare the given character data & nullMap with rowsNumber for equality.
	//
	void PythonExtensionApiTests::CheckRawDataEquality(
		SQLULEN    rowsNumber,
		SQLCHAR    *expectedColumnData,
		SQLCHAR    *columnData,
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

					SQLCHAR *expectedValue =
						static_cast<SQLCHAR*>(expectedColumnData) + cumulativeLength;
					SQLCHAR *actualValue =
						static_cast<SQLCHAR*>(columnData) + cumulativeLength;

					for (SQLINTEGER i = 0; i < expectedColumnStrLenOrInd[index]; ++i)
					{
						EXPECT_EQ(actualValue[i], expectedValue[i]);
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
}
