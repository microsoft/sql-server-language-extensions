//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonGetOutputParamTests.cpp
//
// Purpose:
//  Tests the Python Extension's implementation of the external language GetOutputParam API.
//
//*************************************************************************************************
#include "PythonExtensionApiTests.h"

using namespace std;

namespace ExtensionApiTest
{
	// Name: GetIntegerOutputParamTest
	//
	// Description:
	// Test multiple SQLINTEGER values
	//
	TEST_F(PythonExtensionApiTests, GetIntegerOutputParamTest)
	{
		SQLINTEGER p0 = m_MaxInt;
		SQLINTEGER p1 = m_MinInt;
		SQLINTEGER p2 = 4;
		SQLINTEGER p3 = 0;

		string scriptString =
			"param0 = " + to_string(p0) + ";"
			"param1 = " + to_string(p1) + ";"
			"param2 = " + to_string(p2) + ";"
			"param3 = " + to_string(p3) + ";"
			"param4 = None;";

		int paramsNumber = 5;

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			paramsNumber,   // parametersNumber
			0,              // inputSchemaColumnsNumber
			scriptString);

		for(int i=0; i<paramsNumber; ++i)
		{
			TestParameter<SQLINTEGER, SQL_C_SLONG>(
				i,         // paramNumber
				0,         // paramValue
				false,     // isNull
				SQL_PARAM_INPUT_OUTPUT,
				false);    // validate
		}

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = Execute(
			*m_sessionId,
			m_taskId,
			0,
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		vector<SQLINTEGER*> paramValues = { &p0, &p1, &p2, &p3, nullptr};
		vector<SQLINTEGER> strLenOrIndValues = {
			m_IntSize,
			m_IntSize,
			m_IntSize,
			m_IntSize,
			SQL_NULL_DATA };

		// Verify that the parameters we get back are what we expect
		//
		TestGetOutputParam<SQLINTEGER>(
			paramValues,
			strLenOrIndValues);
	}

	// Name: GetBitOutputParamTest
	//
	// Description:
	// Test multiple bit values
	//
	TEST_F(PythonExtensionApiTests, GetBitOutputParamTest)
	{
		SQLCHAR p0 = '\x1';
		SQLCHAR p1 = '\0';
		SQLCHAR p2 = '\x1';
		SQLCHAR p3 = '\0';

		string scriptString =
			"param0 = True;"
			"param1 = False;"
			"param2 = 3;"
			"param3 = 0;"
			"param4 = None;";

		int paramsNumber = 5;

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			paramsNumber,   // parametersNumber
			0,              // inputSchemaColumnsNumber
			scriptString);

		for (int i = 0; i < paramsNumber; ++i)
		{
			TestParameter<SQLCHAR, SQL_C_BIT>(
				i,         // paramNumber
				0,         // paramValue
				false,     // isNull
				SQL_PARAM_INPUT_OUTPUT,
				false);    // validate
		}

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = Execute(
			*m_sessionId,
			m_taskId,
			0,
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		vector<SQLCHAR*> paramValues = { &p0, &p1, &p2, &p3, nullptr };
		vector<SQLINTEGER> strLenOrIndValues = {
			m_BooleanSize,
			m_BooleanSize,
			m_BooleanSize,
			m_BooleanSize,
			SQL_NULL_DATA };

		// Verify that the parameters we get back are what we expect
		//
		TestGetOutputParam<SQLCHAR>(
			paramValues,
			strLenOrIndValues);
	}

	// Name: GetRealOutputParamTest
	//
	// Description:
	// Test multiple real values
	//
	TEST_F(PythonExtensionApiTests, GetRealOutputParamTest)
	{
		SQLREAL p0 = m_MaxReal;
		SQLREAL p1 = m_MinReal;
		SQLREAL p2 = 2.3e4;
		SQLREAL p3 = 0;

		string scriptString =
			"param0 = " + to_string(p0) + ";"
			"param1 = " + to_string(p1) + ";"
			"param2 = " + to_string(p2) + ";"
			"param3 = " + to_string(p3) + ";"
			"param4 = None;";

		int paramsNumber = 5;

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			paramsNumber,   // parametersNumber
			0,              // inputSchemaColumnsNumber
			scriptString);

		for (int i = 0; i < paramsNumber; ++i)
		{
			TestParameter<SQLREAL, SQL_C_FLOAT>(
				i,         // paramNumber
				0,         // paramValue
				false,     // isNull
				SQL_PARAM_INPUT_OUTPUT,
				false);    // validate
		}

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = Execute(
			*m_sessionId,
			m_taskId,
			0,
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		vector<SQLREAL*> paramValues = { &p0, &p1, &p2, &p3, nullptr };
		vector<SQLINTEGER> strLenOrIndValues = {
			m_RealSize,
			m_RealSize,
			m_RealSize,
			m_RealSize,
			SQL_NULL_DATA };

		// Verify that the parameters we get back are what we expect
		//
		TestGetOutputParam<SQLREAL>(
			paramValues,
			strLenOrIndValues);
	}

	// Name: GetDoubleOutputParamTest
	//
	// Description:
	// Test multiple double values
	//
	TEST_F(PythonExtensionApiTests, GetDoubleOutputParamTest)
	{
		SQLDOUBLE p0 = m_MaxDouble;
		SQLDOUBLE p1 = m_MinDouble;
		SQLDOUBLE p2 = 1.45e38;
		SQLDOUBLE p3 = 0;

		string scriptString =
			"param0 = " + to_string(p0) + ";"
			"param1 = " + to_string(p1) + ";"
			"param2 = " + to_string(p2) + ";"
			"param3 = " + to_string(p3) + ";"
			"param4 = None;";

		int paramsNumber = 5;

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			paramsNumber,   // parametersNumber
			0,              // inputSchemaColumnsNumber
			scriptString);

		for (int i = 0; i < paramsNumber; ++i)
		{
			TestParameter<SQLDOUBLE, SQL_C_DOUBLE>(
				i,         // paramNumber
				0,         // paramValue
				false,     // isNull
				SQL_PARAM_INPUT_OUTPUT,
				false);    // validate
		}

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = Execute(
			*m_sessionId,
			m_taskId,
			0,
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		vector<SQLDOUBLE*> paramValues = { &p0, &p1, &p2, &p3, nullptr };
		vector<SQLINTEGER> strLenOrIndValues = {
			m_DoubleSize,
			m_DoubleSize,
			m_DoubleSize,
			m_DoubleSize,
			SQL_NULL_DATA };

		// Verify that the parameters we get back are what we expect
		//
		TestGetOutputParam<SQLDOUBLE>(
			paramValues,
			strLenOrIndValues);
	}

	// Name: GetBigIntOutputParamTest
	//
	// Description:
	// Test multiple big int values
	//
	TEST_F(PythonExtensionApiTests, GetBigIntOutputParamTest)
	{
		SQLBIGINT p0 = m_MaxBigInt;
		SQLBIGINT p1 = m_MinBigInt;
		SQLBIGINT p2 = 9372036854775;
		SQLBIGINT p3 = 0;

		string scriptString =
			"param0 = " + to_string(p0) + ";"
			"param1 = " + to_string(p1) + ";"
			"param2 = " + to_string(p2) + ";"
			"param3 = " + to_string(p3) + ";"
			"param4 = None;";

		int paramsNumber = 5;

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			paramsNumber,   // parametersNumber
			0,              // inputSchemaColumnsNumber
			scriptString);

		for (int i = 0; i < paramsNumber; ++i)
		{
			TestParameter<SQLBIGINT, SQL_C_SBIGINT>(
				i,         // paramNumber
				0,         // paramValue
				false,     // isNull
				SQL_PARAM_INPUT_OUTPUT,
				false);    // validate
		}

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = Execute(
			*m_sessionId,
			m_taskId,
			0,
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		vector<SQLBIGINT*> paramValues = { &p0, &p1, &p2, &p3, nullptr };
		vector<SQLINTEGER> strLenOrIndValues = {
			m_BigIntSize,
			m_BigIntSize,
			m_BigIntSize,
			m_BigIntSize,
			SQL_NULL_DATA };

		// Verify that the parameters we get back are what we expect
		//
		TestGetOutputParam<SQLBIGINT>(
			paramValues,
			strLenOrIndValues);
	}

	// Name: GetSmallIntOutputParamTest
	//
	// Description:
	// Test multiple small int values
	//
	TEST_F(PythonExtensionApiTests, GetSmallIntOutputParamTest)
	{
		SQLSMALLINT p0 = m_MaxSmallInt;
		SQLSMALLINT p1 = m_MinSmallInt;
		SQLSMALLINT p2 = 3007;
		SQLSMALLINT p3 = 0;

		string scriptString =
			"param0 = " + to_string(p0) + ";"
			"param1 = " + to_string(p1) + ";"
			"param2 = " + to_string(p2) + ";"
			"param3 = " + to_string(p3) + ";"
			"param4 = None;";

		int paramsNumber = 5;

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			paramsNumber,   // parametersNumber
			0,              // inputSchemaColumnsNumber
			scriptString);

		for (int i = 0; i < paramsNumber; ++i)
		{
			TestParameter<SQLSMALLINT, SQL_C_SSHORT>(
				i,         // paramNumber
				0,         // paramValue
				false,     // isNull
				SQL_PARAM_INPUT_OUTPUT,
				false);    // validate
		}

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = Execute(
			*m_sessionId,
			m_taskId,
			0,
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		vector<SQLSMALLINT*> paramValues = { &p0, &p1, &p2, &p3, nullptr };
		vector<SQLINTEGER> strLenOrIndValues = {
			m_SmallIntSize,
			m_SmallIntSize,
			m_SmallIntSize,
			m_SmallIntSize,
			SQL_NULL_DATA };

		// Verify that the parameters we get back are what we expect
		//
		TestGetOutputParam<SQLSMALLINT>(
			paramValues,
			strLenOrIndValues);
	}

	// Name: GetTinyIntOutputParamTest
	//
	// Description:
	// Test multiple tiny int values
	//
	TEST_F(PythonExtensionApiTests, GetTinyIntOutputParamTest)
	{
		SQLCHAR p0 = m_MaxTinyInt;
		SQLCHAR p1 = m_MinTinyInt;
		SQLCHAR p2 = 123;
		SQLCHAR p3 = -1;

		string scriptString =
			"param0 = " + to_string(p0) + ";"
			"param1 = " + to_string(p1) + ";"
			"param2 = " + to_string(p2) + ";"
			"param3 = " + to_string(p3) + ";"
			"param4 = None;";

		int paramsNumber = 5;

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			paramsNumber,   // parametersNumber
			0,              // inputSchemaColumnsNumber
			scriptString);

		for (int i = 0; i < paramsNumber; ++i)
		{
			TestParameter<SQLCHAR, SQL_C_UTINYINT>(
				i,         // paramNumber
				0,         // paramValue
				false,     // isNull
				SQL_PARAM_INPUT_OUTPUT,
				false);    // validate
		}

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = Execute(
			*m_sessionId,
			m_taskId,
			0,
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		// p3 is -1, which is expected to underflow into max value (which is p0)
		//
		vector<SQLCHAR*> paramValues = { &p0, &p1, &p2, &p0, nullptr };
		vector<SQLINTEGER> strLenOrIndValues = {
			m_TinyIntSize,
			m_TinyIntSize,
			m_TinyIntSize,
			m_TinyIntSize,
			SQL_NULL_DATA };

		// Verify that the parameters we get back are what we expect
		//
		TestGetOutputParam<SQLCHAR>(
			paramValues,
			strLenOrIndValues);
	}

	// Name: GetStringOutputParamTest
	//
	// Description:
	// Test multiple string values
	//
	TEST_F(PythonExtensionApiTests, GetStringOutputParamTest)
	{
		// Construct value greater than USHRT_MAX to test truncating behavior
		//
		string value4 = CreateInputSizeRandomStr(128000);
		string scriptString = 
			"param0 = 'HELLO';"
			"param1 = 'PyExtension';"
			"param2 = '';"
			"param3 = 'WORLD';"
			"param4 = '" + value4 + "';"
			"param5 = None;"
			"param6 = None;";

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			7,   // parametersNumber
			0,   // inputSchemaColumnsNumber
			scriptString);

		// Note: The behavior of fixed and varying character types is same when it comes to output
		// parameters. So it doesn't matter if we initialize these output parameters as fixed type.
		//
		vector<bool> isFixedType = { true, false, true, false, true, true, false };
		vector<SQLULEN> paramSizes = { 5, 6, 10, 5, USHRT_MAX, 5, 5 };

		for(SQLULEN paramNumber=0; paramNumber < paramSizes.size(); ++paramNumber)
		{
			TestStringParameter(
				paramNumber,
				"",             // paramValue
				paramSizes[paramNumber],
				isFixedType[paramNumber],
				SQL_PARAM_INPUT_OUTPUT,
				false); // validate
		}

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = Execute(
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
			// Above python script sets the parameter to "PyExtension" but we only expect "PyExte".
			//
			"PyExte",
			// Test a 0 length string
			//
			"" ,
			// Test CHAR(10) value with string length less than the type allows.
			//
			"WORLD",
			// Test varchar(max) with string length of 128000.
			//
			value4};

		vector<const char*> expectedParamValues = {
			ExpectedParamValueStrings[0].c_str(),
			ExpectedParamValueStrings[1].c_str(),
			ExpectedParamValueStrings[2].c_str(),
			ExpectedParamValueStrings[3].c_str(),
			ExpectedParamValueStrings[4].c_str(),

			// Test None returned in a VARCHAR(5) parameter.
			//
			nullptr,

			// Test None CHAR(5) value.
			//
			nullptr };

		vector<SQLINTEGER> expectedStrLenOrInd = {
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[0].length()),
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[1].length()),
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[2].length()),
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[3].length()),
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[4].length()),
			SQL_NULL_DATA,
			SQL_NULL_DATA };

		TestGetStringOutputParam(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	// Name: GetWStringOutputParamTest
	//
	// Description:
	// Test multiple wstring values
	//
	TEST_F(PythonExtensionApiTests, GetWStringOutputParamTest)
	{
		// Construct the values that correspond to 你好
		//
		vector<char> chineseBytes = { -28, -67, -96, -27, -91, -67 };
		string chineseString = string(chineseBytes.data(), 6);
		// Construct value greater than USHRT_MAX to test truncating behavior
		//
		string value7 = CreateInputSizeRandomStr(128000); 
		string scriptString = 
			"param0 = 'HELLO';"
			"param1 = 'PyExtension';"
			"param2 = '';"
			"param3 = 'WORLD';"
			"param4 = '" + chineseString + "';"
			"param5 = None;"
			"param6 = None;"
			"param7 = '" + value7 + "';";

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			8,   // parametersNumber
			0,   // inputSchemaColumnsNumber
			scriptString);

		// Note: The behavior of fixed and varying character types is same when it comes to output
		// parameters. So it doesn't matter if we initialize these output parameters as fixed type.
		//
		vector<bool> isFixedType = { true, false, true , true, true, false, true, false};
		vector<SQLULEN> paramSizes = { 5, 6, 5, 10, 2, 5, 5, USHRT_MAX };

		for (SQLULEN paramNumber = 0; paramNumber < paramSizes.size(); ++paramNumber)
		{
			TestWStringParameter(
				paramNumber,
				nullptr,                 // paramValue
				paramSizes[paramNumber],
				isFixedType[paramNumber],
				SQL_PARAM_INPUT_OUTPUT,
				false);                  // validate
		}

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = Execute(
			*m_sessionId,
			m_taskId,
			0,       // rowsNumber
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);
		wstring wideStrOfValue7 = wstring(value7.begin(), value7.end());
		const wchar_t* wideCStrOfValue7 = wideStrOfValue7.c_str();
		vector<const wchar_t*> expectedParamValues = {
			// Test simple CHAR(5) value with exact string length as the type allows i.e. here 5.
			//
			L"HELLO",
			// Test VARCHAR(6) value with string length more than the type allows - expected truncation.
			// Above python script sets the parameter to "PyExtension" but we only expect "PyExte".
			//
			L"PyExte",
			// Test a 0 length string
			//
			L"" ,
			// Test CHAR(10) value with string length less than the type allows.
			//
			L"WORLD",
			// Test a Unicode string
			//
			L"你好",
			nullptr,
			nullptr,
			wideCStrOfValue7};

		vector<SQLINTEGER> expectedStrLenOrInd = {
			static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
			static_cast<SQLINTEGER>(6 * sizeof(wchar_t)),
			static_cast<SQLINTEGER>(0 * sizeof(wchar_t)),
			static_cast<SQLINTEGER>(5 * sizeof(wchar_t)),
			static_cast<SQLINTEGER>(2 * sizeof(wchar_t)),
			SQL_NULL_DATA,
			SQL_NULL_DATA,
			static_cast<SQLINTEGER>(128000 * sizeof(wchar_t)),

		};

		TestGetWStringOutputParam(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	// Name: GetRawOutputParamTest
	//
	// Description:
	// Test multiple raw binary/bytes values
	//
	TEST_F(PythonExtensionApiTests, GetRawOutputParamTest)
	{
		string value4 = CreateInputSizeRandomStr(128000);
		string scriptString = 
			"param0 = b'HELLO';"
			"param1 = b'PyExtension';"
			"param2 = b'';"
			"param3 = b'WORLD';"
			"param4 = b'" + value4 + "';"
			"param5 = None;"
			"param6 = None";

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			7,   // parametersNumber
			0,   // inputSchemaColumnsNumber
			scriptString);

		// Note: The behavior of fixed and varying character types is same when it comes to output
		// parameters. So it doesn't matter if we initialize these output parameters as fixed type.
		//
		vector<bool> isFixedType = { true, false, true, true, false, false, true };
		vector<SQLULEN> paramSizes = { 5, 6, 5, 10, USHRT_MAX, 5, 5 };

		for (SQLULEN paramNumber = 0; paramNumber < paramSizes.size(); ++paramNumber)
		{
			vector<SQLCHAR> dummy = { 0x00 };

			TestRawParameter(
				paramNumber,
				dummy.data(),              // paramValue
				paramSizes[paramNumber],
				isFixedType[paramNumber],
				SQL_PARAM_INPUT_OUTPUT,
				false);                    // validate
		}

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = Execute(
			*m_sessionId,
			m_taskId,
			0,       // rowsNumber
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		const vector<string> ExpectedParamValueStrings = {
			// Test simple BINARY(5) value with exact string length as the type allows i.e. here 5.
			//
			"HELLO",
			// Test VARBINARY(6) value with string length more than the type allows - expected truncation.
			// Above python script sets the parameter to "PyExtension" but we only expect "PyExte".
			//
			"PyExte",
			// Test a 0 length raw
			//
			"",
			// Test BINARY(10) value with string length less than the type allows.
			//
			"WORLD",
			// Test VARBINARY(max) with string length of 128000.
			//			
			value4 };

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
			// Test None returned in a VARCHAR(5) parameter.
			//
			nullptr,
			// Test None CHAR(5) value.
			//
			nullptr };

		vector<SQLINTEGER> expectedStrLenOrInd = {
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[0].length()),
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[1].length()),
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[2].length()),
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[3].length()),
			static_cast<SQLINTEGER>(ExpectedParamValueStrings[4].length()),
			SQL_NULL_DATA,
			SQL_NULL_DATA };

		TestGetRawOutputParam(
			expectedParamValues,
			expectedStrLenOrInd);
	}

	// Name: GetDateTimeOutputParamTest
	//
	// Description:
	// Test multiple DATETIME values
	//
	TEST_F(PythonExtensionApiTests, GetDateTimeOutputParamTest)
	{
		string scriptString =
			"import datetime;import pandas;import numpy;"
			"param0 = datetime.datetime(9999,12,31,23,59,59,999999);"
			"param1 = datetime.datetime(1,1,1,0,0,0,0);"
			"param2 = datetime.datetime(2004,2,29,17,47,52,123456);" // Leap day
			"param3 = datetime.datetime(1470,7,27,17,47,52,123456);"
			"param4 = pandas.Timestamp(2070,7,27,17,47,52,123456);"
			"param5 = numpy.datetime64('NaT');"
			"param6 = None;";

		int paramsNumber = 7;

		SQL_TIMESTAMP_STRUCT p0 = { 9999,12,31,23,59,59,999999000 };
		SQL_TIMESTAMP_STRUCT p1 = { 1,1,1,0,0,0,0 };
		SQL_TIMESTAMP_STRUCT p2 = { 2004,2,29,17,47,52,123456000 };
		SQL_TIMESTAMP_STRUCT p3 = { 1470,7,27,17,47,52,123456000 };
		SQL_TIMESTAMP_STRUCT p4 = { 2070,7,27,17,47,52,123456000 };

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			paramsNumber,   // parametersNumber
			0,              // inputSchemaColumnsNumber
			scriptString);

		for (int i = 0; i < paramsNumber; ++i)
		{
			TestDateTimeParameter<SQL_TIMESTAMP_STRUCT, SQL_C_TYPE_TIMESTAMP>(
				i,                      // paramNumber
				p1,                     // paramValue
				false,                  // isNull
				SQL_PARAM_INPUT_OUTPUT,
				false);                 // validate
		}

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = Execute(
			*m_sessionId,
			m_taskId,
			0,
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		vector<SQL_TIMESTAMP_STRUCT *> paramValues = { &p0, &p1, &p2, &p3, &p4, nullptr, nullptr };
		vector<SQLINTEGER> strLenOrIndValues = {
			sizeof(SQL_TIMESTAMP_STRUCT),
			sizeof(SQL_TIMESTAMP_STRUCT),
			sizeof(SQL_TIMESTAMP_STRUCT),
			sizeof(SQL_TIMESTAMP_STRUCT),
			sizeof(SQL_TIMESTAMP_STRUCT),
			SQL_NULL_DATA,
			SQL_NULL_DATA };

		// Verify that the parameters we get back are what we expect
		//
		TestGetDateTimeOutputParam<SQL_TIMESTAMP_STRUCT>(
			paramValues,
			strLenOrIndValues);
	}

	// Name: GetDateOutputParamTest
	//
	// Description:
	// Test multiple DATE values
	//
	TEST_F(PythonExtensionApiTests, GetDateOutputParamTest)
	{
		string scriptString =
			"import datetime;"
			"param0 = datetime.date(9999,12,31);"
			"param1 = datetime.date(1,1,1);"
			"param2 = datetime.date(1470,7,27);"
			"param3 = datetime.date(2004,2,29);" // Leap Day
			"param4 = None;";

		int paramsNumber = 5;
		SQL_DATE_STRUCT p0 = { 9999,12,31 };
		SQL_DATE_STRUCT p1 = { 1,1,1 };
		SQL_DATE_STRUCT p2 = { 1470,7,27 };
		SQL_DATE_STRUCT p3 = { 2004,2,29 };

		// Initialize with a Session that executes the above script
		// that sets output parameters.
		//
		InitializeSession(
			paramsNumber,   // parametersNumber
			0,              // inputSchemaColumnsNumber
			scriptString);

		for (int i = 0; i < paramsNumber; ++i)
		{
			TestDateTimeParameter<SQL_DATE_STRUCT, SQL_C_TYPE_DATE>(
				i,                      // paramNumber
				{},                     // paramValue
				true,                   // isNull
				SQL_PARAM_INPUT_OUTPUT,
				false);                 // validate
		}

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = Execute(
			*m_sessionId,
			m_taskId,
			0,
			nullptr, // dataSet
			nullptr, // strLen_or_Ind
			&outputSchemaColumnsNumber);
		ASSERT_EQ(result, SQL_SUCCESS);

		EXPECT_EQ(outputSchemaColumnsNumber, 0);

		vector<SQL_DATE_STRUCT *> paramValues = { &p0, &p1, &p2, &p3, nullptr };

		// We return a TIMESTAMP_STRUCT because it is more encompassing.
		// It will just have 0s for hour/minute/sec/usec
		//
		vector<SQLINTEGER> strLenOrIndValues = {
			sizeof(SQL_TIMESTAMP_STRUCT),
			sizeof(SQL_TIMESTAMP_STRUCT),
			sizeof(SQL_TIMESTAMP_STRUCT),
			sizeof(SQL_TIMESTAMP_STRUCT),
			SQL_NULL_DATA };

		// Verify that the parameters we get back are what we expect
		//
		TestGetDateTimeOutputParam<SQL_DATE_STRUCT>(
			paramValues,
			strLenOrIndValues);
	}

	// Name: GetInvalidOutputParamTest
	//
	// Description:
	// Negative tests.
	//
	TEST_F(PythonExtensionApiTests, GetInvalidOutputParamTest)
	{
		InitializeSession(
			2,   // parametersNumber
			0,   // inputSchemaColumnsNumber
			""); // scriptString

		TestParameter<SQLINTEGER, SQL_C_SLONG>(
			0,       // paramNumber
			0,       // paramValue
			false,   // isNull
			SQL_PARAM_INPUT,
			false);  // validate

		SQLUSMALLINT outputSchemaColumnsNumber = 0;
		SQLRETURN result = Execute(
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
		result = GetOutputParam(
			*m_sessionId,
			m_taskId,
			0, // paramNumber
			&paramValue,
			&strLen_or_Ind);

		ASSERT_EQ(result, SQL_ERROR);

		// Test uninitialized parameter.
		//
		result = SQL_SUCCESS;
		result = GetOutputParam(
			*m_sessionId,
			m_taskId,
			1, // paramNumber
			&paramValue,
			&strLen_or_Ind);

		ASSERT_EQ(result, SQL_ERROR);
	}

	// Name: CreateInputSizeRandomStr
	//
	// Description:
	// Create a string of random characters of the provided size.
	//
	string PythonExtensionApiTests::CreateInputSizeRandomStr(int size)
	{
		string result;
		for (int i = 0; i < size; i++)
		{
			char randomChar = 'a' + rand()%26;
			result += randomChar;
		}

		return result;
	}

	// Name: TestGetOutputParam
	//
	// Description:
	// Templatized function to test output param value and strLenOrInd is as expected.
	//
	template<class SQLType>
	void PythonExtensionApiTests::TestGetOutputParam(
		vector<SQLType*>   expectedParamValueVector,
		vector<SQLINTEGER> expectedStrLenOrIndVector)
	{
		ASSERT_EQ(expectedParamValueVector.size(), expectedStrLenOrIndVector.size());

		for(SQLULEN i=0; i< expectedParamValueVector.size(); ++i)
		{
			SQLType *expectedParamValue = expectedParamValueVector[i];
			SQLINTEGER expectedStrLenOrInd = expectedStrLenOrIndVector[i];

			SQLPOINTER paramValue = nullptr;
			SQLINTEGER strLen_or_Ind = 0;

			SQLRETURN result = SQL_ERROR;
			result = GetOutputParam(
					*m_sessionId,
					m_taskId,
					i,  // paramNumber
					&paramValue,
					&strLen_or_Ind);
			ASSERT_EQ(result, SQL_SUCCESS);

			EXPECT_EQ(strLen_or_Ind, expectedStrLenOrInd);

			if (expectedParamValue != nullptr)
			{
				EXPECT_NE(paramValue, nullptr);

				EXPECT_EQ(*(static_cast<SQLType*>(paramValue)), *expectedParamValue);
			}
			else
			{
				EXPECT_EQ(paramValue, nullptr);
			}
		}
	}

	// Name: TestGetStringOutputParam
	//
	// Description:
	// Test string output param value and strLenOrInd is as expected.
	//
	void PythonExtensionApiTests::TestGetStringOutputParam(
		vector<const char*> expectedParamValues,
		vector<SQLINTEGER>  expectedStrLenOrInd)
	{
		ASSERT_EQ(expectedParamValues.size(), expectedStrLenOrInd.size());

		for (SQLUSMALLINT paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
		{
			SQLPOINTER paramValue = nullptr;
			SQLINTEGER strLen_or_Ind = 0;
			SQLRETURN result = SQL_ERROR;

			result = GetOutputParam(
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
				string expectedParamValueString(expectedParamValues[paramNumber],
					expectedStrLenOrInd[paramNumber]);

				EXPECT_EQ(paramValueString, expectedParamValueString);
			}
			else
			{
				EXPECT_EQ(paramValue, nullptr);
			}
		}
	}

	// Name: TestGetWStringOutputParam
	//
	// Description:
	// Test wstring output param value and strLenOrInd is as expected.
	//
	void PythonExtensionApiTests::TestGetWStringOutputParam(
		vector<const wchar_t*> expectedParamValues,
		vector<SQLINTEGER>     expectedStrLenOrInd)
	{
		ASSERT_EQ(expectedParamValues.size(), expectedStrLenOrInd.size());

		for (SQLUSMALLINT paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
		{
			SQLPOINTER paramValue = nullptr;
			SQLINTEGER strLen_or_Ind = 0;
			SQLRETURN result = SQL_ERROR;

			result = GetOutputParam(
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
				const char *expectedParamBytes = reinterpret_cast<const char*>(expectedParamValueString.c_str());

				for(SQLINTEGER i=0; i<strLen_or_Ind; ++i)
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

	// Name: TestGetRawOutputParam
	//
	// Description:
	// Test Binary (binary) output param value and strLenOrInd is as expected.
	//
	void PythonExtensionApiTests::TestGetRawOutputParam(
		vector<SQLCHAR*>   expectedParamValues,
		vector<SQLINTEGER> expectedStrLenOrInd)
	{
		ASSERT_EQ(expectedParamValues.size(), expectedStrLenOrInd.size());

		for (SQLUSMALLINT paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
		{
			SQLPOINTER paramValue = nullptr;
			SQLINTEGER strLen_or_Ind = 0;

			SQLRETURN result = SQL_ERROR;
			result = GetOutputParam(
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

	// Name: TestGetDateTimeOutputParam
	//
	// Description:
	// Templatized function to test output param value and strLenOrInd is as expected.
	//
	template<class DateTimeStruct>
	void PythonExtensionApiTests::TestGetDateTimeOutputParam(
		vector<DateTimeStruct *> expectedParamValueVector,
		vector<SQLINTEGER>       expectedStrLenOrIndVector)
	{
		ASSERT_EQ(expectedParamValueVector.size(), expectedStrLenOrIndVector.size());

		for (SQLULEN i = 0; i < expectedParamValueVector.size(); ++i)
		{
			DateTimeStruct *expectedParamValue = expectedParamValueVector[i];
			SQLINTEGER expectedStrLenOrInd = expectedStrLenOrIndVector[i];

			SQLPOINTER paramValue = nullptr;
			SQLINTEGER strLen_or_Ind = 0;

			SQLRETURN result = SQL_ERROR;
			result = GetOutputParam(
				*m_sessionId,
				m_taskId,
				i,  // paramNumber
				&paramValue,
				&strLen_or_Ind);
			ASSERT_EQ(result, SQL_SUCCESS);

			EXPECT_EQ(strLen_or_Ind, expectedStrLenOrInd);

			if (expectedParamValue != nullptr)
			{
				EXPECT_NE(paramValue, nullptr);

				DateTimeStruct expectedValue = *expectedParamValue;
				DateTimeStruct actualValue = *(static_cast<DateTimeStruct *>(paramValue));
			
				if constexpr (is_same_v<DateTimeStruct, SQL_TIMESTAMP_STRUCT>)
				{
					EXPECT_EQ(expectedValue.year, actualValue.year);
					EXPECT_EQ(expectedValue.month, actualValue.month);
					EXPECT_EQ(expectedValue.day, actualValue.day);
					EXPECT_EQ(expectedValue.hour, actualValue.hour);
					EXPECT_EQ(expectedValue.minute, actualValue.minute);
					EXPECT_EQ(expectedValue.second, actualValue.second);
					EXPECT_EQ(expectedValue.fraction, actualValue.fraction);
				}
				else
				{
					EXPECT_EQ(expectedValue.year, actualValue.year);
					EXPECT_EQ(expectedValue.month, actualValue.month);
					EXPECT_EQ(expectedValue.day, actualValue.day);
				}
			}
			else
			{
				EXPECT_EQ(paramValue, nullptr);
			}
		}
	}
}
