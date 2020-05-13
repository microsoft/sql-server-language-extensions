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
// @File: RExtensionApiGetOutputParamTests.cpp
//
// Purpose:
//  Tests the RExtension's implementation of the external language GetOutputParam API.
//
//*************************************************************************************************
#ifdef _WIN64
#include <windows.h>
#endif
#include <math.h>
#include <memory>
#include <iostream>
#include <sqlext.h>
#include <sqltypes.h>
#include "gtest/gtest.h"
#include "Rcpp.h"

#include "RExtensionApiTest.h"

using namespace std;

namespace ExtensionApiTest
{
	// Name: GetIntegerOutputParamTest
	//
	// Description:
	// Test multiple SQLINTEGER values
	//
	TEST_F(RExtensionApiTest, GetIntegerOutputParamTest)
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
		SQLRETURN result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
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

	// Name: GetLogicalOutputParamTest
	//
	// Description:
	// Test multiple logical (bit) values
	//
	TEST_F(RExtensionApiTest, GetLogicalOutputParamTest)
	{
		string scriptString = "param1 <- TRUE;"
			"param2 <- FALSE;"
			"param3 <- NA;"
			"param4 <- as.logical();";

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			0,   // inputSchemaColumnsNumber
			scriptString,
			4);  // parametersNumber

		vector<shared_ptr<SQLCHAR>> initParamValues = {
			make_shared<SQLCHAR>(0),
			make_shared<SQLCHAR>(0),
			make_shared<SQLCHAR>(0),
			nullptr };
		vector<SQLINTEGER> strLenOrInd = { 0, 0, 0, SQL_NULL_DATA };
		vector<SQLSMALLINT> inputOutputTypes(initParamValues.size(), SQL_PARAM_INPUT_OUTPUT);

		InitParam<SQLCHAR, Rcpp::LogicalVector, SQL_C_BIT>(
			initParamValues,
			strLenOrInd,
			inputOutputTypes,
			false);  // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		// Test '1', '0', NA and NULL BIT values.
		// When testing out of range NA value with respect to R,
		// returned paramValue is '\0' even though parameter is NA in R
		// since type casting NA (which is -2'147'483'648) to SQLCHAR returns \0.
		//
		vector<shared_ptr<SQLCHAR>> expectedParamValues = {
			make_shared<SQLCHAR>('1'),
			make_shared<SQLCHAR>('0'),
			make_shared<SQLCHAR>('\0'),
			nullptr };
		vector<SQLINTEGER> expectedStrLenOrInd = { m_LogicalSize, m_LogicalSize,
			SQL_NULL_DATA, SQL_NULL_DATA};

		GetOutputParam<SQLCHAR>(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	// Name: GetRealOutputParamTest
	//
	// Description:
	// Test multiple real values
	//
	TEST_F(RExtensionApiTest, GetRealOutputParamTest)
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
		SQLRETURN result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
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

	// Name: GetDoubleOutputParamTest
	//
	// Description:
	// Test multiple double values
	//
	TEST_F(RExtensionApiTest, GetDoubleOutputParamTest)
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
		SQLRETURN result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
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

	// Name: GetBigIntOutputParamTest
	//
	// Description:
	// Test multiple big int values
	//
	TEST_F(RExtensionApiTest, GetBigIntOutputParamTest)
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
		SQLRETURN result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
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

	// Name: GetSmallIntOutputParamTest
	//
	// Description:
	// Test multiple small int values
	//
	TEST_F(RExtensionApiTest, GetSmallIntOutputParamTest)
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
		SQLRETURN result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
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

	// Name: GetTinyIntOutputParamTest
	//
	// Description:
	// Test multiple tiny int values
	//
	TEST_F(RExtensionApiTest, GetTinyIntOutputParamTest)
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
		SQLRETURN result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
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

	// Name: GetCharOutputParamTest
	//
	// Description:
	// Test multiple character values
	//
	TEST_F(RExtensionApiTest, GetCharOutputParamTest)
	{
		string scriptString = "param1 <- 'HELLO';"
			"param2 <- 'RExtension';"
			"param3 <- 'WORLD';"
			"param4 <- as.character(NA);"
			"param5 <- as.character();"
			"param6 <- ''";

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			0,   // inputSchemaColumnsNumber
			scriptString,
			6);  // parametersNumber

		vector<const char*> initParamValues(6,"");
		vector<SQLULEN> paramSizes = { 5, 6, 10, 5, 5, 5 };

		// Note: The behavior of fixed and varying character types is same when it comes to output
		// parameters. So it doesn't matter if we initialize these output parameters as fixed type.
		//
		vector<bool> isFixedType = { true, false, true, false, true, true };
		vector<SQLSMALLINT> inputOutputTypes(initParamValues.size(), SQL_PARAM_INPUT_OUTPUT);

		InitCharParam(
			initParamValues,
			paramSizes,
			isFixedType,
			inputOutputTypes,
			false); // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
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
			// Above R script sets the parameter to "RExtension!" but we only expect "RExten".
			//
			"RExten",
			// Test CHAR(10) value with string length less than the type allows.
			//
			"WORLD",
			// Test a 0 length string
			//
			""};
		vector<SQLCHAR*> expectedParamValues = {
			static_cast<SQLCHAR*>(
				static_cast<void*>(const_cast<char *>(ExpectedParamValueStrings[0].c_str()))),
			static_cast<SQLCHAR*>(
				static_cast<void*>(const_cast<char *>(ExpectedParamValueStrings[1].c_str()))),
			static_cast<SQLCHAR*>(
				static_cast<void*>(const_cast<char *>(ExpectedParamValueStrings[2].c_str()))),
			// Test NA returned in a VARCHAR(5) parameter.
			//
			nullptr,
			// Test nullptr CHAR(5) value.
			//
			nullptr,
			static_cast<SQLCHAR*>(
				static_cast<void*>(const_cast<char *>(ExpectedParamValueStrings[3].c_str()))) };

		vector<SQLINTEGER> expectedStrLenOrInd = {
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[0].length()),
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[1].length()),
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[2].length()),
			SQL_NULL_DATA,
			SQL_NULL_DATA,
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[3].length())};

		GetCharOutputParam(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	// Name: GetRawOutputParamTest
	//
	// Description:
	// Test multiple binary values
	//
	TEST_F(RExtensionApiTest, GetRawOutputParamTest)
	{
		string scriptString = "param1 <- c(as.raw(0x00), as.raw(0x01), as.raw(0xe2), as.raw(0x40));"
			"param2 <- charToRaw(paste(letters[1:10], collapse=''));"
			"param3 <- c(as.raw(0x01), as.raw(0x3f));"
			"param4 <- as.raw(NA);"
			"param5 <- raw()";

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			0,   // inputSchemaColumnsNumber
			scriptString,
			5);  // parametersNumber

		const vector<SQLCHAR> dummyValue = { };
		vector<SQLCHAR*> initParamValues(5, const_cast<SQLCHAR*>(dummyValue.data()));
		vector<SQLINTEGER> strLenOrInd(5, static_cast<SQLINTEGER>(dummyValue.size())/m_BinarySize);
		vector<SQLULEN> paramSizes = { 4, 5, 10, 5, 5 };
		vector<SQLSMALLINT> inputOutputTypes(initParamValues.size(), SQL_PARAM_INPUT_OUTPUT);

		InitRawParam(
			initParamValues,
			strLenOrInd,
			paramSizes,
			inputOutputTypes,
			false); // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = (*m_executeFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,
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
			// Test NULL BINARY(5) value
			//
			nullptr };

		vector<SQLINTEGER> expectedStrLenOrInd = {
			static_cast<SQLINTEGER>(BinaryValue1.size())/m_BinarySize,
			static_cast<SQLINTEGER>(BinaryValue2.size())/m_BinarySize,
			static_cast<SQLINTEGER>(BinaryValue3.size())/m_BinarySize,
			static_cast<SQLINTEGER>(BinaryValue4.size())/m_BinarySize,
			SQL_NULL_DATA };

		GetRawOutputParam(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	// Name: GetInvalidOutputParamTest
	//
	// Description:
	// Negative tests.
	//
	TEST_F(RExtensionApiTest, GetInvalidOutputParamTest)
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
		SQLRETURN result = (*m_executeFuncPtr)(
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
		result = (*m_getOutputParamFuncPtr)(
			*m_sessionId,
			m_taskId,
			0, // paramNumber
			&paramValue,
			&strLen_or_Ind);

		ASSERT_EQ(result, SQL_ERROR);

		// Test uninitialized parameter.
		//
		result = SQL_SUCCESS;
		result = (*m_getOutputParamFuncPtr)(
			*m_sessionId,
			m_taskId,
			1, // paramNumber
			&paramValue,
			&strLen_or_Ind);

		ASSERT_EQ(result, SQL_ERROR);
	}

	// Name: GetOutputParam
	//
	// Description:
	// Templatized function to test output param value and strLenOrInd is as expected.
	//
	template<class SQLType>
	void RExtensionApiTest::GetOutputParam(
		vector<shared_ptr<SQLType>> expectedParamValues,
		vector<SQLINTEGER>          expectedStrLenOrInd)
	{
		for (SQLUSMALLINT paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
		{
			SQLPOINTER paramValue = nullptr;
			SQLINTEGER strLen_or_Ind = 0;
			SQLRETURN result = SQL_ERROR;
			result = (*m_getOutputParamFuncPtr)(
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
				else if ((is_same<SQLType, SQLREAL>::value
					|| is_same<SQLType, SQLDOUBLE>::value))
				{
					EXPECT_TRUE(isnan(static_cast<SQLDOUBLE>(
						*(static_cast<SQLType*>(paramValue)))));
				}
				else
				{
					EXPECT_EQ(*(static_cast<SQLType*>(paramValue)),
						*expectedParamValues[paramNumber]);
				}
			}
		}
	}

	// Name: GetCharOutputParam
	//
	// Description:
	// Test character output param value and strLenOrInd is as expected.
	//
	void RExtensionApiTest::GetCharOutputParam(
		vector<SQLCHAR*>   expectedParamValues,
		vector<SQLINTEGER> expectedStrLenOrInd)
	{
		for (SQLUSMALLINT paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
		{
			SQLPOINTER paramValue = nullptr;
			SQLINTEGER strLen_or_Ind = 0;
			SQLRETURN result = SQL_ERROR;

			result = (*m_getOutputParamFuncPtr)(
					*m_sessionId,
					m_taskId,
					paramNumber,
					&paramValue,
					&strLen_or_Ind);
			ASSERT_EQ(result, SQL_SUCCESS);

			EXPECT_EQ(strLen_or_Ind, expectedStrLenOrInd[paramNumber]);

			if (expectedParamValues[paramNumber] != nullptr)
			{
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

	// Name: GetRawOutputParam
	//
	// Description:
	// Test raw (binary) output param value and strLenOrInd is as expected.
	//
	void RExtensionApiTest::GetRawOutputParam(
		vector<SQLCHAR*>   expectedParamValues,
		vector<SQLINTEGER> expectedStrLenOrInd)
	{
		for (SQLUSMALLINT paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
		{
			SQLPOINTER paramValue = nullptr;
			SQLINTEGER strLen_or_Ind = 0;

			SQLRETURN result = SQL_ERROR;
			result = (*m_getOutputParamFuncPtr)(
					*m_sessionId,
					m_taskId,
					paramNumber,
					&paramValue,
					&strLen_or_Ind);
			ASSERT_EQ(result, SQL_SUCCESS);

			EXPECT_EQ(strLen_or_Ind, expectedStrLenOrInd[paramNumber]);

			if (expectedParamValues[paramNumber] != nullptr)
			{
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
}
