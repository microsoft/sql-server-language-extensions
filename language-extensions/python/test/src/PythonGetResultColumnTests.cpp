//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonGetResultColumnTests.cpp
//
// Purpose:
//  Tests the Python Extension's implementation of the external language GetResultColumn API.
//
//*************************************************************************************************
#include "PythonExtensionApiTests.h"

using namespace std;

namespace ExtensionApiTest
{
	// Name: GetIntegerResultColumnsTest
	//
	// Description:
	//  Test GetResultColumn with default script expecting an OutputDataSet of Integer columns.
	//
	TEST_F(PythonExtensionApiTests, GetIntegerResultColumnsTest)
	{
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

		TestGetResultColumn(0,  // columnNumber
			SQL_C_SLONG,        // dataType
			m_IntSize,          // columnSize
			0,                  // decimalDigits
			SQL_NO_NULLS);      // nullable

		// Returns most generic int type because NULLs change the column to object
		//
		TestGetResultColumn(1,  // columnNumber
			SQL_C_SBIGINT,      // dataType
			m_BigIntSize,       // columnSize
			0,                  // decimalDigits
			SQL_NULLABLE);      // nullable
	}

	// Name: GetBooleanResultColumnsTest
	//
	// Description:
	//  Test GetResultColumn with default script using an OutputDataSet of Boolean columns.
	//
	TEST_F(PythonExtensionApiTests, GetBooleanResultColumnsTest)
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

		TestGetResultColumn(0, // columnNumber
			SQL_C_BIT,         // dataType
			m_BooleanSize,     // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		TestGetResultColumn(1, // columnNumber
			SQL_C_BIT,         // dataType
			m_BooleanSize,     // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetRealResultColumnsTest
	//
	// Description:
	//  Test GetResultColumn with default script using an OutputDataSet of Real columns.
	//
	TEST_F(PythonExtensionApiTests, GetRealResultColumnsTest)
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

		TestGetResultColumn(0, // columnNumber
			SQL_C_FLOAT,       // dataType
			m_RealSize,        // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		// Returns most generic float type because NULLs change the column to object
		//
		TestGetResultColumn(1, // columnNumber
			SQL_C_DOUBLE,      // dataType
			m_DoubleSize,      // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetDoubleResultColumnsTest
	//
	// Description:
	//  Test GetResultColumn with default script using an OutputDataSet of Double columns.
	//
	TEST_F(PythonExtensionApiTests, GetDoubleResultColumnsTest)
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

		TestGetResultColumn(0, // columnNumber
			SQL_C_DOUBLE,      // dataType
			m_DoubleSize,      // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		TestGetResultColumn(1, // columnNumber
			SQL_C_DOUBLE,      // dataType
			m_DoubleSize,      // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetBigIntResultColumnsTest
	//
	// Description:
	//  Test GetResultColumn with default script using an OutputDataSet of BigInteger columns.
	//
	TEST_F(PythonExtensionApiTests, GetBigIntResultColumnsTest)
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

		TestGetResultColumn(0, // columnNumber
			SQL_C_SBIGINT,     // dataType
			m_BigIntSize,      // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		TestGetResultColumn(1, // columnNumber
			SQL_C_SBIGINT,     // dataType
			m_BigIntSize,      // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetSmallIntResultColumnsTest
	//
	// Description:
	//  Test GetResultColumn with default script using an OutputDataSet of SmallInt columns.
	//
	TEST_F(PythonExtensionApiTests, GetSmallIntResultColumnsTest)
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

		TestGetResultColumn(0, // columnNumber
			SQL_C_SSHORT,      // dataType
			m_SmallIntSize,    // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		// Returns most generic int type because NULLs change the column to object
		//
		TestGetResultColumn(1, // columnNumber
			SQL_C_SBIGINT,     // dataType
			m_BigIntSize,      // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetTinyIntResultColumnsTest
	//
	// Description:
	//  Test GetResultColumn with default script using an OutputDataSet of TinyInt columns.
	//
	TEST_F(PythonExtensionApiTests, GetTinyIntResultColumnsTest)
	{
		// Initialize with a default Session that prints Hello PythonExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(0, // parametersNumber
			(*m_tinyIntInfo).GetColumnsNumber(),
			m_scriptString);

		InitializeColumns<SQLCHAR, SQL_C_UTINYINT>(m_tinyIntInfo.get());

		TestExecute<SQLCHAR, SQL_C_UTINYINT>(
			ColumnInfo<SQLCHAR>::sm_rowsNumber,
			(*m_tinyIntInfo).m_dataSet.data(),
			(*m_tinyIntInfo).m_strLen_or_Ind.data(),
			(*m_tinyIntInfo).m_columnNames,
			false);  // validate

		TestGetResultColumn(0, // columnNumber
			SQL_C_UTINYINT,    // dataType
			m_TinyIntSize,     // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		// Returns most generic int type because NULLs change the column to object
		//
		TestGetResultColumn(1, // columnNumber
			SQL_C_SBIGINT,     // dataType
			m_BigIntSize,      // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetStringResultColumnsTest
	//
	// Description:
	//  Test GetResultColumn with default script using an OutputDataSet of String columns.
	//
	TEST_F(PythonExtensionApiTests, GetStringResultColumnsTest)
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

		vector<const char *> stringCol1{ "Hello", "test", "data", "World", "-123" };
		vector<const char *> stringCol2{ "", 0, nullptr, "verify", "-1" };

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

		vector<SQLINTEGER *> strLen_or_Ind{ strLenOrIndCol1.data(),
			strLenOrIndCol2.data(), nullptr };

		// Coalesce the arrays of each row of each column
		// into a contiguous array for each column.
		//
		vector<char> stringCol1Data = GenerateContiguousData<char>(stringCol1, strLenOrIndCol1.data());
		vector<char> stringCol2Data = GenerateContiguousData<char>(stringCol2, strLenOrIndCol2.data());

		void *dataSet[] = { stringCol1Data.data(),
							stringCol2Data.data(),
							nullptr };

		int rowsNumber = stringCol1.size();

		vector<string> columnNames{ stringColumn1Name, stringColumn2Name, stringColumn3Name };

		TestExecute<SQLCHAR, SQL_C_CHAR>(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames,
			false); // validate

		SQLULEN maxCol1Len = GetMaxLength(strLenOrIndCol1.data(), rowsNumber);
		SQLULEN maxCol2Len = GetMaxLength(strLenOrIndCol2.data(), rowsNumber);

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
			sizeof(SQLCHAR),   // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetWStringResultColumnsTest
	//
	// Description:
	//  Test GetResultColumn with default script using an OutputDataSet of wstring columns.
	//
	TEST_F(PythonExtensionApiTests, GetWStringResultColumnsTest)
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

		vector<const wchar_t *> wstringCol1{ L"Hello", L"test", L"data", L"World", L"你" };
		vector<const wchar_t *> wstringCol2{ L"", 0, nullptr, L"verify", L"-1" };

		int rowsNumber = wstringCol1.size();

		vector<SQLINTEGER> strLenOrIndCol1 =
		{ static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(1 * sizeof(wchar_t)) };
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
		vector<wchar_t> wstringCol1Data =
			GenerateContiguousData<wchar_t>(wstringCol1, strLenOrIndCol1.data());

		vector<wchar_t> wstringCol2Data =
			GenerateContiguousData<wchar_t>(wstringCol2, strLenOrIndCol2.data());

		void *dataSet[] = { wstringCol1Data.data(),
							wstringCol2Data.data(),
							nullptr };

		vector<string> columnNames{ wstringColumn1Name, wstringColumn2Name, wstringColumn3Name };

		TestExecute<wchar_t, SQL_C_WCHAR>(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames,
			false); // validate

		// Because Python is UTF-8, we always return UTF-8 strings. 
		// Thus, we expect output result columns to be strings, not wstrings, so we need to resize
		// our expected column size by dividing by the sizeof(wchar_t).
		//
		SQLULEN maxCol1Len = GetMaxLength(strLenOrIndCol1.data(), rowsNumber) / sizeof(wchar_t);
		SQLULEN maxCol2Len = GetMaxLength(strLenOrIndCol2.data(), rowsNumber) / sizeof(wchar_t);

		TestGetResultColumn(0, // columnNumber
			SQL_C_CHAR,        // expectedDataType
			maxCol1Len,        // expectedColumnSize
			0,                 // expectedDecimalDigits
			SQL_NO_NULLS);     // expectedNullable

		TestGetResultColumn(1, // columnNumber
			SQL_C_CHAR,        // expectedDataType
			maxCol2Len,        // expectedColumnSize
			0,                 // expectedDecimalDigits
			SQL_NULLABLE);     // expectedNullable

		TestGetResultColumn(2, // columnNumber
			SQL_C_CHAR,        // expectedDataType
			sizeof(SQLCHAR),   // expectedColumnSize
			0,                 // expectedDecimalDigits
			SQL_NULLABLE);     // expectedNullable
	}

	// Name: GetRawResultColumnTest
	//
	// Description:
	//  Test GetResultColumn with a script that returns OutputDataSet with raw columns.
	//
	TEST_F(PythonExtensionApiTests, GetRawResultColumnsTest)
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

		SQLULEN maxCol1Len = GetMaxLength(strLenOrIndCol1, rowsNumber);
		SQLULEN maxCol2Len = GetMaxLength(strLenOrIndCol2, rowsNumber);

		TestExecute<SQLCHAR, SQL_C_BINARY>(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames,
			false);

		TestGetResultColumn(0, // columnNumber
			SQL_C_BINARY,      // dataType
			maxCol1Len,        // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		TestGetResultColumn(1, // columnNumber
			SQL_C_BINARY,      // dataType
			maxCol2Len,        // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable

		TestGetResultColumn(2, // columnNumber
			SQL_C_CHAR,        // dataType; since the whole column is None, we use CHAR NoneType
			sizeof(SQLCHAR),   // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetDateTimeResultColumnsTest
	//
	// Description:
	//  Test GetResultColumn with default script expecting an OutputDataSet of DateTime columns.
	//
	TEST_F(PythonExtensionApiTests, GetDateTimeResultColumnsTest)
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
			(*m_dateTimeInfo).m_columnNames,
			false);

		TestGetResultColumn(0,    // columnNumber
			SQL_C_TYPE_TIMESTAMP, // dataType
			m_DateTimeSize,       // columnSize
			0,                    // decimalDigits
			SQL_NO_NULLS);        // nullable

		TestGetResultColumn(1,    // columnNumber
			SQL_C_TYPE_TIMESTAMP, // dataType
			m_DateTimeSize,       // columnSize
			0,                    // decimalDigits
			SQL_NULLABLE);        // nullable
	}

	// Name: GetDateResultColumnsTest
	//
	// Description:
	//  Test GetResultColumn with default script expecting an OutputDataSet of Date columns.
	//
	TEST_F(PythonExtensionApiTests, GetDateResultColumnsTest)
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

		TestGetResultColumn(0, // columnNumber
			SQL_C_TYPE_DATE,   // dataType
			m_DateSize,        // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		TestGetResultColumn(1, // columnNumber
			SQL_C_TYPE_DATE,   // dataType
			m_DateSize,        // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable
	}

	// Name: GetDifferentResultColumnsTest
	//
	// Description:
	//  Test GetResultColumn with default script using an OutputDataSet of different column types.
	//
	TEST_F(PythonExtensionApiTests, GetDifferentResultColumnsTest)
	{
		SQLUSMALLINT inputSchemaColumnsNumber = 3;

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

		string stringColumnName = "StringColumn";
		InitializeColumn(2, stringColumnName, SQL_C_CHAR, m_CharSize);

		vector<SQLINTEGER> intColData{ m_MaxInt, m_MinInt, 0, 1320, -1 };
		vector<SQLDOUBLE> doubleColData{ m_MinDouble, 1.33, 83.98, 72.45, m_MaxDouble };
		vector<const char*> stringCol{ "Hello", "test", "data", "World", "-123" };

		const SQLINTEGER intSize = m_IntSize;
		SQLINTEGER strLenOrIndCol1[] = { intSize, intSize, SQL_NULL_DATA, SQL_NULL_DATA, intSize };
		SQLINTEGER strLenOrIndCol3[] =
		{ static_cast<SQLINTEGER>(strlen(stringCol[0])),
		static_cast<SQLINTEGER>(strlen(stringCol[1])),
		static_cast<SQLINTEGER>(strlen(stringCol[2])),
		static_cast<SQLINTEGER>(strlen(stringCol[3])),
		static_cast<SQLINTEGER>(strlen(stringCol[4])) };
		vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1, nullptr, strLenOrIndCol3 };

		int rowsNumber = intColData.size();

		vector<char> stringColData = GenerateContiguousData<char>(stringCol, strLenOrIndCol3);

		vector<void*> dataSet{ intColData.data(), doubleColData.data(), stringColData.data() };

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = Execute(
			*m_sessionId,
			m_taskId,
			rowsNumber,
			dataSet.data(),
			strLen_or_Ind.data(),
			&outputschemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		TestGetResultColumn(0, // columnNumber
			SQL_C_SBIGINT,     // dataType
			m_BigIntSize,      // columnSize
			0,                 // decimalDigits
			SQL_NULLABLE);     // nullable

		TestGetResultColumn(1, // columnNumber
			SQL_C_DOUBLE,      // dataType
			m_DoubleSize,      // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable

		SQLULEN maxLen = GetMaxLength(strLenOrIndCol3, rowsNumber);

		TestGetResultColumn(2, // columnNumber
			SQL_C_CHAR,        // dataType
			maxLen,            // columnSize
			0,                 // decimalDigits
			SQL_NO_NULLS);     // nullable
	}

	// Name: GetEmptyResultColumnTest
	//
	// Description:
	//  Test GetResultColumn with a script that returns OutputDataSet with empty rows.
	//
	TEST_F(PythonExtensionApiTests, GetEmptyResultColumnTest)
	{
		string scriptString = "from pandas import DataFrame; import numpy as np;"
		"OutputDataSet = DataFrame({'intCol' : np.array([], dtype=np.int32)});"
		"print(OutputDataSet)";

		// Initialize with a Session that executes the above script
		// that creates an empty row 1 column OutputDataSet.
		//
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

		TestGetResultColumn(0,  // columnNumber
			SQL_C_SLONG,        // dataType
			m_IntSize,          // columnSize
			0,                  // decimalDigits
			SQL_NO_NULLS);      // nullable
	}

	// Name: MixedColumnNamesTest
	//
	// Description:
	//  Test GetResultColumn with a script that returns a dataset with mixed type column names
	//  i.e. integer and string.
	//  This makes sure that we can access columns with integer column names.
	//
	TEST_F(PythonExtensionApiTests, MixedColumnNamesTest)
	{
		// With this script, we create a DataFrame with unusual column names.
		// This tests non-string and mixed type column names.
		//
		string scriptString = "from pandas import DataFrame;"
			"OutputDataSet = DataFrame([['ABC', 123],['DEF', 456]], columns=[3, 'col2']);"
			"print(OutputDataSet)";

		// Initialize with a Session that executes the above script
		// that creates an OutputDataSet with mixed column names.
		//
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

		EXPECT_EQ(outputschemaColumnsNumber, 2);
		
		TestGetResultColumn(0,  // columnNumber
			SQL_C_CHAR,         // dataType
			3,                  // columnSize
			0,                  // decimalDigits
			SQL_NO_NULLS);      // nullable

		TestGetResultColumn(1,  // columnNumber
			SQL_C_SBIGINT,      // dataType
			m_BigIntSize,       // columnSize
			0,                  // decimalDigits
			SQL_NO_NULLS);      // nullable
	}

	// Name: AllNonesColumnTest
	//
	// Description:
	//  Test GetResultColumn with a script that returns a dataset with a column of all Nones.
	//  We expect the output to be of type CHAR and have the minimum columnSize of 1.
	//
	TEST_F(PythonExtensionApiTests, AllNonesColumnTest)
	{
		// With this script, we create a DataFrame with column of Nones.
		//
		string scriptString = "from pandas import DataFrame;"
			"OutputDataSet = DataFrame({'noneColumn': [None, None, None, None, None]});"
			"print(OutputDataSet)";

		// Initialize with a Session that executes the above script
		// that creates an OutputDataSet with column of Nones.
		//
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

		TestGetResultColumn(0,  // columnNumber
			SQL_C_CHAR,         // dataType
			sizeof(SQLCHAR),    // columnSize
			0,                  // decimalDigits
			SQL_NULLABLE);      // nullable
	}

	// Name: TestGetResultColumn
	//
	// Description:
	//  Test GetResultColumn to verify the expected result column information is obtained.
	//
	void PythonExtensionApiTests::TestGetResultColumn(
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

		SQLRETURN result = GetResultColumn(
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
