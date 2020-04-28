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

using namespace std;
namespace py = boost::python;

namespace ExtensionApiTest
{
	// Name: ExecuteIntegerColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of Integer columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteIntegerColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(2);    // inputSchemaColumnsNumber

		string integerColumn1Name = "IntegerColumn1";
		InitializeColumn(0, integerColumn1Name, SQL_C_SLONG, sizeof(SQLINTEGER));

		string integerColumn2Name = "IntegerColumn2";
		InitializeColumn(1, integerColumn2Name, SQL_C_SLONG, sizeof(SQLINTEGER));

		vector<SQLINTEGER> intCol1Data{ 0, 1, 2, 3 };
		vector<SQLINTEGER> intCol2Data{ 2'147'483'647, -2'147'483'647, 10, -1 };
		vector<void*> dataSet = { intCol1Data.data(), intCol2Data.data() };

		SQLINTEGER strLenOrIndCol2[] = { 0, 0, SQL_NULL_DATA, 0 };
		vector<SQLINTEGER*> strLen_or_Ind{ nullptr, strLenOrIndCol2 };

		vector<string> columnNames{ integerColumn1Name, integerColumn2Name };

		TestExecute<SQLINTEGER, SQL_C_SLONG, int>(
			intCol1Data.size(),
			dataSet.data(),
			strLen_or_Ind.data(),
			columnNames,
			m_intNull);
	}

	// Name: ExecuteBooleanColumnsTest
	//
	// Description:
	// Test Execute using an InputDataSet of Boolean columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteBooleanColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(2);    // inputSchemaColumnsNumber

		string booleanColumn1Name = "BooleanColumn1";
		InitializeColumn(0, booleanColumn1Name, SQL_C_BIT, sizeof(SQLCHAR));

		string booleanColumn2Name = "BooleanColumn2";
		InitializeColumn(1, booleanColumn2Name, SQL_C_BIT, sizeof(SQLCHAR));

		vector<SQLCHAR> booleanCol1Data{ '1', '0', '1', '0', '1' };
		vector<SQLCHAR> booleanCol2Data{ '0', '2', '1', '0', '1' };
		vector<void*> dataSet = { booleanCol1Data.data(), booleanCol2Data.data() };

		SQLINTEGER strLenOrIndCol2[] = { SQL_NULL_DATA, 0, 0, 0, SQL_NULL_DATA };
		vector<SQLINTEGER*> strLen_or_Ind{ nullptr, strLenOrIndCol2 };

		vector<string> columnNames{ booleanColumn1Name, booleanColumn2Name };

		TestExecute<SQLCHAR, SQL_C_BIT, char>(
			booleanCol1Data.size(),
			dataSet.data(),
			strLen_or_Ind.data(),
			columnNames,
			m_boolNull);
	}

	// Name: ExecuteRealColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of Real columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteRealColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(2);    // inputSchemaColumnsNumber

		string realColumn1Name = "RealColumn1";
		InitializeColumn(0, realColumn1Name, SQL_C_FLOAT, sizeof(SQLREAL));

		string realColumn2Name = "RealColumn2";
		InitializeColumn(1, realColumn2Name, SQL_C_FLOAT, sizeof(SQLREAL));

		vector<SQLREAL> realCol1Data{ 0.34f, 1.33f, 83.98f, 72.45f, 68e10f };
		vector<SQLREAL> realCol2Data{ 3.4e38F, 0, -3.4e38F, -1, 0 };
		vector<void*> dataSet = { realCol1Data.data(), realCol2Data.data() };

		SQLINTEGER strLenOrIndCol2[] = { 0, SQL_NULL_DATA, 0, SQL_NULL_DATA, 0 };
		vector<SQLINTEGER*> strLen_or_Ind{ nullptr, strLenOrIndCol2 };

		vector<string> columnNames{ realColumn1Name, realColumn2Name };

		TestExecute<SQLREAL, SQL_C_FLOAT, double>(
			realCol1Data.size(),
			dataSet.data(),
			strLen_or_Ind.data(),
			columnNames,
			m_doubleNull);
	}

	// Name: ExecuteDoubleColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of Double columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteDoubleColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(2);    // inputSchemaColumnsNumber

		string doubleColumn1Name = "DoubleColumn1";
		InitializeColumn(0, doubleColumn1Name, SQL_C_DOUBLE, sizeof(SQLDOUBLE));

		string doubleColumn2Name = "DoubleColumn2";
		InitializeColumn(1, doubleColumn2Name, SQL_C_DOUBLE, sizeof(SQLDOUBLE));

		vector<SQLDOUBLE> doubleCol1Data{ -1.79e301, 1.33, 83.98, 72.45, 1.79e30 };
		vector<SQLDOUBLE> doubleCol2Data{ 0, 1.79e308, 0, -1.79e308, -1 };
		vector<void*> dataSet = { doubleCol1Data.data(), doubleCol2Data.data() };

		SQLINTEGER strLenOrIndCol2[] = { SQL_NULL_DATA, 0,
			SQL_NULL_DATA, 0, SQL_NULL_DATA };
		vector<SQLINTEGER*> strLen_or_Ind{ nullptr, strLenOrIndCol2 };

		vector<string> columnNames{ doubleColumn1Name, doubleColumn2Name };

		TestExecute<SQLDOUBLE, SQL_C_DOUBLE, double>(
			doubleCol1Data.size(),
			dataSet.data(),
			strLen_or_Ind.data(),
			columnNames,
			m_doubleNull);
	}

	// Name: ExecuteBigIntColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of BigInteger columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteBigIntColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(2);    // inputSchemaColumnsNumber

		string bigIntColumn1Name = "BigIntColumn1";
		InitializeColumn(0, bigIntColumn1Name, SQL_C_SBIGINT, sizeof(SQLBIGINT));

		string bigIntColumn2Name = "BigIntColumn2";
		InitializeColumn(1, bigIntColumn2Name, SQL_C_SBIGINT, sizeof(SQLBIGINT));

		vector<SQLBIGINT> bigIntCol1Data{ 9'223'372'036'854'775'807LL, 1,
			88883939, -9'223'372'036'854'775'807LL, -622280108 };
		vector<SQLBIGINT> bigIntCol2Data(5, 0);
		vector<void*> dataSet = { bigIntCol1Data.data(), bigIntCol2Data.data() };

		SQLINTEGER strLenOrIndCol2[] = { SQL_NULL_DATA, SQL_NULL_DATA,
			SQL_NULL_DATA, SQL_NULL_DATA, SQL_NULL_DATA };
		vector<SQLINTEGER*> strLen_or_Ind{ nullptr, strLenOrIndCol2 };

		vector<string> columnNames{ bigIntColumn1Name, bigIntColumn2Name };

		TestExecute<SQLBIGINT, SQL_C_SBIGINT, int>(
			bigIntCol1Data.size(),
			dataSet.data(),
			strLen_or_Ind.data(),
			columnNames,
			m_intNull);
	}

	// Name: ExecuteSmallIntColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of SmallInt columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteSmallIntColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(2);    // inputSchemaColumnsNumber

		string smallIntColumn1Name = "SmallIntColumn1";
		InitializeColumn(0, smallIntColumn1Name, SQL_C_SSHORT, sizeof(SQLSMALLINT));

		string smallIntColumn2Name = "SmallIntColumn2";
		InitializeColumn(1, smallIntColumn2Name, SQL_C_SSHORT, sizeof(SQLSMALLINT));

		vector<SQLSMALLINT> smallIntCol1Data{ 223, 33, 9811, -725, 6810 };
		vector<SQLSMALLINT> smallIntCol2Data{ -1, 0, 32'767, -32'768, 3'276 };
		vector<void*> dataSet = { smallIntCol1Data.data(), smallIntCol2Data.data() };

		SQLINTEGER strLenOrIndCol2[] = { 0 };
		vector<SQLINTEGER*> strLen_or_Ind{ nullptr, strLenOrIndCol2 };

		vector<string> columnNames{ smallIntColumn1Name, smallIntColumn2Name };

		TestExecute<SQLSMALLINT, SQL_C_SSHORT, int>(
			smallIntCol1Data.size(),
			dataSet.data(),
			strLen_or_Ind.data(),
			columnNames,
			m_intNull);
	}

	// Name: ExecuteTinyIntColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of TinyInt columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteTinyIntColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(2);    // inputSchemaColumnsNumber

		string tinyIntColumn1Name = "TinyIntColumn1";
		InitializeColumn(0, tinyIntColumn1Name, SQL_C_UTINYINT, sizeof(SQLCHAR));

		string tinyIntColumn2Name = "TinyIntColumn2";
		InitializeColumn(1, tinyIntColumn2Name, SQL_C_UTINYINT, sizeof(SQLCHAR));

		vector<SQLCHAR> tinyIntCol1Data{ 34, 133, 98, 72, 10 };
		vector<SQLCHAR> tinyIntCol2Data{ 255, 0, 1, 0, 128 };
		vector<void*> dataSet = { tinyIntCol1Data.data(), tinyIntCol2Data.data() };

		SQLINTEGER strLenOrIndCol2[] = { 0, SQL_NULL_DATA,
			SQL_NULL_DATA, SQL_NULL_DATA, 0 };
		vector<SQLINTEGER*> strLen_or_Ind{ nullptr, strLenOrIndCol2 };

		vector<string> columnNames{ tinyIntColumn1Name, tinyIntColumn2Name };

		TestExecute<SQLCHAR, SQL_C_UTINYINT, int>(
			tinyIntCol1Data.size(),
			dataSet.data(),
			strLen_or_Ind.data(),
			columnNames,
			m_intNull);
	}

	// Name: ExecuteStringColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of string columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteStringColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(3);

		string charColumn1Name = "CharColumn1";
		InitializeColumn(0, charColumn1Name, SQL_C_CHAR, sizeof(SQLCHAR));

		string charColumn2Name = "CharColumn2";
		InitializeColumn(1, charColumn2Name, SQL_C_CHAR, sizeof(SQLCHAR));

		string charColumn3Name = "CharColumn3";
		InitializeColumn(2, charColumn3Name, SQL_C_CHAR, sizeof(SQLCHAR));

		vector<const char*> charCol1{ "Hello", "test", "data", "World", "-123" };
		vector<const char*> charCol2{ "", 0, nullptr, "verify", "-1" };

		SQLINTEGER strLenOrIndCol1[] =
		{ static_cast<SQLINTEGER>(strlen(charCol1[0])),
		static_cast<SQLINTEGER>(strlen(charCol1[1])),
		static_cast<SQLINTEGER>(strlen(charCol1[2])),
		static_cast<SQLINTEGER>(strlen(charCol1[3])),
		static_cast<SQLINTEGER>(strlen(charCol1[4])) };

		SQLINTEGER strLenOrIndCol2[] =
		{ 0, SQL_NULL_DATA, SQL_NULL_DATA,
		static_cast<SQLINTEGER>(strlen(charCol2[3])),
		static_cast<SQLINTEGER>(strlen(charCol2[4])) };

		vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1, strLenOrIndCol2, nullptr };

		// Coalesce the arrays of each row of each column
		// into a contiguous array for each column.
		//
		int rowsNumber = charCol1.size();

		vector<char> charCol1Data = GenerateContiguousData<char>(charCol1, strLenOrIndCol1);
		vector<char> charCol2Data = GenerateContiguousData<char>(charCol2, strLenOrIndCol2);

		void* dataSet[] = { charCol1Data.data(),
							charCol2Data.data(),
							nullptr };

		vector<string> columnNames{ charColumn1Name, charColumn2Name, charColumn3Name };

		TestExecute<SQLCHAR, SQL_C_CHAR, char>(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames,
			m_charNull);
	}

	// Name: ExecuteRawColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of binary columns.
	//
	TEST_F(PythonExtensionApiTests, ExecuteRawColumnsTest)
	{
		// Initialize with a default Session that prints Hello World
		// and assigns InputDataSet to OutputDataSet
		//
		InitializeSession(3);

		SQLCHAR binaryValue1[] = { 0x01, 0x01, 0xe2, 0x40 };
		SQLCHAR binaryValue2[] = { 0x04, 0x05, 0xe1 };
		SQLCHAR binaryValue3[] = { 0x00, 0x00, 0x00, 0x01 };
		SQLCHAR binaryValue4[] = { 0xff };

		SQLCHAR binaryValue5[] = { 0x00 };
		SQLCHAR binaryValue6[] = { 0xff, 0xff, 0xff, 0xff };
		SQLCHAR binaryValue7[] = { 0x00, 0x12, 0xd2, 0xff, 0x00, 0x12, 0xd2, 0xff, 0x00, 0x12, 0xd2, 0xff };

		string binaryColumn1Name = "BinaryColumn1";
		InitializeColumn(0, binaryColumn1Name, SQL_C_BINARY, sizeof(SQLCHAR));

		string binaryColumn2Name = "BinaryColumn2";
		InitializeColumn(1, binaryColumn2Name, SQL_C_BINARY, sizeof(SQLCHAR));

		string binaryColumn3Name = "BinaryColumn3";
		InitializeColumn(2, binaryColumn3Name, SQL_C_BINARY, sizeof(SQLCHAR));

		vector<const SQLCHAR*> binaryCol1{ binaryValue1, binaryValue2, binaryValue3, binaryValue4 };
		vector<const SQLCHAR*> binaryCol2{ binaryValue5, binaryValue6, nullptr, binaryValue7};

		SQLINTEGER strLenOrIndCol1[] =
		{
			static_cast<SQLINTEGER>(sizeof(binaryValue1) / sizeof(binaryValue1[0])),
			static_cast<SQLINTEGER>(sizeof(binaryValue2) / sizeof(binaryValue2[0])),
			static_cast<SQLINTEGER>(sizeof(binaryValue3) / sizeof(binaryValue3[0])),
			static_cast<SQLINTEGER>(sizeof(binaryValue4) / sizeof(binaryValue4[0]))
		};

		SQLINTEGER strLenOrIndCol2[] =
		{
			SQL_NULL_DATA,
			static_cast<SQLINTEGER>(sizeof(binaryValue6) / sizeof(binaryValue6[0])),
			SQL_NULL_DATA,
			static_cast<SQLINTEGER>(sizeof(binaryValue7) / sizeof(binaryValue7[0]))
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

		TestExecute<SQLCHAR, SQL_C_BINARY, char>(
			rowsNumber,
			dataSet,
			strLen_or_Ind.data(),
			columnNames,
			m_charNull);
	}

	// Name: ExecuteDifferentColumnsTest
	//
	// Description:
	// Test Execute with default script using an InputDataSet of different column types.
	//
	TEST_F(PythonExtensionApiTests, ExecuteDifferentColumnsTest)
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

		vector<SQLINTEGER> intColData{ 2'147'483'647, -2'147'483'647, 0, 1320, -1 };
		vector<SQLDOUBLE> doubleColData{ -1.79e301, 1.33, 83.98, 72.45, 1.79e30 };
		vector<const char*> charCol{ "Hello", "test", "data", "World", "-123" };

		SQLINTEGER strLenOrIndCol1[] = { 0, 0, SQL_NULL_DATA, SQL_NULL_DATA, 0 };
		SQLINTEGER strLenOrIndCol3[] =
		{ static_cast<SQLINTEGER>(strlen(charCol[0])),
		static_cast<SQLINTEGER>(strlen(charCol[1])),
		static_cast<SQLINTEGER>(strlen(charCol[2])),
		static_cast<SQLINTEGER>(strlen(charCol[3])),
		static_cast<SQLINTEGER>(strlen(charCol[4])) };
		vector<SQLINTEGER*> strLen_or_Ind{ strLenOrIndCol1, nullptr, strLenOrIndCol3 };

		int rowsNumber = intColData.size();

		vector<char> charColData = GenerateContiguousData<char>(charCol, strLenOrIndCol3);

		vector<void *> dataSet{ intColData.data(), doubleColData.data(), charColData.data() };

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
			py::dict inputDataSet = py::extract<py::dict>(py::eval(createDictScript.c_str(), m_mainNamespace));

			createDictScript = m_outputDataNameString + ".to_dict()";
			py::dict outputDataSet = py::extract<py::dict>(py::eval(createDictScript.c_str(), m_mainNamespace));

			for(py::dict ds : {inputDataSet, outputDataSet})
			{
				py::dict intColumn = py::extract<py::dict>(ds.get(integerColumnName));
				CheckColumnEquality<SQLINTEGER, SQL_C_SLONG, int>(
					rowsNumber,
					intColumn,
					dataSet[0],
					strLen_or_Ind[0],
					m_intNull);

				py::dict numericColumn = py::extract<py::dict>(ds.get(doubleColumnName));
				CheckColumnEquality<SQLDOUBLE, SQL_C_DOUBLE, double>(
					rowsNumber,
					numericColumn,
					dataSet[1],
					strLen_or_Ind[1],
					m_doubleNull);

				py::dict charColumn = py::extract<py::dict>(ds.get(charColumnName));
				CheckStringColumnEquality(
					rowsNumber,
					charColumn,
					dataSet[2],
					strLen_or_Ind[2]);
			}
		}
		catch (py::error_already_set &)
		{
			string pyError = ParsePythonException();
			throw runtime_error("Error running python:\n" + pyError);
		}
	}

	// Name: GenerateContiguousData
	//
	// Description:
	// Fill a contiguous array columnData with members from the given columnVector
	// having lengths defined in strLenOrInd, unless it is SQL_NULL_DATA.
	//
	template<class SQLType>
	vector<SQLType> PythonExtensionApiTests::GenerateContiguousData(
		vector<const SQLType*> columnVector,
		SQLINTEGER             *strLenOrInd)
	{
		vector<SQLType> retVal;

		for (SQLULEN index = 0; index < columnVector.size(); index++)
		{
			if (strLenOrInd[index] != SQL_NULL_DATA)
			{
				vector<SQLType> data(columnVector[index], columnVector[index] + strLenOrInd[index]);
				retVal.insert(retVal.end(), data.begin(), data.end());
			}
		}

		return retVal;
	}

	// Name: InitializeColumn
	//
	// Description:
	// Call InitColumn for the given columnNumber, columnName, dataType and columnSize.
	//
	void PythonExtensionApiTests::InitializeColumn(
		SQLSMALLINT columnNumber,
		string      columnNameString,
		SQLSMALLINT dataType,
		SQLULEN     columnSize)
	{
		SQLCHAR *columnName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(columnNameString.c_str()))
			);

		SQLRETURN result = SQL_ERROR;

		result = InitColumn(
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
	// Template function to Test Execute with default script that assigns Input to Output.
	// It tests the correctness of the:
	//  1. Executed script,
	//  2. InputDataSet and
	//  3. OutputDataSet
	//
	template<class SQLType, SQLSMALLINT dataType, class NullType>
	void PythonExtensionApiTests::TestExecute(
		SQLULEN        rowsNumber,
		void           **dataSet,
		SQLINTEGER     **strLen_or_Ind,
		vector<string> columnNames,
		const NullType valueForNull)
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

		// Test print message was printed correctly
		//
		string output = testing::internal::GetCapturedStdout();
		cout << output;
		ASSERT_TRUE(output.find(m_printMessage) != string::npos);

		try{
			string createDictScript = m_inputDataNameString + ".to_dict()";
			py::dict inputDataSet = py::extract<py::dict>(py::eval(createDictScript.c_str(), m_mainNamespace));

			createDictScript = m_outputDataNameString + ".to_dict()";
			py::dict outputDataSet = py::extract<py::dict>(py::eval(createDictScript.c_str(), m_mainNamespace));

			for (SQLUSMALLINT columnIndex = 0; columnIndex < columnNames.size(); columnIndex++)
			{
				py::dict inputColumnToTest = py::extract<py::dict>(inputDataSet.get(columnNames[columnIndex]));
				py::dict outputColumnToTest = py::extract<py::dict>(outputDataSet.get(columnNames[columnIndex]));

				for (py::dict column : { inputColumnToTest, outputColumnToTest })
				{
					switch (dataType)
					{
					case SQL_C_BIT:
						CheckBooleanColumnEquality(
							rowsNumber,
							column,
							dataSet[columnIndex],
							strLen_or_Ind[columnIndex]);
						break;

					case SQL_C_CHAR:
						CheckStringColumnEquality(
							rowsNumber,
							column,
							dataSet[columnIndex],
							strLen_or_Ind[columnIndex]);
						break;

					case SQL_C_BINARY:
						CheckRawColumnEquality(
							rowsNumber,
							column,
							dataSet[columnIndex],
							strLen_or_Ind[columnIndex]);
						break;

					default:
						CheckColumnEquality<SQLType, dataType, NullType>(
							rowsNumber,
							column,
							dataSet[columnIndex],
							strLen_or_Ind[columnIndex],
							valueForNull);
						break;
					}
				}
			}
		}
		catch (py::error_already_set &)
		{
			string pyError = ParsePythonException();
			throw runtime_error("Error running python:\n" + pyError);
		}
	}

	// Name: CheckColumnEquality
	//
	// Description:
	// Template function to compare the given columns for equality
	//
	template<class SQLType, SQLSMALLINT dataType, class NullType>
	void PythonExtensionApiTests::CheckColumnEquality(
		SQLULEN        expectedRowsNumber,
		py::dict       columnToTest,
		void           *expectedColumn,
		SQLINTEGER     *strLen_or_Ind,
		const NullType valueForNull)
	{
		ASSERT_EQ(static_cast<SQLULEN>(py::len(columnToTest)), expectedRowsNumber);

		for (SQLULEN index = 0; index < expectedRowsNumber; index++)
		{
			py::object val = columnToTest[index];
			SQLType typeVal = py::extract<SQLType>(val);
			SQLType expectedValue = static_cast<SQLType*>(expectedColumn)[index];

			if (strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA)
			{
				// Check for NAN by comparing to itself, will always be false if NAN
				//
				if(valueForNull != valueForNull)
				{
					EXPECT_NE(typeVal, typeVal);
				}
				else
				{
					EXPECT_EQ(typeVal, valueForNull);
				}
			}
			else
			{
				EXPECT_EQ(typeVal, expectedValue);
			}
		}
	}

	// Name: CheckBooleanColumnEquality
	//
	// Description:
	//  Check boolean columns for equality
	//
	void PythonExtensionApiTests::CheckBooleanColumnEquality(
		SQLULEN    expectedRowsNumber,
		py::dict   columnToTest,
		void       *expectedColumn,
		SQLINTEGER *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<SQLULEN>(py::len(columnToTest)), expectedRowsNumber);

		for (SQLULEN index = 0; index < expectedRowsNumber; index++)
		{
			py::object val = columnToTest[index];
			bool expectedValue = static_cast<bool*>(expectedColumn)[index];

			bool typeVal = py::extract<bool>(val);

			if (strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA)
			{
				EXPECT_EQ(typeVal, false);
			}
			else
			{
				EXPECT_EQ(typeVal, expectedValue);
			}
		}
	}

	// Name: CheckStringColumnEquality
	//
	// Description:
	// Compare string column with the given data and corresponding strLen_or_Ind.
	// The expectedData is input as a void*, hence we input the expectedRowsNumber as well.
	// Where strLen_or_Ind == SQL_NULL_DATA, check for is_na.
	//
	void PythonExtensionApiTests::CheckStringColumnEquality(
		SQLULEN    expectedRowsNumber,
		py::dict   columnToTest,
		void       *expectedColumn,
		SQLINTEGER *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<SQLULEN>(py::len(columnToTest)), expectedRowsNumber);

		SQLINTEGER cumulativeLength = 0;

		for (SQLULEN index = 0; index < expectedRowsNumber; index++)
		{
			py::object val = columnToTest[index];
			if (strLen_or_Ind == nullptr ||
				(strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA))
			{
				EXPECT_TRUE(val.is_none());
			}
			else
			{
				string typeVal = py::extract<string>(val);
				string expectedString = string(
					static_cast<char*>(expectedColumn) + cumulativeLength,
					strLen_or_Ind[index]);

				EXPECT_EQ(typeVal, expectedString);
				cumulativeLength += strLen_or_Ind[index];
			}
		}
	}

	// Name: CheckRawColumnEquality
	//
	// Description:
	// Compare raw column with the given data and corresponding strLen_or_Ind.
	// The expectedData is input as a void*, hence we input the expectedRowsNumber as well.
	// Where strLen_or_Ind == SQL_NULL_DATA, check for is_na.
	//
	void PythonExtensionApiTests::CheckRawColumnEquality(
		SQLULEN    expectedRowsNumber,
		py::dict   columnToTest,
		void       *expectedColumn,
		SQLINTEGER *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<SQLULEN>(py::len(columnToTest)), expectedRowsNumber);

		SQLINTEGER cumulativeLength = 0;

		for (SQLULEN index = 0; index < expectedRowsNumber; index++)
		{
			py::object val = columnToTest[index];

			if (strLen_or_Ind == nullptr ||
				(strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA))
			{
				EXPECT_TRUE(val.is_none());
			}
			else
			{
				// Create iterator on the binary value
				//
				py::stl_input_iterator<SQLCHAR> begin(val);

				SQLCHAR *expectedValue = static_cast<SQLCHAR*>(expectedColumn) + cumulativeLength;

				// Always compare using strLenOrInd because
				// we copy only those many bytes into the raw column value
				//
				for (SQLINTEGER i = 0; i < strLen_or_Ind[index]; ++i)
				{
					EXPECT_EQ(val[i], expectedValue[i]);
				}

				cumulativeLength += strLen_or_Ind[index];
			}
		}
	}
}
