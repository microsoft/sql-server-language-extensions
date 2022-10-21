//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonExtensionApiTests.h
//
// Purpose:
//  Define common methods and variables needed to test the Extension API
//
//*************************************************************************************************
#pragma once
#include "Common.h"
#include "PythonTestUtilities.h"
#include <unordered_map>

namespace ExtensionApiTest
{
	// Forward declaration
	//
	template<class SQLType>
	class ColumnInfo;

	// All the tests in the PythonextensionApiTest suite run one after the other
	//
	class PythonExtensionApiTests : public ::testing::Test
	{
	protected:

		// Code here will be called immediately after the constructor (right
		// before each test).
		//
		void SetUp() override;

		// Code here will be called immediately after each test (right
		// before the destructor).
		//
		void TearDown() override;

		// Initialize a valid session.
		//
		void InitializeSession(
			SQLUSMALLINT parametersNumber = 0,
			SQLUSMALLINT inputSchemaColumnsNumber = 0,
			std::string  scriptString = "");

		// Initialize a column
		//
		void InitializeColumn(
			SQLSMALLINT columnNumber,
			std::string columnNameString,
			SQLSMALLINT dataType,
			SQLULEN     columnSize,
			SQLSMALLINT partitionByNumber = -1);

		// Get max length of all strings from strLenOrInd.
		//
		SQLINTEGER GetMaxLength(SQLINTEGER *strLenOrInd, SQLULEN rowsNumber);

		// Get length of a wstring
		//
		SQLINTEGER GetWStringLength(const wchar_t *str);

		template<class SQLType, SQLSMALLINT dataType>
		void InitializeColumns(ColumnInfo<SQLType> *ColumnInfo);

		// Set up default, valid variables for use in tests
		//
		void SetupVariables();

		// Call Cleanup on the PythonExtension.
		// Testing if Cleanup is implemented correctly.
		//
		void DoCleanup();

		// Template to test all input parameter data types
		//
		template<class SQLType, SQLSMALLINT dataType>
		void TestParameter(
			int         paramNumber,
			SQLType     paramValue,
			bool        isNull = false,
			SQLSMALLINT inputOutputType = SQL_PARAM_INPUT_OUTPUT,
			bool        validate = true);

		// Template to test date/datetime data types
		//
		template<class DateTimeStruct, SQLSMALLINT dataType>
		void TestDateTimeParameter(
			int            paramNumber,
			DateTimeStruct paramValue,
			bool           isNull = false,
			SQLSMALLINT    inputOutputType = SQL_PARAM_INPUT_OUTPUT,
			bool           validate = true);

		// Test a string parameter
		//
		void TestStringParameter(
			int         paramNumber,
			const char  *paramValue,
			SQLULEN     paramSize,
			bool        isFixedType,
			SQLSMALLINT inputOutputType = SQL_PARAM_INPUT_OUTPUT,
			bool        validate = true,
			bool        expectSuccess = true);

		// Test a wstring parameter
		//
		void TestWStringParameter(
			int           paramNumber,
			const wchar_t *paramValue,
			SQLINTEGER    paramSize,
			bool          isFixedType,
			SQLSMALLINT   inputOutputType = SQL_PARAM_INPUT_OUTPUT,
			bool          validate = true,
			bool          expectSuccess = true);

		// Test a binary parameter
		//
		void TestRawParameter(
			int           paramNumber,
			const SQLCHAR *paramValue,
			SQLULEN       paramSize,
			bool          isFixedType,
			SQLSMALLINT   inputOutputType = SQL_PARAM_INPUT_OUTPUT,
			bool          validate = true);

		// Fill a contiguous array columnData with members from the given columnVector
		// having lengths defined in strLenOrInd, unless it is SQL_NULL_DATA.
		//
		template<class SQLType>
		std::vector<SQLType> GenerateContiguousData(
			std::vector<const SQLType*> columnVector,
			SQLINTEGER                  *strLenOrInd);

		// Template function to Test Execute with default script
		// The "validate" parameter can be false to run the execution
		// without validating DataSets to set up outputs.
		//
		template<class SQLType, SQLSMALLINT dataType>
		void TestExecute(
			SQLULEN                  rowsNumber,
			void                     **dataSet,
			SQLINTEGER               **strLen_or_Ind,
			std::vector<std::string> columnNames,
			bool                     validate = true);

		// Template function to compare the given integer/float column for equality.
		//
		template<class SQLType>
		void CheckColumnEquality(
			SQLULEN             expectedRowsNumber,
			boost::python::dict columnToTest,
			void                *expectedColumn,
			SQLINTEGER          *strLen_or_Ind);

		// Compare a given boolean column with another for equality
		//
		void CheckBooleanColumnEquality(
			SQLULEN             expectedRowsNumber,
			boost::python::dict columnToTest,
			void                *expectedColumn,
			SQLINTEGER          *strLen_or_Ind);

		// Compare a given string column with another for equality
		//
		void CheckStringColumnEquality(
			SQLULEN             expectedRowsNumber,
			boost::python::dict columnToTest,
			void                *expectedColumn,
			SQLINTEGER          *strLen_or_Ind);

		// Compare a given wstring column with another for equality
		//
		void CheckWStringColumnEquality(
			SQLULEN             expectedRowsNumber,
			boost::python::dict columnToTest,
			void                *expectedColumn,
			SQLINTEGER          *strLen_or_Ind);

		// Compare a given binary column with another for equality
		//
		void CheckRawColumnEquality(
			SQLULEN             expectedRowsNumber,
			boost::python::dict columnToTest,
			void                *expectedColumn,
			SQLINTEGER          *strLen_or_Ind);

		// Compare a given datetime column with another for equality
		//
		template<class DateTimeStruct>
		void CheckDateTimeColumnEquality(
			SQLULEN             expectedRowsNumber,
			boost::python::dict columnToTest,
			void                *expectedColumn,
			SQLINTEGER          *strLen_or_Ind);

		// Test GetResultColumn to verify the expected result column information.
		//
		void TestGetResultColumn(
			SQLUSMALLINT columnNumber,
			SQLSMALLINT  expectedDataType,
			SQLULEN      expectedColumnSize,
			SQLSMALLINT  expectedDecimalDigits,
			SQLSMALLINT  expectedNullable);

		// Test GetResults to verify the expected results are obtained.
		// For numeric and integer types.
		//
		template<class SQLType, class InputCType, SQLSMALLINT dataType>
		void TestGetResults(
			SQLULEN                  expectedRowsNumber,
			SQLPOINTER               *expectedData,
			SQLINTEGER               **expectedStrLen_or_Ind,
			std::vector<std::string> columnNames);

		// Test GetResults to verify the expected string results are obtained.
		//
		void TestGetStringResults(
			SQLULEN                  expectedRowsNumber,
			SQLPOINTER               *expectedData,
			SQLINTEGER               **expectedStrLen_or_Ind,
			std::vector<std::string> columnNames);

		// Test GetResults to verify the expected raw results are obtained.
		//
		void TestGetRawResults(
			SQLULEN                  expectedRowsNumber,
			SQLPOINTER               *expectedData,
			SQLINTEGER               **expectedStrLen_or_Ind,
			std::vector<std::string> columnNames);

		// Test GetResults to verify the expected datetime results are obtained.
		//
		template<class DateTimeStruct>
		void TestGetDateTimeResults(
			SQLULEN                  expectedRowsNumber,
			SQLPOINTER               *expectedData,
			SQLINTEGER               **expectedStrLen_or_Ind,
			std::vector<std::string> columnNames);

		// Template function to compare the given column data and nullMap for equality.
		// For numeric, boolean and integer types.
		//
		template<class SQLType>
		void CheckColumnDataEquality(
			SQLULEN    rowsNumber,
			SQLType    *expectedColumnData,
			SQLType    *columnData,
			SQLINTEGER *expectedColumnStrLenOrInd,
			SQLINTEGER *columnStrLenOrInd);

		// Template function to compare the given column data and nullMap for equality.
		// For numeric, boolean and integer types, WITH NULLS.
		// Because NULLS for float types are NAN, there are complications that need to be checked for.
		//
		template<class SQLType, class DefaultType>
		void CheckColumnDataEqualityForNullable(
			SQLULEN    rowsNumber,
			SQLType    *expectedColumnData,
			void       *pColumnData,
			SQLINTEGER *expectedColumnStrLenOrInd,
			SQLINTEGER *columnStrLenOrInd);

		// Compare the given string column data and nullMap for equality.
		//
		void CheckStringDataEquality(
			SQLULEN    rowsNumber,
			char       *expectedColumnData,
			char       *columnData,
			SQLINTEGER *expectedColumnStrLenOrInd,
			SQLINTEGER *columnStrLenOrInd);

		// Compare the given raw column data and nullMap for equality.
		//
		void CheckRawDataEquality(
			SQLULEN    rowsNumber,
			SQLCHAR    *expectedColumnData,
			SQLCHAR    *columnData,
			SQLINTEGER *expectedColumnStrLenOrInd,
			SQLINTEGER *columnStrLenOrInd);

		// Convert a datetime string to a Date/Timestamp struct.
		// Datetime strings should be YYYY-MM-DD hh:mm:ss.fraction.
		//
		template<class DateTimeStruct>
		void StringToDateTimeColumn(
			SQLULEN                     rowsNumber,
			char                        *columnData,
			SQLINTEGER                  *columnStrLenOrInd,
			std::vector<DateTimeStruct> *results
		);

		// Compare the given datetime column data and nullMap for equality.
		//
		template<class DateTimeStruct>
		void CheckDateTimeDataEquality(
			SQLULEN        rowsNumber,
			DateTimeStruct *expectedColumnData,
			DateTimeStruct *columnData,
			SQLINTEGER     *expectedColumnStrLenOrInd,
			SQLINTEGER     *columnStrLenOrInd);

		// Helper function to create a string with random characters of the provided size.
		//
		std::string CreateInputSizeRandomStr(int size);

		// Template function to test output param value and strLenOrInd is as expected.
		//
		template<class SQLType>
		void TestGetOutputParam(
			std::vector<SQLType*>   expectedParamValueVector,
			std::vector<SQLINTEGER> expectedStrLenOrIndVector);

		// Template function to test date/datetime output param value and strLenOrInd is as expected.
		//
		template<class DateTimeStruct>
		void TestGetDateTimeOutputParam(
			std::vector<DateTimeStruct *> expectedParamValueVector,
			std::vector<SQLINTEGER>       expectedStrLenOrIndVector);

		// Test output string param value and strLenOrInd is as expected.
		//
		void TestGetStringOutputParam(
			std::vector<const char*> expectedParamValueVector,
			std::vector<SQLINTEGER>  expectedStrLenOrIndVector);

		// Test output string param value and strLenOrInd is as expected.
		//
		void TestGetWStringOutputParam(
			std::vector<const wchar_t*> expectedParamValueVector,
			std::vector<SQLINTEGER>     expectedStrLenOrIndVector);

		// Test output raw param value and strLenOrInd is as expected.
		//
		void TestGetRawOutputParam(
			std::vector<SQLCHAR*>   expectedParamValueVector,
			std::vector<SQLINTEGER> expectedStrLenOrIndVector);

		// Objects declared here can be used by all tests in the test suite.
		//
		SQLGUID *m_sessionId;
		SQLUSMALLINT m_taskId;
		SQLUSMALLINT m_numTasks;

		SQLCHAR *m_script;
		std::string m_scriptString;
		SQLSMALLINT m_scriptLength;

		SQLCHAR *m_inputDataName;
		std::string m_inputDataNameString;
		SQLSMALLINT m_inputDataNameLength;

		SQLCHAR *m_outputDataName;
		std::string m_outputDataNameString;
		SQLSMALLINT m_outputDataNameLength;

		const std::string m_printMessage = "Hello PythonExtension!";
		const std::string m_streamingParamName = "@r_rowsPerRead";

		// A value of 2'147'483'648
		//
		const SQLINTEGER m_MaxInt = (std::numeric_limits<SQLINTEGER>::max)();

		// A value of -2'147'483'648
		//
		const SQLINTEGER m_MinInt = (std::numeric_limits<SQLINTEGER>::min)();

		// A value of 9'223'372'036'854'775'807LL
		//
		const SQLBIGINT m_MaxBigInt = (std::numeric_limits<SQLBIGINT>::max)();

		// A value of -9'223'372'036'854'775'808LL
		//
		const SQLBIGINT m_MinBigInt = (std::numeric_limits<SQLBIGINT>::min)();

		// A value of 32'767
		//
		const SQLSMALLINT m_MaxSmallInt = (std::numeric_limits<SQLSMALLINT>::max)();

		// A value of -32'768
		//
		const SQLSMALLINT m_MinSmallInt = (std::numeric_limits<SQLSMALLINT>::min)();

		// A value of 255
		//
		const SQLCHAR m_MaxTinyInt = (std::numeric_limits<SQLCHAR>::max)();

		// A value of 0
		//
		const SQLCHAR m_MinTinyInt = (std::numeric_limits<SQLCHAR>::min)();

		// For floating types, not using numeric_limits because they can't be
		// used for equality comparisons.
		//
		const SQLREAL m_MaxReal = 3.4e38F;
		const SQLREAL m_MinReal = -3.4e38F;
		const SQLDOUBLE m_MaxDouble = 1.79e308;
		const SQLDOUBLE m_MinDouble = -1.79e308;

		const SQLINTEGER m_IntSize = sizeof(SQLINTEGER);
		const SQLINTEGER m_BooleanSize = sizeof(SQLCHAR);
		const SQLINTEGER m_RealSize = sizeof(SQLREAL);
		const SQLINTEGER m_DoubleSize = sizeof(SQLDOUBLE);
		const SQLINTEGER m_BigIntSize = sizeof(SQLBIGINT);
		const SQLINTEGER m_SmallIntSize = sizeof(SQLSMALLINT);
		const SQLINTEGER m_TinyIntSize = sizeof(SQLCHAR);
		const SQLINTEGER m_CharSize = sizeof(SQLCHAR);
		const SQLINTEGER m_WCharSize = sizeof(wchar_t);
		const SQLINTEGER m_BinarySize = sizeof(SQLCHAR);
		const SQLINTEGER m_DateTimeSize = sizeof(SQL_TIMESTAMP_STRUCT);
		const SQLINTEGER m_DateSize = sizeof(SQL_DATE_STRUCT);

		std::unique_ptr<ColumnInfo<SQLINTEGER>> m_integerInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLCHAR>> m_booleanInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLREAL>> m_realInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLDOUBLE>> m_doubleInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLBIGINT>> m_bigIntInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLSMALLINT>> m_smallIntInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLCHAR>> m_tinyIntInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLCHAR>> m_charInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQL_TIMESTAMP_STRUCT>> m_dateTimeInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQL_DATE_STRUCT>> m_dateInfo = nullptr;

		// Used for partitioning test
		//
		std::unique_ptr<ColumnInfo<SQLINTEGER>> m_partition_integerInfo = nullptr;

		const float m_floatNull = NAN;
		const int m_intNull = 0;
		const bool m_boolNull = false;

		// The boost python module; python will run in this object
		//
		boost::python::object m_mainModule;

		// The boost python namespace; dictionary containing all python variables
		//
		boost::python::object m_mainNamespace;

		// Check column equality function pointer definition
		//
		using fnCheckColumnEquality = void (PythonExtensionApiTests::*)(
			SQLULEN             expectedRowsNumber,
			boost::python::dict columnToTest,
			void                *expectedColumn,
			SQLINTEGER          *strLen_or_Ind);

		// Function map to add columns to the data frame and its typedef
		//
		static const std::unordered_map<SQLSMALLINT, fnCheckColumnEquality> sm_FnCheckColumnEqualityMap;
		typedef std::unordered_map<SQLSMALLINT, fnCheckColumnEquality> CheckColumnEqualityFnMap;
	};

	// ColumnInfo template class to store information
	// about integer, basic numeric, logical and date(time) columns.
	// This assumes two columns and five rows.
	//
	template<class SQLType>
	class ColumnInfo
	{
	public:
		ColumnInfo(
			std::string column1Name, std::vector<SQLType> column1,
			std::vector<SQLINTEGER> col1StrLenOrInd,
			std::string column2Name, std::vector<SQLType> column2,
			std::vector<SQLINTEGER> col2StrLenOrInd,
			std::vector<SQLSMALLINT> partitionByIndexes = { -1, -1 });

		SQLUSMALLINT GetColumnsNumber() const
		{
			return m_columnNames.size();
		}

		static const SQLULEN sm_rowsNumber = 5;
		std::vector<std::string> m_columnNames;
		std::vector<SQLType> m_column1;
		std::vector<SQLType> m_column2;
		std::vector<void *> m_dataSet;
		std::vector<SQLINTEGER> m_col1StrLenOrInd;
		std::vector<SQLINTEGER> m_col2StrLenOrInd;
		std::vector<SQLINTEGER*> m_strLen_or_Ind;
		std::vector<SQLSMALLINT> m_partitionByIndexes;
	};
}
