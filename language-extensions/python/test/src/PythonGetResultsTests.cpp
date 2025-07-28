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
namespace bp = boost::python;

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
		InitializeSession(0, // parametersNumber
			(*m_integerInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLINTEGER, SQL_C_SLONG>(m_integerInfo.get());

		TestExecute<SQLINTEGER, SQL_C_SLONG>(
			ColumnInfo<SQLINTEGER>::sm_rowsNumber,
			(*m_integerInfo).m_dataSet.data(),
			(*m_integerInfo).m_strLen_or_Ind.data(),
			(*m_integerInfo).m_columnNames,
			false);  // validate

		TestGetResults<SQLINTEGER, int, SQL_C_SLONG>(
			ColumnInfo<SQLINTEGER>::sm_rowsNumber,
			(*m_integerInfo).m_dataSet.data(),
			(*m_integerInfo).m_strLen_or_Ind.data(),
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
		InitializeSession(0, // parametersNumber
			(*m_booleanInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLCHAR, SQL_C_BIT>(m_booleanInfo.get());

		TestExecute<SQLCHAR, SQL_C_BIT>(
			ColumnInfo<SQLCHAR>::sm_rowsNumber,
			(*m_booleanInfo).m_dataSet.data(),
			(*m_booleanInfo).m_strLen_or_Ind.data(),
			(*m_booleanInfo).m_columnNames,
			false);  // validate

		vector<void *> expectedReturnData = {  };
		expectedReturnData.push_back((m_booleanInfo->m_dataSet[0]));
		string result = "TrueTrueFalse";
		expectedReturnData.push_back(const_cast<char*>(result.c_str()));

		TestGetResults<SQLCHAR, bool, SQL_C_BIT>(
			ColumnInfo<SQLCHAR>::sm_rowsNumber,
			expectedReturnData.data(),
			(*m_booleanInfo).m_strLen_or_Ind.data(),
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
		InitializeSession(0, // parametersNumber
			(*m_realInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLREAL, SQL_C_FLOAT>(m_realInfo.get());

		TestExecute<SQLREAL, SQL_C_FLOAT>(
			ColumnInfo<SQLREAL>::sm_rowsNumber,
			(*m_realInfo).m_dataSet.data(),
			(*m_realInfo).m_strLen_or_Ind.data(),
			(*m_realInfo).m_columnNames,
			false);  // validate

		TestGetResults<SQLREAL, double, SQL_C_FLOAT>(
			ColumnInfo<SQLREAL>::sm_rowsNumber,
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
		InitializeSession(0, // parametersNumber
			(*m_doubleInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLDOUBLE, SQL_C_DOUBLE>(m_doubleInfo.get());

		TestExecute<SQLDOUBLE, SQL_C_DOUBLE>(
			ColumnInfo<SQLDOUBLE>::sm_rowsNumber,
			(*m_doubleInfo).m_dataSet.data(),
			(*m_doubleInfo).m_strLen_or_Ind.data(),
			(*m_doubleInfo).m_columnNames,
			false);  // validate

		TestGetResults<SQLDOUBLE, double, SQL_C_DOUBLE>(
			ColumnInfo<SQLDOUBLE>::sm_rowsNumber,
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
		InitializeSession(0, // parametersNumber
			(*m_bigIntInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLBIGINT, SQL_C_SBIGINT>(m_bigIntInfo.get());

		TestExecute<SQLBIGINT, SQL_C_SBIGINT>(
			ColumnInfo<SQLBIGINT>::sm_rowsNumber,
			(*m_bigIntInfo).m_dataSet.data(),
			(*m_bigIntInfo).m_strLen_or_Ind.data(),
			(*m_bigIntInfo).m_columnNames,
			false); // validate

		TestGetResults<SQLBIGINT, int, SQL_C_SBIGINT>(
			ColumnInfo<SQLBIGINT>::sm_rowsNumber,
			(*m_bigIntInfo).m_dataSet.data(),
			(*m_bigIntInfo).m_strLen_or_Ind.data(),
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
		InitializeSession(0, // parametersNumber
			(*m_smallIntInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLSMALLINT, SQL_C_SSHORT>(m_smallIntInfo.get());

		TestExecute<SQLSMALLINT, SQL_C_SSHORT>(
			ColumnInfo<SQLSMALLINT>::sm_rowsNumber,
			(*m_smallIntInfo).m_dataSet.data(),
			(*m_smallIntInfo).m_strLen_or_Ind.data(),
			(*m_smallIntInfo).m_columnNames,
			false);  // validate

		TestGetResults<SQLSMALLINT, int, SQL_C_SSHORT>(
			ColumnInfo<SQLSMALLINT>::sm_rowsNumber,
			(*m_smallIntInfo).m_dataSet.data(),
			(*m_smallIntInfo).m_strLen_or_Ind.data(),
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
		InitializeSession(0, // parametersNumber
			(*m_tinyIntInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLCHAR, SQL_C_UTINYINT>(m_tinyIntInfo.get());

		TestExecute<SQLCHAR,SQL_C_UTINYINT>(
			ColumnInfo<SQLCHAR>::sm_rowsNumber,
			(*m_tinyIntInfo).m_dataSet.data(),
			(*m_tinyIntInfo).m_strLen_or_Ind.data(),
			(*m_tinyIntInfo).m_columnNames,
			false);  // validate

		TestGetResults<SQLCHAR, int, SQL_C_UTINYINT>(
			ColumnInfo<SQLCHAR>::sm_rowsNumber,
			(*m_tinyIntInfo).m_dataSet.data(),
			(*m_tinyIntInfo).m_strLen_or_Ind.data(),
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
		InitializeSession(0, // parametersNumber
			inputSchemaColumnsNumber,
			m_scriptString);

		string stringColumn1Name = "StringColumn1";
		InitializeColumn(0, stringColumn1Name, SQL_C_CHAR, m_CharSize);

		string stringColumn2Name = "StringColumn2";
		InitializeColumn(1, stringColumn2Name, SQL_C_CHAR, m_CharSize);

		string stringColumn3Name = "StringColumn3";
		InitializeColumn(2, stringColumn3Name, SQL_C_CHAR, m_CharSize);
		
		string goodUTF8 = string("a") + "\xE2" + "\x82" + "\xAC";
		
		vector<const char*> stringCol1{ "Hello", "test", "data", "World", "-123" };
		vector<const char*> stringCol2{ "", 0, nullptr, u8"абвг", goodUTF8.c_str() };

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
	
	// Name: GetWStringResultsTest
	//
	// Description:
	//  Test GetResults with default script expecting an OutputDataSet of NChar columns.
	//
	TEST_F(PythonExtensionApiTests, GetWStringResultsTest)
	{
		SQLUSMALLINT inputSchemaColumnsNumber = 3;
		
		// Initialize with a default Session that prints Hello PythonExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(0, // parametersNumber
			inputSchemaColumnsNumber,
			m_scriptString);

		string wstringColumn1Name = "WstringColumn1";
		InitializeColumn(0, wstringColumn1Name, SQL_C_WCHAR, m_CharSize);

		string wstringColumn2Name = "WstringColumn2";
		InitializeColumn(1, wstringColumn2Name, SQL_C_WCHAR, m_CharSize);

		string wstringColumn3Name = "WstringColumn3";
		InitializeColumn(2, wstringColumn3Name, SQL_C_WCHAR, m_CharSize);

		// Test NCHAR with self-constructed UTF-16 char (𐐷)
		// https://en.wikipedia.org/wiki/UTF-16#Examples
		// We need to use u16string here because wstring doesn't 
		// handle multibyte characters well in Linux with the -fshort-wchar option.
		//
		u16string goodUTF16 = u16string(u"a") + u"\xd801\xdc37" + u"b";

		vector<const wchar_t*> wstringCol1{ L"Hello", L"test", L"data", L"World", L"你好" };
		vector<const wchar_t*> wstringCol2{ L"", 0, nullptr, L"абвг", 
			reinterpret_cast<const wchar_t *>(goodUTF16.c_str()) };

		int rowsNumber = wstringCol1.size();

		vector<SQLINTEGER> strLenOrIndCol1 =
		{ static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(2 * sizeof(wchar_t)) };
		vector<SQLINTEGER> strLenOrIndCol2 =
		{ 0, SQL_NULL_DATA, SQL_NULL_DATA,
		  static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(goodUTF16.size() * sizeof(wchar_t)) };
		vector<SQLINTEGER> strLenOrIndCol3(rowsNumber, SQL_NULL_DATA);

		vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1.data(),
			strLenOrIndCol2.data(), strLenOrIndCol3.data() };

		// Coalesce the arrays of each row of each column
		// into a contiguous array for each column.
		//
		vector<wchar_t> wstringCol1Data =
			GenerateContiguousData<wchar_t>(wstringCol1, strLenOrIndCol1.data());

		vector<wchar_t> wstringCol2Data =
			GenerateContiguousData<wchar_t>(wstringCol2, strLenOrIndCol2.data());

		void* dataSet[] = { wstringCol1Data.data(),
							wstringCol2Data.data(),
							nullptr };

		vector<string> columnNames{ wstringColumn1Name, wstringColumn2Name, wstringColumn3Name };

		TestExecute<wchar_t, SQL_C_WCHAR>(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames,
			false); // validate

		//
		// Because Python is a UTF-8 language, we always return UTF-8 for the OutputDataSet.
		// Here we construct the expected output, which is the same as the input but normal char 
		// instead of wchar_t.
		// Since we are retrieving UTF-8 strings, we also need to redo strLenOrInd.
		//
		
		vector<const char*> stringCol1{ "Hello", "test", "data", "World", u8"你好" };
		vector<const char*> stringCol2{ "", 0, nullptr, u8"абвг", u8"a𐐷b" };

		strLenOrIndCol1 =
		{ static_cast<SQLINTEGER>(strlen(stringCol1[0])),
		  static_cast<SQLINTEGER>(strlen(stringCol1[1])),
		  static_cast<SQLINTEGER>(strlen(stringCol1[2])),
		  static_cast<SQLINTEGER>(strlen(stringCol1[3])),
		  static_cast<SQLINTEGER>(strlen(stringCol1[4])) };
		strLenOrIndCol2 =
		{ 0, SQL_NULL_DATA, SQL_NULL_DATA,
		  static_cast<SQLINTEGER>(strlen(stringCol2[3])),
		  static_cast<SQLINTEGER>(strlen(stringCol2[4])) };

		strLen_or_Ind = { strLenOrIndCol1.data(),
			strLenOrIndCol2.data(), strLenOrIndCol3.data() };

		// Coalesce the arrays of each row of each column
		// into a contiguous array for each column.
		//
		vector<char> stringCol1Data =
			GenerateContiguousData<char>(stringCol1, strLenOrIndCol1.data());

		vector<char> stringCol2Data =
			GenerateContiguousData<char>(stringCol2, strLenOrIndCol2.data());

		void* expectedDataSet[] = { stringCol1Data.data(),
									stringCol2Data.data(),
									nullptr };

		TestGetStringResults(
			rowsNumber,
			expectedDataSet,
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
		InitializeSession(0, // parametersNumber
			inputSchemaColumnsNumber,
			m_scriptString);

		const SQLCHAR BinaryValue1[] = { 0x01, 0x01, 0xe2, 0x40 };
		const SQLCHAR BinaryValue2[] = { 0x04, 0x05, 0xe1 };
		const SQLCHAR BinaryValue3[] = { 0x00, 0x00, 0x00, 0x01 };
		const SQLCHAR BinaryValue4[] = { 0xff };

		const SQLCHAR BinaryValue5[] = { 0x00 };
		const SQLCHAR BinaryValue6[] = { 0xff, 0xff, 0xff, 0xff };
		const SQLCHAR BinaryValue7[] = { 0x00, 0x12, 0xd2, 0xff, 0x00, 0x12, 0xd2, 0xff, 0x00, 0x12, 0xd2, 0xff };

		string binaryColumn1Name = "BinaryColumn1";
		InitializeColumn(0, binaryColumn1Name, SQL_C_BINARY, m_BinarySize);

		string binaryColumn2Name = "BinaryColumn2";
		InitializeColumn(1, binaryColumn2Name, SQL_C_BINARY, m_BinarySize);

		string binaryColumn3Name = "BinaryColumn3";
		InitializeColumn(2, binaryColumn3Name, SQL_C_BINARY, m_BinarySize);

		vector<const SQLCHAR*> binaryCol1{ BinaryValue1, BinaryValue2, BinaryValue3, BinaryValue4 };
		vector<const SQLCHAR*> binaryCol2{ BinaryValue5, BinaryValue6, nullptr, BinaryValue7 };

		SQLINTEGER strLenOrIndCol1[] =
		{
			static_cast<SQLINTEGER>(sizeof(BinaryValue1) / m_BinarySize),
			static_cast<SQLINTEGER>(sizeof(BinaryValue2) / m_BinarySize),
			static_cast<SQLINTEGER>(sizeof(BinaryValue3) / m_BinarySize),
			static_cast<SQLINTEGER>(sizeof(BinaryValue4) / m_BinarySize)
		};

		SQLINTEGER strLenOrIndCol2[] =
		{
			SQL_NULL_DATA,
			static_cast<SQLINTEGER>(sizeof(BinaryValue6) / m_BinarySize),
			SQL_NULL_DATA,
			static_cast<SQLINTEGER>(sizeof(BinaryValue7) / m_BinarySize)
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

	// Name: GetDateTimeResultsTest
	//
	// Description:
	//  Test GetResults with default script expecting an OutputDataSet of DateTime column data.
	//
	TEST_F(PythonExtensionApiTests, GetDateTimeResultsTest)
	{
		// Initialize with a default Session that prints Hello PythonExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(0, // parametersNumber
			(*m_dateTimeInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQL_TIMESTAMP_STRUCT, SQL_C_TYPE_TIMESTAMP>(m_dateTimeInfo.get());

		TestExecute<SQL_TIMESTAMP_STRUCT, SQL_C_TYPE_TIMESTAMP>(
			ColumnInfo<SQL_TIMESTAMP_STRUCT>::sm_rowsNumber,
			(*m_dateTimeInfo).m_dataSet.data(),
			(*m_dateTimeInfo).m_strLen_or_Ind.data(),
			(*m_dateTimeInfo).m_columnNames);

		TestGetDateTimeResults<SQL_TIMESTAMP_STRUCT>(
			ColumnInfo<SQL_TIMESTAMP_STRUCT>::sm_rowsNumber,
			(*m_dateTimeInfo).m_dataSet.data(),
			(*m_dateTimeInfo).m_strLen_or_Ind.data(),
			(*m_dateTimeInfo).m_columnNames);
	}

	// Name: GetDateResultsTest
	//
	// Description:
	//  Test GetResults with default script expecting an OutputDataSet of Date column data.
	//
	TEST_F(PythonExtensionApiTests, GetDateResultsTest)
	{
		// Initialize with a default Session that prints Hello PythonExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(0, // parametersNumber
			(*m_dateInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQL_DATE_STRUCT, SQL_C_TYPE_DATE>(m_dateInfo.get());

		TestExecute<SQL_DATE_STRUCT, SQL_C_TYPE_DATE>(
			ColumnInfo<SQL_DATE_STRUCT>::sm_rowsNumber,
			(*m_dateInfo).m_dataSet.data(),
			(*m_dateInfo).m_strLen_or_Ind.data(),
			(*m_dateInfo).m_columnNames,
			false);

		TestGetDateTimeResults<SQL_DATE_STRUCT>(
			ColumnInfo<SQL_DATE_STRUCT>::sm_rowsNumber,
			(*m_dateInfo).m_dataSet.data(),
			(*m_dateInfo).m_strLen_or_Ind.data(),
			(*m_dateInfo).m_columnNames);
	}

	// Name: NaTDateTimeResultTest
	//
	// Description:
	//  Test GetResults with a script that returns a datetime column with NaT.
	//
	TEST_F(PythonExtensionApiTests, NaTDateTimeResultTest)
	{
		// With this script, we create a DataFrame with one column, a datetime with NaT.
		//
		string scriptString = "from pandas import DataFrame;"
			"import numpy as np;"
			"OutputDataSet = DataFrame([np.datetime64('NaT')], columns=['Column1']);";

		InitializeSession(0, // parametersNumber
			0,               // inputSchemaColumnsNumber
			scriptString);

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

		// Construct the expected output data set
		//
		SQLULEN rowsNum = 1;
		vector<string> colNames = { "Column1" };
			
		vector<SQLINTEGER> strLen = { SQL_NULL_DATA };
		vector<SQLINTEGER *> pStrLen = { strLen.data() };

		vector<SQL_TIMESTAMP_STRUCT> data = { {} };
		vector<void *> pData = { data.data() };

		TestGetDateTimeResults<SQL_DATE_STRUCT>(
			rowsNum,
			pData.data(),
			pStrLen.data(),
			colNames);
	}

	// Name: GetDifferentResultsTest
	//
	// Description:
	//  Test GetResults with default script using an OutputDataSet of different column types.
	//
	TEST_F(PythonExtensionApiTests, GetDifferentColumnResultsTest)
	{
		int inputSchemaColumnsNumber = 3;

		// Initialize with a default Session that prints Hello PythonExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(0, // parametersNumber
			inputSchemaColumnsNumber,
			m_scriptString);

		string integerColumnName = "IntegerColumn";
		InitializeColumn(0, integerColumnName, SQL_C_SLONG, m_IntSize);

		string doubleColumnName = "DoubleColumn";
		InitializeColumn(1, doubleColumnName, SQL_C_DOUBLE, m_DoubleSize);

		string stringColumnName = "CharColumn";
		InitializeColumn(2, stringColumnName, SQL_C_CHAR, m_CharSize);

		SQLULEN expectedRowsNumber = 5;
		vector<SQLINTEGER> intColData{ m_MaxInt, m_MinInt, 0, 0, -1 };
		vector<SQLDOUBLE> doubleColData{ m_MinDouble, 1.33, 83.98, 72.45, m_MaxDouble };
		vector<const char*> stringCol{ "Hello", "test", "data", "World", "-123" };

		vector<SQLINTEGER> strLenOrIndCol1 = { m_IntSize, m_IntSize, SQL_NULL_DATA,
			SQL_NULL_DATA, m_IntSize };
		vector<SQLINTEGER> strLenOrIndCol2(expectedRowsNumber, m_DoubleSize);
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

		vector<SQLINTEGER> outputStrLenOrIndCol1 = { m_DoubleSize, m_DoubleSize, SQL_NULL_DATA,
			SQL_NULL_DATA, m_DoubleSize };

		// Test the data returned.
		//
		CheckColumnDataEqualityForNullable<SQLINTEGER, SQLDOUBLE>(
			rowsNumber,
			static_cast<SQLINTEGER*>(expectedData[0]),
			static_cast<SQLINTEGER*>(data[0]),
			outputStrLenOrIndCol1.data(),
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
		bp::dict outputDataSet = bp::extract<bp::dict>(
			bp::eval(createDictScript.c_str(), m_mainNamespace));

		EXPECT_EQ(outputschemaColumnsNumber, bp::len(outputDataSet.keys()));

		bp::dict intColumn = bp::extract<bp::dict>(outputDataSet[integerColumnName]);
		CheckColumnEquality<SQLDOUBLE>(
			rowsNumber,
			intColumn,
			data[0],
			strLen_or_Ind[0]);

		bp::dict doubleColumn = bp::extract<bp::dict>(outputDataSet[doubleColumnName]);
		CheckColumnEquality<SQLDOUBLE>(
			rowsNumber,
			doubleColumn,
			data[1],
			strLen_or_Ind[1]);

		bp::dict stringColumn = bp::extract<bp::dict>(outputDataSet[stringColumnName]);
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

		// Initialize with a Session that executes the above script
		// that creates an empty row 1 column OutputDataSet.
		//
		InitializeSession(0, // parametersNumber
			0,               //inputSchemaColumnsNumber
			scriptString);

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

		TestGetResults<SQLINTEGER, int, SQL_C_SLONG>(
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
	template<class SQLType, class InputCType, SQLSMALLINT DataType>
	void PythonExtensionApiTests::TestGetResults(
		SQLULEN        expectedRowsNumber,
		SQLPOINTER     *expectedData,
		SQLINTEGER     **expectedStrLen_or_Ind,
		vector<string> columnNames)
	{
		SQLULEN    rowsNumber = 0;
		SQLPOINTER *data = nullptr;
		SQLINTEGER **strLen_or_Ind = nullptr;
		SQLRETURN  result = GetResults(
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
		bp::dict outputDataSet = bp::extract<bp::dict>(
			bp::eval(createDictScript.c_str(), m_mainNamespace));

		for (size_t columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
		{
			bp::dict column = bp::extract<bp::dict>(outputDataSet[columnNames[columnNumber]]);

			SQLINTEGER *expectedColumnStrLenOrInd = expectedStrLen_or_Ind[columnNumber];
			SQLINTEGER *columnStrLenOrInd = strLen_or_Ind[columnNumber];

			bool hasNulls = false;
			for (SQLULEN i = 0; i < rowsNumber; ++i)
			{
				if (expectedColumnStrLenOrInd[i] == SQL_NULL_DATA)
				{
					hasNulls = true;
					break;
				}
			}

			SQLType *expectedColumnData = static_cast<SQLType *>(expectedData[columnNumber]);
			SQLType *columnData = static_cast<SQLType *>(data[columnNumber]);

			if (!(hasNulls))
			{
				CheckColumnDataEquality<SQLType>(
					rowsNumber,
					expectedColumnData,
					columnData,
					expectedColumnStrLenOrInd,
					columnStrLenOrInd);

				CheckColumnEqualityFnMap::const_iterator it = sm_FnCheckColumnEqualityMap.find(DataType);

				if (it == sm_FnCheckColumnEqualityMap.end())
				{
					throw runtime_error("Unsupported column type " + to_string(DataType) +
						" encountered when testing column equality");
				}

				(this->*it->second)(
					rowsNumber,
					column,
					columnData,
					columnStrLenOrInd);
			}
			else if (is_same_v<InputCType, bool>)
			{
				CheckColumnDataEquality<SQLType>(
					rowsNumber,
					expectedColumnData,
					columnData,
					expectedColumnStrLenOrInd,
					columnStrLenOrInd);

				CheckStringColumnEquality(
					rowsNumber,
					column,
					columnData,
					columnStrLenOrInd);
			}
			else
			{
				CheckColumnDataEqualityForNullable<SQLType, SQLDOUBLE>(
					rowsNumber,
					expectedColumnData,
					columnData,
					expectedColumnStrLenOrInd,
					columnStrLenOrInd);

				CheckColumnEquality<SQLDOUBLE>(
					rowsNumber,
					column,
					columnData,
					columnStrLenOrInd);
			}
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

			EXPECT_EQ(columnData[index], expectedColumnData[index]);
		}
	}

	// Name: CheckColumnDataEqualityForNullable
	//
	// Description:
	//  Template function to compare the given column data
	//  and nullMap with rowsNumber for equality.
	//  Nullable columns have complications - because the python value is None or NAN,
	//  the type of the output is not the same as the type of the input; it will be broad types.
	//
	template<class SQLType, class DefaultType>
	void PythonExtensionApiTests::CheckColumnDataEqualityForNullable(
		SQLULEN    rowsNumber,
		SQLType    *expectedColumnData,
		void       *pColumnData,
		SQLINTEGER *expectedColumnStrLenOrInd,
		SQLINTEGER *columnStrLenOrInd)
	{
		DefaultType *columnData = static_cast<DefaultType*>(pColumnData);
		if (rowsNumber == 0 || columnStrLenOrInd == nullptr)
		{
			EXPECT_EQ(columnData, nullptr);
			EXPECT_EQ(columnStrLenOrInd, nullptr);
		}

		for (SQLULEN index = 0; index < rowsNumber; ++index)
		{
			SQLINTEGER strLen = expectedColumnStrLenOrInd[index];
			if (strLen != SQL_NULL_DATA)
			{
				strLen = sizeof(DefaultType);
				EXPECT_EQ(columnStrLenOrInd[index], strLen);

				if (columnStrLenOrInd[index] != SQL_NULL_DATA)
				{
					EXPECT_EQ(columnData[index], static_cast<DefaultType>(expectedColumnData[index]));
				}
				else if constexpr (is_same_v<DefaultType, SQLDOUBLE>)
				{
					// Double NULLs will be NAN.
					//
					EXPECT_TRUE(isnan(columnData[index]));
				}
				else
				{
					// Int NANs are defined as 0.
					//
					EXPECT_EQ(columnData[index], 0);
				}
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
		bp::dict outputDataSet = bp::extract<bp::dict>(
			bp::eval(createDictScript.c_str(), m_mainNamespace));

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

			bp::dict column = bp::extract<bp::dict>(outputDataSet[columnNames[columnNumber]]);
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
		bp::dict outputDataSet = bp::extract<bp::dict>(
			bp::eval(createDictScript.c_str(), m_mainNamespace));

		for (size_t columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
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

			bp::dict column = bp::extract<bp::dict>(outputDataSet[columnNames[columnNumber]]);
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

	// Name: TestGetDateTimeResults
	//
	// Description:
	//  Test GetResults to verify the expected results are obtained for datetime data.
	//
	template<class DateTimeStruct>
	void PythonExtensionApiTests::TestGetDateTimeResults(
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
		bp::dict outputDataSet = bp::extract<bp::dict>(
			bp::eval(createDictScript.c_str(), m_mainNamespace));

		for (size_t columnNumber = 0; columnNumber < columnNames.size(); ++columnNumber)
		{
			bp::dict column = bp::extract<bp::dict>(outputDataSet[columnNames[columnNumber]]);

			SQLINTEGER *expectedColumnStrLenOrInd = expectedStrLen_or_Ind[columnNumber];
			SQLINTEGER *columnStrLenOrInd = strLen_or_Ind[columnNumber];
			
			DateTimeStruct *expectedColumnData = static_cast<DateTimeStruct *>(expectedData[columnNumber]);
			char *columnData = static_cast<char *>(data[columnNumber]);
			vector<DateTimeStruct> results;
			StringToDateTimeColumn<DateTimeStruct>(rowsNumber, columnData, columnStrLenOrInd, &results);


			CheckDateTimeDataEquality<DateTimeStruct>(
				rowsNumber,
				expectedColumnData,
				results.data(),
				expectedColumnStrLenOrInd,
				columnStrLenOrInd);

			CheckDateTimeColumnEquality<DateTimeStruct>(
				rowsNumber,
				column,
				results.data(),
				columnStrLenOrInd);
		}
	}

	// Name: StringToDateTimeColumn
	//
	// Description:
	//  Convert a datetime string to a Date/Timestamp struct.
	//  Datetime strings should be YYYY-MM-DD hh:mm:ss.fraction.
	//
	template<class DateTimeStruct>
	void PythonExtensionApiTests::StringToDateTimeColumn(
		SQLULEN                rowsNumber,
		char                   *columnData,
		SQLINTEGER             *columnStrLenOrInd,
		vector<DateTimeStruct> *results)
	{
		int len = 0;

		for (SQLULEN row=0; row < rowsNumber; ++row) 
		{
			DateTimeStruct value = {};
			if (columnStrLenOrInd[row] != SQL_NULL_DATA)
			{
				string stringDate = string(columnData + len, columnStrLenOrInd[row]);

				vector<char *> out;
				PythonTestUtilities::Tokenize(const_cast<char *>(stringDate.c_str()), " ", &out);

				vector<char *> date;
				PythonTestUtilities::Tokenize(out[0], "-", &date);

				value.year = stoi(date[0]);
				value.month = stoi(date[1]);
				value.day = stoi(date[2]);

				if constexpr (is_same_v<DateTimeStruct, SQL_TIMESTAMP_STRUCT>)
				{
					vector<char *> time;
					PythonTestUtilities::Tokenize(out[1], ":", &time);

					value.hour = stoi(time[0]);
					value.minute = stoi(time[1]);
					value.second = stoi(time[2]);

					vector<char *> fraction;
					PythonTestUtilities::Tokenize(time[2], ".", &fraction);

					if (fraction.size() == 2)
					{
						value.fraction = stoi(fraction[1]) * 1000;
					}
				}

				len += columnStrLenOrInd[row];
			}

			(*results).push_back(value);
		}
	}

	// Name: CheckDateTimeDataEquality
	//
	// Description:
	//  Compare the given datetime data & nullMap for equality.
	//
	template<class DateTimeStruct>
	void PythonExtensionApiTests::CheckDateTimeDataEquality(
		SQLULEN        rowsNumber,
		DateTimeStruct *expectedColumnData,
		DateTimeStruct *columnData,
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
			if (expectedColumnStrLenOrInd != nullptr)
			{
				EXPECT_EQ(columnStrLenOrInd[index], expectedColumnStrLenOrInd[index]);

				if (columnStrLenOrInd[index] != SQL_NULL_DATA)
				{
					EXPECT_EQ(expectedColumnData[index].year, columnData[index].year);
					EXPECT_EQ(expectedColumnData[index].month, columnData[index].month);
					EXPECT_EQ(expectedColumnData[index].day, columnData[index].day);

					if constexpr (is_same_v<DateTimeStruct, SQL_TIMESTAMP_STRUCT>)
					{
						EXPECT_EQ(expectedColumnData[index].hour, columnData[index].hour);
						EXPECT_EQ(expectedColumnData[index].minute, columnData[index].minute);
						EXPECT_EQ(expectedColumnData[index].second, columnData[index].second);
						EXPECT_EQ(expectedColumnData[index].fraction, columnData[index].fraction);
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
