//*************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server
// external language communication protocol.
// Copyright (C) 2019 Microsoft Corporation.
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
// @File: RExtensionApiTest.cpp
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
	const string RExtensionLibName = "libRExtension.dll";
#else
	const string RExtensionLibName = "libRExtension.so.1.0";
#endif // _WIN64

extern int g_argc;
extern const char **g_argv;

namespace ExtensionApiTest
{
	// Initialize all the static members
	//
	void *RExtensionApiTest::m_libHandle = nullptr;
	FN_init *RExtensionApiTest::m_initFuncPtr = nullptr;
	FN_initSession *RExtensionApiTest::m_initSessionFuncPtr = nullptr;
	FN_initColumn *RExtensionApiTest::m_initColumnFuncPtr = nullptr;
	FN_initParam *RExtensionApiTest::m_initParamFuncPtr = nullptr;
	FN_execute *RExtensionApiTest::m_executeFuncPtr = nullptr;
	FN_getResultColumn *RExtensionApiTest::m_getResultColumnFuncPtr = nullptr;
	FN_getResults *RExtensionApiTest::m_getResultsFuncPtr = nullptr;
	FN_getOutputParam *RExtensionApiTest::m_getOutputParamFuncPtr = nullptr;
	FN_cleanupSession *RExtensionApiTest::m_cleanupSessionFuncPtr = nullptr;
	FN_cleanup *RExtensionApiTest::m_cleanupFuncPtr = nullptr;
#ifdef _WIN64
	const string RExtensionApiTest::m_RHomePath = "";
#else
	const string RExtensionApiTest::m_RHomePath = "/usr/lib/R";
#endif

	// Per-test-suite set-up.
	// Called before the first test in this test suite.
	//
	void RExtensionApiTest::SetUpTestSuite()
	{
		ASSERT_NO_THROW(GetHandles());
		DoInit();
	}

	// Per-test-suite tear-down.
	// Called after the last test in this test suite.
	//
	void RExtensionApiTest::TearDownTestSuite()
	{
		DoCleanup();
		ASSERT_NO_THROW(ReleaseHandles());
	}

	// Code here will be called immediately after the constructor (right
	// before each test).
	//
	void RExtensionApiTest::SetUp()
	{
		SetupVariables();
	}

	// Code here will be called immediately after each test (right
	// before the destructor).
	//
	void RExtensionApiTest::TearDown()
	{
		CleanupSession();
		CleanupVariables();
	}

	// Name: GetHandles
	//
	// Description:
	// Load library and get handles to different functions
	// Testing if RExtension is successfully loaded dynamically
	//
	void RExtensionApiTest::GetHandles()
	{
		std::cout << "Loading the RExtension and getting handles for extension APIs.\n";
		m_libHandle = Utilities::CrossPlatLoadLibrary(RExtensionLibName.c_str());
		ASSERT_TRUE(m_libHandle != nullptr);

		m_initFuncPtr = reinterpret_cast<FN_init*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				m_libHandle,
				"Init"));
		ASSERT_TRUE(m_initFuncPtr != nullptr);

		m_initSessionFuncPtr = reinterpret_cast<FN_initSession*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				m_libHandle,
				"InitSession"));
		ASSERT_TRUE(m_initSessionFuncPtr != nullptr);

		m_initColumnFuncPtr = reinterpret_cast<FN_initColumn*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				m_libHandle,
				"InitColumn"));
		ASSERT_TRUE(m_initColumnFuncPtr != nullptr);

		m_initParamFuncPtr = reinterpret_cast<FN_initParam*>(
		Utilities::CrossPlatGetFunctionFromLibHandle(
			m_libHandle,
			"InitParam"));
		ASSERT_TRUE(m_initParamFuncPtr != nullptr);

		m_executeFuncPtr = reinterpret_cast<FN_execute*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				m_libHandle,
				"Execute"));
		ASSERT_TRUE(m_executeFuncPtr != nullptr);

		m_getResultColumnFuncPtr = reinterpret_cast<FN_getResultColumn*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				m_libHandle,
				"GetResultColumn"));
		ASSERT_TRUE(m_getResultColumnFuncPtr != nullptr);

		m_getResultsFuncPtr = reinterpret_cast<FN_getResults*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				m_libHandle,
				"GetResults"));
		ASSERT_TRUE(m_getResultsFuncPtr != nullptr);

		m_getOutputParamFuncPtr = reinterpret_cast<FN_getOutputParam*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				m_libHandle,
				"GetOutputParam"));
		ASSERT_TRUE(m_getOutputParamFuncPtr != nullptr);

		m_cleanupSessionFuncPtr = reinterpret_cast<FN_cleanupSession*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				m_libHandle,
				"CleanupSession"));
		ASSERT_TRUE(m_cleanupSessionFuncPtr != nullptr);

		m_cleanupFuncPtr = reinterpret_cast<FN_cleanup*>(
			Utilities::CrossPlatGetFunctionFromLibHandle(
				m_libHandle,
				"Cleanup"));
		ASSERT_TRUE(m_cleanupFuncPtr != nullptr);
	}

	// Name: DoInit
	//
	// Description:
	// Do Init where embedded R is initialized - can be called only once in the test suite.
	// Testing if Init is implemented correctly.
	//
	void RExtensionApiTest::DoInit()
	{
		std::cout << "Calling Init.\n";
		SQLRETURN result = SQL_ERROR;
		const string cmdLine = "dummyInputScript --no-save";
		int paramsLength = strlen(cmdLine.c_str());
		unique_ptr<SQLCHAR[]> extensionParams = make_unique<SQLCHAR[]>(paramsLength);
		memcpy(extensionParams.get(), cmdLine.c_str(), paramsLength);
		SQLCHAR *extensionPath = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_RHomePath.c_str())));
		result = (*m_initFuncPtr)(
			extensionParams.get(),
			paramsLength,
			extensionPath,
			m_RHomePath.length(),
			nullptr,
			0,
			nullptr,
			0);
		ASSERT_EQ(result, SQL_SUCCESS);
	}

	// Name: SetupVariables
	//
	// Description:
	// Set up default, valid variables for use in tests
	//
	void RExtensionApiTest::SetupVariables()
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
				SQL_NULL_DATA, m_IntSize });

		m_logicalInfo = make_unique<ColumnInfo<SQLCHAR>>(
			"LogicalColumn1",
			vector<SQLCHAR>{ '1', '0', '1', '0', '1' },
			vector<SQLINTEGER>(ColumnInfo<SQLCHAR>::sm_rowsNumber, m_LogicalSize),
			"LogicalColumn2",
			vector<SQLCHAR>{ '\0', '2', '1', '0',
				'\0' }, // static_cast from NA_LOGICAL to SQLCHAR is '\0'.
			vector<SQLINTEGER>{ SQL_NULL_DATA, m_LogicalSize, m_LogicalSize,
				m_LogicalSize, SQL_NULL_DATA });

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
				SQL_NULL_DATA, m_DoubleSize });

		m_doubleInfo = make_unique<ColumnInfo<SQLDOUBLE>>(
			"DoubleColumn1",
			vector<SQLDOUBLE>{ -1.79e301, 1.33, 83.98, 72.45, 1.79e30 },
			vector<SQLINTEGER>(ColumnInfo<SQLDOUBLE>::sm_rowsNumber, m_DoubleSize),
			"DoubleColumn2",
			vector<SQLDOUBLE>{ NAN, m_MaxDouble, NAN, m_MinDouble, NAN },
			vector<SQLINTEGER>{ SQL_NULL_DATA, m_DoubleSize, SQL_NULL_DATA,
				m_DoubleSize, SQL_NULL_DATA });

		m_bigIntInfo = make_unique<ColumnInfo<SQLBIGINT>>(
			"BigIntColumn1",
			vector<SQLBIGINT>{ m_MaxBigInt, 1,
				88883939, m_MinBigInt, -622280108 },
			vector<SQLINTEGER>(ColumnInfo<SQLBIGINT>::sm_rowsNumber, m_DoubleSize),
			"BigIntColumn2",
			vector<SQLBIGINT>(ColumnInfo<SQLBIGINT>::sm_rowsNumber, NA_REAL),
			vector<SQLINTEGER>{ SQL_NULL_DATA, SQL_NULL_DATA,
				SQL_NULL_DATA, SQL_NULL_DATA, SQL_NULL_DATA });

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
			vector<SQLINTEGER>(ColumnInfo<SQLSMALLINT>::sm_rowsNumber, m_IntSize));

		m_tinyIntInfo = make_unique<ColumnInfo<SQLCHAR>>(
			"TinyIntColumn1",
			vector<SQLCHAR>{ 34, 133, 98, 72, 10 },
			vector<SQLINTEGER>(ColumnInfo<SQLCHAR>::sm_rowsNumber, m_IntSize),
			"TinyIntColumn2",
			vector<SQLCHAR>{ m_MaxTinyInt, m_MinTinyInt, 1, 0, 128 },
			vector<SQLINTEGER>{ m_IntSize, SQL_NULL_DATA,
				SQL_NULL_DATA, SQL_NULL_DATA, m_IntSize });

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
			SQL_NULL_DATA, SQL_NULL_DATA, SQL_NULL_DATA });

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
			SQL_NULL_DATA, SQL_NULL_DATA, SQL_NULL_DATA });

		// Retrieve the global environment
		//
		m_globalEnvironment = Rcpp::Environment::global_env();
	}

	// Name: CleanupVariables
	//
	// Description:
	// Delete the memory allocated to default variables.
	//
	void RExtensionApiTest::CleanupVariables()
	{
		if (m_sessionId != nullptr)
		{
			delete m_sessionId;
		}
	}

	// Name: ReleaseHandles
	//
	// Description:
	// Close the handle to the library.
	//
	void RExtensionApiTest::ReleaseHandles()
	{
		std::cout << "Closing the library handle.\n";
		Utilities::CrossPlatCloseLibrary(m_libHandle);
	}

	// Name: DoCleanup
	//
	// Description:
	// Call Cleanup on the RExtension.
	// Testing if Cleanup is implemented correctly.
	//
	void RExtensionApiTest::DoCleanup()
	{
		std::cout << "Calling Cleanup.\n";
		SQLRETURN result = SQL_ERROR;
		result = (*m_cleanupFuncPtr)();
		ASSERT_EQ(result, SQL_SUCCESS);
	}

	// Name: InitializeSession
	//
	// Description:
	// Initialize a valid, default session for later tests
	//
	void RExtensionApiTest::InitializeSession(
		SQLUSMALLINT inputSchemaColumnsNumber,
		string       scriptString,
		SQLUSMALLINT parametersNumber)
	{
		SQLRETURN result = SQL_SUCCESS;

		SQLCHAR *script = static_cast<SQLCHAR*>(
			static_cast<void*>(const_cast<char*>(scriptString.c_str())));

		result = (*m_initSessionFuncPtr)(
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

	// Name: CleanupSession
	//
	// Description:
	// Cleanup a valid, default session for later tests
	//
	void RExtensionApiTest::CleanupSession()
	{
		SQLRETURN result = SQL_SUCCESS;

		result = (*m_cleanupSessionFuncPtr)(
			*m_sessionId,
			m_taskId);

		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Name: InitializeColumns
	//
	// Description:
	// Templatized function to call InitializeColumn for all columns in ColumnInfo.
	//
	template<class SQLType, SQLSMALLINT dataType>
	void RExtensionApiTest::InitializeColumns(ColumnInfo<SQLType> *ColumnInfo)
	{
		SQLUSMALLINT inputSchemaColumnsNumber = ColumnInfo->GetColumnsNumber();
		for (SQLUSMALLINT columnNumber = 0; columnNumber < inputSchemaColumnsNumber; ++columnNumber)
		{
			InitializeColumn(columnNumber,
				ColumnInfo->m_columnNames[columnNumber],
				dataType,
				sizeof(SQLType));
		}
	}

	// Template instantiations
	//
	template void RExtensionApiTest::InitializeColumns<SQLINTEGER, SQL_C_SLONG>(
		ColumnInfo<SQLINTEGER> *ColumnInfo);
	template void RExtensionApiTest::InitializeColumns<SQLCHAR, SQL_C_BIT>(
		ColumnInfo<SQLCHAR> *ColumnInfo);
	template void RExtensionApiTest::InitializeColumns<SQLREAL, SQL_C_FLOAT>(
		ColumnInfo<SQLREAL> *ColumnInfo);
	template void RExtensionApiTest::InitializeColumns<SQLDOUBLE, SQL_C_DOUBLE>(
		ColumnInfo<SQLDOUBLE> *ColumnInfo);
	template void RExtensionApiTest::InitializeColumns<SQLBIGINT, SQL_C_SBIGINT>(
		ColumnInfo<SQLBIGINT> *ColumnInfo);
	template void RExtensionApiTest::InitializeColumns<SQLSMALLINT, SQL_C_SSHORT>(
		ColumnInfo<SQLSMALLINT> *ColumnInfo);
	template void RExtensionApiTest::InitializeColumns<SQLCHAR, SQL_C_UTINYINT>(
		ColumnInfo<SQLCHAR> *ColumnInfo);
	template void RExtensionApiTest::InitializeColumns<SQL_DATE_STRUCT, SQL_C_TYPE_DATE>(
		ColumnInfo<SQL_DATE_STRUCT> *ColumnInfo);
	template void RExtensionApiTest::InitializeColumns<SQL_TIMESTAMP_STRUCT, SQL_C_TYPE_TIMESTAMP>(
		ColumnInfo<SQL_TIMESTAMP_STRUCT> *ColumnInfo);

	// Name: InitializeColumn
	//
	// Description:
	// Call InitColumn for the given columnNumber, columnName, dataType and columnSize.
	//
	void RExtensionApiTest::InitializeColumn(
		SQLSMALLINT columnNumber,
		string      columnNameString,
		SQLSMALLINT dataType,
		SQLULEN     columnSize)
	{
		SQLCHAR *columnName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(columnNameString.c_str()))
			);

		SQLRETURN result = SQL_ERROR;

		result = (*m_initColumnFuncPtr)(
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

	// Name: GenerateContiguousData
	//
	// Description:
	//  Fill a contiguous array columnData with members from the given columnVector
	//  having lengths defined in strLenOrInd, unless it is SQL_NULL_DATA.
	//
	template<class SQLType>
	vector<SQLType> RExtensionApiTest::GenerateContiguousData(
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
	template vector<char> RExtensionApiTest::GenerateContiguousData(
		vector<const char*> columnVector,
		SQLINTEGER          *strLenOrInd);
	template vector<SQLCHAR> RExtensionApiTest::GenerateContiguousData(
		vector<const SQLCHAR*> columnVector,
		SQLINTEGER             *strLenOrInd);
	template vector<wchar_t> RExtensionApiTest::GenerateContiguousData(
		vector<const wchar_t*> columnVector,
		SQLINTEGER             *strLenOrInd);

	// Name: CheckVectorEquality
	//
	// Description:
	// Templatized function to compare the given vector and data for equality.
	// for integer/numeric/logical data types.
	// The expectedData is input as a void*, hence we input the expectedRowsNumber as well.
	// Where strLen_or_Ind == SQL_NULL_DATA, check for is_na.
	//
	template<class SQLType, class RType, SQLSMALLINT dataType>
	void RExtensionApiTest::CheckVectorEquality(
		SQLULEN    expectedRowsNumber,
		RType      vectorToTest,
		void       *expectedData,
		SQLINTEGER *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<size_t>(vectorToTest.size()), expectedRowsNumber);
		for(SQLULEN index = 0 ; index < expectedRowsNumber; ++index)
		{
			if (strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA)
			{
				EXPECT_TRUE(RType::is_na(vectorToTest[index]));
			}
			else
			{
				SQLType expectedValue = static_cast<SQLType*>(expectedData)[index];
				if (dataType == SQL_C_BIT)
				{
					EXPECT_EQ(vectorToTest[index],
						expectedValue != '0' ? true : false);
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
	template void RExtensionApiTest::CheckVectorEquality<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
		SQLULEN              expectedRowsNumber,
		Rcpp::IntegerVector  vectorToTest,
		void                 *expectedData,
		SQLINTEGER           *strLenOrInd);
	template void RExtensionApiTest::CheckVectorEquality<SQLCHAR, Rcpp::LogicalVector, SQL_C_BIT>(
		SQLULEN             expectedRowsNumber,
		Rcpp::LogicalVector vectorToTest,
		void                *expectedData,
		SQLINTEGER          *strLen_or_Ind);
	template void RExtensionApiTest::CheckVectorEquality<SQLREAL, Rcpp::NumericVector, SQL_C_FLOAT>(
		SQLULEN             expectedRowsNumber,
		Rcpp::NumericVector vectorToTest,
		void                *expectedData,
		SQLINTEGER          *strLen_or_Ind);
	template void RExtensionApiTest::CheckVectorEquality<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
		SQLULEN             expectedRowsNumber,
		Rcpp::NumericVector vectorToTest,
		void                *expectedData,
		SQLINTEGER          *strLen_or_Ind);
	template void RExtensionApiTest::CheckVectorEquality<SQLBIGINT, Rcpp::NumericVector, SQL_C_SBIGINT>(
		SQLULEN             expectedRowsNumber,
		Rcpp::NumericVector vectorToTest,
		void                *expectedData,
		SQLINTEGER          *strLen_or_Ind);
	template void RExtensionApiTest::CheckVectorEquality<SQLSMALLINT, Rcpp::IntegerVector, SQL_C_SSHORT>(
		SQLULEN             expectedRowsNumber,
		Rcpp::IntegerVector vectorToTest,
		void                *expectedData,
		SQLINTEGER          *strLen_or_Ind);
	template void RExtensionApiTest::CheckVectorEquality<SQLCHAR, Rcpp::IntegerVector, SQL_C_UTINYINT>(
		SQLULEN             expectedRowsNumber,
		Rcpp::IntegerVector vectorToTest,
		void                *expectedData,
		SQLINTEGER          *strLen_or_Ind);

	// Name: CheckCharacterVectorEquality
	//
	// Description:
	// Compare character vector with the given data and corresponding strLen_or_Ind.
	// The expectedData is input as a void*, hence we input the expectedRowsNumber as well.
	// Where strLen_or_Ind == SQL_NULL_DATA, check for is_na.
	//
	template<class CharType>
	void RExtensionApiTest::CheckCharacterVectorEquality(
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
	template void RExtensionApiTest::CheckCharacterVectorEquality<char>(
		SQLULEN               expectedRowsNumber,
		Rcpp::CharacterVector vectorToTest,
		void                  *expectedData,
		SQLINTEGER            *strLen_or_Ind);

	template void RExtensionApiTest::CheckCharacterVectorEquality<wchar_t>(
		SQLULEN               expectedRowsNumber,
		Rcpp::CharacterVector vectorToTest,
		void                  *expectedData,
		SQLINTEGER            *strLen_or_Ind);

	// Name: CheckDateTimeVectorEquality
	//
	// Description:
	// Compare the given R Date(time) vector and data for equality
	//
	template<class SQLType, class RType, class DateTimeTypeInR>
	void RExtensionApiTest::CheckDateTimeVectorEquality(
		SQLULEN    expectedRowsNumber,
		RType      vectorToTest,
		void       *expectedData,
		SQLINTEGER *strLen_or_Ind)
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
	template void RExtensionApiTest::CheckDateTimeVectorEquality
		<SQL_DATE_STRUCT, Rcpp::DateVector, Rcpp::Date>(
			SQLULEN          expectedRowsNumber,
			Rcpp::DateVector vectorToTest,
			void             *expectedData,
			SQLINTEGER       *strLen_or_Ind);

	template void RExtensionApiTest::CheckDateTimeVectorEquality
		<SQL_TIMESTAMP_STRUCT, Rcpp::DatetimeVector, Rcpp::Datetime>(
			SQLULEN              expectedRowsNumber,
			Rcpp::DatetimeVector vectorToTest,
			void                 *expectedData,
			SQLINTEGER           *strLen_or_Ind);

	// Name: ColumnInfo
	//
	// Description:
	// Templetized constructor for the type information.
	// Useful for ColumnInfo of integer, basic numeric and logical types.
	//
	template<class SQLType>
	ColumnInfo<SQLType>::ColumnInfo(
		string column1Name, vector<SQLType> column1, vector<SQLINTEGER> col1StrLenOrInd,
		string column2Name, vector<SQLType> column2, vector<SQLINTEGER> col2StrLenOrInd)
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

	}
}
