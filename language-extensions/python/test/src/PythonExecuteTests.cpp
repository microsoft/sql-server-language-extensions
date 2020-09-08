//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonExecuteTests.cpp
//
// Purpose:
//  Tests the PythonExtension's implementation of the external language Execute API.
//
//*************************************************************************************************

#include <sstream>

#include "PythonExtensionApiTests.h"
#include "PythonTestUtilities.h"

using namespace std;
namespace bp = boost::python;

namespace ExtensionApiTest
{
	// Name: ExecuteIntegerColumnsTest
	//
	// Description:
	//  Test Execute with default script using an InputDataSet of Integer columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteIntegerColumnsTest)
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
			(*m_integerInfo).m_columnNames);
	}

	// Name: ExecuteBooleanColumnsTest
	//
	// Description:
	//  Test Execute using an InputDataSet of Boolean columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteBooleanColumnsTest)
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
			(*m_booleanInfo).m_columnNames);
	}

	// Name: ExecuteRealColumnsTest
	//
	// Description:
	//  Test Execute with default script using an InputDataSet of Real columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteRealColumnsTest)
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
			(*m_realInfo).m_columnNames);
	}

	// Name: ExecuteDoubleColumnsTest
	//
	// Description:
	//  Test Execute with default script using an InputDataSet of Double columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteDoubleColumnsTest)
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
			(*m_doubleInfo).m_columnNames);
	}

	// Name: ExecuteBigIntColumnsTest
	//
	// Description:
	//  Test Execute with default script using an InputDataSet of BigInteger columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteBigIntColumnsTest)
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
			(*m_bigIntInfo).m_columnNames);
	}

	// Name: ExecuteSmallIntColumnsTest
	//
	// Description:
	//  Test Execute with default script using an InputDataSet of SmallInt columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteSmallIntColumnsTest)
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
			(*m_smallIntInfo).m_columnNames);
	}

	// Name: ExecuteTinyIntColumnsTest
	//
	// Description:
	//  Test Execute with default script using an InputDataSet of TinyInt columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteTinyIntColumnsTest)
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
			(*m_tinyIntInfo).m_columnNames);
	}

	// Name: ExecuteStringColumnsTest
	//
	// Description:
	//  Test Execute with default script using an InputDataSet of string columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteStringColumnsTest)
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

		vector<const char*> stringCol1{ "Hello", "test", "data", "World", "-123" };
		vector<const char*> stringCol2{ "", 0, nullptr, "verify", "-1" };

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

		vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1.data(),
			strLenOrIndCol2.data(), nullptr };

		// Coalesce the arrays of each row of each column
		// into a contiguous array for each column.
		//
		vector<char> stringCol1Data = GenerateContiguousData<char>(stringCol1, strLenOrIndCol1.data());
		vector<char> stringCol2Data = GenerateContiguousData<char>(stringCol2, strLenOrIndCol2.data());

		void* dataSet[] = { stringCol1Data.data(),
							stringCol2Data.data(),
							nullptr };

		int rowsNumber = stringCol1.size();

		vector<string> columnNames{ stringColumn1Name, stringColumn2Name, stringColumn3Name };

		TestExecute<SQLCHAR, SQL_C_CHAR>(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames);
	}

	// Name: ExecuteWStringColumnsTest
	//
	// Description:
	//  Test Execute with default script using an InputDataSet of wstring columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteWStringColumnsTest)
	{
		SQLUSMALLINT inputSchemaColumnsNumber = 3;

		// Initialize with a default Session that prints Hello PythonExtension
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(0, // parametersNumber
			inputSchemaColumnsNumber,
			m_scriptString);

		string wstringColumn1Name = "WStringColumn1";
		InitializeColumn(0, wstringColumn1Name, SQL_C_WCHAR, m_WCharSize);

		string wstringColumn2Name = "WStringColumn2";
		InitializeColumn(1, wstringColumn2Name, SQL_C_WCHAR, m_WCharSize);

		string wstringColumn3Name = "WStringColumn3";
		InitializeColumn(2, wstringColumn3Name, SQL_C_WCHAR, m_WCharSize);

		vector<const wchar_t*> wstringCol1{ L"Hello", L"test", L"data", L"World", L"你好" };
		vector<const wchar_t*> wstringCol2{ L"", 0, nullptr, L"verify", L"-1" };

		vector<SQLINTEGER> strLenOrIndCol1 =
		{ static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(4 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(2 * sizeof(wchar_t)) };
		vector<SQLINTEGER> strLenOrIndCol2 =
		{ 0, SQL_NULL_DATA, SQL_NULL_DATA,
		  static_cast<SQLINTEGER>(6 * sizeof(wchar_t)),
		  static_cast<SQLINTEGER>(2 * sizeof(wchar_t)) };

		vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1.data(),
			strLenOrIndCol2.data(), nullptr };

		// Coalesce the arrays of each row of each column
		// into a contiguous array for each column.
		//
		vector<wchar_t> wstringCol1Data = GenerateContiguousData<wchar_t>(wstringCol1, strLenOrIndCol1.data());
		vector<wchar_t> wstringCol2Data = GenerateContiguousData<wchar_t>(wstringCol2, strLenOrIndCol2.data());

		void* dataSet[] = { wstringCol1Data.data(),
							wstringCol2Data.data(),
							nullptr };

		int rowsNumber = wstringCol1.size();

		vector<string> columnNames{ wstringColumn1Name, wstringColumn2Name, wstringColumn3Name };

		TestExecute<wchar_t, SQL_C_WCHAR>(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames);
	}

	// Name: ExecuteRawColumnsTest
	//
	// Description:
	//  Test Execute with default script using an InputDataSet of binary columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteRawColumnsTest)
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
		vector<const SQLCHAR*> binaryCol2{ BinaryValue5, BinaryValue6, nullptr, BinaryValue7};

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
			columnNames);
	}

	// Name: ExecuteDifferentColumnsTest
	//
	// Description:
	//  Test Execute with default script using an InputDataSet of different column types.
	//
	TEST_F(PythonExtensionApiTests, ExecuteDifferentColumnsTest)
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

		SQLINTEGER strLenOrIndCol1[] = { 0, 0, SQL_NULL_DATA, SQL_NULL_DATA, 0 };
		SQLINTEGER strLenOrIndCol3[] =
		{ static_cast<SQLINTEGER>(strlen(stringCol[0])),
		static_cast<SQLINTEGER>(strlen(stringCol[1])),
		static_cast<SQLINTEGER>(strlen(stringCol[2])),
		static_cast<SQLINTEGER>(strlen(stringCol[3])),
		static_cast<SQLINTEGER>(strlen(stringCol[4])) };
		vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1, nullptr, strLenOrIndCol3 };

		int rowsNumber = intColData.size();

		vector<char> stringColData = GenerateContiguousData<char>(stringCol, strLenOrIndCol3);

		vector<void *> dataSet{ intColData.data(), doubleColData.data(), stringColData.data() };

		testing::internal::CaptureStdout();

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = Execute(
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

		try {
			string createDictScript = m_inputDataNameString + ".to_dict()";
			bp::dict inputDataSet = bp::extract<bp::dict>(bp::eval(createDictScript.c_str(), m_mainNamespace));

			createDictScript = m_outputDataNameString + ".to_dict()";
			bp::dict outputDataSet = bp::extract<bp::dict>(bp::eval(createDictScript.c_str(), m_mainNamespace));

			for(bp::dict ds : {inputDataSet, outputDataSet})
			{
				bp::dict intColumn = bp::extract<bp::dict>(ds.get(integerColumnName));
				CheckColumnEquality<SQLINTEGER>(
					rowsNumber,
					intColumn,
					dataSet[0],
					strLen_or_Ind[0]);

				bp::dict numericColumn = bp::extract<bp::dict>(ds.get(doubleColumnName));
				CheckColumnEquality<SQLDOUBLE>(
					rowsNumber,
					numericColumn,
					dataSet[1],
					strLen_or_Ind[1]);

				bp::dict stringColumn = bp::extract<bp::dict>(ds.get(stringColumnName));
				CheckStringColumnEquality(
					rowsNumber,
					stringColumn,
					dataSet[2],
					strLen_or_Ind[2]);
			}
		}
		catch (bp::error_already_set &)
		{
			string pyError = PythonTestUtilities::ParsePythonException();
			throw runtime_error("Error running python:\n" + pyError);
		}
	}

	// Name: ExecuteDateTimeColumnsTest
	//
	// Description:
	//  Test Execute with default script using an InputDataSet of DateTime columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteDateTimeColumnsTest)
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
	}

	// Name: ExecuteDateColumnsTest
	//
	// Description:
	//  Test Execute with default script using an InputDataSet of Date columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteDateColumnsTest)
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
			(*m_dateInfo).m_columnNames);
	}

	// Name: TestExecute
	//
	// Description:
	//  Template function to Test Execute with default script that assigns Input to Output.
	//  It tests the correctness of the:
	//   1. Executed script,
	//   2. InputDataSet and
	//   3. OutputDataSet
	//  This can also be run without the validation steps by setting "validate" to false.
	//
	template<class SQLType, SQLSMALLINT dataType>
	void PythonExtensionApiTests::TestExecute(
		SQLULEN        rowsNumber,
		void           **dataSet,
		SQLINTEGER     **strLen_or_Ind,
		vector<string> columnNames,
		bool           validate)
	{
		testing::internal::CaptureStdout();

		SQLUSMALLINT outputschemaColumnsNumber = 0;
		SQLRETURN result = Execute(
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
			// Verify print message was printed correctly
			//
			ASSERT_TRUE(output.find(m_printMessage) != string::npos);

			try {
				string createDictScript = m_inputDataNameString + ".to_dict()";
				bp::dict inputDataSet = bp::extract<bp::dict>(bp::eval(createDictScript.c_str(), m_mainNamespace));

				createDictScript = m_outputDataNameString + ".to_dict()";
				bp::dict outputDataSet = bp::extract<bp::dict>(bp::eval(createDictScript.c_str(), m_mainNamespace));

				for (SQLUSMALLINT columnIndex = 0; columnIndex < columnNames.size(); columnIndex++)
				{
					bp::dict inputColumnToTest = bp::extract<bp::dict>(inputDataSet.get(columnNames[columnIndex]));
					bp::dict outputColumnToTest = bp::extract<bp::dict>(outputDataSet.get(columnNames[columnIndex]));

					for (bp::dict column : { inputColumnToTest, outputColumnToTest })
					{
						CheckColumnEqualityFnMap::const_iterator it = sm_FnCheckColumnEqualityMap.find(dataType);

						if (it == sm_FnCheckColumnEqualityMap.end())
						{
							throw runtime_error("Unsupported column type encountered when testing column equality");
						}

						(this->*it->second)(
							rowsNumber,
							column,
							dataSet[columnIndex],
							strLen_or_Ind[columnIndex]);
					}
				}
			}
			catch (bp::error_already_set &)
			{
				string pyError = PythonTestUtilities::ParsePythonException();
				throw runtime_error("Error running python:\n" + pyError);
			}
		}
	}
}
