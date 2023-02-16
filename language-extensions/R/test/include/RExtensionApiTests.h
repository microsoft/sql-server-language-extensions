//**************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server
// external language communication protocol.
// Copyright (C) 2020 Microsoft Corporation.

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

// @File: RExtensionApiTests.h
//
// Purpose:
//  Specifies some typedefinitions to be used in the test.
//  And declares the base test fixture class RExtensionApiTests.
//
//**************************************************************************************************

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

	// All the tests in the RExtensionApiTests suite run one after the other
	//
	class RExtensionApiTests : public ::testing::Test
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
		virtual void SetUp() override;

		// Code here will be called immediately after each test (right
		// before the destructor).
		//
		virtual void TearDown() override;

		// Check R_HOME and set it if not defined.
		//
		static void CheckAndSetRHome();

		// Set the library path variables.
		//
		static void SetupLibPaths();

		// Create the library paths.
		//
		static void CreateLibPaths();

		// Do Init where embedded R is initialized - can be called only once in the validate suite.
		// Testing if Init is implemented correctly.
		//
		static void DoInit();

		// Call Cleanup on the RExtension.
		// Testing if Cleanup is implemented correctly.
		//
		static void DoCleanup();

		// Load library and get handles to different functions
		// Testing if RExtension is successfully loaded dynamically
		//
		static void GetHandles();

		// Close the handle to the library.
		//
		static void ReleaseHandles();

		// Cleanup all the library paths created.
		//
		static void CleanupLibPaths();

		// Set up default, valid variables for use in tests.
		//
		virtual void SetupVariables();

		// Delete the memory allocated to default variables.
		//
		void CleanupVariables();

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
			SQLULEN     columnSize,
			SQLSMALLINT decimalDigits,
			SQLSMALLINT nullable,
			SQLSMALLINT partitionByNumber = -1);

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
		template<class SQLType, class RVectorType, SQLSMALLINT DataType>
		void InitParam(
			std::vector<std::shared_ptr<SQLType>> expectedParamValues,
			std::vector<SQLINTEGER>               strLenOrInd,
			std::vector<SQLSMALLINT>              inputOutputTypes,
			bool                                  validate = true);

		// Testing if InitParam is implemented correctly for the (n)char/(n)varchar dataType.
		//
		template<class CharType, SQLSMALLINT DataType>
		void InitCharParam(
			std::vector<const CharType*> expectedParamValues,
			std::vector<SQLULEN>         paramSizes,
			std::vector<bool>            isFixedType,
			std::vector<SQLSMALLINT>     inputOutputTypes,
			bool                         validate = true,
			bool                         expectSuccess = true);

		// Testing if InitParam is implemented correctly for the binary/varbinary dataType.
		//
		void InitRawParam(
			std::vector<SQLCHAR*>    expectedParamValues,
			std::vector<SQLINTEGER>  strLenOrInd,
			std::vector<SQLULEN>     paramSizes,
			std::vector<SQLSMALLINT> inputOutputTypes,
			bool                     validate = true);

		// Testing if InitParam is implemented correctly for the date/datetime dataTypes.
		//
		template<class SQLType, class RVectorType, class DateTimeTypeInR, SQLSMALLINT DataType>
		void InitDateTimeParam(
			std::vector<SQLType>     expectedParamValues,
			std::vector<SQLINTEGER>  strLenOrInd,
			std::vector<SQLSMALLINT> inputOutputTypes,
			bool                     validate = true);

		// Testing if InitParam is implemented correctly for the decimal/numeric dataTypes.
		//
		void InitNumericParam(
			std::vector<SQL_NUMERIC_STRUCT> initParamValues,
			std::vector<SQLINTEGER>         strLenOrInd,
			std::vector<SQLSMALLINT>        inputOutputTypes,
			std::vector<SQLULEN>            precisionAsParamSize,
			std::vector<SQLSMALLINT>        decimalDigits,
			std::vector<double>             expectedParamValues = std::vector<double>(0));

		// Fill a contiguous array columnData with members from the given columnVector
		// having lengths defined in strLenOrInd, unless it is SQL_NULL_DATA.
		//
		template<class SQLType>
		std::vector<SQLType> GenerateContiguousData(
			std::vector<const SQLType*> columnVector,
			SQLINTEGER                  *strLenOrInd);

		// Templatized function to compare the given vector and data for equality.
		//
		template<class SQLType, class RVectorType, SQLSMALLINT DataType>
		void CheckRVectorColumnDataEquality(
			SQLULEN     rowsNumber,
			RVectorType vectorToTest,
			void        *expectedVector,
			SQLINTEGER  *strLen_or_Ind,
			SQLSMALLINT nullable = SQL_NULLABLE);

		// Compare the given R character vector and data for equality
		//
		template<class CharType>
		void CheckRCharacterVectorColumnDataEquality(
			SQLULEN               expectedRowsNumber,
			Rcpp::CharacterVector vectorToTest,
			void                  *expectedData,
			SQLINTEGER            *strLen_or_Ind);

		// Compare the given R Date(time) vector and data for equality
		//
		template<class SQLType, class RVectorType, class DateTimeTypeInR>
		void CheckRDateTimeVectorColumnDataEquality(
			SQLULEN     expectedRowsNumber,
			RVectorType vectorToTest,
			void        *expectedData,
			SQLINTEGER  *strLen_or_Ind);

		// Templatized function to compare the given vectors for equality.
		//
		template<class RVectorType>
		void CheckRVectorEquality(
			RVectorType vectorToTest,
			RVectorType expectedVector);

		// Templatized function to compare the given dataframes for equality.
		//
		template<class RVectorType>
		void CheckDataFrameEquality(
			Rcpp::DataFrame dataFrameToTest,
			Rcpp::DataFrame expectedDataFrame);

		// Templatized function to Test Execute with default script
		//
		template<class SQLType, class RVectorType, SQLSMALLINT dataType>
		void Execute(
			SQLULEN                  rowsNumber,
			void                     **dataSet,
			SQLINTEGER               **strLen_or_Ind,
			std::vector<std::string> columnNames,
			bool                     validate = true);

		// Test Execute with default script for Character columns.
		//
		template<class CharType>
		void ExecuteChar(
			SQLULEN                  rowsNumber,
			void                     **dataSet,
			SQLINTEGER               **strLen_or_Ind,
			std::vector<std::string> columnNames,
			bool                     validate = true);

		// Test Execute with default script for Date/Datetime columns.
		//
		template<class SQLType, class RVectorType, class DateTimeTypeInR>
		void ExecuteDateTime(
			SQLULEN                  rowsNumber,
			void                     **dataSet,
			SQLINTEGER               **strLen_or_Ind,
			std::vector<std::string> columnNames,
			bool                     validate = true);

		// Test Execute with default script for numeric/decimal input columns.
		//
		void ExecuteNumeric(
			SQLULEN                             rowsNumber,
			void                                **dataSet,
			SQLINTEGER                          **strLen_or_Ind,
			std::vector<std::string>            columnNames,
			std::vector<std::vector<SQLDOUBLE>> expectedDataSet);

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
		template<class InputSQLType, class RVectorType, class OutputSQLType, SQLSMALLINT OutputDataType>
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

		// Test GetResults to verify the expected results are obtained for date(time) data.
		//
		template<class SQLType, class RVectorType, class DateTimeTypeInR>
		void GetDateTimeResults(
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

		// Compare the given datetime data & nullMap for equality.
		//
		template<class SQLType>
		void CheckDateTimeDataEquality(
			SQLULEN    rowsNumber,
			SQLType    *expectedColumnData,
			SQLType    *columnData,
			SQLINTEGER *expectedColumnStrLenOrInd,
			SQLINTEGER *columnStrLenOrInd);

		// Templatized function to test output param value and strLenOrInd are as expected.
		//
		template<class SQLType>
		void GetOutputParam(
			std::vector<std::shared_ptr<SQLType>> expectedParamValues,
			std::vector<SQLINTEGER>               expectedStrLenOrInd);

		// Test (var)char(n) output param value and strLenOrInd are as expected.
		//
		void GetCharOutputParam(
			std::vector<SQLCHAR*>   expectedParamValues,
			std::vector<SQLINTEGER> expectedStrLenOrInd);

		// Test nchar(n)/nvarchar(n) output param value and strLenOrInd are as expected.
		//
		void GetNCharOutputParam(
			std::vector<const wchar_t*> expectedParamValues,
			std::vector<SQLINTEGER>	    expectedStrLenOrInd);

		// Test raw(binary) output param value and strLenOrInd are as expected.
		//
		void GetRawOutputParam(
			std::vector<SQLCHAR*>   expectedParamValues,
			std::vector<SQLINTEGER> expectedStrLenOrInd);

		// Test date/datetime output param values and strLenOrInd are as expected.
		//
		template<class SQLType>
		void GetDateTimeOutputParam(
			std::vector<SQLType>    expectedParamValues,
			std::vector<SQLINTEGER> expectedStrLenOrInd);

		// Tests numeric output param value and strLenOrInd are as expected.
		//
		void GetNumericOutputParam(
			std::vector<SQL_NUMERIC_STRUCT> expectedParamValues,
			std::vector<SQLINTEGER>         expectedStrLenOrInd);

		// Helper function to create a string with random characters of the provided size.
		//
		std::string CreateInputSizeRandomStr(int size);

		// Helper function to create the script string which will generate the raw values
		// needed within the R environment of the provided size.
		//
		std::string CreateInputSizeBinaryStringR(int size);

		// Objects declared here can be used by all tests in the test suite.
		//
		static const std::string sm_RHomePath;
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

		const std::string m_streamingParamName = "@r_rowsPerRead";

		// A value greater than USHRT_MAX to test truncation.
		//
		const int m_ValueLargerThanUShrtMax = 128000;

		// An intermediate size value for expanding test space.
		const int m_IntermediateValue = 100;
		
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
		const SQLINTEGER m_CharSize = sizeof(SQLCHAR);
		const SQLINTEGER m_NCharSize = sizeof(SQLWCHAR);
		const SQLINTEGER m_BinarySize = sizeof(SQLCHAR);
		const SQLINTEGER m_DateSize = sizeof(SQL_DATE_STRUCT);
		const SQLINTEGER m_DateTimeSize = sizeof(SQL_TIMESTAMP_STRUCT);

		std::unique_ptr<ColumnInfo<SQLINTEGER>> m_integerInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLCHAR>> m_logicalInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLREAL>> m_realInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLDOUBLE>> m_doubleInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLBIGINT>> m_bigIntInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLSMALLINT>> m_smallIntInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLCHAR>> m_tinyIntInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQLCHAR>> m_charInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQL_DATE_STRUCT>> m_dateInfo = nullptr;
		std::unique_ptr<ColumnInfo<SQL_TIMESTAMP_STRUCT>> m_dateTimeInfo = nullptr;

		// Used for partitioning test
		//
		std::unique_ptr<ColumnInfo<SQLINTEGER>> m_partition_integerInfo = nullptr;
		
		// R global environment
		//
		Rcpp::Environment m_globalEnvironment;

		// Indicates the number of suites that have been initialized.
		//
		static SQLUSMALLINT sm_numberOfSuitesInitialized;

		// Pointer handle to the library libRextension
		//
		static void *sm_libHandle;

		// Pointer to the Init function
		//
		static FN_init *sm_initFuncPtr;

		// Pointer to the InitSession function
		//
		static FN_initSession *sm_initSessionFuncPtr;

		// Pointer to the InitColumn function
		//
		static FN_initColumn *sm_initColumnFuncPtr;

		// Pointer to the InitParam function
		//
		static FN_initParam *sm_initParamFuncPtr;

		// Pointer to the Execute function
		//
		static FN_execute *sm_executeFuncPtr;

		// Pointer to the GetResultColumn function
		//
		static FN_getResultColumn *sm_getResultColumnFuncPtr;

		// Pointer to the GetResults function
		//
		static FN_getResults *sm_getResultsFuncPtr;

		// Pointer to the GetOutputParam function
		//
		static FN_getOutputParam *sm_getOutputParamFuncPtr;

		// Pointer to the CleanupSession function
		//
		static FN_cleanupSession *sm_cleanupSessionFuncPtr;

		// Pointer to the Cleanup function
		//
		static FN_cleanup *sm_cleanupFuncPtr;

		// The following target library paths are defined here in the base
		// class since we need to use them in the Init() call
		// which has to be unique across the entire test run.
		//

		// Root path of the libraries where the test installs them.
		//
		static std::string sm_libraryRootPath;

		// Some temp paths for public and private libraries as child folders of the
		// m_libraryRootPath above.
		//
		static std::string sm_publicLibraryPath;
		static std::string sm_privateLibraryPath;
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
			std::vector<SQLSMALLINT> nullable,
			std::vector<SQLSMALLINT> partitionByIndexes = {-1, -1});

		SQLUSMALLINT GetColumnsNumber() const
		{
			return m_columnNames.size();
		}

		static const SQLULEN sm_rowsNumber = 5;
		std::vector<std::string> m_columnNames;
		std::vector<SQLType> m_column1;
		std::vector<SQLType> m_column2;
		std::vector<void*> m_dataSet;
		std::vector<SQLINTEGER> m_col1StrLenOrInd;
		std::vector<SQLINTEGER> m_col2StrLenOrInd;
		std::vector<SQLINTEGER*> m_strLen_or_Ind;
		std::vector<SQLSMALLINT> m_nullable;
		std::vector<SQLSMALLINT> m_partitionByIndexes;
	};
}
