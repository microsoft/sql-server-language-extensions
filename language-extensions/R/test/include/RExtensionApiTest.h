//******************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server external language communication protocol.
// Copyright (C) 2019 Microsoft Corporation.

// This file is part of RExtension-test.

// RExtension-test is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// RExtension-test is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with RExtension-test.  If not, see <https://www.gnu.org/licenses/>.

// @File: RExtensionApiTest.h
//
// Purpose:
//  Specifies some typedefinitions to be used in the test.
//  And declares the base test fixture class RExtensionApiTest.
//
//******************************************************************************************************
#pragma once

typedef SQLRETURN FN_init(
	SQLCHAR *, // extensionParams
	SQLULEN,   // extensionParamsLength
	SQLCHAR *, // extensionPath
	SQLULEN,   // extensionPathLength
	SQLCHAR *, // publicLibraryPath
	SQLULEN,   // publicLibraryPathLength
	SQLCHAR *, // privateLibraryPath
	SQLULEN);  // privateLibraryPathLength

typedef SQLRETURN FN_initSession(
	SQLGUID,       // sessionId
	SQLUSMALLINT,  // taskId
	SQLUSMALLINT,  // numTasks
	SQLCHAR *,     // script
	SQLULEN,       // scriptLength
	SQLUSMALLINT,  // inputSchemaColumnsNumber
	SQLUSMALLINT,  // parametersNumber
	SQLCHAR*,      // inputDataName
	SQLUSMALLINT,  // inputDataNameLength
	SQLCHAR*,      // outputDataName
	SQLUSMALLINT); // outputDataNameLength

typedef SQLRETURN FN_initColumn(
	SQLGUID,      // sessionId
	SQLUSMALLINT, // taskId
	SQLUSMALLINT, // columnNumber
	SQLCHAR *,    // columnName
	SQLSMALLINT,  // columnNameLenth
	SQLSMALLINT,  // dataType
	SQLULEN,      // columnSize,
	SQLSMALLINT,  // decimalDigits
	SQLSMALLINT,  // nullable
	SQLSMALLINT,  // partitionByNumber
	SQLSMALLINT); // orderByNumber

typedef SQLRETURN FN_initParam(
	SQLGUID,      // sessionId
	SQLUSMALLINT, // taskId
	SQLUSMALLINT, // paramNumber
	SQLCHAR*,     // paramName,
	SQLSMALLINT,  // paramNameLength
	SQLSMALLINT,  // dataType
	SQLULEN,      // argSize
	SQLSMALLINT,  // decimalDigits
	SQLPOINTER,   // paramValue
	SQLINTEGER,   // strLen_or_Ind
	SQLSMALLINT); // inputOutputType

typedef SQLRETURN FN_execute(
	SQLGUID,        // sessionId
	SQLUSMALLINT,   // taskId
	SQLULEN,        // rowsNumber
	SQLPOINTER *,   // data
	SQLINTEGER **,  // strLenOrInd
	SQLUSMALLINT *);// outputSchemaColumnsNumber

typedef SQLRETURN FN_getResultColumn(
	SQLGUID,        // sessionId
	SQLUSMALLINT,   // taskId
	SQLUSMALLINT,   // columnNumber
	SQLSMALLINT *,  // dataType,
	SQLULEN *,      // columnSize,
	SQLSMALLINT *,  // decimalDigits,
	SQLSMALLINT *); // nullable

typedef SQLRETURN FN_getResults(
	SQLGUID,         // sessionId
	SQLUSMALLINT,    // taskId
	SQLULEN *,       // rowsNumber
	SQLPOINTER **,   // data
	SQLINTEGER ***); // strLen_or_Ind

typedef SQLRETURN FN_getOutputParam(
	SQLGUID,       // sessionId
	SQLUSMALLINT,  // taskId
	SQLUSMALLINT,  // paramNumber
	SQLPOINTER *,  // paramValue
	SQLINTEGER *); // strLen_or_Ind

typedef SQLRETURN FN_cleanupSession(
	SQLGUID,       // sessionId
	SQLUSMALLINT); // taskId

typedef SQLRETURN FN_cleanup();

namespace ExtensionApiTest
{

	// Forward declaration
	//
	template<class SQLType>
	class ColumnInfo;

	// All the tests in the RExtensionApiTest suite run one after the other
	//
	class RExtensionApiTest : public ::testing::Test
	{
	protected:

		// Per-test-suite set-up.
		// Called before the first test in this test suite.
		//
		static void SetUpTestSuite();

		// Per-test-suite tear-down.
		// Called after the last test in this test suite.
		//
		static void TearDownTestSuite();

		// Code here will be called immediately after the constructor (right
		// before each test).
		//
		void SetUp() override;

		// Code here will be called immediately after each test (right
		// before the destructor).
		//
		void TearDown() override;

		// Check R_HOME and set it if not defined.
		//
		static void CheckAndSetRHome();

		// Load library and get handles to different functions
		// Testing if RExtension is successfully loaded dynamically
		//
		static void GetHandles();

		// Do Init where embedded R is initialized - can be called only once in the validate suite.
		// Testing if Init is implemented correctly.
		//
		static void DoInit();

		// Set up default, valid variables for use in tests.
		//
		void SetupVariables();

		// Delete the memory allocated to default variables.
		//
		void CleanupVariables();

		// Close the handle to the library.
		//
		static void ReleaseHandles();

		// Call Cleanup on the RExtension.
		// Testing if Cleanup is implemented correctly.
		//
		static void DoCleanup();

		// Initialize a valid session.
		//
		void InitializeSession(
			SQLUSMALLINT inputSchemaColumnsNumber = 0,
			std::string  scriptString = "",
			SQLUSMALLINT parametersNumber = 0);

		// Initialize a column.
		//
		void InitializeColumn(
			SQLSMALLINT columnNumber,
			std::string columnNameString,
			SQLSMALLINT dataType,
			SQLULEN     columnSize);

		// Templatized function to call InitializeColumn for all columns in ColumnInfo.
		//
		template<class SQLType, SQLSMALLINT dataType>
		void InitializeColumns(ColumnInfo<SQLType> *ColumnInfo);

		// Cleanup a valid session.
		//
		void CleanupSession();

		// Templatized function to call InitParam for the given paramValue and dataType.
		// Testing if InitParam is implemented correctly for integer/numeric/logical dataTypes.
		//
		template<class SQLType, class RType, SQLSMALLINT DataType>
		void InitParam(
			std::vector<std::shared_ptr<SQLType>> expectedParamValues,
			std::vector<SQLINTEGER>               strLenOrInd,
			std::vector<SQLSMALLINT>              inputOutputTypes,
			bool                                  validate = true);

		// Testing if InitParam is implemented correctly for the char/varchar dataType.
		//
		void InitCharParam(
			std::vector<const char*> expectedParamValues,
			std::vector<SQLULEN>     paramSizes,
			std::vector<bool>        isFixedType,
			std::vector<SQLSMALLINT> inputOutputTypes,
			bool                     validate = true);

		// Testing if InitParam is implemented correctly for the binary/varbinary dataType.
		//
		void InitRawParam(
			std::vector<SQLCHAR*>    expectedParamValues,
			std::vector<SQLINTEGER>  strLenOrInd,
			std::vector<SQLULEN>     paramSizes,
			std::vector<SQLSMALLINT> inputOutputTypes,
			bool                     validate = true);

		// Fill a contiguous array columnData with members from the given columnVector
		//
		void GenerateContiguousData(
			char                     *charCol1Data,
			std::vector<const char*> charVector,
			SQLINTEGER               *strLenOrInd);

		// Get sum of lengths in strLenOrInd skipping SQL_NULL_DATA.
		//
		SQLINTEGER GetSumOfLengths(
			SQLINTEGER *strLenOrInd,
			SQLULEN    rowsNumber,
			SQLINTEGER *maxLen);

		// Templatized function to compare the given vector and data for equality.
		//
		template<class SQLType, class RType, SQLSMALLINT dataType>
		void CheckVectorEquality(
			SQLULEN     rowsNumber,
			RType       vectorToTest,
			void        *expectedVector,
			SQLINTEGER  *strLen_or_Ind);

		// Compare the given character vector and data for equality
		//
		void CheckCharacterVectorEquality(
			SQLULEN               expectedRowsNumber,
			Rcpp::CharacterVector vectorToTest,
			void                  *expectedData,
			SQLINTEGER            *strLen_or_Ind);

		// Templatized function to compare the given vectors for equality.
		//
		template<class RType>
		void CheckVectorEquality(
			RType vectorToTest,
			RType expectedVector);

		// Templatized function to compare the given dataframes for equality.
		//
		template<class RType>
		void CheckDataFrameEquality(
			Rcpp::DataFrame dataFrameToTest,
			Rcpp::DataFrame expectedDataFrame);

		// Templatized function to Test Execute with default script
		//
		template<class SQLType, class RType, SQLSMALLINT dataType>
		void Execute(
			SQLULEN                  rowsNumber,
			void                     **dataSet,
			SQLINTEGER               **strLen_or_Ind,
			std::vector<std::string> columnNames,
			bool                     validate = true);

		// Test Execute with default script for Character columns.
		//
		void ExecuteChar(
			SQLULEN                  rowsNumber,
			void                     **dataSet,
			SQLINTEGER               **strLen_or_Ind,
			std::vector<std::string> columnNames,
			bool                     validate = true);

		// Test GetResultColumn to verify the expected result column information.
		//
		void GetResultColumn(
			SQLUSMALLINT columnNumber,
			SQLSMALLINT  expectedDataType,
			SQLULEN      expectedColumnSize,
			SQLSMALLINT  expectedDecimalDigits,
			SQLSMALLINT  expectedNullable);

		// Test GetResults to verify the expected results are obtained.
		//
		template<class InputSQLType, class RType, class OutputSQLType, SQLSMALLINT outputDataType>
		void GetResults(
			SQLULEN                  expectedRowsNumber,
			SQLPOINTER               *expectedData,
			SQLINTEGER               **expectedStrLen_or_Ind,
			std::vector<std::string> columnNames);

		// Test GetResults to verify the expected results are obtained for character data.
		//
		void GetCharResults(
			SQLULEN                  expectedRowsNumber,
			SQLPOINTER               *expectedData,
			SQLINTEGER               **expectedStrLen_or_Ind,
			std::vector<std::string> columnNames);

		// Templatized function to compare the given column data
		// & nullMap with rowsNumber for equality.
		//
		template<class InputSQLType, class OutputSQLType, SQLSMALLINT outputDataType>
		void CheckColumnDataEquality(
			SQLULEN        rowsNumber,
			InputSQLType   *expectedColumnData,
			OutputSQLType  *columnData,
			SQLINTEGER     *expectedColumnStrLenOrInd,
			SQLINTEGER     *columnStrLenOrInd);

		// Compare the given character data & nullMap with rowsNumber for equality.
		//
		void CheckCharDataEquality(
			SQLULEN    rowsNumber,
			char*      expectedColumnData,
			char*      columnData,
			SQLINTEGER *expectedColumnStrLenOrInd,
			SQLINTEGER *columnStrLenOrInd);

		// Templatized function to test output param value and strLenOrInd is as expected.
		//
		template<class SQLType>
		void GetOutputParam(
			std::vector<std::shared_ptr<SQLType>> expectedParamValues,
			std::vector<SQLINTEGER>               expectedStrLenOrInd);

		// Test character output param value and strLenOrInd is as expected.
		//
		void GetCharOutputParam(
			std::vector<SQLCHAR*>   expectedParamValues,
			std::vector<SQLINTEGER> expectedStrLenOrInd);

		// Test raw(binary) output param value and strLenOrInd is as expected.
		//
		void GetRawOutputParam(
			std::vector<SQLCHAR*>   expectedParamValues,
			std::vector<SQLINTEGER> expectedStrLenOrInd);

		// Objects declared here can be used by all tests in the test suite.
		//
		SQLGUID *m_sessionId;
		SQLUSMALLINT m_taskId;
		SQLUSMALLINT m_numTasks;

		SQLUSMALLINT m_parametersNumber;

		SQLCHAR *m_columnName = nullptr;
		std::string m_columnNameString;

		SQLCHAR *m_script = nullptr;
		std::string m_scriptString;

		SQLUSMALLINT m_inputSchemaColumnsNumber;
		SQLCHAR *m_inputDataName;
		std::string m_inputDataNameString;

		SQLCHAR *m_outputDataName = nullptr;
		std::string m_outputDataNameString;

		const std::string m_printMessage = "Hello RExtension!";

		// A value of 2'147'483'647
		//
		const SQLINTEGER m_MaxInt = std::numeric_limits<SQLINTEGER>::max();

		// A value of -2'147'483'647
		// In R, std::numeric_limits<SQLINTEGER>::min() i.e. -2'147'483'648 represents NA
		// So, m_MinInt is 1 greater than the actual min.
		//
		const SQLINTEGER m_MinInt = std::numeric_limits<SQLINTEGER>::min() + 1;

		// A value of 9'223'372'036'854'775'807LL
		//
		const SQLBIGINT m_MaxBigInt = std::numeric_limits<SQLBIGINT>::max();

		// A value of -9'223'372'036'854'775'808LL gives compiler error;
		// Use numeric_limits to represent this value.
		// In R, -9'223'372'036'854'775'808LL = -9.223372e+18
		//
		const SQLBIGINT m_MinBigInt = std::numeric_limits<SQLBIGINT>::min();

		// A value of 32'767
		//
		const SQLSMALLINT m_MaxSmallInt = std::numeric_limits<SQLSMALLINT>::max();

		// A value of -32'768
		//
		const SQLSMALLINT m_MinSmallInt = std::numeric_limits<SQLSMALLINT>::min();

		// A value of 255
		//
		const SQLCHAR m_MaxTinyInt = std::numeric_limits<SQLCHAR>::max();

		// A value of 0
		//
		const SQLCHAR m_MinTinyInt = std::numeric_limits<SQLCHAR>::min();

		// For floating types, not using numeric_limits because they can't be
		// used for equality comparisons.
		//
		const SQLREAL m_MaxReal = 3.4e38F;
		const SQLREAL m_MinReal = -3.4e38F;
		const SQLDOUBLE m_MaxDouble = 1.79e308;
		const SQLDOUBLE m_MinDouble = -1.79e308;

		const SQLINTEGER m_IntSize = sizeof(SQLINTEGER);
		const SQLINTEGER m_LogicalSize = sizeof(SQLCHAR);
		const SQLINTEGER m_RealSize = sizeof(SQLREAL);
		const SQLINTEGER m_DoubleSize = sizeof(SQLDOUBLE);
		const SQLINTEGER m_BigIntSize = sizeof(SQLBIGINT);
		const SQLINTEGER m_SmallIntSize = sizeof(SQLSMALLINT);
		const SQLINTEGER m_TinyIntSize = sizeof(SQLCHAR);
		const SQLINTEGER m_BinarySize = sizeof(SQLCHAR);

		std::unique_ptr<ColumnInfo<SQLINTEGER>> m_integerInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLCHAR>> m_logicalInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLREAL>> m_realInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLDOUBLE>> m_doubleInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLBIGINT>> m_bigIntInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLSMALLINT>> m_smallIntInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLCHAR>> m_tinyIntInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLCHAR>> m_charInfo = nullptr;

		// R global environment
		//
		Rcpp::Environment m_globalEnvironment;

		// Pointer handle to the library libRextension
		//
		static void *m_libHandle;

		// Pointer to the Init function
		//
		static FN_init *m_initFuncPtr;

		// Pointer to the InitSession function
		//
		static FN_initSession *m_initSessionFuncPtr;

		// Pointer to the InitColumn function
		//
		static FN_initColumn *m_initColumnFuncPtr;

		// Pointer to the InitParam function
		//
		static FN_initParam *m_initParamFuncPtr;

		// Pointer to the Execute function
		//
		static FN_execute *m_executeFuncPtr;

		// Pointer to the GetResultColumn function
		//
		static FN_getResultColumn *m_getResultColumnFuncPtr;

		// Pointer to the GetResults function
		//
		static FN_getResults *m_getResultsFuncPtr;

		// Pointer to the GetOutputParam function
		//
		static FN_getOutputParam *m_getOutputParamFuncPtr;

		// Pointer to the CleanupSession function
		//
		static FN_cleanupSession *m_cleanupSessionFuncPtr;

		// Pointer to the Cleanup function
		//
		static FN_cleanup *m_cleanupFuncPtr;
	};

	// ColumnInfo template class to store information
	// about integer, basic numeric and logical columns.
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
			std::vector<SQLINTEGER> col2StrLenOrInd);

		SQLUSMALLINT GetColumnsNumber() const
		{
			return m_columnNames.size();
		}

		static const SQLULEN m_rowsNumber = 5;
		std::vector<std::string> m_columnNames;
		std::vector<SQLType> m_column1;
		std::vector<SQLType> m_column2;
		std::vector<void*> m_dataSet;
		std::vector<SQLINTEGER> m_col1StrLenOrInd;
		std::vector<SQLINTEGER> m_col2StrLenOrInd;
		std::vector<SQLINTEGER*> m_strLen_or_Ind;
	};
}
