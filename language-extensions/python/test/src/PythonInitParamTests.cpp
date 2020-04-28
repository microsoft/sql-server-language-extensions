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
#include <memory>

using namespace std;
namespace py = boost::python;

namespace ExtensionApiTest
{
	// Negative test
	// Test InitParam() API with null parameter name
	//
	TEST_F(PythonExtensionApiTests, InitNullNameParamTest)
	{
		InitializeSession();

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
			sizeof(SLONG),   // argSize
			0,               // decimalDigits
			&paramValue,     // argValue
			SQL_NULL_DATA,   // strLenOrInd
			1);              // inputOutputType

		EXPECT_EQ(result, SQL_ERROR);
	}

	// Negative test
	// Test InitParam() API with bad param numbers (too big)
	//
	TEST_F(PythonExtensionApiTests, InitInvalidParamNumberTest)
	{
		InitializeSession();

		SQLCHAR *unsignedParamName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(("@" + m_paramName).c_str())));

		SQLSMALLINT paramNameLength = m_paramNameLength + 1;

		SQLINTEGER paramValue = 123;

		SQLRETURN result = InitParam(
			*m_sessionId,
			m_taskId,
			m_parametersNumber + 1, // paramNumber outside of range
			unsignedParamName,      // paramName
			paramNameLength,
			SQL_C_SLONG,
			sizeof(SQLINTEGER),     // argSize
			0,                      // decimalDigits
			&paramValue,            // argValue
			SQL_NULL_DATA,          // strLenOrInd
			1);                     // inputOutputType

		EXPECT_EQ(result, SQL_ERROR);
	}

	// Test multiple SQLINTEGER values
	//
	TEST_F(PythonExtensionApiTests, InitIntegerParamTest)
	{
		InitializeSession();

		// Test max INT value
		//
		TestParameter<SQLINTEGER, SQL_C_SLONG>(
			2147483647);

		// Test min INT value
		//
		TestParameter<SQLINTEGER, SQL_C_SLONG>(
			-2147483647);

		// Test a normal value
		//
		TestParameter<SQLINTEGER, SQL_C_SLONG>(
			4);

		// Test null INT value
		//
		TestParameter<SQLINTEGER, SQL_C_SLONG>(
			0,
			true); // isNull
	}

	// Test multiple BIT values
	//
	TEST_F(PythonExtensionApiTests, InitBitParamTest)
	{
		InitializeSession();

		// Test '1' BIT value
		//
		TestParameter<SQLCHAR, SQL_C_BIT>(
			'1');

		// Test '0' BIT value
		//
		TestParameter<SQLCHAR, SQL_C_BIT>(
			'0');

		// Test null BIT value
		//
		TestParameter<SQLCHAR, SQL_C_BIT>(
			0,
			true); // isNull

		// Test > 1 BIT value
		//
		TestParameter<SQLCHAR, SQL_C_BIT>(
			'2');
	}

	// Test multiple FLOAT(24) (SQLREAL) values
	//
	TEST_F(PythonExtensionApiTests, InitFloatParamTest)
	{
		InitializeSession();

		// Test max FLOAT(24) i.e. REAL value
		//
		TestParameter<SQLREAL, SQL_C_FLOAT>(
			3.4e38F);

		// Test min FLOAT(24) i.e. REAL value
		//
		TestParameter<SQLREAL, SQL_C_FLOAT>(
			-3.4e38F);

		// Test a normal FLOAT(24) value
		//
		TestParameter<SQLREAL, SQL_C_FLOAT>(
			2.3e4F);

		// Test null FLOAT(24) i.e. REAL value
		//
		TestParameter<SQLREAL, SQL_C_FLOAT>(
			0,
			true); // isNull
	}

	// Test multiple FLOAT(53) (DOUBLE) values
	//
	TEST_F(PythonExtensionApiTests, InitDoubleParamTest)
	{
		InitializeSession();

		// Test max FLOAT(53) i.e. DOUBLE PRECISION value
		//
		TestParameter<SQLDOUBLE, SQL_C_DOUBLE>(
			1.79e308);

		// Test min FLOAT(53) i.e. DOUBLE PRECISION value
		//
		TestParameter<SQLDOUBLE, SQL_C_DOUBLE>(
			-1.79e308);

		// Test normal FLOAT(53) i.e. DOUBLE PRECISION value
		//
		TestParameter<SQLDOUBLE, SQL_C_DOUBLE>(
			1.45e38);

		// Test null FLOAT(53) value
		//
		TestParameter<SQLDOUBLE, SQL_C_DOUBLE>(
			0,
			true); // isNull
	}

	// Test multiple SQLBIGINT values
	//
	TEST_F(PythonExtensionApiTests, InitBigIntParamTest)
	{
		InitializeSession();

		// Test max BIGINT value
		//
		TestParameter<SQLBIGINT, SQL_C_SBIGINT>(
			9223372036854775807LL);

		// Test min BIGINT value : -9223372036854775808 gives compiler error
		//
		TestParameter<SQLBIGINT, SQL_C_SBIGINT>(
			-9223372036854775807LL);

		// Test normal BIGINT value
		//
		TestParameter<SQLBIGINT, SQL_C_SBIGINT>(
			9'372'036'854'775LL);

		// Test null BIGINT value
		//
		TestParameter<SQLBIGINT, SQL_C_SBIGINT>(
			0,
			true); // isNull
	}

	// Test multiple TINYINT values
	//
	TEST_F(PythonExtensionApiTests, InitTinyIntParamTest)
	{
		InitializeSession();

		// Test max TINYINT value
		//
		TestParameter<SQLCHAR, SQL_C_UTINYINT>(
			255);

		// Test min TINYINT value
		//
		TestParameter<SQLCHAR, SQL_C_UTINYINT>(
			0);

		// Test normal TINYINT value
		//
		TestParameter<SQLCHAR, SQL_C_UTINYINT>(
			123);

		// Test null TINYINT value
		//
		TestParameter<SQLCHAR, SQL_C_UTINYINT>(
			0,
			true); // isNull

		// Test -1 TINYINT value converted to 255
		//
		TestParameter<SQLCHAR, SQL_C_UTINYINT>(
			-1);
	}

	// Test multiple SMALLINT values
	//
	TEST_F(PythonExtensionApiTests, InitSmallIntParamTest)
	{
		InitializeSession();

		// Test max SMALLINT value
		//
		TestParameter<SQLSMALLINT, SQL_C_SSHORT>(
			32767);

		// Test min SMALLINT value
		//
		TestParameter<SQLSMALLINT, SQL_C_SSHORT>(
			-32768);

		// Test normal SMALLINT value
		//
		TestParameter<SQLSMALLINT, SQL_C_SSHORT>(
			3'007);

		// Test null SMALLINT value
		//
		TestParameter<SQLSMALLINT, SQL_C_SSHORT>(
			0,
			true); // isNull
	}

	// Test multiple CHAR and VARCHAR values
	//
	TEST_F(PythonExtensionApiTests, InitCharParamTest)
	{
		InitializeSession();

		// Test simple CHAR(5) value
		//
		TestStringParameter(
			"HELLO",
			5,       // paramSize
			true);   // isFixedType

		// Test simple CHAR(6) value with parameter length less than size - should be padded.
		//
		TestStringParameter(
			"WORLD",
			6,       // paramSize
			true);   // isFixedType

		// Test null CHAR(5) value
		//
		TestStringParameter(
			nullptr, // paramValue
			5,       // paramSize
			true);   // isFixedType

		// Test simple VARCHAR(6) value
		//
		TestStringParameter(
			"WORLD!",
			6,        // paramSize
			false);   // isFixedType

		// Test simple VARCHAR(8) value with parameter length less than size - no padding.
		//
		TestStringParameter(
			"WORLD",
			8,       // paramSize
			false);  // isFixedType

		// Test null VARCHAR(5) value
		//
		TestStringParameter(
			nullptr, // paramValue
			5,       // paramSize
			false);  // isFixedType
	}

	// Test multiple BINARY and VARBINARY values
	//
	TEST_F(PythonExtensionApiTests, InitBinaryParamTest)
	{
		InitializeSession();

		// Test simple binary(4) value
		//
		SQLCHAR binaryValue[] = { 0x01, 0x01, 0xe2, 0x40 };

		TestRawParameter(
			binaryValue,
			sizeof(binaryValue) / sizeof(SQLCHAR), // strLenOrInd
			sizeof(binaryValue) / sizeof(SQLCHAR), // paramSize
			true);                                 // isFixedType

		// Test null binary(4) value
		//
		TestRawParameter(
			nullptr,       // paramValue
			SQL_NULL_DATA, // strLenOrInd
			4,             // paramSize
			true);         // isFixedType

		// Test binary(5) value with length less than size - should be padded.
		//
		TestRawParameter(
			binaryValue,
			5,           // strLenOrInd
			5,           // paramSize
			true);       // isFixedType

		// Test simple varbinary(4) value
		//
		TestRawParameter(
			binaryValue,
			sizeof(binaryValue) / sizeof(SQLCHAR), // strLenOrInd
			sizeof(binaryValue) / sizeof(SQLCHAR), // paramSize
			false);                                // isFixedType

		// Test null varbinary(5) value
		//
		TestRawParameter(
			nullptr,       // paramValue
			SQL_NULL_DATA, // strLenOrInd
			4,             // paramSize
			false);        // isFixedType

		// Test varbinary(5) value with length less than size.
		//
		TestRawParameter(
			binaryValue,
			sizeof(binaryValue) / sizeof(SQLCHAR), // strLenOrInd
			5,                                     // paramSize
			false);                                // isFixedType
	}

	// Name: TestParameter
	//
	// Description:
	// Templatized function to call InitParam for the given paramValue and dataType.
	// Testing if InitParam is implemented correctly for integer/numeric/boolean dataTypes.
	//
	template<class SQLType, SQLSMALLINT dataType>
	void PythonExtensionApiTests::TestParameter(
		SQLType paramValue,
		bool    isNull)
	{
		SQLCHAR *unsignedParamName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(("@" + m_paramName).c_str())));

		int paramNameLength = m_paramNameLength + 1;

		SQLType *pParamValue = nullptr;

		if (!isNull)
		{
			pParamValue = &(paramValue);
		}

		SQLRETURN result = SQL_ERROR;
		result = InitParam(
			*m_sessionId,
			m_taskId,
			0,                                          // paramNumber
			unsignedParamName,
			paramNameLength,
			dataType,
			sizeof(SQLType),                            // paramSize
			0,                                          // decimalDigits
			pParamValue,                                // paramValue
			pParamValue != nullptr ? 0 : SQL_NULL_DATA, // strLenOrInd
			1);                                         // inputOutputType

		EXPECT_EQ(result, SQL_SUCCESS);

		py::object obj(py::extract<py::dict>(m_mainNamespace)().get(m_paramName));

		if (!isNull)
		{
			EXPECT_FALSE(obj.is_none());

			if (dataType == SQL_C_BIT)
			{
				bool param = py::extract<bool>(obj);
				EXPECT_EQ(param, paramValue != '0' ? true : false);
			}
			else
			{
				SQLType param = py::extract<SQLType>(obj);
				EXPECT_EQ(param, paramValue);
			}
		}
		else
		{
			EXPECT_TRUE(obj.is_none());
		}
	}

	// Name: TestStringParameter
	//
	// Description:
	// Testing if InitParam is implemented correctly for the char/varchar dataType.
	//
	void PythonExtensionApiTests::TestStringParameter(
		const char    *paramValue,
		const SQLULEN paramSize,
		bool          isFixedType)
	{
		SQLCHAR *unsignedParamName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(("@" + m_paramName).c_str())));

		int paramNameLength = m_paramNameLength + 1;

		vector<char> fixedParamValue(paramSize + 1);
		SQLINTEGER strLenOrInd = 0;
		char *expectedParamValue = nullptr;

		if (paramValue != nullptr)
		{
			SQLINTEGER paramLength = strlen(paramValue);
			if (isFixedType)
			{
				memcpy(&(fixedParamValue[0]), paramValue, paramLength);
				strLenOrInd = static_cast<SQLINTEGER>(paramSize);

				// pad the rest of the array
				//
				for (SQLINTEGER index = paramLength; index < strLenOrInd; ++index)
				{
					fixedParamValue[index] = ' ';
				}

				expectedParamValue = &(fixedParamValue[0]);
			}
			else
			{
				expectedParamValue = const_cast<char*>(paramValue);
				strLenOrInd = paramLength;
			}
		}
		else
		{
			strLenOrInd = SQL_NULL_DATA;
		}

		SQLRETURN result = SQL_ERROR;

		result = InitParam(
			*m_sessionId,
			m_taskId,
			0,                  // paramNumber
			unsignedParamName,
			paramNameLength,
			SQL_C_CHAR,
			paramSize,
			0,                  // decimalDigits
			expectedParamValue,
			strLenOrInd,
			1);                 // inputOutputType

		EXPECT_EQ(result, SQL_SUCCESS);

		py::object obj(py::extract<py::dict>(m_mainNamespace)().get(m_paramName));

		if (paramValue != nullptr)
		{
			EXPECT_FALSE(obj.is_none());

			char *param = py::extract<char *>(obj);

			EXPECT_STREQ(param, expectedParamValue);
		}
		else
		{
			EXPECT_TRUE(obj.is_none());
		}
	}

	// Name: TestRawParameter
	//
	// Description:
	// Testing if InitParam is implemented correctly for the binary/varbinary dataType.
	//
	void PythonExtensionApiTests::TestRawParameter(
		const SQLCHAR paramValue[],
		SQLINTEGER    strLenOrInd,
		SQLULEN       paramSize,
		bool          isFixedType)
	{
		SQLCHAR *unsignedParamName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(("@" + m_paramName).c_str())));

		int paramNameLength = m_paramNameLength + 1;

		vector<SQLCHAR> fixedParamValue(paramSize);
		SQLCHAR *expectedParamValue = nullptr;

		if (paramValue != nullptr)
		{
			if (isFixedType)
			{
				memcpy(&(fixedParamValue[0]), paramValue, strLenOrInd);
				expectedParamValue = &(fixedParamValue[0]);
			}
			else
			{
				expectedParamValue = const_cast<SQLCHAR*>(paramValue);
			}
		}

		SQLRETURN result = SQL_ERROR;
		result = InitParam(
			*m_sessionId,
			m_taskId,
			0,                  // paramNumber
			unsignedParamName,
			paramNameLength,
			SQL_C_BINARY,
			paramSize,
			0,                  // decimalDigits
			expectedParamValue,
			strLenOrInd,
			1);                 // inputOutputType

		EXPECT_EQ(result, SQL_SUCCESS);

		py::object obj(py::extract<py::dict>(m_mainNamespace)().get(m_paramName));

		try
		{
			if (paramValue != nullptr)
			{
				EXPECT_FALSE(obj.is_none());

				// The uninitialized iterator is equivalent to the end of the iterable
				//
				py::stl_input_iterator<SQLCHAR> begin(obj), end;

				// Copy the py_buffer into a local buffer with known continguous memory.
				//
				std::vector<SQLCHAR> buffer(begin, end);

				// Always compare using strLenOrInd because
				// we copy only those many bytes into the raw parameter
				//
				for (SQLINTEGER i = 0; i < strLenOrInd; ++i)
				{
					EXPECT_EQ(buffer[i], expectedParamValue[i]);
				}
			}
			else
			{
				EXPECT_TRUE(obj.is_none());
			}
		}
		catch (py::error_already_set&)
		{
			throw runtime_error("Error in python converting bytes object back to C++");
		}
		catch (exception e)
		{
			throw runtime_error("Error loading main module and namespace");
		}
	}
}
