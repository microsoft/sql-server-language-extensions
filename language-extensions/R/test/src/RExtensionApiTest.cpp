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
#ifdef _WIN64
#include <windows.h>
#endif

#include <memory>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"

#include "Rcpp.h"

#include "RExtensionApiTest.h"
#include "Utilities.h"

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
	FN_cleanupSession *RExtensionApiTest::m_cleanupSessionFuncPtr = nullptr;
	FN_cleanup *RExtensionApiTest::m_cleanupFuncPtr = nullptr;

	// Per-test-suite set-up.
	// Called before the first test in this test suite.
	//
	void RExtensionApiTest::SetUpTestSuite()
	{
		CheckAndSetRHome();
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
		CleanupVariables();
		CleanupSession();
	}

	// Name: CheckAndSetRHome()
	//
	// Description:
	// Check R_HOME and set it if not defined.
	//
	void RExtensionApiTest::CheckAndSetRHome()
	{
		std::cout << "Checking and setting R_HOME.\n";
		char *RHome = getenv("R_HOME");

		// If RHome is not defined it could be passed as the first argument to the test executable
		// In terms of gtest, the first argument is the name of the test executable
		// and RHome then becomes the second argument.
		//
		if (RHome == nullptr && g_argc >= 2)
		{
			RHome = const_cast<char *>(g_argv[1]);
			putenv(RHome);
		}
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
		unique_ptr<SQLCHAR> extensionParams(new SQLCHAR[paramsLength]);
		memcpy(extensionParams.get(), cmdLine.c_str(), paramsLength);
		result = (*m_initFuncPtr)(
			extensionParams.get(),
			paramsLength,
			nullptr,
			0,
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
		m_parametersNumber = 1;

		m_paramNameString = "@param1";
		m_paramName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_paramNameString.c_str()))
			);

		m_columnNameString = "Column1";
		m_columnName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_columnNameString.c_str()))
			);

		m_scriptString = "print('" + m_printMessage + "');"
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

		const SQLINTEGER intSize = sizeof(SQLINTEGER);
		m_integerInfo = make_unique<ColumnInfo<SQLINTEGER>>(
			"IntegerColumn1",
			vector<SQLINTEGER>{ 1, 2, 3, 4, 5 },
			vector<SQLINTEGER>(ColumnInfo<SQLINTEGER>::m_rowsNumber, intSize),
			"IntegerColumn2",
			vector<SQLINTEGER>{ 2'147'483'647, -2'147'483'647, 0, -2'147'483'648, -1 },
			vector<SQLINTEGER>{ intSize, intSize, SQL_NULL_DATA,
				SQL_NULL_DATA, intSize });

		const SQLINTEGER logicalSize = sizeof(SQLCHAR);
		m_logicalInfo = make_unique<ColumnInfo<SQLCHAR>>(
			"LogicalColumn1",
			vector<SQLCHAR>{ '1', '0', '1', '0', '1' },
			vector<SQLINTEGER>(ColumnInfo<SQLCHAR>::m_rowsNumber, logicalSize),
			"LogicalColumn2",
			vector<SQLCHAR>{ '0', '2', '1', '0', '1' },
			vector<SQLINTEGER>{ SQL_NULL_DATA, logicalSize, logicalSize,
				logicalSize, SQL_NULL_DATA });

		const SQLINTEGER doubleSize = sizeof(SQLDOUBLE);

		// the input column information set here is the same used to test the expected output
		// column results; real/bigInt input columns are returned back as double
		// output columns and so the strLenInd values are set to doubleSize.
		//
		m_realInfo = make_unique<ColumnInfo<SQLREAL>>(
			"RealColumn1",
			vector<SQLREAL>{ 0.34, 1.33, 83.98, 72.45, 68e10 },
			vector<SQLINTEGER>(ColumnInfo<SQLREAL>::m_rowsNumber, doubleSize),
			"RealColumn2",
			vector<SQLREAL>{ 3.4e38F, 0, -3.4e38F, -1, 0 },
			vector<SQLINTEGER>{ doubleSize, SQL_NULL_DATA, doubleSize,
				SQL_NULL_DATA, doubleSize });

		m_doubleInfo = make_unique<ColumnInfo<SQLDOUBLE>>(
			"DoubleColumn1",
			vector<SQLDOUBLE>{ -1.79e301, 1.33, 83.98, 72.45, 1.79e30 },
			vector<SQLINTEGER>(ColumnInfo<SQLDOUBLE>::m_rowsNumber, doubleSize),
			"DoubleColumn2",
			vector<SQLDOUBLE>{ 0, 1.79e308, 0, -1.79e308, -1 },
			vector<SQLINTEGER>{ SQL_NULL_DATA, doubleSize, SQL_NULL_DATA,
				doubleSize, SQL_NULL_DATA });

		m_bigIntInfo = make_unique<ColumnInfo<SQLBIGINT>>(
			"BigIntColumn1",
			vector<SQLBIGINT>{ 9'223'372'036'854'775'807LL, 1,
				88883939, -9'223'372'036'854'775'807LL, -622280108 },
			vector<SQLINTEGER>(ColumnInfo<SQLBIGINT>::m_rowsNumber, doubleSize),
			"BigIntColumn2",
			vector<SQLBIGINT>(ColumnInfo<SQLBIGINT>::m_rowsNumber, 0),
			vector<SQLINTEGER>{ SQL_NULL_DATA, SQL_NULL_DATA,
				SQL_NULL_DATA, SQL_NULL_DATA, SQL_NULL_DATA });

		// the input column information set here is the same used to test the
		// expected output column results; smallint/tinyint input columns are returned back as
		// integer output columns and so the strLenInd values here are set to intSize.
		//
		m_smallIntInfo = make_unique<ColumnInfo<SQLSMALLINT>>(
			"SmallIntColumn1",
			vector<SQLSMALLINT>{ 223, 33, 9811, -725, 6810 },
			vector<SQLINTEGER>(ColumnInfo<SQLSMALLINT>::m_rowsNumber, intSize),
			"SmallIntColumn2",
			vector<SQLSMALLINT>{ -1, 0, 32'767, -32'768, 3'276 },
			vector<SQLINTEGER>(ColumnInfo<SQLSMALLINT>::m_rowsNumber, intSize));

		m_tinyIntInfo = make_unique<ColumnInfo<SQLCHAR>>(
			"TinyIntColumn1",
			vector<SQLCHAR>{ 34, 133, 98, 72, 10 },
			vector<SQLINTEGER>(ColumnInfo<SQLCHAR>::m_rowsNumber, intSize),
			"TinyIntColumn2",
			vector<SQLCHAR>{ 255, 0, 1, 0, 128 },
			vector<SQLINTEGER>{ intSize, SQL_NULL_DATA,
				SQL_NULL_DATA, SQL_NULL_DATA, intSize });

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
		SQLCHAR      *script,
		SQLULEN      scriptStringLength)
	{
		SQLRETURN result = SQL_SUCCESS;

		result = (*m_initSessionFuncPtr)(
			*m_sessionId,
			m_taskId,
			m_numTasks,
			script,
			scriptStringLength,
			inputSchemaColumnsNumber,
			m_parametersNumber,
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
	// Fill a contiguous array columnData with members from the given columnVector
	// having lengths defined in strLenOrInd, unless it is SQL_NULL_DATA.
	//
	void RExtensionApiTest::GenerateContiguousData(
		char                *columnData,
		vector<const char*> columnVector,
		SQLINTEGER          *strLenOrInd)
	{
		SQLINTEGER baseIndex = 0;
		for (SQLULEN index = 0 ; index < columnVector.size(); index++)
		{
			if (strLenOrInd[index] != SQL_NULL_DATA)
			{
				memcpy(columnData + baseIndex, columnVector[index], strLenOrInd[index]);
				baseIndex += strLenOrInd[index];
			}
		}
	}

	// Name: GetSumOfLengths
	//
	// Description:
	// Get sum of lengths in strLenOrInd skipping SQL_NULL_DATA.
	//
	SQLINTEGER RExtensionApiTest::GetSumOfLengths(
		SQLINTEGER *strLenOrInd,
		SQLULEN    rowsNumber,
		SQLINTEGER *maxLen)
	{
		SQLINTEGER sumOfLengths = 0;
		for(SQLULEN index = 0 ; index < rowsNumber; index++)
		{
			if(strLenOrInd[index] != SQL_NULL_DATA)
			{
				sumOfLengths += strLenOrInd[index];

				if (maxLen != nullptr && *maxLen < strLenOrInd[index])
				{
					*maxLen = strLenOrInd[index];
				}
			}
		}

		return sumOfLengths;
	}

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
		for(SQLULEN index = 0 ; index < expectedRowsNumber; index++)
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
		SQLULEN              expectedRowsNumber,
		Rcpp::IntegerVector vectorToTest,
		void                 *expectedData,
		SQLINTEGER           *strLen_or_Ind);

	// Name: CheckCharacterVectorEquality
	//
	// Description:
	// Compare character vector with the given data and corresponding strLen_or_Ind.
	// The expectedData is input as a void*, hence we input the expectedRowsNumber as well.
	// Where strLen_or_Ind == SQL_NULL_DATA, check for is_na.
	//
	void RExtensionApiTest::CheckCharacterVectorEquality(
		SQLULEN               expectedRowsNumber,
		Rcpp::CharacterVector vectorToTest,
		void                  *expectedData,
		SQLINTEGER            *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<size_t>(vectorToTest.size()), expectedRowsNumber);
		SQLINTEGER cumulativeLength = 0 ;
		for(SQLULEN index = 0 ; index < expectedRowsNumber; index++)
		{
			if (strLen_or_Ind == nullptr ||
			  (strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA))
			{
				EXPECT_TRUE(Rcpp::CharacterVector::is_na(vectorToTest[index]));
			}
			else
			{
				string expectedString = string(
					static_cast<char*>(expectedData) + cumulativeLength,
					strLen_or_Ind[index]);
				EXPECT_EQ(vectorToTest[index], expectedString);
				cumulativeLength += strLen_or_Ind[index];
			}
		}
	}

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
