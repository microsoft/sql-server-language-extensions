//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonInitParamTests.cpp
//
// Purpose:
//  Test the Python extension parameters using the Extension API
//
//*************************************************************************************************

#include "PythonExtensionApiTests.h"

#include <boost/python/stl_iterator.hpp>
#include <datetime.h>
#include <memory>

using namespace std;
namespace bp = boost::python;

namespace ExtensionApiTest
{
	// Name: InitIntegerParamTest
	//
	// Description:
	//  Test multiple SQLINTEGER values
	//
	TEST_F(PythonExtensionApiTests, InitIntegerParamTest)
	{
		InitializeSession(1); //parametersNumber

		// Test max INT value
		//
		TestParameter<SQLINTEGER, SQL_C_SLONG>(0, //paramNumber
			2147483647);

		// Test min INT value
		//
		TestParameter<SQLINTEGER, SQL_C_SLONG>(0, //paramNumber
			-2147483647);

		// Test a normal value
		//
		TestParameter<SQLINTEGER, SQL_C_SLONG>(0, //paramNumber
			4);

		// Test null INT value
		//
		TestParameter<SQLINTEGER, SQL_C_SLONG>(0, //paramNumber
			0,
			true); // isNull
	}

	// Name: InitBitParamTest
	//
	// Description:
	//  Test multiple BIT values
	//
	TEST_F(PythonExtensionApiTests, InitBitParamTest)
	{
		InitializeSession(1); //parametersNumber

		// Test '1' BIT value
		//
		TestParameter<SQLCHAR, SQL_C_BIT>(0, //paramNumber
			'1');

		// Test '0' BIT value
		//
		TestParameter<SQLCHAR, SQL_C_BIT>(0, //paramNumber
			'0');

		// Test 1 BIT value
		//
		TestParameter<SQLCHAR, SQL_C_BIT>(0, //paramNumber
			1);

		// Test 0 BIT value
		//
		TestParameter<SQLCHAR, SQL_C_BIT>(0, //paramNumber
			0);

		// Test null BIT value
		//
		TestParameter<SQLCHAR, SQL_C_BIT>(0, //paramNumber
			0,
			true); // isNull

		// Test > 1 BIT value, should be True
		//
		TestParameter<SQLCHAR, SQL_C_BIT>(0, //paramNumber
			2);

		// Test != '1' and != '0' BIT value, should be True
		//
		TestParameter<SQLCHAR, SQL_C_BIT>(0, //paramNumber
			'3');
	}

	// Name: InitFloatParamTest
	//
	// Description:
	//  Test multiple FLOAT(24) (SQLREAL) values
	//
	TEST_F(PythonExtensionApiTests, InitFloatParamTest)
	{
		InitializeSession(1); //parametersNumber

		// Test max FLOAT(24) i.e. REAL value
		//
		TestParameter<SQLREAL, SQL_C_FLOAT>(0, //paramNumber
			m_MaxReal);

		// Test min FLOAT(24) i.e. REAL value
		//
		TestParameter<SQLREAL, SQL_C_FLOAT>(0, //paramNumber
			m_MinReal);

		// Test a normal FLOAT(24) value
		//
		TestParameter<SQLREAL, SQL_C_FLOAT>(0, //paramNumber
			2.3e4F);

		// Test null FLOAT(24) i.e. REAL value
		//
		TestParameter<SQLREAL, SQL_C_FLOAT>(0, //paramNumber
			0,
			true); // isNull
	}

	// Name: InitDoubleParamTest
	//
	// Description:
	//  Test multiple FLOAT(53) (DOUBLE) values
	//
	TEST_F(PythonExtensionApiTests, InitDoubleParamTest)
	{
		InitializeSession(1); //parametersNumber

		// Test max FLOAT(53) i.e. DOUBLE PRECISION value
		//
		TestParameter<SQLDOUBLE, SQL_C_DOUBLE>(0, //paramNumber
			m_MaxDouble);

		// Test min FLOAT(53) i.e. DOUBLE PRECISION value
		//
		TestParameter<SQLDOUBLE, SQL_C_DOUBLE>(0, //paramNumber
			m_MinDouble);

		// Test normal FLOAT(53) i.e. DOUBLE PRECISION value
		//
		TestParameter<SQLDOUBLE, SQL_C_DOUBLE>(0, //paramNumber
			1.45e38);

		// Test null FLOAT(53) value
		//
		TestParameter<SQLDOUBLE, SQL_C_DOUBLE>(0, //paramNumber
			0,
			true); // isNull
	}

	// Name: InitBigIntParamTest
	//
	// Description:
	//  Test multiple SQLBIGINT values
	//
	TEST_F(PythonExtensionApiTests, InitBigIntParamTest)
	{
		InitializeSession(1); //parametersNumber

		// Test max BIGINT value
		//
		TestParameter<SQLBIGINT, SQL_C_SBIGINT>(0, //paramNumber
			9223372036854775807LL);

		// Test min BIGINT value : -9223372036854775808 gives compiler error
		//
		TestParameter<SQLBIGINT, SQL_C_SBIGINT>(0, //paramNumber
			-9223372036854775807LL);

		// Test normal BIGINT value
		//
		TestParameter<SQLBIGINT, SQL_C_SBIGINT>(0, //paramNumber
			9'372'036'854'775LL);

		// Test null BIGINT value
		//
		TestParameter<SQLBIGINT, SQL_C_SBIGINT>(0, //paramNumber
			0,
			true); // isNull
	}

	// Name: InitTinyIntParamTest
	//
	// Description:
	//  Test multiple TINYINT values
	//
	TEST_F(PythonExtensionApiTests, InitTinyIntParamTest)
	{
		InitializeSession(1); //parametersNumber

		// Test max TINYINT value
		//
		TestParameter<SQLCHAR, SQL_C_UTINYINT>(0, //paramNumber
			m_MaxTinyInt);

		// Test min TINYINT value
		//
		TestParameter<SQLCHAR, SQL_C_UTINYINT>(0, //paramNumber
			m_MinTinyInt);

		// Test normal TINYINT value
		//
		TestParameter<SQLCHAR, SQL_C_UTINYINT>(0, //paramNumber
			123);

		// Test null TINYINT value
		//
		TestParameter<SQLCHAR, SQL_C_UTINYINT>(0, //paramNumber
			0,
			true); // isNull

		// Test -1 TINYINT value underflows to m_MaxTinyInt
		//
		TestParameter<SQLCHAR, SQL_C_UTINYINT>(0, //paramNumber
			-1);
	}

	// Name: InitSmallIntParamTest
	//
	// Description:
	//  Test multiple SMALLINT values
	//
	TEST_F(PythonExtensionApiTests, InitSmallIntParamTest)
	{
		InitializeSession(1); //parametersNumber

		// Test max SMALLINT value
		//
		TestParameter<SQLSMALLINT, SQL_C_SSHORT>(0, //paramNumber
			32767);

		// Test min SMALLINT value
		//
		TestParameter<SQLSMALLINT, SQL_C_SSHORT>(0, //paramNumber
			-32768);

		// Test normal SMALLINT value
		//
		TestParameter<SQLSMALLINT, SQL_C_SSHORT>(0, //paramNumber
			3'007);

		// Test null SMALLINT value
		//
		TestParameter<SQLSMALLINT, SQL_C_SSHORT>(0, //paramNumber
			0,
			true); // isNull
	}

	// Name: InitStringParamTest
	//
	// Description:
	//  Test multiple CHAR and VARCHAR values
	//
	TEST_F(PythonExtensionApiTests, InitStringParamTest)
	{
		InitializeSession(1); //parametersNumber

		// Test simple CHAR(5) value
		//
		TestStringParameter(
			0,       // paramNumber
			"HELLO",
			5,       // paramSize
			true);   // isFixedType

		// Test simple CHAR(6) value with parameter length less than size - should be padded.
		//
		TestStringParameter(
			0,       // paramNumber
			"WORLD",
			6,       // paramSize
			true);   // isFixedType

		// Test CHAR(6) value with parameter length more than size - should be truncated.
		//
		TestStringParameter(
			0,       // paramNumber
			"PYEXTENSION",
			6,       // paramSize
			true);   // isFixedType

		// Test null CHAR(5) value
		//
		TestStringParameter(
			0,       // paramNumber
			nullptr, // paramValue
			5,       // paramSize
			true);   // isFixedType

		// Test simple VARCHAR(6) value
		//
		TestStringParameter(
			0,        // paramNumber
			"WORLD!",
			6,        // paramSize
			false);   // isFixedType

		// Test simple VARCHAR(8) value with parameter length less than size - NO padding.
		//
		TestStringParameter(
			0,       // paramNumber
			"WORLD",
			8,       // paramSize
			false);  // isFixedType

		// Test VARCHAR(6) value with parameter length more than size - should be truncated.
		//
		TestStringParameter(
			0,       // paramNumber
			"PYEXTENSION",
			6,       // paramSize
			false);  // isFixedType

		// Test CHAR value with UTF-8 encoded string (with chinese)
		//
		string utfstring = u8"中文编码";
		TestStringParameter(
			0,                  // paramNumber
			utfstring.c_str(),  // paramValue
			utfstring.length(), // paramSize
			true);              // isFixedType

		// Test VARCHAR value with UTF-8 encoded string (with cyrillic)
		//
		utfstring = u8"абвг";
		TestStringParameter(
			0,                  // paramNumber
			utfstring.c_str(),  // paramValue
			utfstring.length(), // paramSize
			false);             // isFixedType

		// Test null VARCHAR(5) value
		//
		TestStringParameter(
			0,       // paramNumber
			nullptr, // paramValue
			5,       // paramSize
			false);  // isFixedType

		// Test CHAR value with UTF-8 self-constructed encoded character (Euro sign)
		// https://en.wikipedia.org/wiki/UTF-8#Examples
		//
		string goodUTF8 = string("a") + "\xE2" + "\x82" + "\xAC";
		TestStringParameter(
			0,                 // paramNumber
			goodUTF8.c_str(),  // paramValue
			goodUTF8.length(), // paramSize
			true);             // isFixedType
	}

	// Name: InitWStringParamTest
	//
	// Description:
	//  Test multiple NCHAR and NVARCHAR values
	//
	TEST_F(PythonExtensionApiTests, InitWStringParamTest)
	{
		InitializeSession(1); // parametersNumber

		// Test simple NCHAR(5) value
		//
		TestWStringParameter(
			0,       // paramNumber
			L"HELLO",
			5,       // paramSize
			true);   // isFixedType

		// Test empty NCHAR(5) value
		//
		TestWStringParameter(
			0,       // paramNumber
			L"",
			5,       // paramSize
			true);   // isFixedType

		// Test simple NCHAR(6) value with parameter length less than size - should be padded.
		//
		TestWStringParameter(
			0,       // paramNumber
			L"WORLD",
			6,       // paramSize
			true);   // isFixedType

		// Test NCHAR(6) value with parameter length more than size - should be truncated.
		//
		TestWStringParameter(
			0,       // paramNumber
			L"PYEXTENSION",
			6,       // paramSize
			true);   // isFixedType

		// Test null NCHAR(5) value
		//
		TestWStringParameter(
			0,       // paramNumber
			nullptr, // paramValue
			5,       // paramSize
			true);   // isFixedType

		// Test simple NVARCHAR(6) value
		//
		TestWStringParameter(
			0,        // paramNumber
			L"WORLD!",
			6,        // paramSize
			false);   // isFixedType

		// Test simple NVARCHAR(8) value with parameter length less than size - NO padding.
		//
		TestWStringParameter(
			0,       // paramNumber
			L"WORLD",
			8,       // paramSize
			false);  // isFixedType

		// Test NVARCHAR(6) value with parameter length more than size - should be truncated.
		//
		TestWStringParameter(
			0,       // paramNumber
			L"PYEXTENSION",
			6,       // paramSize
			false);  // isFixedType

		// Test null NVARCHAR(5) value
		//
		TestWStringParameter(
			0,       // paramNumber
			nullptr, // paramValue
			5,       // paramSize
			false);  // isFixedType

		// Test Unicode NCHAR(2) value
		//
		TestWStringParameter(
			0,        // paramNumber
			L"你好",
			2,        // paramSize
			true);    // isFixedType

		// Test Unicode NVARCHAR value (with cyrillic)
		//
		TestWStringParameter(
			0,       // paramNumber
			L"абвг",
			10,      // paramSize
			false);  // isFixedType

		// Test NCHAR with self-constructed UTF-16 char (𐐷)
		// https://en.wikipedia.org/wiki/UTF-16#Examples
		// We need to use u16string here because wstring doesn't 
		// handle multibyte characters well in Linux with the -fshort-wchar option.
		//
		u16string goodUTF16 = u16string(u"a") + u"\xd801\xdc37" + u"b";
		TestWStringParameter(
			0,                                                    // paramNumber
			reinterpret_cast<const wchar_t *>(goodUTF16.c_str()), // paramValue
			goodUTF16.size(),                                     // paramSize
			true);                                                // isFixedType
	}

	// Name: InitBinaryParamTest
	//
	// Description:
	//  Test multiple BINARY and VARBINARY values
	//
	TEST_F(PythonExtensionApiTests, InitBinaryParamTest)
	{
		InitializeSession(1); //parametersNumber

		// Test simple binary(4)
		//
		SQLCHAR binaryValue[] = { 0x01, 0x01, 0xe2, 0x40 };

		TestRawParameter(
			0,                                  // paramNumber
			binaryValue,
			sizeof(binaryValue) / m_BinarySize, // paramSize
			true);                              // isFixedType

		// Test null binary(4)
		//
		TestRawParameter(
			0,             // paramNumber
			nullptr,       // paramValue
			4,             // paramSize
			true);         // isFixedType

		// Test binary(5) value with value less than size - should be padded.
		//
		TestRawParameter(
			0,           // paramNumber
			binaryValue,
			5,           // paramSize
			true);       // isFixedType

		// Test binary(2) value with value greater than size - should be truncated.
		//
		TestRawParameter(
			0,           // paramNumber
			binaryValue,
			2,           // paramSize
			true);       // isFixedType

		// Test simple varbinary(4)
		//
		TestRawParameter(
			0,                                  // paramNumber
			binaryValue,
			sizeof(binaryValue) / m_BinarySize, // paramSize
			false);                             // isFixedType

		// Test null varbinary(5)
		//
		TestRawParameter(
			0,             // paramNumber
			nullptr,       // paramValue
			5,             // paramSize
			false);        // isFixedType

		// Test varbinary(5) value with value less than size.
		//
		TestRawParameter(
			0,             // paramNumber
			binaryValue,
			5,             // paramSize
			false);        // isFixedType

		// Test binary(2) value with value greater than size - should be truncated.
		//
		TestRawParameter(
			0,           // paramNumber
			binaryValue,
			2,           // paramSize
			true);       // isFixedType
	}

	// Name: InitDateTimeParamTest
	//
	// Description:
	//  Test multiple DATETIME values
	//
	TEST_F(PythonExtensionApiTests, InitDateTimeParamTest)
	{
		InitializeSession(1); //parametersNumber

		// Test max SQL_TIMESTAMP_STRUCT value
		//
		SQL_TIMESTAMP_STRUCT paramValue = { 9999,12,31,23,59,59,999999 };
		TestDateTimeParameter<SQL_TIMESTAMP_STRUCT, SQL_C_TYPE_TIMESTAMP>(
			0,          // paramNumber
			paramValue
		);

		// Test min SQL_TIMESTAMP_STRUCT value
		//
		paramValue = { 1,1,1,0,0,0,0 };
		TestDateTimeParameter<SQL_TIMESTAMP_STRUCT, SQL_C_TYPE_TIMESTAMP>(
			0,          // paramNumber
			paramValue
		);

		// Test normal SQL_TIMESTAMP_STRUCT value
		//
		paramValue = { 1470,7,27,17,47,52,123456 };
		TestDateTimeParameter<SQL_TIMESTAMP_STRUCT, SQL_C_TYPE_TIMESTAMP>(
			0,          // paramNumber
			paramValue
		);

		// Test null SQL_TIMESTAMP_STRUCT value
		//
		TestDateTimeParameter<SQL_TIMESTAMP_STRUCT, SQL_C_TYPE_TIMESTAMP>(
			0,   // paramNumber
			{},  // paramValue
			true // isNull
		);
	}

	// Name: InitDateParamTest
	//
	// Description:
	//  Test multiple DATE values
	//
	TEST_F(PythonExtensionApiTests, InitDateParamTest)
	{
		InitializeSession(1); //parametersNumber

		// Test max SQL_DATE_STRUCT value
		//
		SQL_DATE_STRUCT paramValue = { 9999,12,31 };
		TestDateTimeParameter<SQL_DATE_STRUCT, SQL_C_TYPE_DATE>(
			0,          // paramNumber
			paramValue 
		);
		
		// Test min SQL_DATE_STRUCT value
		//
		paramValue = { 1,1,1 };
		TestDateTimeParameter<SQL_DATE_STRUCT, SQL_C_TYPE_DATE>(
			0,          // paramNumber
			paramValue
		);

		// Test normal SQL_DATE_STRUCT value
		//
		paramValue = { 1470,7,27 };
		TestDateTimeParameter<SQL_DATE_STRUCT, SQL_C_TYPE_DATE>(
			0,          // paramNumber
			paramValue
		);

		// Test null SQL_DATE_STRUCT value
		//
		TestDateTimeParameter<SQL_DATE_STRUCT, SQL_C_TYPE_DATE>(
			0,   // paramNumber
			{},  // paramValue
			true // isNull
			);
	}

	//
	// Negative Tests
	//

	// Name: InitNullNameParamTest
	//
	// Description:
	//  Test InitParam() API with null parameter name
	//
	TEST_F(PythonExtensionApiTests, InitNullNameParamTest)
	{
		InitializeSession(1); //parametersNumber

		SQLCHAR *paramName = nullptr;
		SQLSMALLINT paramNameLength = 0;
		SQLINTEGER paramValue = 123;

		SQLRETURN result = InitParam(
			*m_sessionId,
			m_taskId,
			0,               // paramNumber
			paramName,
			paramNameLength,
			SQL_C_SLONG,
			m_IntSize,       // argSize
			0,               // decimalDigits
			&paramValue,     // argValue
			SQL_NULL_DATA,   // strLenOrInd
			1);              // inputOutputType

		EXPECT_EQ(result, SQL_ERROR);
	}

	// Name: InitInvalidParamNumberTest
	//
	// Description:
	//  Test InitParam() API with bad param numbers (too big)
	//
	TEST_F(PythonExtensionApiTests, InitInvalidParamNumberTest)
	{
		int parametersNumber = 1;
		InitializeSession(parametersNumber);

		SQLCHAR *unsignedParamName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>("@param1")));

		SQLSMALLINT paramNameLength = 7;

		SQLINTEGER paramValue = 123;

		SQLRETURN result = InitParam(
			*m_sessionId,
			m_taskId,
			parametersNumber + 1,  // paramNumber outside of range
			unsignedParamName, // paramName
			paramNameLength,
			SQL_C_SLONG,
			m_IntSize,         // argSize
			0,                 // decimalDigits
			&paramValue,       // argValue
			SQL_NULL_DATA,     // strLenOrInd
			1);                // inputOutputType

		EXPECT_EQ(result, SQL_ERROR);
	}

	// Name: InitBadEncodingParamTest
	//
	// Description:
	//  Test InitParam() API with bad strings (out of encoding range)
	//
	TEST_F(PythonExtensionApiTests, InitBadEncodingParamTest)
	{
		InitializeSession(1);  // parametersNumber

		// Construct a bad UTF-8 string:
		// https://en.wikipedia.org/wiki/UTF-8#Encoding
		// 0xF7 defines a 4-byte character and expects three more chars of range 0x80-0xBF
		//
		string badUTF8 = string("a") + "\xF7" + "\xFF" + "b";
		TestStringParameter(
			0,                 // paramNumber
			badUTF8.c_str(),   // paramValue
			badUTF8.length(),  // paramSize
			true,              // isFixedType
			SQL_PARAM_INPUT_OUTPUT,
			false,             // validate
			false);            // expectSuccess

		// Construct a bad UTF-16 string:
		// https://en.wikipedia.org/wiki/UTF-16#Code_points_from_U+010000_to_U+10FFFF
		// 0xd800 (high surrogate) expects a low surrogate afterwards (0xdc00-0xdfff)
		// We need to use u16string here because wstring doesn't 
		// handle multibyte characters well in Linux with the -fshort-wchar option.
		//
		u16string badUTF16 = u16string(u"a") + u"\xd800\xd800" + u"b";
		TestWStringParameter(
			0,                                                   // paramNumber
			reinterpret_cast<const wchar_t *>(badUTF16.c_str()), // paramValue
			badUTF16.size(),                                     // paramSize
			true,                                                // isFixedType
			SQL_PARAM_INPUT_OUTPUT,
			false,                                               // validate
			false);                                              // expectSuccess
	}

	// Name: TestParameter
	//
	// Description:
	// Templatized function to call InitParam for the given paramValue and dataType.
	// Testing if InitParam is implemented correctly for integer/numeric/boolean dataTypes.
	//
	template<class SQLType, SQLSMALLINT dataType>
	void PythonExtensionApiTests::TestParameter(
		int         paramNumber,
		SQLType     paramValue,
		bool        isNull,
		SQLSMALLINT inputOutputType,
		bool        validate)
	{
		string paramName = "param" + to_string(paramNumber);
		string atParam = "@" + paramName;
		SQLCHAR *unsignedParamName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(atParam.c_str())));

		int paramNameLength = atParam.length();

		SQLType *pParamValue = nullptr;

		if (!isNull)
		{
			pParamValue = &(paramValue);
		}

		SQLRETURN result = SQL_ERROR;
		result = InitParam(
			*m_sessionId,
			m_taskId,
			paramNumber,
			unsignedParamName,
			paramNameLength,
			dataType,
			sizeof(SQLType),                            // paramSize
			0,                                          // decimalDigits
			pParamValue,                                // paramValue
			pParamValue != nullptr ? 0 : SQL_NULL_DATA, // strLenOrInd
			inputOutputType);                           // inputOutputType

		EXPECT_EQ(result, SQL_SUCCESS);

		if (validate)
		{
			bp::object obj(bp::extract<bp::dict>(m_mainNamespace)().get(paramName));

			if (!isNull)
			{
				ASSERT_FALSE(obj.is_none());

				if (dataType == SQL_C_BIT)
				{
					bool param = bp::extract<bool>(obj);
					EXPECT_EQ(param, paramValue != '0' && paramValue != 0);
				}
				else
				{
					SQLType param = bp::extract<SQLType>(obj);
					EXPECT_EQ(param, paramValue);
				}
			}
			else
			{
				ASSERT_TRUE(obj.is_none());
			}
		}
	}

	// Name: TestStringParameter
	//
	// Description:
	// Testing if InitParam is implemented correctly for the char/varchar dataType.
	//
	void PythonExtensionApiTests::TestStringParameter(
		int           paramNumber,
		const char    *paramValue,
		const SQLULEN paramSize,
		bool          isFixedType,
		SQLSMALLINT   inputOutputType,
		bool          validate,
		bool          expectSuccess)
	{
		string paramName = "param" + to_string(paramNumber);
		string atParam = "@" + paramName;
		SQLCHAR *unsignedParamName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(atParam.c_str())));

		int paramNameLength = atParam.length();

		vector<char> fixedParamValue(paramSize + 1);
		SQLINTEGER strLenOrInd = 0;
		char *expectedParamValue = nullptr;

		if (paramValue != nullptr)
		{
			SQLULEN paramLength = strlen(paramValue);

			copy(paramValue, paramValue + min(paramLength, paramSize), fixedParamValue.begin());

			if (isFixedType)
			{
				strLenOrInd = paramSize;

				// pad the rest of the array
				//
				for (SQLULEN index = paramLength; index < paramSize; ++index)
				{
					fixedParamValue[index] = ' ';
				}
			}
			else
			{
				strLenOrInd = min(paramLength, paramSize);
			}

			expectedParamValue = fixedParamValue.data();
		}
		else
		{
			strLenOrInd = SQL_NULL_DATA;
		}

		SQLRETURN result = SQL_ERROR;

		result = InitParam(
			*m_sessionId,
			m_taskId,
			paramNumber,
			unsignedParamName,
			paramNameLength,
			SQL_C_CHAR,
			paramSize,
			0,                  // decimalDigits
			expectedParamValue,
			strLenOrInd,
			inputOutputType);

		if (!expectSuccess)
		{
			EXPECT_EQ(result, SQL_ERROR);
		} 
		else 
		{
			EXPECT_EQ(result, SQL_SUCCESS);
			if (validate)
			{
				bp::object obj(bp::extract<bp::dict>(m_mainNamespace)().get(paramName));

				if (paramValue != nullptr)
				{
					ASSERT_FALSE(obj.is_none());

					char *param = bp::extract<char *>(obj);

					EXPECT_STREQ(param, expectedParamValue);
				}
				else
				{
					ASSERT_TRUE(obj.is_none());
				}
			}

		}
	}

	// Name: TestWStringParameter
	//
	// Description:
	// Testing if InitParam is implemented correctly for the nchar/nvarchar dataType.
	//
	void PythonExtensionApiTests::TestWStringParameter(
		int              paramNumber,
		const wchar_t    *paramValue,
		const SQLINTEGER paramSize,
		bool             isFixedType,
		SQLSMALLINT      inputOutputType,
		bool             validate,
		bool             expectSuccess)
	{
		string paramName = "param" + to_string(paramNumber);
		string atParam = "@" + paramName;
		SQLCHAR *unsignedParamName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(atParam.c_str())));

		int paramNameLength = atParam.length();

		SQLINTEGER strLenOrInd = 0;
		wchar_t *expectedParamValue = nullptr;
		vector<wchar_t> fixedParamValue;

		if (paramValue != nullptr)
		{
			SQLINTEGER paramLength = GetWStringLength(paramValue);

			fixedParamValue.assign(paramValue, paramValue + min(paramLength, paramSize));

			if (isFixedType)
			{
				strLenOrInd = paramSize;

				fixedParamValue.resize(paramSize + 1, ' ');
			}
			else
			{
				strLenOrInd = min(paramLength, paramSize);
			}

			strLenOrInd *= sizeof(wchar_t);
			expectedParamValue = fixedParamValue.data();
		}
		else
		{
			strLenOrInd = SQL_NULL_DATA;
		}

		SQLRETURN result = SQL_ERROR;

		result = InitParam(
			*m_sessionId,
			m_taskId,
			paramNumber,
			unsignedParamName,
			paramNameLength,
			SQL_C_WCHAR,
			paramSize,
			0,                  // decimalDigits
			expectedParamValue,
			strLenOrInd,
			inputOutputType);

		if (!expectSuccess)
		{
			EXPECT_EQ(result, SQL_ERROR);
		}
		else
		{
			EXPECT_EQ(result, SQL_SUCCESS);

			if (validate)
			{
				bp::object obj(bp::extract<bp::dict>(m_mainNamespace)().get(paramName));

				if (paramValue != nullptr)
				{
					ASSERT_FALSE(obj.is_none());

					// Get a byte representation of the string as UTF16.
					// PyUnicode_AsUTF16String adds a BOM to the front of every string.
					//
					char *utf16str = PyBytes_AsString(PyUnicode_AsUTF16String(obj.ptr()));

					// Ignore the 2 byte BOM added above
					//
					char *actualBytes = utf16str + 2;

					// Compare the two wstrings byte by byte because EXPECT_STREQ and EXPECT_EQ
					// don't work properly for wstrings in Linux with -fshort-wchar
					//
					char *expectedParamBytes = reinterpret_cast<char *>(expectedParamValue);

					for (SQLINTEGER i = 0; i < strLenOrInd; ++i)
					{
						EXPECT_EQ(actualBytes[i], expectedParamBytes[i]);
					}
				}
				else
				{
					ASSERT_TRUE(obj.is_none());
				}
			}
		}
	}

	// Name: TestRawParameter
	//
	// Description:
	// Testing if InitParam is implemented correctly for the binary/varbinary dataType.
	//
	void PythonExtensionApiTests::TestRawParameter(
		int           paramNumber,
		const SQLCHAR *paramValue,
		SQLULEN       paramSize,
		bool          isFixedType,
		SQLSMALLINT   inputOutputType,
		bool          validate)
	{
		string paramName = "param" + to_string(paramNumber);
		string atParam = "@" + paramName;
		SQLCHAR *unsignedParamName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(atParam.c_str())));

		int paramNameLength = atParam.length();

		vector<SQLCHAR> fixedParamValue(paramSize);
		SQLCHAR *expectedParamValue = nullptr;

		SQLULEN strLenOrInd = 0;

		if (paramValue != nullptr)
		{
			SQLULEN paramLength = strlen(static_cast<const char*>(
				static_cast<const void *>(paramValue)));

			copy(paramValue, paramValue + min(paramLength, paramSize), fixedParamValue.begin());

			if (isFixedType)
			{
				strLenOrInd = paramSize;

				// pad the rest of the array
				//
				for (SQLULEN index = paramLength; index < paramSize; ++index)
				{
					fixedParamValue[index] = ' ';
				}
			}
			else
			{
				strLenOrInd = min(paramLength, paramSize);
			}

			expectedParamValue = fixedParamValue.data();
		}
		else
		{
			strLenOrInd = SQL_NULL_DATA;
		}

		SQLRETURN result = SQL_ERROR;

		result = InitParam(
			*m_sessionId,
			m_taskId,
			paramNumber,
			unsignedParamName,
			paramNameLength,
			SQL_C_BINARY,
			paramSize,
			0,                  // decimalDigits
			expectedParamValue,
			strLenOrInd,
			inputOutputType);

		EXPECT_EQ(result, SQL_SUCCESS);

		if(validate)
		{
			bp::object obj(bp::extract<bp::dict>(m_mainNamespace)().get(paramName));

			if (paramValue != nullptr)
			{
				ASSERT_FALSE(obj.is_none());

				// The uninitialized iterator is equivalent to the end of the iterable
				//
				bp::stl_input_iterator<SQLCHAR> begin(obj), end;

				// Copy the py_buffer into a local buffer with known continguous memory.
				//
				std::vector<SQLCHAR> buffer(begin, end);

				// Always compare using strLenOrInd because
				// we copy only those many bytes into the raw parameter
				//
				for (SQLULEN i = 0; i < strLenOrInd; ++i)
				{
					EXPECT_EQ(buffer[i], expectedParamValue[i]);
				}
			}
			else
			{
				ASSERT_TRUE(obj.is_none());
			}
		}
	}

	// Name: TestDateTimeParameter
	//
	// Description:
	// Testing if InitParam is implemented correctly for the date/datetime dataTypes.
	//
	template<class DateTimeStruct, SQLSMALLINT dataType>
	void PythonExtensionApiTests::TestDateTimeParameter(
		int            paramNumber,
		DateTimeStruct paramValue,
		bool           isNull,
		SQLSMALLINT    inputOutputType,
		bool           validate)
	{
		string paramName = "param" + to_string(paramNumber);
		string atParam = "@" + paramName;
		SQLCHAR *unsignedParamName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(atParam.c_str())));

		int paramNameLength = atParam.length();

		SQLINTEGER strLenOrInd = SQL_NULL_DATA;

		if (!isNull)
		{
			strLenOrInd = sizeof(DateTimeStruct);
		}

		SQLRETURN result = SQL_ERROR;
		result = InitParam(
			*m_sessionId,
			m_taskId,
			paramNumber,
			unsignedParamName,
			paramNameLength,
			dataType,
			sizeof(DateTimeStruct), // paramSize
			0,                      // decimalDigits
			&paramValue,
			strLenOrInd,
			inputOutputType);       // inputOutputType

		EXPECT_EQ(result, SQL_SUCCESS);

		if (validate)
		{
			bp::object obj(bp::extract<bp::dict>(m_mainNamespace)().get(paramName));

			if (!isNull)
			{
				ASSERT_FALSE(obj.is_none());
				
				PyObject *dateObject = obj.ptr();

				// Import the PyDateTime API
				//
				PyDateTime_IMPORT;

				if (dataType == SQL_C_TYPE_DATE)
				{
					ASSERT_TRUE(PyDate_CheckExact(dateObject));
				}

				SQLSMALLINT year = PyDateTime_GET_YEAR(dateObject);
				SQLUSMALLINT month = PyDateTime_GET_MONTH(dateObject);
				SQLUSMALLINT day = PyDateTime_GET_DAY(dateObject);

				EXPECT_EQ(year, paramValue.year);
				EXPECT_EQ(month, paramValue.month);
				EXPECT_EQ(day, paramValue.day);

				if constexpr (dataType == SQL_C_TYPE_TIMESTAMP)
				{
					ASSERT_TRUE(PyDateTime_CheckExact(dateObject));

					SQLUSMALLINT hour = PyDateTime_DATE_GET_HOUR(dateObject);
					SQLUSMALLINT minute = PyDateTime_DATE_GET_MINUTE(dateObject);
					SQLUSMALLINT second = PyDateTime_DATE_GET_SECOND(dateObject);
					SQLUSMALLINT usec = PyDateTime_DATE_GET_MICROSECOND(dateObject);

					EXPECT_EQ(hour, paramValue.hour);
					EXPECT_EQ(minute, paramValue.minute);
					EXPECT_EQ(second, paramValue.second);

					// Fraction is stored in nanoseconds, Python uses microseconds
					//
					EXPECT_EQ(usec, paramValue.fraction / 1000);
				}
			}
			else
			{
				ASSERT_TRUE(obj.is_none());
			}
		}
	}
}
