//**************************************************************************************************
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
// @File: RExtensionApiGetOutputParamTests.cpp
//
// Purpose:
//  Tests the RExtension's implementation of the external language GetOutputParam API.
//
//**************************************************************************************************

#include "Common.h"

using namespace std;

namespace ExtensionApiTest
{
	//----------------------------------------------------------------------------------------------
	// Name: GetIntegerOutputParamTest
	//
	// Description:
	//  Tests multiple SQLINTEGER values.
	//
	TEST_F(RExtensionApiTests, GetIntegerOutputParamTest)
	{
		string scriptString = "param1 <- as.integer(" + to_string(m_MaxInt) + ");"
			"param2 <- as.integer(" + to_string(m_MinInt) + ");"
			"param3 <- c(as.integer(5), as.integer(4));"
			"param4 <- as.integer(NA);"
			"param5 <- as.integer()";

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			0,   // inputSchemaColumnsNumber
			scriptString,
			5);  // parametersNumber

		vector<shared_ptr<SQLINTEGER>> initParamValues = {
			make_shared<SQLINTEGER>(0),
			make_shared<SQLINTEGER>(0),
			make_shared<SQLINTEGER>(0),
			make_shared<SQLINTEGER>(0),
			nullptr };
		vector<SQLINTEGER> strLenOrInd = { 0, 0, 0, 0, SQL_NULL_DATA};
		vector<SQLSMALLINT> inputOutputTypes(initParamValues.size(), SQL_PARAM_INPUT_OUTPUT);

		InitParam<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
			initParamValues,
			strLenOrInd,
			inputOutputTypes,
			false);  // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,       // rowsNumber
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		// Test max INT value, min INT value with respect to R,
		// a normal value, NA value and
		// a NULL INT value.
		//
		vector<shared_ptr<SQLINTEGER>> expectedParamValues = {
			make_shared<SQLINTEGER>(m_MaxInt),
			make_shared<SQLINTEGER>(m_MinInt),
			make_shared<SQLINTEGER>(5),
			make_shared<SQLINTEGER>(R_NaInt),
			nullptr };
		vector<SQLINTEGER> expectedStrLenOrInd = { m_IntSize, m_IntSize, m_IntSize,
			SQL_NULL_DATA, SQL_NULL_DATA};

		GetOutputParam<SQLINTEGER>(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetLogicalOutputParamTest
	//
	// Description:
	//  Tests multiple logical (bit) values.
	//
	TEST_F(RExtensionApiTests, GetLogicalOutputParamTest)
	{
		int parametersNumber = 7;
		string scriptString = "param1 <- TRUE;"
			"param2 <- FALSE;"
			"param3 <- NA;"
			// param4 is FALSE
			//
			"param4 <- as.logical(0);"
			// param5 is TRUE
			//
			"param5 <- as.logical(2);"
			// param6 is NA
			//
			"param6 <- as.logical('1');"
			"param7 <- as.logical();";

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			0,   // inputSchemaColumnsNumber
			scriptString,
			parametersNumber);  // parametersNumber

		vector<shared_ptr<SQLCHAR>> initParamValues = {
			make_shared<SQLCHAR>(0),
			make_shared<SQLCHAR>(0),
			make_shared<SQLCHAR>(0),
			make_shared<SQLCHAR>(0),
			make_shared<SQLCHAR>(0),
			make_shared<SQLCHAR>(0),
			nullptr };
		vector<SQLINTEGER> strLenOrInd(parametersNumber, 0);
		strLenOrInd[parametersNumber - 1] = SQL_NULL_DATA;
		vector<SQLSMALLINT> inputOutputTypes(parametersNumber, SQL_PARAM_INPUT_OUTPUT);

		InitParam<SQLCHAR, Rcpp::LogicalVector, SQL_C_BIT>(
			initParamValues,
			strLenOrInd,
			inputOutputTypes,
			false);  // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,       // rowsNumber
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		// Test '1', '0', NA, 0, >1 and NULL BIT values.
		// When testing out of range NA value with respect to R,
		// returned paramValue is '\0' even though parameter is NA in R
		// since type casting NA (which is -2'147'483'648) to SQLCHAR returns \0.
		//
		vector<shared_ptr<SQLCHAR>> expectedParamValues = {
			// TRUE
			//
			make_shared<SQLCHAR>(1),
			// FALSE
			//
			make_shared<SQLCHAR>(0),
			// NA
			//
			make_shared<SQLCHAR>('\0'),
			// as.logical(0)
			//
			make_shared<SQLCHAR>(0),
			// as.logical(2)
			//
			make_shared<SQLCHAR>(1),
			// as.logical('1') - NA in R
			//
			make_shared<SQLCHAR>('\0'),
			nullptr };
		vector<SQLINTEGER> expectedStrLenOrInd = {
			m_LogicalSize, m_LogicalSize, SQL_NULL_DATA,
			m_LogicalSize, m_LogicalSize, SQL_NULL_DATA, SQL_NULL_DATA };

		GetOutputParam<SQLCHAR>(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetRealOutputParamTest
	//
	// Description:
	//  Tests multiple real values.
	//
	TEST_F(RExtensionApiTests, GetRealOutputParamTest)
	{
		string scriptString = "param1 <- " + to_string(m_MaxReal) + ";"
			"param2 <- " + to_string(m_MinReal) + ";"
			"param3 <- 2.3e4;"
			"param4 <- as.numeric(NA);"
			"param5 <- as.numeric()";

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			0,   // inputSchemaColumnsNumber
			scriptString,
			5);  // parametersNumber

		vector<shared_ptr<SQLREAL>> initParamValues = {
			make_shared<SQLREAL>(0),
			make_shared<SQLREAL>(0),
			make_shared<SQLREAL>(0),
			make_shared<SQLREAL>(0),
			nullptr };
		vector<SQLINTEGER> strLenOrInd = { 0, 0, 0, 0, SQL_NULL_DATA };
		vector<SQLSMALLINT> inputOutputTypes(initParamValues.size(), SQL_PARAM_INPUT_OUTPUT);

		InitParam<SQLREAL, Rcpp::NumericVector, SQL_C_FLOAT>(
			initParamValues,
			strLenOrInd,
			inputOutputTypes,
			false);  // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,       // rowsNumber
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		// Test max FLOAT(24), min FLOAT(24), a normal, NAN and nullptr REAL values
		//
		vector<shared_ptr<SQLREAL>> expectedParamValues = {
			make_shared<SQLREAL>(m_MaxReal),
			make_shared<SQLREAL>(m_MinReal),
			make_shared<SQLREAL>(2.3e4),
			make_shared<SQLREAL>(NAN),
			nullptr };

		vector<SQLINTEGER> expectedStrLenOrInd = { m_RealSize, m_RealSize, m_RealSize,
			SQL_NULL_DATA, SQL_NULL_DATA};
		GetOutputParam<SQLREAL>(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetDoubleOutputParamTest
	//
	// Description:
	//  Tests multiple double values.
	//
	TEST_F(RExtensionApiTests, GetDoubleOutputParamTest)
	{
		string scriptString = "param1 <- " + to_string(m_MaxDouble) + ";"
			"param2 <- " + to_string(m_MinDouble) + ";"
			"param3 <- 1.45e38;"
			"param4 <- as.numeric(NA);"
			"param5 <- as.numeric()";

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			0,   // inputSchemaColumnsNumber
			scriptString,
			5);  // parametersNumber

		vector<shared_ptr<SQLDOUBLE>> initParamValues = {
			make_shared<SQLDOUBLE>(0),
			make_shared<SQLDOUBLE>(0),
			make_shared<SQLDOUBLE>(0),
			make_shared<SQLDOUBLE>(0),
			nullptr };
		vector<SQLINTEGER> strLenOrInd = { 0, 0, 0, 0, SQL_NULL_DATA};
		vector<SQLSMALLINT> inputOutputTypes(initParamValues.size(), SQL_PARAM_INPUT_OUTPUT);

		InitParam<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
			initParamValues,
			strLenOrInd,
			inputOutputTypes,
			false);  // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,       // rowsNumber
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		// Test max FLOAT(53), min FLOAT(53), a normal, NAN and nullptr DOUBLE values
		//
		vector<shared_ptr<SQLDOUBLE>> expectedParamValues = {
			make_shared<SQLDOUBLE>(m_MaxDouble),
			make_shared<SQLDOUBLE>(m_MinDouble),
			make_shared<SQLDOUBLE>(1.45e38),
			make_shared<SQLDOUBLE>(NAN),
			nullptr };
		vector<SQLINTEGER> expectedStrLenOrInd = { m_DoubleSize, m_DoubleSize, m_DoubleSize,
			SQL_NULL_DATA, SQL_NULL_DATA};

		GetOutputParam<SQLDOUBLE>(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetBigIntOutputParamTest
	//
	// Description:
	//  Tests multiple big int values.
	//
	TEST_F(RExtensionApiTests, GetBigIntOutputParamTest)
	{
		string scriptString = "param1 <- " + to_string(m_MaxBigInt) + ";"
			"param2 <- " + to_string(m_MinBigInt) + ";"
			"param3 <- 9372036854775;"
			"param4 <- as.numeric(NA);"
			"param5 <- as.numeric()";

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			0,   // inputSchemaColumnsNumber
			scriptString,
			5);  // parametersNumber

		vector<shared_ptr<SQLBIGINT>> initParamValues = {
			make_shared<SQLBIGINT>(0),
			make_shared<SQLBIGINT>(0),
			make_shared<SQLBIGINT>(0),
			make_shared<SQLBIGINT>(0),
			nullptr };
		vector<SQLINTEGER> strLenOrInd = { 0, 0, 0, 0, SQL_NULL_DATA };
		vector<SQLSMALLINT> inputOutputTypes(initParamValues.size(), SQL_PARAM_INPUT_OUTPUT);

		InitParam<SQLBIGINT, Rcpp::NumericVector, SQL_C_SBIGINT>(
			initParamValues,
			strLenOrInd,
			inputOutputTypes,
			false);  // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,       // rowsNumber
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		// Test max, min, normal, NA and NULL BIGINT values
		//
		vector<shared_ptr<SQLBIGINT>> expectedParamValues = {
			// Even though parameter is assigned max BIGINT value in R i.e. 9.223372e+18,
			// returned paramValue when static type casted to BIGINT is the min value.
			//
			make_shared<SQLBIGINT>(m_MinBigInt),
			make_shared<SQLBIGINT>(m_MinBigInt),
			make_shared<SQLBIGINT>(9'372'036'854'775),
			// Even though NA in R for BIGINT is NA_REAL,
			// typecasting that to a BIGINT value is MinBigInt
			//
			make_shared<SQLBIGINT>(m_MinBigInt),
			nullptr };
		vector<SQLINTEGER> expectedStrLenOrInd = { m_BigIntSize, m_BigIntSize, m_BigIntSize,
			SQL_NULL_DATA, SQL_NULL_DATA };

		GetOutputParam<SQLBIGINT>(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetSmallIntOutputParamTest
	//
	// Description:
	//  Tests multiple small int values.
	//
	TEST_F(RExtensionApiTests, GetSmallIntOutputParamTest)
	{
		string scriptString = "param1 <- as.integer(" + to_string(m_MaxSmallInt) + ");"
			"param2 <- as.integer(" + to_string(m_MinSmallInt) + ");"
			"param3 <- as.integer(0);"
			"param4 <- as.integer(NA);"
			"param5 <- as.integer()";

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			0,   // inputSchemaColumnsNumber
			scriptString,
			5);  // parametersNumber

		vector<shared_ptr<SQLSMALLINT>> initParamValues = {
			make_shared<SQLSMALLINT>(0),
			make_shared<SQLSMALLINT>(0),
			make_shared<SQLSMALLINT>(0),
			make_shared<SQLSMALLINT>(0),
			nullptr };
		vector<SQLINTEGER> strLenOrInd = { 0, 0, 0, 0, SQL_NULL_DATA};
		vector<SQLSMALLINT> inputOutputTypes(initParamValues.size(), SQL_PARAM_INPUT_OUTPUT);

		InitParam<SQLSMALLINT, Rcpp::IntegerVector, SQL_C_SSHORT>(
			initParamValues,
			strLenOrInd,
			inputOutputTypes,
			false);  // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,       // rowsNumber
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		// Test max, min, normal, NA_INTEGER and NULL SMALLINT values
		//
		vector<shared_ptr<SQLSMALLINT>> expectedParamValues = {
			make_shared<SQLSMALLINT>(m_MaxSmallInt),
			make_shared<SQLSMALLINT>(m_MinSmallInt),
			make_shared<SQLSMALLINT>(0),
			// Since NA_INTEGER in R is a bigger value than the type SQLSMALLINT can hold,
			// static type casting leads to a 0 value. But this is differentiated by SQL_NULL_DATA.
			//
			make_shared<SQLSMALLINT>(0),
			nullptr };
		vector<SQLINTEGER> expectedStrLenOrInd = { m_SmallIntSize, m_SmallIntSize, m_SmallIntSize,
			SQL_NULL_DATA, SQL_NULL_DATA};

		GetOutputParam<SQLSMALLINT>(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetTinyIntOutputParamTest
	//
	// Description:
	//  Tests multiple tiny int values.
	//
	TEST_F(RExtensionApiTests, GetTinyIntOutputParamTest)
	{
		string scriptString = "param1 <- as.integer(" + to_string(m_MaxTinyInt) + ");"
			"param2 <- as.integer(" + to_string(m_MinTinyInt) + ");"
			"param3 <- as.integer(123);"
			"param4 <- as.integer(NA);"
			"param5 <- as.integer();"
			"param6 <- as.integer(-1)";

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			0,   // inputSchemaColumnsNumber
			scriptString,
			6);  // parametersNumber

		vector<shared_ptr<SQLCHAR>> initParamValues = {
			make_shared<SQLCHAR>(0),
			make_shared<SQLCHAR>(0),
			make_shared<SQLCHAR>(0),
			make_shared<SQLCHAR>(0),
			nullptr,
			make_shared<SQLCHAR>(0)};
		vector<SQLINTEGER> strLenOrInd = { 0, 0, 0, 0, SQL_NULL_DATA, 0 };
		vector<SQLSMALLINT> inputOutputTypes(initParamValues.size(), SQL_PARAM_INPUT_OUTPUT);

		InitParam<SQLCHAR, Rcpp::IntegerVector, SQL_C_UTINYINT>(
			initParamValues,
			strLenOrInd,
			inputOutputTypes,
			false);  // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,       // rowsNumber
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		// Test max, min, normal, NA, NULL, (-1) overflowing to max TINYINT values
		//
		vector<shared_ptr<SQLCHAR>> expectedParamValues = {
			make_shared<SQLCHAR>(m_MaxTinyInt),
			make_shared<SQLCHAR>(m_MinTinyInt),
			make_shared<SQLCHAR>(123),
			// When value is NA, expectedParamValue is type casted to '\0'.
			// even though in R it is NA_INTEGER which is larger than what SQLCHAR can hold.
			//
			make_shared<SQLCHAR>('\0'),
			nullptr,
			make_shared<SQLCHAR>(m_MaxTinyInt)};
		vector<SQLINTEGER> expectedStrLenOrInd = { m_TinyIntSize, m_TinyIntSize, m_TinyIntSize,
			SQL_NULL_DATA, SQL_NULL_DATA, m_TinyIntSize };

		GetOutputParam<SQLCHAR>(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetCharOutputParamTest
	//
	// Description:
	//  Tests multiple character values.
	//
	TEST_F(RExtensionApiTests, GetCharOutputParamTest)
	{
		int paramsNumber = 7;

		// Construct value greater than USHRT_MAX to test truncating behavior
		//
		string value5 = CreateInputSizeRandomStr(m_ValueLargerThanUShrtMax);

		string scriptString = "param1 <- 'HELLO';"
			"param2 <- 'RExtension';"
			"param3 <- '';"
			"param4 <- 'WORLD';"
			"param5 <- '" + value5 + "';"
			"param6 <- as.character(NA);"
			"param7 <- as.character();";

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			0,   // inputSchemaColumnsNumber
			scriptString,
			paramsNumber);

		vector<const char*> initParamValues(paramsNumber, "");
		vector<SQLULEN> paramSizes = { 5, 6, 5, 10, USHRT_MAX, 5, 5 };

		// Note: The behavior of fixed and varying character types is same when it comes to output
		// parameters. So it doesn't matter if we initialize these output parameters as fixed type.
		//
		vector<bool> isFixedType = { true, false, true, false, false, true, true };
		vector<SQLSMALLINT> inputOutputTypes(paramsNumber, SQL_PARAM_INPUT_OUTPUT);

		InitCharParam<char, SQL_C_CHAR>(
			initParamValues,
			paramSizes,
			isFixedType,
			inputOutputTypes,
			false); // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,       // rowsNumber
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);
		const vector<string> ExpectedParamValueStrings = {
			// Test simple CHAR(5) value with exact string length as the type allows i.e. here 5.
			//
			"HELLO",
			// Test VARCHAR(6) value with string length more than the type allows - expected truncation.
			// Above R script sets the parameter to "RExtension" but we only expect "RExten".
			//
			"RExten",
			// Test a 0 length string
			//
			"",
			// Test CHAR(10) value with string length less than the type allows.
			//
			"WORLD",
			// Test VARCHAR(max) with string length of 128000.
			//	
			value5};

		vector<SQLCHAR*> expectedParamValues = {
			static_cast<SQLCHAR*>(
				static_cast<void*>(const_cast<char *>(ExpectedParamValueStrings[0].c_str()))),
			static_cast<SQLCHAR*>(
				static_cast<void*>(const_cast<char *>(ExpectedParamValueStrings[1].c_str()))),
			static_cast<SQLCHAR*>(
				static_cast<void*>(const_cast<char *>(ExpectedParamValueStrings[2].c_str()))),
			static_cast<SQLCHAR*>(
				static_cast<void*>(const_cast<char *>(ExpectedParamValueStrings[3].c_str()))),
			static_cast<SQLCHAR*>(
				static_cast<void*>(const_cast<char *>(ExpectedParamValueStrings[4].c_str()))),
			// Test NA returned in a VARCHAR(5) parameter.
			//
			nullptr,
			// Test nullptr CHAR(5) value.
			//
			nullptr };

		vector<SQLINTEGER> expectedStrLenOrInd = {
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[0].length()),
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[1].length()),
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[2].length()),
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[3].length()),
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[4].length()),
			SQL_NULL_DATA,
			SQL_NULL_DATA};


		GetCharOutputParam(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetNCharOutputParamTest
	//
	// Description:
	//  Tests multiple nchar/nvarchar values.
	//
	TEST_F(RExtensionApiTests, GetNCharOutputParamTest)
	{
		int paramsNumber = 8;

		// Construct the values that correspond to 你好
		//
		vector<char> chineseString = { -28, -67, -96, -27, -91, -67 };
		string dummystring = string(chineseString.data(), 6);

		string scriptString = "param1 <- 'HELLO';"
			"param2 <- 'RExtension';"
			"param3 <- '';"
			"param4 <- 'WORLD';"
			"param5 <- '" + dummystring + "';"
			"param6 <- 'Mix " + dummystring + "';"
			"param7 <- as.character(NA);"
			"param8 <- as.character();";

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			0,  // inputSchemaColumnsNumber
			scriptString,
			paramsNumber);

		// Note: The behavior of fixed and varying character types is same when it comes to output
		// parameters. So it doesn't matter if they are initialized as fixed or varying.
		//
		vector<bool> isFixedType = { true, false, true, true, true, false, true, true };
		vector<SQLULEN> paramSizes = { 5, 6, 5, 10, 2, 6, 5, 5 };
		vector<const wchar_t*> initParamValues(paramsNumber, L"");
		vector<SQLSMALLINT> inputOutputTypes(paramsNumber, SQL_PARAM_INPUT_OUTPUT);

		InitCharParam<wchar_t, SQL_C_WCHAR>(
			initParamValues,
			paramSizes,
			isFixedType,
			inputOutputTypes,
			false);           // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,       // rowsNumber
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		vector<const wchar_t*> expectedParamValues = {
			// Test simple NCHAR(5) value with exact string length as the type allows i.e. here 5.
			//
			L"HELLO",
			// Test NVARCHAR(6) value with string length more than the type allows - expected truncation.
			// Above python script sets the parameter to "RExtension" but we only expect "RExten".
			//
			L"RExten",
			// Test a 0 length string
			//
			L"",
			// Test NCHAR(10) value with string length less than the type allows.
			//
			L"WORLD",
			// Test a Unicode string
			//
			L"你好",
			// Test a mixture of one-byte and multi-byte Unicode string
			//
			L"Mix 你好",
			nullptr,
			nullptr};

		vector<SQLINTEGER> expectedStrLenOrInd = {
			static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
			static_cast<SQLINTEGER>(6 * sizeof(wchar_t)),
			static_cast<SQLINTEGER>(0 * sizeof(wchar_t)),
			static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
			static_cast<SQLINTEGER>(2 * sizeof(wchar_t)),
			static_cast<SQLINTEGER>(6 * sizeof(wchar_t)),
			SQL_NULL_DATA,
			SQL_NULL_DATA
		};

		GetNCharOutputParam(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetRawOutputParamTest
	//
	// Description:
	//  Tests multiple binary values.
	//
	TEST_F(RExtensionApiTests, GetRawOutputParamTest)
	{
		int paramsNumber = 6;

		// Construct values greater than USHRT_MAX to test truncating behavior
		//
		vector<SQLCHAR> BinaryValue5(m_ValueLargerThanUShrtMax, 0x01);
		string value5 = CreateInputSizeBinaryStringR(m_ValueLargerThanUShrtMax);
		string scriptString = "param1 <- c(as.raw(0x00), as.raw(0x01), as.raw(0xe2), as.raw(0x40));"
			"param2 <- charToRaw(paste(letters[1:10], collapse=''));"
			"param3 <- c(as.raw(0x01), as.raw(0x3f));"
			"param4 <- as.raw(NA);"
			"param5 <- c(" + value5 + ");"
			"param6 <- raw();";

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			0,   // inputSchemaColumnsNumber
			scriptString,
			paramsNumber);

		const vector<SQLCHAR> dummyValue = { };
		vector<SQLCHAR*> initParamValues(paramsNumber, const_cast<SQLCHAR*>(dummyValue.data()));
		vector<SQLINTEGER> strLenOrInd(paramsNumber, static_cast<SQLINTEGER>(dummyValue.size())/m_BinarySize);

		vector<SQLULEN> paramSizes = { 4, 5, 10, 5, USHRT_MAX, 5 };
		vector<SQLSMALLINT> inputOutputTypes(initParamValues.size(), SQL_PARAM_INPUT_OUTPUT);

		InitRawParam(
			initParamValues,
			strLenOrInd,
			paramSizes,
			inputOutputTypes,
			false); // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,       // rowsNumber
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		// Test binary(4) value - binary length matching with type.
		//
		const vector<SQLCHAR> BinaryValue1 = { 0x00, 0x01, 0xe2, 0x40 };

		// Test VARBINARY(5) value with length more than the type allows - expected truncation.
		// Above R script sets the parameter to 10 raw chars but we only expect 5.
		//
		const vector<SQLCHAR> BinaryValue2 = { 0x61, 0x62, 0x63, 0x64, 0x65 };

		// Test BINARY(10) value with length less than the type allows.
		//
		const vector<SQLCHAR> BinaryValue3 = { 0x01, 0x3f };

		// Test as.raw(NA) value returned as 00 in a VARBINARY(5) parameter.
		//
		const vector<SQLCHAR> BinaryValue4 = { 0x00 };

		vector<SQLCHAR*> expectedParamValues = {
			const_cast<SQLCHAR*>(BinaryValue1.data()),
			const_cast<SQLCHAR*>(BinaryValue2.data()),
			const_cast<SQLCHAR*>(BinaryValue3.data()),
			const_cast<SQLCHAR*>(BinaryValue4.data()),
			const_cast<SQLCHAR*>(BinaryValue5.data()),
			// Test NULL BINARY(5) value
			//
			nullptr };

		vector<SQLINTEGER> expectedStrLenOrInd = {
			static_cast<SQLINTEGER>(BinaryValue1.size())/m_BinarySize,
			static_cast<SQLINTEGER>(BinaryValue2.size())/m_BinarySize,
			static_cast<SQLINTEGER>(BinaryValue3.size())/m_BinarySize,
			static_cast<SQLINTEGER>(BinaryValue4.size())/m_BinarySize,
			static_cast<SQLINTEGER>(BinaryValue5.size())/m_BinarySize,
			SQL_NULL_DATA };

		GetRawOutputParam(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetDateOutputParamTest
	//
	// Description:
	//  Tests multiple DATE values.
	//
	TEST_F(RExtensionApiTests, GetDateOutputParamTest)
	{
		string scriptString =
			// Max Date value
			//
			"param1 <- as.Date('9999/12/31');"
			// Min Date value
			//
			"param2 <- as.Date('1/1/1');"
			// Normal Date value
			//
			"param3 <- as.Date('1470/7/27');"
			// Leap date
			//
			"param4 <- as.Date('2000/02/29');"
			// Today's default date;
			// Default is the current timezone on Linux but UTC on Windows.
			//
			"param5 <- Sys.Date();"
			// Today's UTC date
			//
			"Sys.setenv(TZ = 'UTC'); param6 <- Sys.Date(); Sys.unsetenv('TZ');"
			// Explicit NA
			//
			"param7 <- as.Date(NA);"
			// 0 Date is NA
			//
			"param8 <- as.Date('0000/00/00', 'YYYY/MM/DD');"
			// Invalid Date is also NA
			//
			"param9 <- as.Date('1000/13/09', 'YYYY/MM/DD');";

		int paramsNumber = 9;
		int numberOfNAs = 3;

		vector<SQL_DATE_STRUCT> initParamValues(paramsNumber, { 0, 0, 0 });
		vector<SQL_DATE_STRUCT> expectedParamValues = {
			  { 9999,12,31 },
			  { 1, 1, 1 },
			  { 1470,7,27 },
			  { 2000, 02, 29 },
			  // On Windows, default Sys.Date() is in UTC where Linux it is Local Time Zone.
			  //
#ifdef _WIN64
			  Utilities::GetTodaysDate<UTC_DATE>(),
#else
			  Utilities::GetTodaysDate<LOCAL_DATE>(),
#endif
			  Utilities::GetTodaysDate<UTC_DATE>() };
		expectedParamValues.insert(expectedParamValues.end(), numberOfNAs, { 0, 0, 0 });

		vector<SQLSMALLINT> inputOutputTypes(paramsNumber, SQL_PARAM_INPUT_OUTPUT);
		vector<SQLINTEGER> expectedStrLenOrInd(paramsNumber - numberOfNAs, sizeof(SQL_DATE_STRUCT));
		expectedStrLenOrInd.insert(expectedStrLenOrInd.end(), numberOfNAs, SQL_NULL_DATA);

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			0,             // inputSchemaColumnsNumber
			scriptString,
			paramsNumber);

		InitDateTimeParam<SQL_DATE_STRUCT, Rcpp::DateVector, Rcpp::Date, SQL_C_TYPE_DATE>(
			initParamValues,
			expectedStrLenOrInd,
			inputOutputTypes,
			false); // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,       // rowsNumber
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		// Verify that the parameters we get back are what we expect
		//
		GetDateTimeOutputParam<SQL_DATE_STRUCT>(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetDateTimeOutputParamTest
	//
	// Description:
	//  Tests multiple DATETIME values.
	//
	TEST_F(RExtensionApiTests, GetDateTimeOutputParamTest)
	{
		string scriptString =
			// Set the time zone to UTC explicitly
			//
			"Sys.setenv(TZ = 'UTC');"
			// Max Datetime value
			//
			"param1 <- as.POSIXct(strptime('9999/12/31 23:59:59.000092', '%Y/%m/%d %H:%M:%OS'));"
			// Min Datetime value
			//
			"param2 <- as.POSIXct(strptime('1/1/1 0:0:0.0', '%Y/%m/%d %H:%M:%OS'));"
			// Normal Datetime value
			//
			"param3 <- as.POSIXct(strptime('1470/7/27 17:47:52.000123456', '%Y/%m/%d %H:%M:%OS'));"
			// Nanoseconds are divisible by 10^9
			//
			"param4 <- as.POSIXct(strptime('2020/9/01 18:23:58.100000000', '%Y/%m/%d %H:%M:%OS'));"
			// Nanoseconds are all 9 digits
			//
			"param5 <- as.POSIXct(strptime('1970/10/31 8:30:2.123654489', '%Y/%m/%d %H:%M:%OS'));"
			// Unset the time zone.
			//
			"Sys.unsetenv('TZ');"
			// Default is the local time zone on Linux, whereas it is UTC on Windows - test that now
			//
			"param6 <- as.POSIXct(strptime('1970/10/31 8:30:2.123654489', '%Y/%m/%d %H:%M:%OS'));"
			// Following NA values don't matter in which time zone they are tested.
			// Explicit NA value
			//
			"param7 <- as.POSIXct(NA);"
			// 0 Datetime value
			//
			"param8 <- as.POSIXct(strptime('0000/00/00 00:00:00.00000', '%Y/%m/%d %H:%M:%OS'));"
			// Invalid Datetime is also NA
			//
			"param9 <- as.POSIXct(strptime('1000/13/09 00:59:00.00000', '%Y/%m/%d %H:%M:%OS'));";

		int paramsNumber = 9;
		int numberOfNAs = 3;

		vector<SQL_TIMESTAMP_STRUCT> initParamValues(paramsNumber, { 0, 0, 0, 0, 0, 0, 0 });
		vector<SQL_TIMESTAMP_STRUCT> expectedParamValues = {
			{ 9999, 12, 31, 23, 59, 59, 92000 },
			{ 1, 1, 1, 0, 0, 0, 0 },
			{ 1470, 7, 27, 17, 47, 52, 124'000 },
			{ 2020, 9, 01, 18, 23, 58, 100'000'000 },
			{ 1970, 10, 31, 8, 30, 2, 123'654'000 } };

		// If R has this value in the local time zone - true on Linux,
		// convert the last dateTime value into UTC since Rcpp and hence RExtension
		// return in UTC
		// On Windows, the default time zone is UTC itself so no need to do the conversion.
		//
#ifdef _WIN64
		expectedParamValues.push_back(expectedParamValues[expectedParamValues.size()-1]);
#else
		expectedParamValues.push_back(Utilities::ToUtc<SQL_TIMESTAMP_STRUCT>
				(expectedParamValues[expectedParamValues.size()-1]));
#endif

		expectedParamValues.insert(expectedParamValues.end(), numberOfNAs, { 0, 0, 0, 0, 0, 0, 0 });

		vector<SQLSMALLINT> inputOutputTypes(paramsNumber, SQL_PARAM_INPUT_OUTPUT);
		vector<SQLINTEGER> expectedStrLenOrInd(paramsNumber - numberOfNAs, sizeof(SQL_TIMESTAMP_STRUCT));
		expectedStrLenOrInd.insert(expectedStrLenOrInd.end(), numberOfNAs, SQL_NULL_DATA);

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			0,              // inputSchemaColumnsNumber
			scriptString,
			paramsNumber);

		InitDateTimeParam<SQL_TIMESTAMP_STRUCT, Rcpp::DatetimeVector, Rcpp::Datetime, SQL_C_TYPE_TIMESTAMP>(
			initParamValues,
			expectedStrLenOrInd,
			inputOutputTypes,
			false); // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,       // rowsNumber
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		// Verify that the parameters we get back are what we expect
		//
		GetDateTimeOutputParam<SQL_TIMESTAMP_STRUCT>(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetNumericOutputParamTest
	//
	// Description:
	//  Tests multiple numeric output parameter values with varying precision and scale for all
	//  the following storage classes:
	//  Precision  Storage bytes
	//    1 - 9      5
	//    10-19      9
	//    20-28      13
	//    29-38      17
	//
	TEST_F(RExtensionApiTests, GetNumericOutputParamTest)
	{
		string scriptString =
			// Test numeric(38, 0)
			//
			"param1 <- as.numeric(1e38);"
			// Test max numeric(38, 38)
			//
			"param2 <- as.numeric(9999999999999999999999999999999999999e-38);"
			// Test min numeric(38, 38)
			//
			"param3 <- as.numeric(1e-38);"
			// Test numeric(38, 19)
			//
			"param4 <- as.numeric(-5578989.33434e-14);"
			// Test numeric(28, 0)
			//
			"param5 <- as.numeric(1e28);"
			// Test numeric(28, 28)
			//
			"param6 <- as.numeric(1e-28);"
			// Test numeric(28, 14)
			//
			"param7 <- as.numeric(-5578989.33434e-9);"
			// Test numeric(19, 0)
			//
			"param8 <- as.numeric(1e18);"
			// Test numeric(19, 19)
			//
			"param9 <- as.numeric(1e-19);"
			// Test numeric(19, 9)
			//
			"param10 <- as.numeric(-5578989.33434e-4);"
			// Test numeric(9, 0)
			//
			"param11 <- as.numeric(1e8);"
			// Test numeric(9, 9)
			//
			"param12 <- as.numeric(1e-9);"
			// Test numeric(9, 5)
			//
			"param13 <- as.numeric(-5578.33434);"
			// Test ULLONG_MAX; R considers this as ULLONG_MAX + 1
			//
			"param14 <- as.numeric(18446744073709551615);"
			// Test ULLONG_MAX + 1; R considers this as ULLONG_MAX + 1
			//
			"param15 <- as.numeric(18446744073709551616);"
			// Test NA
			//
			"param16 <- as.numeric(NA);";

		int paramsNumber = 16;
		int numberOfNAs = 1;

		vector<SQL_NUMERIC_STRUCT> initParamValues(paramsNumber, { 0, 0, 0, 0, 0, 0, 0 });
		vector<SQLINTEGER> expectedStrLenOrInd(paramsNumber - numberOfNAs, sizeof(SQL_NUMERIC_STRUCT));
		expectedStrLenOrInd.insert(expectedStrLenOrInd.end(), numberOfNAs, SQL_NULL_DATA);
		vector<SQLSMALLINT> inputOutputTypes(paramsNumber, SQL_PARAM_INPUT_OUTPUT);

		vector<SQL_NUMERIC_STRUCT> expectedParamValues = {
			{ 38, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 196, 134, 90, 168, 76, 59, 75 } },
			{ 38, 38, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 71, 218, 213, 16, 238, 133, 7 } },
			{ 38, 38, 1, { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
			{ 38, 19, 0, { 186, 36, 94, 229, 129, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
			{ 28, 0, 1, { 0, 0, 0, 0, 0, 2, 37, 62, 94, 206, 79, 32, 0, 0, 0, 0 } },
			{ 28, 28, 1, { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
			{ 28, 14, 0, { 186, 36, 94, 229, 129, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
			{ 19, 0, 1, { 0, 0, 100, 167, 179, 182, 224, 13, 0, 0, 0, 0, 0, 0, 0, 0 } },
			{ 19, 19, 1, { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
			{ 19, 9, 0, { 186, 36, 94, 229, 129, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
			{ 9, 0, 1, { 0, 225, 245, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
			{ 9, 9, 1, { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
			{ 9, 5, 0, { 218, 220, 63, 33, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
			{ 20, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 } },
			{ 20, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 } },
			{ }
		};

		vector<SQLULEN> precisionAsParamSize(paramsNumber, 0);
		vector<SQLSMALLINT> decimalDigits(paramsNumber, 0);
		for (int paramNumber = 0; paramNumber < paramsNumber; ++paramNumber)
		{
			precisionAsParamSize[paramNumber] = expectedParamValues[paramNumber].precision;
			decimalDigits[paramNumber] = expectedParamValues[paramNumber].scale;
		}

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			0,            // inputSchemaColumnsNumber
			scriptString,
			paramsNumber);

		InitNumericParam(
			initParamValues,
			expectedStrLenOrInd,
			inputOutputTypes,
			precisionAsParamSize,
			decimalDigits);

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,       // rowsNumber
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		// Verify that the parameters we get back are what we expect
		//
		GetNumericOutputParam(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	//----------------------------------------------------------------------------------------------
	// Name: GetInvalidOutputParamTest
	//
	// Description:
	//  Negative tests.
	//
	TEST_F(RExtensionApiTests, GetInvalidOutputParamTest)
	{
		// Initialize a Session with empty script and 2 parameters.
		//
		InitializeSession(
			0,  // inputSchemaColumnsNumber
			"", // scriptString
			2); // parametersNumber

		vector<shared_ptr<SQLINTEGER>> initParamValues = {
			make_shared<SQLINTEGER>(0) };
		vector<SQLINTEGER> strLenOrInd = { 0 };
		vector<SQLSMALLINT> inputOutputTypes = { SQL_PARAM_INPUT };

		InitParam<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
			initParamValues,
			strLenOrInd,
			inputOutputTypes,
			false);  // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*sm_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		SQLPOINTER paramValue = nullptr;
		SQLINTEGER strLen_or_Ind = 0;
		result = SQL_SUCCESS;

		// Test input parameter requested as output.
		//
		result = (*sm_getOutputParamFuncPtr)(
			*m_sessionId,
			m_taskId,
			0, // paramNumber
			&paramValue,
			&strLen_or_Ind);

		ASSERT_EQ(result, SQL_ERROR);

		// Test uninitialized parameter.
		//
		result = SQL_SUCCESS;
		result = (*sm_getOutputParamFuncPtr)(
			*m_sessionId,
			m_taskId,
			1, // paramNumber
			&paramValue,
			&strLen_or_Ind);

		ASSERT_EQ(result, SQL_ERROR);
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::GetOutputParam
	//
	// Description:
	//  Templatized function to test output param value and strLenOrInd is as expected.
	//
	template<class SQLType>
	void RExtensionApiTests::GetOutputParam(
		vector<shared_ptr<SQLType>> expectedParamValues,
		vector<SQLINTEGER>          expectedStrLenOrInd)
	{
		ASSERT_EQ(expectedParamValues.size(), expectedStrLenOrInd.size());

		for (SQLUSMALLINT paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
		{
			SQLPOINTER paramValue = nullptr;
			SQLINTEGER strLen_or_Ind = 0;
			SQLRETURN result = SQL_ERROR;
			result = (*sm_getOutputParamFuncPtr)(
				*m_sessionId,
				m_taskId,
				paramNumber,
				&paramValue,
				&strLen_or_Ind);

			ASSERT_EQ(result, SQL_SUCCESS);

			EXPECT_EQ(strLen_or_Ind, expectedStrLenOrInd[paramNumber]);

			if (strLen_or_Ind != SQL_NULL_DATA)
			{
				EXPECT_EQ(*(static_cast<SQLType*>(paramValue)),
					*expectedParamValues[paramNumber]);
			}
			else
			{
				if (expectedParamValues[paramNumber] == nullptr)
				{
					// When expectedStrLenOrInd is SQL_NULL_DATA and expectedParamValue is 0,
					// returned paramValue should be nullptr,
					// indicating the parameter has not been set to a value in R.
					//
					EXPECT_EQ(paramValue, nullptr);
				}
				else if constexpr (is_same_v<SQLType, SQLREAL>
					|| is_same_v<SQLType, SQLDOUBLE>)
				{
					EXPECT_NE(paramValue, nullptr);
					EXPECT_TRUE(isnan(static_cast<SQLDOUBLE>(
						*(static_cast<SQLType*>(paramValue)))));
				}
				else
				{
					EXPECT_NE(paramValue, nullptr);
					EXPECT_EQ(*(static_cast<SQLType*>(paramValue)),
						*expectedParamValues[paramNumber]);
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::GetCharOutputParam
	//
	// Description:
	//  Tests character output param value and strLenOrInd is as expected.
	//
	void RExtensionApiTests::GetCharOutputParam(
		vector<SQLCHAR*>   expectedParamValues,
		vector<SQLINTEGER> expectedStrLenOrInd)
	{
		ASSERT_EQ(expectedParamValues.size(), expectedStrLenOrInd.size());

		for (SQLUSMALLINT paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
		{
			SQLPOINTER paramValue = nullptr;
			SQLINTEGER strLen_or_Ind = 0;
			SQLRETURN result = SQL_ERROR;

			result = (*sm_getOutputParamFuncPtr)(
					*m_sessionId,
					m_taskId,
					paramNumber,
					&paramValue,
					&strLen_or_Ind);
			ASSERT_EQ(result, SQL_SUCCESS);

			EXPECT_EQ(strLen_or_Ind, expectedStrLenOrInd[paramNumber]);
			if (expectedParamValues[paramNumber] != nullptr)
			{
				EXPECT_NE(paramValue, nullptr);
				string paramValueString(static_cast<char*>(paramValue),
					strLen_or_Ind);
				string expectedParamValueString(static_cast<char*>(
						static_cast<void*>(expectedParamValues[paramNumber])),
						expectedStrLenOrInd[paramNumber]);
				EXPECT_EQ(paramValueString, expectedParamValueString);
			}
			else
			{
				EXPECT_EQ(paramValue, nullptr);
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::GetNCharOutputParam
	//
	// Description:
	//  Tests nchar/nvarchar output param value and strLenOrInd are as expected.
	//
	void RExtensionApiTests::GetNCharOutputParam(
		vector<const wchar_t*> expectedParamValues,
		vector<SQLINTEGER>     expectedStrLenOrInd)
	{
		ASSERT_EQ(expectedParamValues.size(), expectedStrLenOrInd.size());

		for (SQLUSMALLINT paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
		{
			SQLPOINTER paramValue = nullptr;
			SQLINTEGER strLen_or_Ind = 0;
			SQLRETURN result = SQL_ERROR;

			result = (*sm_getOutputParamFuncPtr)(
				*m_sessionId,
				m_taskId,
				paramNumber,
				&paramValue,
				&strLen_or_Ind);
			ASSERT_EQ(result, SQL_SUCCESS);

			EXPECT_EQ(strLen_or_Ind, expectedStrLenOrInd[paramNumber]);

			if (expectedParamValues[paramNumber] != nullptr)
			{
				EXPECT_NE(paramValue, nullptr);
				wstring paramValueString(static_cast<wchar_t*>(paramValue),
					strLen_or_Ind / sizeof(wchar_t));
				wstring expectedParamValueString(expectedParamValues[paramNumber],
					expectedStrLenOrInd[paramNumber] / sizeof(wchar_t));

				// Compare the two wstrings byte by byte because EXPECT_STREQ and EXPECT_EQ
				// don't work properly for wstrings in Linux with -fshort-wchar
				//
				const char *paramBytes = reinterpret_cast<const char*>(paramValueString.c_str());
				const char *expectedParamBytes =
					reinterpret_cast<const char*>(expectedParamValueString.c_str());

				for(SQLINTEGER i=0; i < strLen_or_Ind; i++)
				{
					EXPECT_EQ(paramBytes[i], expectedParamBytes[i]);
				}
			}
			else
			{
				EXPECT_EQ(paramValue, nullptr);
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::GetRawOutputParam
	//
	// Description:
	//  Tests raw (binary) output param value and strLenOrInd is as expected.
	//
	void RExtensionApiTests::GetRawOutputParam(
		vector<SQLCHAR*>   expectedParamValues,
		vector<SQLINTEGER> expectedStrLenOrInd)
	{
		ASSERT_EQ(expectedParamValues.size(), expectedStrLenOrInd.size());

		for (SQLUSMALLINT paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
		{
			SQLPOINTER paramValue = nullptr;
			SQLINTEGER strLen_or_Ind = 0;

			SQLRETURN result = SQL_ERROR;
			result = (*sm_getOutputParamFuncPtr)(
					*m_sessionId,
					m_taskId,
					paramNumber,
					&paramValue,
					&strLen_or_Ind);
			ASSERT_EQ(result, SQL_SUCCESS);

			EXPECT_EQ(strLen_or_Ind, expectedStrLenOrInd[paramNumber]);

			if (expectedParamValues[paramNumber] != nullptr)
			{
				EXPECT_NE(paramValue, nullptr);
				for (SQLINTEGER i = 0; i < strLen_or_Ind; ++i)
				{
					EXPECT_EQ(*(static_cast<SQLCHAR*>(paramValue) + i),
						*(expectedParamValues[paramNumber] + i));
				}
			}
			else
			{
				EXPECT_EQ(paramValue, nullptr);
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::GetDateTimeOutputParam
	//
	// Description:
	//  Templatized function to test output param value and strLenOrInd is as expected.
	//
	template<class SQLType>
	void RExtensionApiTests::GetDateTimeOutputParam(
		vector<SQLType>    expectedParamValues,
		vector<SQLINTEGER> expectedStrLenOrInd)
	{
		ASSERT_EQ(expectedParamValues.size(), expectedStrLenOrInd.size());

		for (SQLULEN paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
		{
			SQLType expectedParamValue = expectedParamValues[paramNumber];

			SQLPOINTER paramValue = nullptr;
			SQLINTEGER strLen_or_Ind = 0;
			SQLRETURN result = SQL_ERROR;
			result = (*sm_getOutputParamFuncPtr)(
				*m_sessionId,
				m_taskId,
				paramNumber,
				&paramValue,
				&strLen_or_Ind);

			ASSERT_EQ(result, SQL_SUCCESS);

			ASSERT_EQ(strLen_or_Ind, expectedStrLenOrInd[paramNumber]);

			EXPECT_NE(paramValue, nullptr);
			SQLType actualValue = *(static_cast<SQLType *>(paramValue));

			EXPECT_EQ(expectedParamValue.year, actualValue.year);
			EXPECT_EQ(expectedParamValue.month, actualValue.month);
			EXPECT_EQ(expectedParamValue.day, actualValue.day);
			if constexpr (is_same_v<SQLType, SQL_TIMESTAMP_STRUCT>)
			{
				EXPECT_EQ(expectedParamValue.hour, actualValue.hour);
				EXPECT_EQ(expectedParamValue.minute, actualValue.minute);
				EXPECT_EQ(expectedParamValue.second, actualValue.second);
				EXPECT_EQ(expectedParamValue.fraction, actualValue.fraction);
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::GetNumericOutputParam
	//
	// Description:
	//  Tests numeric output param value and strLenOrInd are as expected.
	//
	void RExtensionApiTests::GetNumericOutputParam(
		vector<SQL_NUMERIC_STRUCT> expectedParamValues,
		vector<SQLINTEGER>         expectedStrLenOrInd)
	{
		ASSERT_EQ(expectedParamValues.size(), expectedStrLenOrInd.size());

		for (SQLULEN paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
		{
			SQL_NUMERIC_STRUCT expectedParamValue = expectedParamValues[paramNumber];

			SQLPOINTER paramValue = nullptr;
			SQLINTEGER strLen_or_Ind = 0;
			SQLRETURN result = SQL_ERROR;
			result = (*sm_getOutputParamFuncPtr)(
				*m_sessionId,
				m_taskId,
				paramNumber,
				&paramValue,
				&strLen_or_Ind);

			ASSERT_EQ(result, SQL_SUCCESS);

			ASSERT_EQ(strLen_or_Ind, expectedStrLenOrInd[paramNumber]);

			if (expectedStrLenOrInd[paramNumber] != SQL_NULL_DATA)
			{
				EXPECT_NE(paramValue, nullptr);
				SQL_NUMERIC_STRUCT actualValue = *(static_cast<SQL_NUMERIC_STRUCT *>(paramValue));

				EXPECT_EQ(expectedParamValue.precision, actualValue.precision);
				EXPECT_EQ(expectedParamValue.scale, actualValue.scale);
				EXPECT_EQ(expectedParamValue.sign, actualValue.sign);
				for (SQLSMALLINT byte = 0; byte < SQL_MAX_NUMERIC_LEN; ++byte)
				{
					EXPECT_EQ(expectedParamValue.val[byte], actualValue.val[byte]);
				}
			}
			else
			{
				EXPECT_EQ(paramValue, nullptr);
			}
		}
	}

	// Name: CreateInputSizeRandomStr
	//
	// Description:
	// Create a string of random characters of the provided size.
	//
	string RExtensionApiTests::CreateInputSizeRandomStr(int size)
	{
		string result;
		for (int i = 0; i < size; i++)
		{
			char randomChar = 'a' + rand()%26;
			result += randomChar;
		}
		
		return result;
	}

	// Name: CreateInputSizeBinaryStringR
	//
	// Description:
	// Create the script string which will generate the raw values
	// needed within the R environment of the provided size.
	//
	string RExtensionApiTests::CreateInputSizeBinaryStringR(int size)
	{
		string result;
		for (int i = 0; i < size - 1; i++)
		{
			result += "as.raw(0x01), ";
		}

		result += "as.raw(0x01)";
		return result;
	}
}
