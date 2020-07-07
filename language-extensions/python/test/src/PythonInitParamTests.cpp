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

	// Negative test
	// Test InitParam() API with bad param numbers (too big)
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

	// Test multiple SQLINTEGER values
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

	// Test multiple BIT values
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

		// Test null BIT value
		//
		TestParameter<SQLCHAR, SQL_C_BIT>(0, //paramNumber
			0,
			true); // isNull

		// Test > 1 BIT value
		//
		TestParameter<SQLCHAR, SQL_C_BIT>(0, //paramNumber
			'2');
	}

	// Test multiple FLOAT(24) (SQLREAL) values
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

	// Test multiple FLOAT(53) (DOUBLE) values
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

	// Test multiple SQLBIGINT values
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

	// Test multiple TINYINT values
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

	// Test multiple SMALLINT values
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

	// Test multiple CHAR and VARCHAR values
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

		// Test null VARCHAR(5) value
		//
		TestStringParameter(
			0,       // paramNumber
			nullptr, // paramValue
			5,       // paramSize
			false);  // isFixedType
	}


	// Test multiple NCHAR and NVARCHAR values
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
			true);   // isFixedType

		// Test Unicode NVARCHAR(6) value
		//
		TestWStringParameter(
			0,        // paramNumber
			L"你好",
			6,        // paramSize
			false);   // isFixedType
	}

	// Test multiple BINARY and VARBINARY values
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
			0,                                  // paramNumber
			binaryValue,
			5,                                  // paramSize
			false);                             // isFixedType

		// Test binary(2) value with value greater than size - should be truncated.
		//
		TestRawParameter(
			0,           // paramNumber
			binaryValue,
			2,           // paramSize
			true);       // isFixedType
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
		SQLSMALLINT inputOutputType,
		bool        isNull,
		bool        validate)
	{
		string paramName = "param" + to_string(paramNumber);
		SQLCHAR *unsignedParamName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(("@" + paramName).c_str())));

		int paramNameLength = paramName.length() + 1;

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
			py::object obj(py::extract<py::dict>(m_mainNamespace)().get(paramName));

			if (!isNull)
			{
				ASSERT_FALSE(obj.is_none());

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
		bool          validate)
	{
		string paramName = "param" + to_string(paramNumber);
		SQLCHAR *unsignedParamName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(("@" + paramName).c_str())));

		// +1 to accomodate "@"
		//
		int paramNameLength = paramName.length() + 1;

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

		EXPECT_EQ(result, SQL_SUCCESS);

		if (validate)
		{
			py::object obj(py::extract<py::dict>(m_mainNamespace)().get(paramName));

			if (paramValue != nullptr)
			{
				ASSERT_FALSE(obj.is_none());

				char *param = py::extract<char *>(obj);

				EXPECT_STREQ(param, expectedParamValue);
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
	// Testing if InitParam is implemented correctly for the nchar/nvarchar dataType.
	//
	void PythonExtensionApiTests::TestWStringParameter(
		int           paramNumber,
		const wchar_t *paramValue,
		const SQLULEN paramSize,
		bool          isFixedType,
		SQLSMALLINT   inputOutputType,
		bool          validate)
	{
		string paramName = "param" + to_string(paramNumber);
		SQLCHAR *unsignedParamName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(("@" + paramName).c_str())));

		// +1 to accomodate "@"
		//
		int paramNameLength = paramName.length() + 1;

		vector<wchar_t> fixedParamValue(paramSize + 1);
		SQLINTEGER strLenOrInd = 0;
		wchar_t *expectedParamValue = nullptr;

		if (paramValue != nullptr)
		{
			SQLULEN paramLength = wcslen(paramValue);

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

		EXPECT_EQ(result, SQL_SUCCESS);

		if (validate)
		{
			py::object obj(py::extract<py::dict>(m_mainNamespace)().get(paramName));

			if (paramValue != nullptr)
			{
				ASSERT_FALSE(obj.is_none());

				SQLINTEGER strlen = strLenOrInd / sizeof(wchar_t);

				// Convert to then extract from python object
				// because of differences in wchar handling
				//
				py::object tempObj = py::object(py::handle<>(
					PyUnicode_FromKindAndData(sizeof(wchar_t), expectedParamValue, strlen)
					));

				wstring expected = py::extract<wstring>(tempObj);
				wstring actual = py::extract<wstring>(obj);

				EXPECT_STREQ(actual.c_str(), expected.c_str());
			}
			else
			{
				ASSERT_TRUE(obj.is_none());
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
		SQLCHAR *unsignedParamName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(("@" + paramName).c_str())));

		int paramNameLength = paramName.length() + 1;

		vector<SQLCHAR> fixedParamValue(paramSize);
		SQLCHAR *expectedParamValue = nullptr;

		SQLINTEGER strLenOrInd = 0;

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
			py::object obj(py::extract<py::dict>(m_mainNamespace)().get(paramName));

			if (paramValue != nullptr)
			{
				ASSERT_FALSE(obj.is_none());

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
				ASSERT_TRUE(obj.is_none());
			}
		}
	}
}
