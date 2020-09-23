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
// @File: RExtensionApiInitParamTests.cpp
//
// Purpose:
//  Tests the RExtension's implementation of the external language InitParam API.
//
//**************************************************************************************************

#include "Common.h"

using namespace std;

namespace ExtensionApiTest
{
	//----------------------------------------------------------------------------------------------
	// Name: InitIntegerParamTest
	//
	// Description:
	//  Tests multiple SQLINTEGER values.
	//
	TEST_F(RExtensionApiTests, InitIntegerParamTest)
	{
		InitializeSession(
			0,  // inputSchemaColumnsNumber
			"", // scriptString
			5); // parametersNumber

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
		vector<SQLINTEGER> strLenOrInd = { 0, 0, 0,
			SQL_NULL_DATA, SQL_NULL_DATA};
		vector<SQLSMALLINT> inputOutputTypes(expectedParamValues.size(), SQL_PARAM_INPUT);

		InitParam<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
			expectedParamValues,
			strLenOrInd,
			inputOutputTypes);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InitLogicalParamTest
	//
	// Description:
	//  Tests multiple logical (bit) values.
	//
	TEST_F(RExtensionApiTests, InitLogicalParamTest)
	{
		InitializeSession(
			0,  // inputSchemaColumnsNumber
			"", // scriptString
			8); // parametersNumber

		// Test '1', '0', NA, 0, 1, NULL and > 1 BIT values.
		// When testing out of range NA value with respect to R,
		// returned paramValue is '\0' even though parameter is NA in R
		// since type casting NA (which is -2'147'483'648) to SQLCHAR returns \0.
		//
		vector<shared_ptr<SQLCHAR>> expectedParamValues = {
			make_shared<SQLCHAR>('1'),
			make_shared<SQLCHAR>('0'),
			make_shared<SQLCHAR>(NA_LOGICAL),
			make_shared<SQLCHAR>(0),
			make_shared<SQLCHAR>(1),
			nullptr,
			make_shared<SQLCHAR>('2'),
			make_shared<SQLCHAR>(3) };
		vector<SQLINTEGER> strLenOrInd = { 0, 0,
			SQL_NULL_DATA, 0, 0, SQL_NULL_DATA, 0, 0 };
		vector<SQLSMALLINT> inputOutputTypes(expectedParamValues.size(), SQL_PARAM_INPUT);

		InitParam<SQLCHAR, Rcpp::LogicalVector, SQL_C_BIT>(
			expectedParamValues,
			strLenOrInd,
			inputOutputTypes);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InitRealParamTest
	//
	// Description:
	//  Tests multiple real values.
	//
	TEST_F(RExtensionApiTests, InitRealParamTest)
	{
		InitializeSession(
			0,  // inputSchemaColumnsNumber
			"", // scriptString
			5); // parametersNumber

		// Test max FLOAT(24), min FLOAT(24), a normal, NA_REAL and nullptr REAL values
		//
		vector<shared_ptr<SQLREAL>> expectedParamValues = {
			make_shared<SQLREAL>(m_MaxReal),
			make_shared<SQLREAL>(m_MinReal),
			make_shared<SQLREAL>(2.3e4),
			make_shared<SQLREAL>(NA_REAL),
			nullptr };
		vector<SQLINTEGER> strLenOrInd = { 0, 0, 0,
			SQL_NULL_DATA, SQL_NULL_DATA };
		vector<SQLSMALLINT> inputOutputTypes(expectedParamValues.size(), SQL_PARAM_INPUT);

		InitParam<SQLREAL, Rcpp::NumericVector, SQL_C_FLOAT>(
			expectedParamValues,
			strLenOrInd,
			inputOutputTypes);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InitDoubleParamTest
	//
	// Description:
	//  Tests multiple double values.
	//
	TEST_F(RExtensionApiTests, InitDoubleParamTest)
	{
		InitializeSession(
			0,  // inputSchemaColumnsNumber
			"", // scriptString
			5); // parametersNumber

		// Test max FLOAT(53), min FLOAT(53), a normal, NA_REAL and nullptr
		// i.e. DOUBLE PRECISION values
		//
		vector<shared_ptr<SQLDOUBLE>> expectedParamValues = {
			make_shared<SQLDOUBLE>(m_MaxDouble),
			make_shared<SQLDOUBLE>(m_MinDouble),
			make_shared<SQLDOUBLE>(1.45e38),
			make_shared<SQLDOUBLE>(NA_REAL),
			nullptr };
		vector<SQLINTEGER> strLenOrInd = { 0, 0, 0,
			SQL_NULL_DATA, SQL_NULL_DATA};
		vector<SQLSMALLINT> inputOutputTypes(expectedParamValues.size(), SQL_PARAM_INPUT);

		InitParam<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
			expectedParamValues,
			strLenOrInd,
			inputOutputTypes);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InitBigIntParamTest
	//
	// Description:
	//  Tests multiple big int values.
	//
	TEST_F(RExtensionApiTests, InitBigIntParamTest)
	{
		InitializeSession(
			0,  // inputSchemaColumnsNumber
			"", // scriptString
			5); // parametersNumber

		// Test max, min, normal, NA and NULL BIGINT values
		//
		vector<shared_ptr<SQLBIGINT>> expectedParamValues = {
			make_shared<SQLBIGINT>(m_MaxBigInt),
			make_shared<SQLBIGINT>(m_MinBigInt),
			make_shared<SQLBIGINT>(9'372'036'854'775),
			make_shared<SQLBIGINT>(NA_REAL),
			nullptr };
		vector<SQLINTEGER> strLenOrInd = { 0, 0, 0,
			SQL_NULL_DATA, SQL_NULL_DATA };
		vector<SQLSMALLINT> inputOutputTypes(expectedParamValues.size(), SQL_PARAM_INPUT);

		InitParam<SQLBIGINT, Rcpp::NumericVector, SQL_C_SBIGINT>(
			expectedParamValues,
			strLenOrInd,
			inputOutputTypes);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InitSmallIntParamTest
	//
	// Description:
	//  Tests multiple small int values.
	//
	TEST_F(RExtensionApiTests, InitSmallIntParamTest)
	{
		InitializeSession(
			0,  // inputSchemaColumnsNumber
			"", // scriptString
			5); // parametersNumber

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
		vector<SQLINTEGER> strLenOrInd = { 0, 0, 0,
			SQL_NULL_DATA, SQL_NULL_DATA};
		vector<SQLSMALLINT> inputOutputTypes(expectedParamValues.size(), SQL_PARAM_INPUT);

		InitParam<SQLSMALLINT, Rcpp::IntegerVector, SQL_C_SSHORT>(
			expectedParamValues,
			strLenOrInd,
			inputOutputTypes);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InitTinyIntParamTest
	//
	// Description:
	//  Tests multiple tiny int values.
	//
	TEST_F(RExtensionApiTests, InitTinyIntParamTest)
	{
		InitializeSession(
			0,  // inputSchemaColumnsNumber
			"", // scriptString
			6); // parametersNumber

		// Test max, min, normal, NA, NULL, (-1) underflows to max TINYINT values
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
			make_shared<SQLCHAR>(-1)};
		vector<SQLINTEGER> strLenOrInd = { 0, 0, 0,
			SQL_NULL_DATA, SQL_NULL_DATA, 0 };
		vector<SQLSMALLINT> inputOutputTypes(expectedParamValues.size(), SQL_PARAM_INPUT);

		InitParam<SQLCHAR, Rcpp::IntegerVector, SQL_C_UTINYINT>(
			expectedParamValues,
			strLenOrInd,
			inputOutputTypes);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InitCharParamTest
	//
	// Description:
	//  Tests multiple character values
	//
	TEST_F(RExtensionApiTests, InitCharParamTest)
	{
		InitializeSession(
			0,  // inputSchemaColumnsNumber
			"", // scriptString
			10); // parametersNumber

		vector<const char*> expectedParamValues = {
			// Test simple CHAR(5) value with exact string length as the type allows i.e. here 5.
			//
			"HELLO",
			// Test CHAR(6) value with parameter length less than size - should be padded.
			//
			"WORLD",
			// Test CHAR(6) value with parameter length more than size - should be truncated.
			//
			"REXTENSION",
			// Test null CHAR(5) value
			//
			nullptr,
			// Test a 0 length CHAR(5) value
			//
			"",
			// Test simple VARCHAR(6) value
			//
			"WORLD!",
			// Test simple VARCHAR(8) value with parameter length less than size - no padding.
			//
			"WORLD",
			// Test VARCHAR(6) value with parameter length more than size - should be truncated.
			//
			"REXTENSION",
			// Test null VARCHAR(5) value
			//
			nullptr,
			// Test 0 length VARCHAR(6) value
			//
			""};

		vector<SQLULEN> paramSizes = { 5, 6, 6, 5, 5, 6, 8, 6, 5, 6 };

		vector<bool> isFixedType = { true, true, true, true, true,
			false, false, false, false, false };
		vector<SQLSMALLINT> inputOutputTypes(expectedParamValues.size(), SQL_PARAM_INPUT);

		InitCharParam<char, SQL_C_CHAR>(
			expectedParamValues,
			paramSizes,
			isFixedType,
			inputOutputTypes);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InitNCharParamTest
	//
	// Description:
	//  Tests multiple NCHAR and NVARCHAR values.
	//
	TEST_F(RExtensionApiTests, InitNCharParamTest)
	{
		InitializeSession(
			0,  // inputSchemaColumnsNumber
			"", // scriptString
			12); // parametersNumber

		vector<const wchar_t*> expectedParamValues = {
			// Test simple NCHAR(5) value with exact string length as the type allows i.e. here 5.
			//
			L"HELLO",
			// Test NCHAR(6) value with parameter length less than size - should be padded.
			//
			L"WORLD",
			// Test NCHAR(6) value with parameter length more than size - should be truncated.
			//
			L"REXTENSION",
			// Test null NCHAR(5) value
			//
			nullptr,
			// Test a 0 length NCHAR(5) value
			//
			L"",
			// Test UNICODE NCHAR(2)
			//
			L"你好",
			// Test simple NVARCHAR(6) value
			//
			L"WORLD!",
			// Test simple NVARCHAR(8) value with parameter length less than size - no padding.
			//
			L"WORLD",
			// Test NVARCHAR(6) value with parameter length more than size - should be truncated.
			//
			L"REXTENSION",
			// Test null NVARCHAR(5) value
			//
			nullptr,
			// Test 0 length NVARCHAR(6) value
			//
			L"",
			// Test Unicode NVARCHAR(6) value
			//
			L"你好"
		};

		vector<SQLULEN> paramSizes = { 5, 6, 6, 5, 5, 2, 6, 8, 6, 5, 6, 6 };

		vector<bool> isFixedType = { true, true, true, true, true, true,
			false, false, false, false, false, false };
		vector<SQLSMALLINT> inputOutputTypes(expectedParamValues.size(), SQL_PARAM_INPUT);

		InitCharParam<wchar_t, SQL_C_WCHAR>(
			expectedParamValues,
			paramSizes,
			isFixedType,
			inputOutputTypes);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InitRawParamTest
	//
	// Description:
	//  Tests multiple binary values.
	//
	TEST_F(RExtensionApiTests, InitRawParamTest)
	{
		InitializeSession(
			0,  // inputSchemaColumnsNumber
			"", // scriptString
			6); // parametersNumber

		// Test binary(4) value - binary length matching with type.
		//
		const vector<SQLCHAR> BinaryValue1 = { 0x00, 0x01, 0xe2, 0x40 };

		// Test binary(6) - actual value less than size but padded with 0x00.
		//
		const vector<SQLCHAR> BinaryValue2 = { 0x61, 0x62, 0x63, 0x64, 0x00, 0x00 };

		// Test simple varbinary(4) value
		//
		const vector<SQLCHAR> BinaryValue3 = { 0x00, 0x01, 0xe2, 0x40 };

		// Test varbinary(5) value with length less than size - no padding
		//
		const vector<SQLCHAR> BinaryValue4 = { 0x61, 0x62, 0x63, 0x64 };

		vector<SQLCHAR*> expectedParamValues = {
			const_cast<SQLCHAR*>(BinaryValue1.data()),
			// Test null binary(4) value
			//
			nullptr,
			const_cast<SQLCHAR*>(BinaryValue2.data()),
			const_cast<SQLCHAR*>(BinaryValue3.data()),
			// Test null varbinary(5) value
			//
			nullptr,
			const_cast<SQLCHAR*>(BinaryValue4.data()) };

		vector<SQLINTEGER> strLenOrInd = {
			static_cast<SQLINTEGER>(BinaryValue1.size())/m_BinarySize,
			SQL_NULL_DATA,
			static_cast<SQLINTEGER>(BinaryValue2.size())/m_BinarySize,
			static_cast<SQLINTEGER>(BinaryValue3.size())/m_BinarySize,
			SQL_NULL_DATA,
			static_cast<SQLINTEGER>(BinaryValue4.size())/m_BinarySize };

		vector<SQLULEN> paramSizes = { 4, 4, 6, 4, 4, 6 };
		vector<SQLSMALLINT> inputOutputTypes(expectedParamValues.size(), SQL_PARAM_INPUT);

		InitRawParam(
			expectedParamValues,
			strLenOrInd,
			paramSizes,
			inputOutputTypes);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InitDateParamTest
	//
	// Description:
	//  Tests multiple DATE values.
	//
	TEST_F(RExtensionApiTests, InitDateParamTest)
	{
		InitializeSession(
			0,  // inputSchemaColumnsNumber
			"", // scriptString
			6); // parametersNumber

		vector<SQL_DATE_STRUCT> expectedParamValues = {
			// Test max SQL_DATE_STRUCT value
			//
			{ 9999,12,31 },
			// Test min SQL_DATE_STRUCT value
			//
			{ 1,1,1 },
			// Test normal SQL_DATE_STRUCT value
			//
			{ 1470,7,27 },
			// Test today's Local date
			//
			Utilities::GetTodaysDate<LOCAL_DATE>(),
			// Test today's UTC date
			//
			Utilities::GetTodaysDate<UTC_DATE>(),
			// Test null SQL_DATE_STRUCT value
			//
			{}};

		vector<SQLINTEGER> strLenOrInd(expectedParamValues.size(), sizeof(SQL_DATE_STRUCT));
		strLenOrInd[expectedParamValues.size() - 1] = SQL_NULL_DATA;

		vector<SQLSMALLINT> inputOutputTypes(expectedParamValues.size(), SQL_PARAM_INPUT);

		InitDateTimeParam<SQL_DATE_STRUCT, Rcpp::DateVector, Rcpp::Date, SQL_C_TYPE_DATE>(
			expectedParamValues,
			strLenOrInd,
			inputOutputTypes);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InitDateTimeParamTest
	//
	// Description:
	//  Tests multiple DATETIME values.
	//
	TEST_F(RExtensionApiTests, InitDateTimeParamTest)
	{
		InitializeSession(
			0,  // inputSchemaColumnsNumber
			"", // scriptString
			6); // parametersNumber

		vector<SQL_TIMESTAMP_STRUCT> expectedParamValues = {
			// Test max SQL_TIMESTAMP_STRUCT value
			//
			{ 9999,12,31,23,59,59,999 },
			// Test min SQL_TIMESTAMP_STRUCT value
			//
			{ 1,1,1,0,0,0,0 },
			// Test normal SQL_TIMESTAMP_STRUCT value
			//
			{ 1470,7,27,17,47,52,123456 },
			// Test a value of nanoseconds divisible by 100'000'000
			//
			{ 2020,9,01,18,23,58,100'000'000 },
			// Test a 9 digit value of nanoseconds
			//
			{ 1970,10,31,8,30,2,123654489 },
			// Test null SQL_TIMESTAMP_STRUCT value
			//
			{}};

		vector<SQLINTEGER> strLenOrInd(expectedParamValues.size(), sizeof(SQL_TIMESTAMP_STRUCT));
		strLenOrInd[expectedParamValues.size() - 1] = SQL_NULL_DATA;

		vector<SQLSMALLINT> inputOutputTypes(expectedParamValues.size(), SQL_PARAM_INPUT);

		InitDateTimeParam<SQL_TIMESTAMP_STRUCT, Rcpp::DatetimeVector, Rcpp::Datetime, SQL_C_TYPE_TIMESTAMP>(
			expectedParamValues,
			strLenOrInd,
			inputOutputTypes);
	}

	//
	// Negative tests
	//
	//----------------------------------------------------------------------------------------------
	// Name: InitNullNameParamTest
	//
	// Description:
	//  Tests InitParam() API with null parameter name. A negative test.
	//
	TEST_F(RExtensionApiTests, InitNullNameParamTest)
	{
		InitializeSession(
			0,  // inputSchemaColumnsNumber
			"", // scriptString
			1); // parametersNumber

		SQLCHAR *paramName = nullptr;
		SQLSMALLINT paramNameLength = 0;
		SQLINTEGER paramValue = 123;

		SQLRETURN result = (*sm_initParamFuncPtr)(
			*m_sessionId,
			m_taskId,
			0,                // paramNumber
			paramName,
			paramNameLength,
			SQL_C_SLONG,
			m_IntSize,        // paramSize
			0,                // decimalDigits
			&paramValue,      // paramValue
			SQL_NULL_DATA,    // strLenOrInd
			SQL_PARAM_INPUT); // inputOutputType

		EXPECT_EQ(result, SQL_ERROR);
	}

	//----------------------------------------------------------------------------------------------
	// Name: InitInvalidParamNumberTest
	//
	// Description:
	//  Tests InitParam() API with bad param numbers (too big). A negative test.
	//
	TEST_F(RExtensionApiTests, InitInvalidParamNumberTest)
	{
		SQLUSMALLINT parametersNumber = 1;
		InitializeSession(
			0,                 // inputSchemaColumnsNumber
			"",                // scriptString
			parametersNumber); // parametersNumber

		string paramNameString = "@param";
		SQLCHAR *paramName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(paramNameString.c_str())));
		SQLINTEGER paramValue = 123;

		SQLRETURN result = (*sm_initParamFuncPtr)(
			*m_sessionId,
			m_taskId,
			parametersNumber + 1, // paramNumber outside of range
			paramName,
			paramNameString.length(),
			SQL_C_SLONG,
			m_IntSize,            // paramSize
			0,                    // decimalDigits
			&paramValue,          // argValue
			0,                    // strLenOrInd
			SQL_PARAM_INPUT);     // inputOutputType

		EXPECT_EQ(result, SQL_ERROR);
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::InitParam
	//
	// Description:
	//  Templatized function to call InitParam for the given paramValues and dataType.
	//  Testing if InitParam is implemented correctly for integer/numeric/logical dataTypes.
	//  When inRange is false and paramValue is 0, it tests a nullptr paramValue given to InitParam.
	//
	template<class SQLType, class RVectorType, SQLSMALLINT DataType>
	void RExtensionApiTests::InitParam(
		vector<shared_ptr<SQLType>> expectedParamValues,
		vector<SQLINTEGER>          strLenOrInd,
		vector<SQLSMALLINT>         inputOutputTypes,
		bool                        validate)
	{
		for (SQLUSMALLINT paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
		{
			string paramNameString = string("@param" + to_string(paramNumber + 1));
			SQLCHAR *paramName = static_cast<SQLCHAR*>(
			static_cast<void*>(const_cast<char *>(paramNameString.c_str())));

			SQLRETURN result = SQL_ERROR;
			result = (*sm_initParamFuncPtr)(
					*m_sessionId,
					m_taskId,
					paramNumber,
					paramName,
					paramNameString.length(),
					DataType,
					sizeof(SQLType),  // paramSize
					0,                // decimalDigits
					expectedParamValues[paramNumber] != nullptr
						? expectedParamValues[paramNumber].get() : nullptr, // paramValue
					strLenOrInd[paramNumber],
					inputOutputTypes[paramNumber]);
			ASSERT_EQ(result, SQL_SUCCESS);

			if (validate)
			{
				// Do + 1 to skip the @ from the paramName
				//
				RVectorType param = m_globalEnvironment[paramNameString.c_str() + 1];
				if (strLenOrInd[paramNumber] != SQL_NULL_DATA)
				{
					SQLType expectedParamValue = *expectedParamValues[paramNumber];
					if constexpr (DataType == SQL_C_BIT)
					{
						EXPECT_EQ(param[0],
							expectedParamValue != '0' && expectedParamValue != 0);
					}
					else
					{
						EXPECT_EQ(param[0], expectedParamValue);
					}
				}
				else
				{
					EXPECT_TRUE(RVectorType::is_na(param[0]));
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: InitCharParam
	//
	// Description:
	//  Templatized function testing if InitParam is implemented correctly for the
	//  (n)char/(n)varchar dataTypes
	//
	template<class CharType, SQLSMALLINT DataType>
	void RExtensionApiTests::InitCharParam(
		vector<const CharType*> expectedParamValues,
		vector<SQLULEN>         paramSizes,
		vector<bool>            isFixedType,
		vector<SQLSMALLINT>     inputOutputTypes,
		bool                    validate)
	{
		for (SQLUSMALLINT paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
		{
			string paramNameString = string("@param" + to_string(paramNumber + 1));
			SQLCHAR *paramName = static_cast<SQLCHAR*>(
				static_cast<void*>(const_cast<char *>(paramNameString.c_str())));
			vector<CharType> fixedParamValue(paramSizes[paramNumber] + 1, 0);
			SQLULEN strLenOrInd = 0;
			CharType *expectedParamValue = nullptr;
			SQLULEN paramLengthAfterTruncationIfAny = 0;

			if (expectedParamValues[paramNumber] != nullptr)
			{
				SQLULEN paramLength = 0;
				if constexpr (is_same_v<CharType, wchar_t>)
				{
					paramLength = Utilities::GetWStringLength(expectedParamValues[paramNumber]);
				}
				else
				{
					paramLength = strlen(expectedParamValues[paramNumber]);
				}

				const CharType *paramValue = expectedParamValues[paramNumber];
				copy(paramValue, paramValue + min(paramLength, paramSizes[paramNumber]),
					fixedParamValue.begin());

				if (isFixedType[paramNumber])
				{
					strLenOrInd = paramSizes[paramNumber];

					// pad the rest of the vector
					//
					for (SQLULEN index = paramLength; index < strLenOrInd; ++index)
					{
						fixedParamValue[index] = ' ';
					}
				}
				else
				{
					strLenOrInd = min(paramLength, paramSizes[paramNumber]);
				}

				paramLengthAfterTruncationIfAny = strLenOrInd;
				strLenOrInd *= sizeof(CharType);
				expectedParamValue = fixedParamValue.data();
			}
			else
			{
				strLenOrInd = SQL_NULL_DATA;
			}

			SQLRETURN result = SQL_ERROR;

			// Even though paramSize doesn't include the null terminator,
			// we create a CharacterVector parameter with a null terminator string
			//
			result = (*sm_initParamFuncPtr)(
					*m_sessionId,
					m_taskId,
					paramNumber,
					paramName,
					paramNameString.length(),
					DataType,
					paramSizes[paramNumber],
					0,               // decimalDigits
					expectedParamValue,
					strLenOrInd,
					inputOutputTypes[paramNumber]);

			ASSERT_EQ(result, SQL_SUCCESS);

			if (validate)
			{
				// Do + 1 to skip the @ from the paramName
				//
				Rcpp::CharacterVector param = m_globalEnvironment[paramNameString.c_str() + 1];
				if (expectedParamValues[paramNumber] != nullptr)
				{
					const char *actualParam = param[0];
					string expectedParamUtf8;
					if constexpr (is_same_v<CharType, wchar_t>)
					{
						estd::ToUtf8(
							reinterpret_cast<char16_t*>(expectedParamValue),
							paramLengthAfterTruncationIfAny,
							expectedParamUtf8);
					}
					else
					{
						expectedParamUtf8 = string(expectedParamValue, paramLengthAfterTruncationIfAny);
					}

					for (SQLULEN index = 0; index < paramLengthAfterTruncationIfAny; ++index)
					{
						EXPECT_EQ(actualParam[index], expectedParamUtf8[index]);
					}
				}
				else
				{
					EXPECT_EQ(param[0], NA_STRING);
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::InitRawParam
	//
	// Description:
	//  Testing if InitParam is implemented correctly for the binary/varbinary dataType.
	//
	void RExtensionApiTests::InitRawParam(
		vector<SQLCHAR*>    expectedParamValues,
		vector<SQLINTEGER>  strLenOrInd,
		vector<SQLULEN>     paramSizes,
		vector<SQLSMALLINT> inputOutputTypes,
		bool                validate)
	{
		for (SQLUSMALLINT paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
		{
			string paramNameString = string("@param" + to_string(paramNumber + 1));
			SQLCHAR *paramName = static_cast<SQLCHAR*>(
				static_cast<void*>(const_cast<char *>(paramNameString.c_str())));
			SQLCHAR *expectedParamValue = expectedParamValues[paramNumber];

			SQLRETURN result = SQL_ERROR;
			result = (*sm_initParamFuncPtr)(
					*m_sessionId,
					m_taskId,
					paramNumber,
					paramName,
					paramNameString.length(),
					SQL_C_BINARY,
					paramSizes[paramNumber],
					0, // decimalDigits
					expectedParamValue,
					strLenOrInd[paramNumber],
					inputOutputTypes[paramNumber]);
			ASSERT_EQ(result, SQL_SUCCESS);

			if (validate)
			{
				// Do + 1 to skip the @ from the paramName
				//
				Rcpp::RawVector param = m_globalEnvironment[paramNameString.c_str() + 1];

				if (expectedParamValue != nullptr)
				{
					// Always compare using strLenOrInd because
					// we copy only those many bytes into the raw parameter
					//
					for (SQLINTEGER i = 0; i < strLenOrInd[paramNumber]; ++i)
					{
						EXPECT_EQ(param[i], expectedParamValue[i]);
					}
				}
				else
				{
					EXPECT_EQ(param[0], 0);
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	// Name: RExtensionApiTest::InitDateTimeParam
	//
	// Description:
	//  Testing if InitParam is implemented correctly for the date/datetime dataTypes.
	//
	template<class SQLType, class RVectorType, class DateTimeTypeInR, SQLSMALLINT DataType>
	void RExtensionApiTests::InitDateTimeParam(
		vector<SQLType>     expectedParamValues,
		vector<SQLINTEGER>  strLenOrInd,
		vector<SQLSMALLINT> inputOutputTypes,
		bool                validate)
	{
		for (SQLUSMALLINT paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
		{
			string paramNameString = string("@param" + to_string(paramNumber + 1));
			SQLCHAR *paramName = static_cast<SQLCHAR*>(
			static_cast<void*>(const_cast<char *>(paramNameString.c_str())));

			SQLType expectedParamValue = expectedParamValues[paramNumber];

			SQLRETURN result = SQL_ERROR;
			result = (*sm_initParamFuncPtr)(
					*m_sessionId,
					m_taskId,
					paramNumber,
					paramName,
					paramNameString.length(),
					DataType,
					sizeof(SQLType),  // paramSize
					0,                // decimalDigits
					&expectedParamValue,
					strLenOrInd[paramNumber],
					inputOutputTypes[paramNumber]);
			ASSERT_EQ(result, SQL_SUCCESS);

			if (validate)
			{
				// Do + 1 to skip the @ from the paramName
				//
				RVectorType param = m_globalEnvironment[paramNameString.c_str() + 1];
				CheckRDateTimeVectorColumnDataEquality<SQLType, RVectorType, DateTimeTypeInR>(
					1,
					param,
					&expectedParamValue,
					&strLenOrInd[paramNumber]);
			}
		}
	}
}
