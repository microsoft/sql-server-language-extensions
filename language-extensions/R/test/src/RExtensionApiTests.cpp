//*************************************************************************************************
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
// @File: RExtensionApiTests.cpp
//
// Purpose:
//  Defines the base test fixture members and tests RExtension's implementation of
//  the external language initialization and cleanup APIs.
//
//*************************************************************************************************

#include <string.h>
#include <stdio.h>

#include "Common.h"

using namespace std;

#ifdef _WIN64
	const string x_RExtensionLibName = "libRExtension.dll";
	namespace fs = filesystem;
#else
	const string x_RExtensionLibName = "libRExtension.so.1.2";
	namespace fs = experimental::filesystem;
#endif // _WIN64

extern int g_argc;
extern const char **g_argv;

namespace ExtensionApiTest
{
	// Initialize all the static members
	//
	SQLUSMALLINT RExtensionApiTests::sm_numberOfSuitesInitialized = 0;
	void *RExtensionApiTests::sm_libHandle = nullptr;
	FN_init *RExtensionApiTests::sm_initFuncPtr = nullptr;
	FN_initSession *RExtensionApiTests::sm_initSessionFuncPtr = nullptr;
	FN_initColumn *RExtensionApiTests::sm_initColumnFuncPtr = nullptr;
	FN_initParam *RExtensionApiTests::sm_initParamFuncPtr = nullptr;
	FN_execute *RExtensionApiTests::sm_executeFuncPtr = nullptr;
	FN_getResultColumn *RExtensionApiTests::sm_getResultColumnFuncPtr = nullptr;
	FN_getResults *RExtensionApiTests::sm_getResultsFuncPtr = nullptr;
	FN_getOutputParam *RExtensionApiTests::sm_getOutputParamFuncPtr = nullptr;
	FN_cleanupSession *RExtensionApiTests::sm_cleanupSessionFuncPtr = nullptr;
	FN_cleanup *RExtensionApiTests::sm_cleanupFuncPtr = nullptr;

	string RExtensionApiTests::sm_libraryRootPath = "testInstallPkgs";
	string RExtensionApiTests::sm_publicLibraryPath;
	string RExtensionApiTests::sm_privateLibraryPath;

#ifdef _WIN64
	const string RExtensionApiTests::sm_RHomePath = "";
#else
	const string RExtensionApiTests::sm_RHomePath = "/usr/lib/R";
#endif

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTests::SetUpTestSuite
	//
	// Description:
	//  Per-test-suite set-up. Called before the first test in every test suite.
	//  But we want to execute this only once in the entire test run before the first test suite
	//  since R can be initialized only once.
	//
	void RExtensionApiTests::SetUpTestSuite()
	{
		// Initialize only if this is the first test suite.
		//
		if (sm_numberOfSuitesInitialized == 0)
		{
			// Set and Create the library paths before initialization of R environment
			// so that they get added to libPaths.
			//
			SetupLibPaths();
			CreateLibPaths();

			ASSERT_NO_THROW(GetHandles());
			DoInit();
		}

		++sm_numberOfSuitesInitialized;
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTests::TearDownTestSuite
	//
	// Description:
	//  Per-test-suite tear-down. Called after the last test in every test suite.
	//  But we want to execute this only once in the entire test run after the last test suite
	//  since R can be torn down only once.
	//
	void RExtensionApiTests::TearDownTestSuite()
	{
		// Cleanup only if this is the last test suite.
		//
		if (sm_numberOfSuitesInitialized == TOTAL_NUMBER_OF_TEST_SUITES)
		{
			DoCleanup();
			ASSERT_NO_THROW(ReleaseHandles());
			CleanupLibPaths();
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTests::SetUp
	//
	// Description:
	//  Per-test setup. Code here will be called immediately after the constructor (right
	//  before each test).
	//
	void RExtensionApiTests::SetUp()
	{
		SetupVariables();
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::TearDown
	//
	// Description:
	//  Per-test tear-down. Code here will be called immediately after each test (right
	// before the destructor).
	//
	void RExtensionApiTests::TearDown()
	{
		CleanupSession();
		CleanupVariables();
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTests::SetupLibPaths
	//
	// Description:
	//  Sets the library path variables.
	//
	void RExtensionApiTests::SetupLibPaths()
	{
		sm_libraryRootPath = fs::absolute(sm_libraryRootPath).string();
		fs::path libPath = sm_libraryRootPath;
		sm_publicLibraryPath = (libPath / "public").string();
		sm_privateLibraryPath = (libPath / "private").string();
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTests::CreateLibPaths
	//
	// Description:
	//  Creates the library path directories.
	//
	void RExtensionApiTests::CreateLibPaths()
	{
		fs::path libPath = fs::absolute(sm_libraryRootPath);
		if (fs::exists(libPath))
		{
			fs::remove_all(libPath);
		}

		fs::path publicLibraryPath = fs::absolute(sm_publicLibraryPath);
		fs::path privateLibraryPath = fs::absolute(sm_privateLibraryPath);

		EXPECT_TRUE(fs::create_directory(libPath));
		EXPECT_TRUE(fs::create_directory(publicLibraryPath));
		EXPECT_TRUE(fs::create_directory(privateLibraryPath));
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::GetHandles
	//
	// Description:
	//  Loads library and gets handles to different functions
	//  Testing if RExtension is successfully loaded dynamically
	//
	void RExtensionApiTests::GetHandles()
	{
		std::cout << "Loading the RExtension and getting handles for extension APIs.\n";
		sm_libHandle = Utilities::CrossPlatLoadLibrary(x_RExtensionLibName.c_str());
		ASSERT_TRUE(sm_libHandle != nullptr);

		sm_initFuncPtr = reinterpret_cast<FN_init*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				sm_libHandle,
				"Init"));
		ASSERT_TRUE(sm_initFuncPtr != nullptr);

		sm_initSessionFuncPtr = reinterpret_cast<FN_initSession*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				sm_libHandle,
				"InitSession"));
		ASSERT_TRUE(sm_initSessionFuncPtr != nullptr);

		sm_initColumnFuncPtr = reinterpret_cast<FN_initColumn*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				sm_libHandle,
				"InitColumn"));
		ASSERT_TRUE(sm_initColumnFuncPtr != nullptr);

		sm_initParamFuncPtr = reinterpret_cast<FN_initParam*>(
		Utilities::CrossPlatGetFunctionFromLibHandle(
			sm_libHandle,
			"InitParam"));
		ASSERT_TRUE(sm_initParamFuncPtr != nullptr);

		sm_executeFuncPtr = reinterpret_cast<FN_execute*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				sm_libHandle,
				"Execute"));
		ASSERT_TRUE(sm_executeFuncPtr != nullptr);

		sm_getResultColumnFuncPtr = reinterpret_cast<FN_getResultColumn*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				sm_libHandle,
				"GetResultColumn"));
		ASSERT_TRUE(sm_getResultColumnFuncPtr != nullptr);

		sm_getResultsFuncPtr = reinterpret_cast<FN_getResults*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				sm_libHandle,
				"GetResults"));
		ASSERT_TRUE(sm_getResultsFuncPtr != nullptr);

		sm_getOutputParamFuncPtr = reinterpret_cast<FN_getOutputParam*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				sm_libHandle,
				"GetOutputParam"));
		ASSERT_TRUE(sm_getOutputParamFuncPtr != nullptr);

		sm_cleanupSessionFuncPtr = reinterpret_cast<FN_cleanupSession*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				sm_libHandle,
				"CleanupSession"));
		ASSERT_TRUE(sm_cleanupSessionFuncPtr != nullptr);

		sm_cleanupFuncPtr = reinterpret_cast<FN_cleanup*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				sm_libHandle,
				"Cleanup"));
		ASSERT_TRUE(sm_cleanupFuncPtr != nullptr);
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::DoInit
	//
	// Description:
	//  Does Init where embedded R is initialized - can be called only once in the test suite.
	//  Testing if Init api is implemented correctly.
	//
	void RExtensionApiTests::DoInit()
	{
		std::cout << "Calling Init.\n";
		SQLRETURN result = SQL_ERROR;
		const string cmdLine = "dummyInputScript --no-save";
		int paramsLength = strlen(cmdLine.c_str());
		unique_ptr<SQLCHAR[]> extensionParams = make_unique<SQLCHAR[]>(paramsLength);
		memcpy(extensionParams.get(), cmdLine.c_str(), paramsLength);

		SQLCHAR *extensionPath = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(sm_RHomePath.c_str())));
		SQLCHAR* publiclibraryPath = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(sm_publicLibraryPath.c_str())));
		SQLCHAR* privateLibraryPath = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(sm_privateLibraryPath.c_str())));

		result = (*sm_initFuncPtr)(
			extensionParams.get(),
			paramsLength,
			extensionPath,
			sm_RHomePath.length(),
			publiclibraryPath,
			sm_publicLibraryPath.length(),
			privateLibraryPath,
			sm_privateLibraryPath.length());

		ASSERT_EQ(result, SQL_SUCCESS);
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::SetupVariables
	//
	// Description:
	//  Sets up default, valid variables for use in tests
	//
	void RExtensionApiTests::SetupVariables()
	{
		m_sessionId = new SQLGUID();
		m_taskId = 0;
		m_numTasks = 1;
		m_inputSchemaColumnsNumber = 1;
		m_parametersNumber = 0;

		m_columnNameString = "Column1";
		m_columnName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_columnNameString.c_str()))
			);

#ifdef _WIN64
		m_scriptString = "message('" + m_printMessage + "');"
#else
		m_scriptString = "print('" + m_printMessage + "');"
#endif
			"OutputDataSet <- InputDataSet;"
			"print('InputDataSet:'); print(InputDataSet);"
			"print('OutputDataSet:'); print(OutputDataSet);";
		m_script = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_scriptString.c_str()))
			);

		m_inputDataNameString = "InputDataSet";
		m_inputDataName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_inputDataNameString.c_str()))
			);

		m_outputDataNameString = "OutputDataSet";
		m_outputDataName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_outputDataNameString.c_str()))
			);

		m_integerInfo = make_unique<ColumnInfo<SQLINTEGER>>(
			"IntegerColumn1",
			vector<SQLINTEGER>{ 1, 2, 3, 4, 5 },
			vector<SQLINTEGER>(ColumnInfo<SQLINTEGER>::sm_rowsNumber, m_IntSize),
			"IntegerColumn2",
			vector<SQLINTEGER>{ m_MaxInt, m_MinInt, NA_INTEGER, NA_INTEGER, -1 },
			vector<SQLINTEGER>{ m_IntSize, m_IntSize, SQL_NULL_DATA,
				SQL_NULL_DATA, m_IntSize },
			vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });

		m_logicalInfo = make_unique<ColumnInfo<SQLCHAR>>(
			"LogicalColumn1",
			vector<SQLCHAR>{ '1', '0', '1', 0, 1 },
			vector<SQLINTEGER>(ColumnInfo<SQLCHAR>::sm_rowsNumber, m_LogicalSize),
			"LogicalColumn2",
			vector<SQLCHAR>{ '\0', '2', '1', '0',
				'\0' }, // static_cast from NA_LOGICAL to SQLCHAR is '\0'.
			vector<SQLINTEGER>{ SQL_NULL_DATA, m_LogicalSize, m_LogicalSize,
				m_LogicalSize, SQL_NULL_DATA },
			vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });

		// the input column information set here is the same used to test the expected output
		// column results; real/bigInt input columns are returned back as double
		// output columns and so the strLenInd values are set to m_DoubleSize.
		//
		m_realInfo = make_unique<ColumnInfo<SQLREAL>>(
			"RealColumn1",
			vector<SQLREAL>{ 0.34, 1.33, 83.98, 72.45, 68e10 },
			vector<SQLINTEGER>(ColumnInfo<SQLREAL>::sm_rowsNumber, m_DoubleSize),
			"RealColumn2",
			vector<SQLREAL>{ m_MaxReal, NAN, m_MinReal, NAN, 0 },
			vector<SQLINTEGER>{ m_DoubleSize, SQL_NULL_DATA, m_DoubleSize,
				SQL_NULL_DATA, m_DoubleSize },
			vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });

		m_doubleInfo = make_unique<ColumnInfo<SQLDOUBLE>>(
			"DoubleColumn1",
			vector<SQLDOUBLE>{ -1.79e301, 1.33, 83.98, 72.45, 1.79e30 },
			vector<SQLINTEGER>(ColumnInfo<SQLDOUBLE>::sm_rowsNumber, m_DoubleSize),
			"DoubleColumn2",
			vector<SQLDOUBLE>{ NAN, m_MaxDouble, NAN, m_MinDouble, NAN },
			vector<SQLINTEGER>{ SQL_NULL_DATA, m_DoubleSize, SQL_NULL_DATA,
				m_DoubleSize, SQL_NULL_DATA },
			vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });

		m_bigIntInfo = make_unique<ColumnInfo<SQLBIGINT>>(
			"BigIntColumn1",
			vector<SQLBIGINT>{ m_MaxBigInt, 1,
				88883939, m_MinBigInt, -622280108 },
			vector<SQLINTEGER>(ColumnInfo<SQLBIGINT>::sm_rowsNumber, m_DoubleSize),
			"BigIntColumn2",
			vector<SQLBIGINT>(ColumnInfo<SQLBIGINT>::sm_rowsNumber, NA_REAL),
			vector<SQLINTEGER>{ SQL_NULL_DATA, SQL_NULL_DATA,
				SQL_NULL_DATA, SQL_NULL_DATA, SQL_NULL_DATA },
			vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });

		// the input column information set here is the same used to test the
		// expected output column results; smallint/tinyint input columns are returned back as
		// integer output columns and so the strLenInd values here are set to m_IntSize.
		//
		m_smallIntInfo = make_unique<ColumnInfo<SQLSMALLINT>>(
			"SmallIntColumn1",
			vector<SQLSMALLINT>{ 223, 33, 9811, -725, 6810 },
			vector<SQLINTEGER>(ColumnInfo<SQLSMALLINT>::sm_rowsNumber, m_IntSize),
			"SmallIntColumn2",
			vector<SQLSMALLINT>{ -1, 0, m_MaxSmallInt, m_MinSmallInt, 3'276 },
			vector<SQLINTEGER>(ColumnInfo<SQLSMALLINT>::sm_rowsNumber, m_IntSize),
			vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NO_NULLS });

		m_tinyIntInfo = make_unique<ColumnInfo<SQLCHAR>>(
			"TinyIntColumn1",
			vector<SQLCHAR>{ 34, 133, 98, 72, 10 },
			vector<SQLINTEGER>(ColumnInfo<SQLCHAR>::sm_rowsNumber, m_IntSize),
			"TinyIntColumn2",
			vector<SQLCHAR>{ m_MaxTinyInt, m_MinTinyInt, 1, 0, 128 },
			vector<SQLINTEGER>{ m_IntSize, SQL_NULL_DATA,
				SQL_NULL_DATA, SQL_NULL_DATA, m_IntSize },
			vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });

		m_dateInfo = make_unique<ColumnInfo<SQL_DATE_STRUCT>>(
			"DateColumn1",
			vector<SQL_DATE_STRUCT>{
				{ 9518, 8, 25 },
				{ 5712, 3, 9 },
				{ 1470, 7, 27 },
				{ 2020, 4, 16 },
				{ 231, 2, 14, },
			},
			vector<SQLINTEGER>(ColumnInfo<SQL_DATE_STRUCT>::sm_rowsNumber, m_DateSize),
			"DateColumn2",
			vector<SQL_DATE_STRUCT>{
				{ 9999, 12, 31 },
				{ 1,1,1 },
				{},
				{},
				{}
			},
			vector<SQLINTEGER>{ m_DateSize, m_DateSize,
			SQL_NULL_DATA, SQL_NULL_DATA, SQL_NULL_DATA },
			vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });

		m_dateTimeInfo = make_unique<ColumnInfo<SQL_TIMESTAMP_STRUCT>>(
			"DateTimeColumn1",
			vector<SQL_TIMESTAMP_STRUCT>{
				{ 9518, 8, 25, 19, 11, 40, 528931000 },
				{ 5712, 3, 9, 2, 24, 32, 770477000 },
				{ 1470, 7, 27, 17, 47, 52, 123455000 },
				{ 2020, 4, 16, 15, 5, 12, 169012000 },
				{ 231, 2, 14, 22, 36, 18, 489105000 },
			},
			vector<SQLINTEGER>(ColumnInfo<SQL_TIMESTAMP_STRUCT>::sm_rowsNumber, m_DateTimeSize),
			"DateTimeColumn2",
			vector<SQL_TIMESTAMP_STRUCT>{
				{ 9999, 12, 31, 23, 59, 59, 92000 },
				{ 1,1,1,0,0,0,0 },
				{},
				{},
				{}
			},
			vector<SQLINTEGER>{ m_DateTimeSize, m_DateTimeSize,
			SQL_NULL_DATA, SQL_NULL_DATA, SQL_NULL_DATA },
			vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });

		// Turn on partitioning via setting partitionByNumber to the partitionBy column index,
		// the non-nullable column with index 0.
		//
		m_partition_integerInfo = make_unique<ColumnInfo<SQLINTEGER>>(
			"PartitionByColumn1",
			vector<SQLINTEGER>{ 1, 2, 3, 4, 5 },
			vector<SQLINTEGER>(ColumnInfo<SQLINTEGER>::sm_rowsNumber, m_IntSize),
			"NonPartitionByColumn2",
			vector<SQLINTEGER>{ m_MaxInt, m_MinInt, NA_INTEGER, NA_INTEGER, -1 },
			vector<SQLINTEGER>{ m_IntSize, m_IntSize, SQL_NULL_DATA,
				SQL_NULL_DATA, m_IntSize },
			vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE },
			vector<SQLSMALLINT>{ 0, -1 });

		// Retrieve the global environment
		//
		m_globalEnvironment = Rcpp::Environment::global_env();
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::CleanupVariables
	//
	// Description:
	//  Deletes the memory allocated to default variables.
	//
	void RExtensionApiTests::CleanupVariables()
	{
		if (m_sessionId != nullptr)
		{
			delete m_sessionId;
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: ReleaseHandles
	//
	// Description:
	//  Closes the handle to the library.
	//
	void RExtensionApiTests::ReleaseHandles()
	{
		std::cout << "Closing the library handle.\n";
		Utilities::CrossPlatCloseLibrary(sm_libHandle);
	}

	//----------------------------------------------------------------------------------------------
	// Name: DoCleanup
	//
	// Description:
	// Calls Cleanup on the RExtension.
	// Testing if Cleanup is implemented correctly.
	//
	void RExtensionApiTests::DoCleanup()
	{
		std::cout << "Calling Cleanup.\n";
		SQLRETURN result = SQL_ERROR;
		result = (*sm_cleanupFuncPtr)();
		ASSERT_EQ(result, SQL_SUCCESS);
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::CleanupLibPaths
	//
	// Description:
	//  Cleans up all the library paths created.
	//
	void RExtensionApiTests::CleanupLibPaths()
	{
		fs::path libPath = fs::absolute(sm_libraryRootPath);

		if (fs::exists(libPath))
		{
			fs::remove_all(libPath);
		}

		EXPECT_FALSE(fs::exists(libPath));
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::InitializeSession
	//
	// Description:
	//  Initializes a valid, default session for later tests
	//
	void RExtensionApiTests::InitializeSession(
		SQLUSMALLINT inputSchemaColumnsNumber,
		string       scriptString,
		SQLUSMALLINT parametersNumber)
	{
		SQLRETURN result = SQL_SUCCESS;

		SQLCHAR *script = static_cast<SQLCHAR*>(
			static_cast<void*>(const_cast<char*>(scriptString.c_str())));

		result = (*sm_initSessionFuncPtr)(
			*m_sessionId,
			m_taskId,
			m_numTasks,
			script,
			scriptString.length(),
			inputSchemaColumnsNumber,
			parametersNumber,
			m_inputDataName,
			m_inputDataNameString.length(),
			m_outputDataName,
			m_outputDataNameString.length());

		EXPECT_EQ(result, SQL_SUCCESS);
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::CleanupSession
	//
	// Description:
	//  Cleans up a valid, default session for later tests
	//
	void RExtensionApiTests::CleanupSession()
	{
		SQLRETURN result = SQL_SUCCESS;

		result = (*sm_cleanupSessionFuncPtr)(
			*m_sessionId,
			m_taskId);

		EXPECT_EQ(result, SQL_SUCCESS);
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::InitializeColumns
	//
	// Description:
	//  Templatized function to call InitializeColumn for all columns in ColumnInfo.
	//
	template<class SQLType, SQLSMALLINT dataType>
	void RExtensionApiTests::InitializeColumns(ColumnInfo<SQLType> *columnInfo)
	{
		SQLUSMALLINT inputSchemaColumnsNumber = columnInfo->GetColumnsNumber();
		for (SQLUSMALLINT columnNumber = 0; columnNumber < inputSchemaColumnsNumber; ++columnNumber)
		{
			if constexpr (is_same_v<SQLType, SQL_NUMERIC_STRUCT>)
			{
				SQL_NUMERIC_STRUCT *columnData =
					static_cast<SQL_NUMERIC_STRUCT*>(columnInfo->m_dataSet[columnNumber]);
				InitializeColumn(columnNumber,
					columnInfo->m_columnNames[columnNumber],
					dataType,
					sizeof(SQLType),
					columnData[0].scale, // decimalDigits
					columnInfo->m_nullable[columnNumber],
					columnInfo->m_partitionByIndexes[columnNumber]);
			}
			else
			{
				InitializeColumn(columnNumber,
					columnInfo->m_columnNames[columnNumber],
					dataType,
					sizeof(SQLType),
					0, // decimalDigits
					columnInfo->m_nullable[columnNumber],
					columnInfo->m_partitionByIndexes[columnNumber]);
			}
		}
	}

	// Template instantiations
	//
	template void RExtensionApiTests::InitializeColumns<SQLINTEGER, SQL_C_SLONG>(
		ColumnInfo<SQLINTEGER> *ColumnInfo);
	template void RExtensionApiTests::InitializeColumns<SQLCHAR, SQL_C_BIT>(
		ColumnInfo<SQLCHAR> *ColumnInfo);
	template void RExtensionApiTests::InitializeColumns<SQLREAL, SQL_C_FLOAT>(
		ColumnInfo<SQLREAL> *ColumnInfo);
	template void RExtensionApiTests::InitializeColumns<SQLDOUBLE, SQL_C_DOUBLE>(
		ColumnInfo<SQLDOUBLE> *ColumnInfo);
	template void RExtensionApiTests::InitializeColumns<SQLBIGINT, SQL_C_SBIGINT>(
		ColumnInfo<SQLBIGINT> *ColumnInfo);
	template void RExtensionApiTests::InitializeColumns<SQLSMALLINT, SQL_C_SSHORT>(
		ColumnInfo<SQLSMALLINT> *ColumnInfo);
	template void RExtensionApiTests::InitializeColumns<SQLCHAR, SQL_C_UTINYINT>(
		ColumnInfo<SQLCHAR> *ColumnInfo);
	template void RExtensionApiTests::InitializeColumns<SQL_DATE_STRUCT, SQL_C_TYPE_DATE>(
		ColumnInfo<SQL_DATE_STRUCT> *ColumnInfo);
	template void RExtensionApiTests::InitializeColumns<SQL_TIMESTAMP_STRUCT, SQL_C_TYPE_TIMESTAMP>(
		ColumnInfo<SQL_TIMESTAMP_STRUCT> *ColumnInfo);
	template void RExtensionApiTests::InitializeColumns<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(
		ColumnInfo<SQL_NUMERIC_STRUCT> *ColumnInfo);

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::RExtensionApiTestInitializeColumn
	//
	// Description:
	//  Calls InitColumn for the given columnNumber, columnName, dataType and columnSize.
	//
	void RExtensionApiTests::InitializeColumn(
		SQLSMALLINT columnNumber,
		string      columnNameString,
		SQLSMALLINT dataType,
		SQLULEN     columnSize,
		SQLSMALLINT decimalDigits,
		SQLSMALLINT nullable,
		SQLSMALLINT partitionByNumber)
	{
		SQLCHAR *columnName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(columnNameString.c_str()))
			);

		SQLRETURN result = SQL_ERROR;

		result = (*sm_initColumnFuncPtr)(
				*m_sessionId,
				m_taskId,
				columnNumber,
				columnName,
				columnNameString.length(),
				dataType,
				columnSize,
				decimalDigits,
				nullable,                 // nullable
				partitionByNumber,        // partitionByNumber
				-1);                      // orderByNumber
		EXPECT_EQ(result, SQL_SUCCESS);
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::GenerateContiguousData
	//
	// Description:
	//  Fills a contiguous array columnData with members from the given columnVector
	//  having lengths defined in strLenOrInd, unless it is SQL_NULL_DATA.
	//
	template<class SQLType>
	vector<SQLType> RExtensionApiTests::GenerateContiguousData(
		vector<const SQLType*> columnVector,
		SQLINTEGER             *strLenOrInd)
	{
		vector<SQLType> retVal;

		for (SQLULEN index = 0; index < columnVector.size(); ++index)
		{
			if (strLenOrInd[index] != SQL_NULL_DATA)
			{
				SQLINTEGER strLen = strLenOrInd[index] / sizeof(SQLType);
				vector<SQLType> data(columnVector[index], columnVector[index] + strLen);
				retVal.insert(retVal.end(), data.begin(), data.end());
			}
		}

		return retVal;
	}

	// Template instantiations
	//
	template vector<char> RExtensionApiTests::GenerateContiguousData(
		vector<const char*> columnVector,
		SQLINTEGER          *strLenOrInd);
	template vector<SQLCHAR> RExtensionApiTests::GenerateContiguousData(
		vector<const SQLCHAR*> columnVector,
		SQLINTEGER             *strLenOrInd);
	template vector<wchar_t> RExtensionApiTests::GenerateContiguousData(
		vector<const wchar_t*> columnVector,
		SQLINTEGER             *strLenOrInd);

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::CheckRVectorColumnDataEquality
	//
	// Description:
	// Templatized function to compare the given vector and data for equality.
	// for integer/numeric/logical data types.
	// The expectedData is input as a void*, hence we input the expectedRowsNumber as well.
	// Where strLen_or_Ind == SQL_NULL_DATA and nullable = SQL_NULLABLE, check for is_na.
	// The default value of nullable is SQL_NULLABLE.
	//
	template<class SQLType, class RVectorType, SQLSMALLINT DataType>
	void RExtensionApiTests::CheckRVectorColumnDataEquality(
		SQLULEN     expectedRowsNumber,
		RVectorType vectorToTest,
		void        *expectedData,
		SQLINTEGER  *strLen_or_Ind,
		SQLSMALLINT nullable)
	{
		ASSERT_EQ(static_cast<size_t>(vectorToTest.size()), expectedRowsNumber);
		for(SQLULEN index = 0 ; index < expectedRowsNumber; ++index)
		{
			if (nullable &&
				strLen_or_Ind != nullptr &&
				strLen_or_Ind[index] == SQL_NULL_DATA)
			{
				EXPECT_TRUE(RVectorType::is_na(vectorToTest[index]));
			}
			else
			{
				SQLType expectedValue = static_cast<SQLType*>(expectedData)[index];
				if constexpr (DataType == SQL_C_BIT)
				{
					EXPECT_EQ(vectorToTest[index],
						expectedValue != '0' && expectedValue != 0);
				}
				else
				{
					EXPECT_EQ(vectorToTest[index], expectedValue);
				}
			}
		}
	}

	// Template instantiations
	//
	template void RExtensionApiTests::CheckRVectorColumnDataEquality<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
		SQLULEN             expectedRowsNumber,
		Rcpp::IntegerVector vectorToTest,
		void                *expectedData,
		SQLINTEGER          *strLenOrInd,
		SQLSMALLINT         nullable);
	template void RExtensionApiTests::CheckRVectorColumnDataEquality<SQLCHAR, Rcpp::LogicalVector, SQL_C_BIT>(
		SQLULEN             expectedRowsNumber,
		Rcpp::LogicalVector vectorToTest,
		void                *expectedData,
		SQLINTEGER          *strLen_or_Ind,
		SQLSMALLINT         nullable);
	template void RExtensionApiTests::CheckRVectorColumnDataEquality<SQLREAL, Rcpp::NumericVector, SQL_C_FLOAT>(
		SQLULEN             expectedRowsNumber,
		Rcpp::NumericVector vectorToTest,
		void                *expectedData,
		SQLINTEGER          *strLen_or_Ind,
		SQLSMALLINT         nullable);
	template void RExtensionApiTests::CheckRVectorColumnDataEquality<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
		SQLULEN             expectedRowsNumber,
		Rcpp::NumericVector vectorToTest,
		void                *expectedData,
		SQLINTEGER          *strLen_or_Ind,
		SQLSMALLINT         nullable);
	template void RExtensionApiTests::CheckRVectorColumnDataEquality<SQLBIGINT, Rcpp::NumericVector, SQL_C_SBIGINT>(
		SQLULEN             expectedRowsNumber,
		Rcpp::NumericVector vectorToTest,
		void                *expectedData,
		SQLINTEGER          *strLen_or_Ind,
		SQLSMALLINT         nullable);
	template void RExtensionApiTests::CheckRVectorColumnDataEquality<SQLSMALLINT, Rcpp::IntegerVector, SQL_C_SSHORT>(
		SQLULEN             expectedRowsNumber,
		Rcpp::IntegerVector vectorToTest,
		void                *expectedData,
		SQLINTEGER          *strLen_or_Ind,
		SQLSMALLINT         nullable);
	template void RExtensionApiTests::CheckRVectorColumnDataEquality<SQLCHAR, Rcpp::IntegerVector, SQL_C_UTINYINT>(
		SQLULEN             expectedRowsNumber,
		Rcpp::IntegerVector vectorToTest,
		void                *expectedData,
		SQLINTEGER          *strLen_or_Ind,
		SQLSMALLINT         nullable);

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::CheckRCharacterVectorColumnDataEquality
	//
	// Description:
	//  Compares character vector with the given data and corresponding strLen_or_Ind.
	//  The expectedData is input as a void*, hence we input the expectedRowsNumber as well.
	//  Where strLen_or_Ind == SQL_NULL_DATA, check for is_na.
	//
	template<class CharType>
	void RExtensionApiTests::CheckRCharacterVectorColumnDataEquality(
		SQLULEN               expectedRowsNumber,
		Rcpp::CharacterVector vectorToTest,
		void                  *expectedData,
		SQLINTEGER            *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<size_t>(vectorToTest.size()), expectedRowsNumber);
		SQLINTEGER cumulativeLength = 0 ;
		for(SQLULEN index = 0 ; index < expectedRowsNumber; ++index)
		{
			if (strLen_or_Ind == nullptr ||
			  (strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA))
			{
				EXPECT_TRUE(Rcpp::CharacterVector::is_na(vectorToTest[index]));
			}
			else
			{
				char* actualDataBytes = vectorToTest[index];
				string expectedDataInUtf8;

				// If expectedData is of wchar_t type with utf-16 encoding, convert it into the utf-8
				// encoding before comparing with actualDatabytes because those are always in utf-8
				// on account of R's default being utf-8.
				//
				if constexpr (is_same_v<CharType, wchar_t>)
				{
					char16_t* expectedDataInUtf16 =
						static_cast<char16_t*>(expectedData) + cumulativeLength;
					estd::ToUtf8(
						expectedDataInUtf16,
						strLen_or_Ind[index] / sizeof(char16_t),
						expectedDataInUtf8);
				}
				else
				{
					expectedDataInUtf8 =
						string(reinterpret_cast<char*>(expectedData) + cumulativeLength,
							   strLen_or_Ind[index] / sizeof(char));
				}

				const char* expectedDataBytes = expectedDataInUtf8.c_str();
				ASSERT_EQ(strlen(expectedDataBytes), strlen(actualDataBytes));

				// Compare upto the strlen of the utf-8 encoded expectedData since
				// strLenOrInd represents the total bytes in the utf-16 encoded expectedData
				// so we cannot use that.
				//
				SQLINTEGER bytesToCompare = static_cast<SQLINTEGER>(strlen(expectedDataBytes));
				for (SQLINTEGER byte = 0; byte < bytesToCompare; ++byte)
				{
					EXPECT_EQ(actualDataBytes[byte], expectedDataBytes[byte]);
				}

				// Advance the length by number of characters, not bytes
				// hence divide by sizeof(CharType)
				//
				cumulativeLength += (strLen_or_Ind[index] / sizeof(CharType));
			}
		}
	}

	// Template instantiations
	//
	template void RExtensionApiTests::CheckRCharacterVectorColumnDataEquality<char>(
		SQLULEN               expectedRowsNumber,
		Rcpp::CharacterVector vectorToTest,
		void                  *expectedData,
		SQLINTEGER            *strLen_or_Ind);

	template void RExtensionApiTests::CheckRCharacterVectorColumnDataEquality<wchar_t>(
		SQLULEN               expectedRowsNumber,
		Rcpp::CharacterVector vectorToTest,
		void                  *expectedData,
		SQLINTEGER            *strLen_or_Ind);

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::CheckRDateTimeVectorColumnDataEquality
	//
	// Description:
	//  Compares the given R Date(time) vector and data for equality
	//
	template<class SQLType, class RVectorType, class DateTimeTypeInR>
	void RExtensionApiTests::CheckRDateTimeVectorColumnDataEquality(
		SQLULEN     expectedRowsNumber,
		RVectorType vectorToTest,
		void        *expectedData,
		SQLINTEGER  *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<SQLULEN>(vectorToTest.size()), expectedRowsNumber);

		for (SQLULEN index = 0; index < expectedRowsNumber; ++index)
		{
			DateTimeTypeInR actualValue = static_cast<DateTimeTypeInR>(vectorToTest[index]);
			if (strLen_or_Ind == nullptr ||
				(strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA))
			{
				EXPECT_TRUE(actualValue.is_na());
			}
			else
			{
				SQLType expectedValue = static_cast<SQLType *>(expectedData)[index];

				SQLSMALLINT year = actualValue.getYear();
				SQLUSMALLINT month = actualValue.getMonth();
				SQLUSMALLINT day = actualValue.getDay();

				EXPECT_EQ(expectedValue.year, year);
				EXPECT_EQ(expectedValue.month, month);
				EXPECT_EQ(expectedValue.day, day);

				if constexpr (is_same_v<DateTimeTypeInR, Rcpp::Datetime>)
				{
					SQLUSMALLINT hour = actualValue.getHours();
					SQLUSMALLINT minute = actualValue.getMinutes();
					SQLUSMALLINT second = actualValue.getSeconds();
					SQLUINTEGER usec = actualValue.getMicroSeconds();

					EXPECT_EQ(expectedValue.hour, hour);
					EXPECT_EQ(expectedValue.minute, minute);
					EXPECT_EQ(expectedValue.second, second);

					// Fraction is in nanoseconds but R uses microseconds
					// so convert to microsecond value (by dividing by 1000) to compare.
					//
					SQLUINTEGER expectedUsec = round(expectedValue.fraction / 1000.0);

					// Sometimes even after rounding, it has a margin error of +-1.
					//
					EXPECT_TRUE(usec == expectedUsec || usec == expectedUsec + 1 || usec == expectedUsec - 1);
				}
			}
		}
	}

	// Template instantiations
	//
	template void RExtensionApiTests::CheckRDateTimeVectorColumnDataEquality
		<SQL_DATE_STRUCT, Rcpp::DateVector, Rcpp::Date>(
			SQLULEN          expectedRowsNumber,
			Rcpp::DateVector vectorToTest,
			void             *expectedData,
			SQLINTEGER       *strLen_or_Ind);

	template void RExtensionApiTests::CheckRDateTimeVectorColumnDataEquality
		<SQL_TIMESTAMP_STRUCT, Rcpp::DatetimeVector, Rcpp::Datetime>(
			SQLULEN              expectedRowsNumber,
			Rcpp::DatetimeVector vectorToTest,
			void                 *expectedData,
			SQLINTEGER           *strLen_or_Ind);

	//----------------------------------------------------------------------------------------------
	// Name: ColumnInfo::ColumnInfo
	//
	// Description:
	// Templetized constructor for the type information.
	// Useful for ColumnInfo of integer, basic numeric, logical and date(time) types.
	//
	template<class SQLType>
	ColumnInfo<SQLType>::ColumnInfo(
		string column1Name, vector<SQLType> column1, vector<SQLINTEGER> col1StrLenOrInd,
		string column2Name, vector<SQLType> column2, vector<SQLINTEGER> col2StrLenOrInd,
		vector<SQLSMALLINT> nullable,
		vector<SQLSMALLINT> partitionByIndexes)
	{
		m_columnNames = { column1Name, column2Name };
		m_column1 = column1;
		m_column2 = column2;
		m_dataSet = { m_column1.data(), m_column2.data()};
		m_col1StrLenOrInd = col1StrLenOrInd;
		if (m_col1StrLenOrInd.empty())
		{
			m_strLen_or_Ind.push_back(nullptr);
		}
		else
		{
			m_strLen_or_Ind.push_back(m_col1StrLenOrInd.data());
		}

		m_col2StrLenOrInd = col2StrLenOrInd;
		if (m_col2StrLenOrInd.empty())
		{
			m_strLen_or_Ind.push_back(nullptr);
		}
		else
		{
			m_strLen_or_Ind.push_back(m_col2StrLenOrInd.data());
		}

		m_nullable = nullable;
		m_partitionByIndexes = partitionByIndexes;
	}

	// Template instantiation
	//
	template ColumnInfo<SQL_NUMERIC_STRUCT>::ColumnInfo(
		string column1Name,
		vector<SQL_NUMERIC_STRUCT> column1,
		vector<SQLINTEGER> col1StrLenOrInd,
		string column2Name,
		vector<SQL_NUMERIC_STRUCT> column2,
		vector<SQLINTEGER> col2StrLenOrInd,
		vector<SQLSMALLINT> nullable,
		vector<SQLSMALLINT> partitionByIndexes);
}
