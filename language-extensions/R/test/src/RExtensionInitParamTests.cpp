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
// @File: RExtensionApiInitParamTests.cpp
//
// Purpose:
//  Tests the RExtension's implementation of the external language InitParam API.
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
	// Name: InitIntegerParamTest
	//
	// Description:
	// Test multiple SQLINTEGER values
	//
	TEST_F(RExtensionApiTest, InitIntegerParamTest)
	{
		InitializeSession();

		// Test max INT value with respect to R
		//
		TestParameter<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
			2'147'483'647); // paramValue

		// Test min INT value with respect to R
		//
		TestParameter<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
			-2'147'483'647); // paramValue

		// Test a normal value
		//
		TestParameter<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
			4); // paramValue

		// Test out of range INT value with respect to R
		//
		TestParameter<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
			-2'147'483'648, // paramValue
			false);         // inRange

		// Test null INT value
		//
		TestParameter<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
			0,               // paramValue
			false);          // inRange
	}

	// Name: InitBitParamTest
	//
	// Description:
	// Test multiple bit values
	//
	TEST_F(RExtensionApiTest, InitBitParamTest)
	{
		InitializeSession();

		// Test '1' BIT value
		//
		TestParameter<SQLCHAR, Rcpp::LogicalVector, SQL_C_BIT>(
			'1');            // paramValue

		// Test '0' BIT value
		//
		TestParameter<SQLCHAR, Rcpp::LogicalVector, SQL_C_BIT>(
			'0');            // paramValue

		TestParameter<SQLCHAR, Rcpp::LogicalVector, SQL_C_BIT>(
			NA_LOGICAL,      // paramValue
			false);          // inRange

		// Test NULL BIT value
		//
		TestParameter<SQLCHAR, Rcpp::LogicalVector, SQL_C_BIT>(
			0,               // paramValue
			false);          // inRange

		// Test > 1 BIT value
		//
		TestParameter<SQLCHAR, Rcpp::LogicalVector, SQL_C_BIT>(
			'2');            // paramValue
	}

	// Name: InitRealParamTest
	//
	// Description:
	// Test multiple real values
	//
	TEST_F(RExtensionApiTest, InitRealParamTest)
	{
		InitializeSession();

		// Test max FLOAT(24) i.e. REAL value
		//
		TestParameter<SQLREAL, Rcpp::NumericVector, SQL_C_FLOAT>(
			3.4e38F);        // paramValue

		// Test min FLOAT(24) i.e. REAL value
		//
		TestParameter<SQLREAL, Rcpp::NumericVector, SQL_C_FLOAT>(
			-3.4e38F);       // paramValue

		// Test a normal FLOAT(24) value
		//
		TestParameter<SQLREAL, Rcpp::NumericVector, SQL_C_FLOAT>(
			2.3e4F);        // paramValue

		// Test NA_REAL FLOAT(24) i.e. REAL value - this is NaN
		//
		TestParameter<SQLREAL, Rcpp::NumericVector, SQL_C_FLOAT>(
			NA_REAL,         // paramValue
			false);          // inRange

		// Test NULL FLOAT(24) i.e. REAL value - this is NA
		//
		TestParameter<SQLREAL, Rcpp::NumericVector, SQL_C_FLOAT>(
			0,               // paramValue
			false);          // inRange
	}

	// Name: InitDoubleParamTest
	//
	// Description:
	// Test multiple double values
	//
	TEST_F(RExtensionApiTest, InitDoubleParamTest)
	{
		InitializeSession();

		// Test max FLOAT(53) i.e. DOUBLE PRECISION value
		//
		TestParameter<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
			1.79e308);       // paramValue

		// Test min FLOAT(53) i.e. DOUBLE PRECISION value
		//
		TestParameter<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
			-1.79e308);      // paramValue

		// Test normal FLOAT(53) i.e. DOUBLE PRECISION value
		//
		TestParameter<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
			1.45e38);       // paramValue

		// Test NA_REAL FLOAT(53) value i.e. DOUBLE PRECISION value
		//
		TestParameter<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
			NA_REAL,         // paramValue
			false);          // inRange

		// Test NULL FLOAT(53) value i.e. DOUBLE PRECISION value
		//
		TestParameter<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
			0,               // paramValue
			false);          // inRange
	}

	// Name: InitBigIntParamTest
	//
	// Description:
	// Test multiple big int values
	//
	TEST_F(RExtensionApiTest, InitBigIntParamTest)
	{
		InitializeSession();

		// Test max BIGINT value
		//
		TestParameter<SQLBIGINT, Rcpp::NumericVector, SQL_C_SBIGINT>(
			9'223'372'036'854'775'807LL); // paramValue

		// Test min BIGINT value : -9'223'372'036'854'775'808 gives compiler error
		// In R, -9'223'372'036'854'775'807 = -9.223372e+18
		//
		TestParameter<SQLBIGINT, Rcpp::NumericVector, SQL_C_SBIGINT>(
			-9'223'372'036'854'775'807LL); // paramValue

		// Test normal BIGINT value
		//
		TestParameter<SQLBIGINT, Rcpp::NumericVector, SQL_C_SBIGINT>(
			9'372'036'854'775LL); // paramValue

		// Test NA_REAL BIGINT value - this is actually in range
		//
		TestParameter<SQLBIGINT, Rcpp::NumericVector, SQL_C_SBIGINT>(
			NA_REAL);         // paramValue

		// Test NULL BIGINT value - need to send a NULL value to test NA_REAL for BIGINT
		//
		TestParameter<SQLBIGINT, Rcpp::NumericVector, SQL_C_SBIGINT>(
			0,               // paramValue
			false);          // inRange
	}

	// Name: InitSmallIntParamTest
	//
	// Description:
	// Test multiple small int values
	//
	TEST_F(RExtensionApiTest, InitSmallIntParamTest)
	{
		InitializeSession();

		// Test max SMALLINT value
		//
		TestParameter<SQLSMALLINT, Rcpp::IntegerVector, SQL_C_SSHORT>(
			32'767);         // paramValue

		// Test min SMALLINT value
		//
		TestParameter<SQLSMALLINT, Rcpp::IntegerVector, SQL_C_SSHORT>(
			-32'768);        // paramValue

		// Test normal SMALLINT value
		//
		TestParameter<SQLSMALLINT, Rcpp::IntegerVector, SQL_C_SSHORT>(
			3'007);         // paramValue

		// Test NA_INTEGER SMALLINT value
		//
		TestParameter<SQLSMALLINT, Rcpp::IntegerVector, SQL_C_SSHORT>(
			NA_INTEGER,      // paramValue
			false);          // inRange

		// Test NULL SMALLINT value
		//
		TestParameter<SQLSMALLINT, Rcpp::IntegerVector, SQL_C_SSHORT>(
			0,               // paramValue
			false);          // inRange
	}

	// Name: InitTinyIntParamTest
	//
	// Description:
	// Test multiple tiny int values
	//
	TEST_F(RExtensionApiTest, InitTinyIntParamTest)
	{
		InitializeSession();

		// Test max TINYINT value
		//
		TestParameter<SQLCHAR, Rcpp::IntegerVector, SQL_C_UTINYINT>(
			255);            // paramValue

		// Test min TINYINT value
		//
		TestParameter<SQLCHAR, Rcpp::IntegerVector, SQL_C_UTINYINT>(
			0);              // paramValue

		// Test normal TINYINT value
		//
		TestParameter<SQLCHAR, Rcpp::IntegerVector, SQL_C_UTINYINT>(
			123);            // paramValue

		// Test NA_INTEGER TINYINT value with respect to R
		//
		TestParameter<SQLCHAR, Rcpp::IntegerVector, SQL_C_UTINYINT>(
			NA_INTEGER,      // paramValue
			false);          // inRange

		// Test NULL TINYINT value
		//
		TestParameter<SQLCHAR, Rcpp::IntegerVector, SQL_C_UTINYINT>(
			0,               // paramValue
			false);          // inRange

		// Test -1 TINYINT value converted to 255
		//
		TestParameter<SQLCHAR, Rcpp::IntegerVector, SQL_C_UTINYINT>(
			-1);             // paramValue
	}

	// Name: InitCharParamTest
	//
	// Description:
	// Test multiple character values
	//
	TEST_F(RExtensionApiTest, InitCharParamTest)
	{
		InitializeSession();

		// Test simple CHAR(5) value
		//
		TestCharParameter(
			"HELLO",
			5,     // paramSize
			true); // isFixedType

		// Test simple CHAR(6) value with parameter length less than size - should be padded.
		//
		TestCharParameter(
			"WORLD",
			6,     // paramSize
			true); // isFixedType

		// Test null CHAR(5) value
		//
		TestCharParameter(
			nullptr,  // paramValue
			5,        // paramSize
			true);    // isFixedType

		// Test simple VARCHAR(6) value
		//
		TestCharParameter(
			"WORLD!",
			6,      // paramSize
			false); // isFixedType

		// Test simple VARCHAR(8) value with parameter length less than size - no padding.
		//
		TestCharParameter(
			"WORLD",
			8,      // paramSize
			false); // isFixedType

		// Test null VARCHAR(5) value
		//
		TestCharParameter(
			nullptr,  // paramValue
			5,        // paramSize
			false);   // isFixedType
	}

	// Name: InitBinaryParamTest
	//
	// Description:
	// Test multiple binary values
	//
	TEST_F(RExtensionApiTest, InitBinaryParamTest)
	{
		InitializeSession();

		// Test simple binary(4) value
		//
		SQLCHAR binaryValue[] = {0x00, 0x01, 0xe2, 0x40};
		TestBinaryParameter(
			binaryValue,
			sizeof(binaryValue)/sizeof(SQLCHAR), // strLenOrInd
			sizeof(binaryValue)/sizeof(SQLCHAR), // paramSize
			true);  // isFixedType

		// Test null binary(4) value
		//
		TestBinaryParameter(
			nullptr, // paramValue
			-1,      // strLenOrInd
			4,       // paramSize
			true);   // isFixedType

		// Test binary(5) value with length less than size - should be padded.
		//
		TestBinaryParameter(
			binaryValue,
			5,       // strLenOrInd
			5,       // paramSize
			true);   // isFixedType

		// Test simple varbinary(4) value
		//
		TestBinaryParameter(
			binaryValue,
			sizeof(binaryValue)/sizeof(SQLCHAR), // strLenOrInd
			sizeof(binaryValue)/sizeof(SQLCHAR), // paramSize
			false);  // isFixedType

		// Test null varbinary(5) value
		//
		TestBinaryParameter(
			nullptr, // paramValue
			-1,      // strLenOrInd
			4,       // paramSize
			false);  // isFixedType

		// Test varbinary(5) value with length less than size.
		//
		TestBinaryParameter(
			binaryValue,
			sizeof(binaryValue)/sizeof(SQLCHAR), // strLenOrInd
			5,       // paramSize
			false);  // isFixedType
	}

	// Name: TestParameter
	//
	// Description:
	// Templatized function to call InitParam for the given paramValue and dataType.
	// Testing if InitParam is implemented correctly for integer/numeric/logical dataTypes.
	//
	template<class SQLType, class RType, SQLSMALLINT dataType>
	void RExtensionApiTest::TestParameter(
		SQLType paramValue,
		bool inRange)
	{
		unique_ptr<SQLType> pParamValue = nullptr;

		// Generate pParamValue if the value is in range OR if its a non-zero out of range value.
		// inRange is with respect to the type in R and not with respect to the Cpp type range.
		//
		if (inRange || paramValue != 0)
		{
			pParamValue = make_unique<SQLType>(paramValue);
		}

		SQLRETURN result = SQL_ERROR;
		result = (*m_initParamFuncPtr)(
				*m_sessionId,
				m_taskId,
				0,                // paramNumber
				m_paramName,
				m_paramNameString.length(),
				dataType,
				sizeof(SQLType),  // paramSize
				0,                // decimalDigits
				pParamValue != nullptr ? pParamValue.get() : nullptr, // paramValue
				pParamValue != nullptr ? 0 : -1, // strLenOrInd
				1);               // inputOutputType
		EXPECT_EQ(result, SQL_SUCCESS);

		// Do + 1 to skip the @ from the paramName
		//
		RType param = m_globalEnvironment[m_paramNameString.c_str() + 1];
		if (inRange)
		{
			if (dataType == SQL_C_BIT)
			{
				EXPECT_EQ(param[0], paramValue != '0' ? true : false);
			}
			else
			{
				EXPECT_EQ(param[0], paramValue);
			}
		}
		else
		{
			EXPECT_TRUE(RType::is_na(param[0]));
		}
	}

	// Name: TestCharParameter
	//
	// Description:
	// Testing if InitParam is implemented correctly for the char/varchar dataType.
	//
	void RExtensionApiTest::TestCharParameter(
		const char *paramValue,
		SQLULEN     paramSize,
		bool        isFixedType)
	{
		char fixedParamValue[paramSize+1] = {0};
		SQLINTEGER strLenOrInd = 0;
		char *expectedParamValue = nullptr;

		if (paramValue != nullptr)
		{
			SQLINTEGER paramLength = strlen(paramValue);
			if (isFixedType)
			{
				memcpy(fixedParamValue, paramValue, paramLength);
				strLenOrInd = static_cast<SQLINTEGER>(paramSize);

				// pad the rest of the array
				//
				for (SQLINTEGER index = paramLength; index < strLenOrInd; ++index)
				{
					fixedParamValue[index] = ' ';
				}

				expectedParamValue = fixedParamValue;
			}
			else
			{
				expectedParamValue = const_cast<char*>(paramValue);
				strLenOrInd = paramLength;
			}
		}
		else
		{
			strLenOrInd = -1;
		}

		SQLRETURN result = SQL_ERROR;

		// Even though paramSize doesnt include the null terminator,
		// we create a CharacterVector parameter with a null terminator string
		//
		result = (*m_initParamFuncPtr)(
				*m_sessionId,
				m_taskId,
				0,                // paramNumber
				m_paramName,
				m_paramNameString.length(),
				SQL_C_CHAR,
				paramSize,
				0,               // decimalDigits
				expectedParamValue,
				strLenOrInd,
				1);              // inputOutputType

		EXPECT_EQ(result, SQL_SUCCESS);

		// Do + 1 to skip the @ from the paramName
		//
		Rcpp::CharacterVector param = m_globalEnvironment[m_paramNameString.c_str() + 1];
		if (paramValue != nullptr)
		{
			EXPECT_EQ(param[0], const_cast<const char*>(expectedParamValue));
		}
		else
		{
			EXPECT_EQ(param[0], NA_STRING);
		}
	}

	// Name: TestBinaryParameter
	//
	// Description:
	// Testing if InitParam is implemented correctly for the binary/varbinary dataType.
	//
	void RExtensionApiTest::TestBinaryParameter(
		const SQLCHAR  paramValue[],
		SQLINTEGER     strLenOrInd,
		SQLULEN        paramSize,
		bool           isFixedType)
	{
		SQLCHAR fixedParamValue[paramSize] = {0};
		SQLCHAR *expectedParamValue = nullptr;

		if (paramValue != nullptr)
		{
			if (isFixedType)
			{
				memcpy(fixedParamValue, paramValue, strLenOrInd);
				expectedParamValue = fixedParamValue;
			}
			else
			{
				expectedParamValue = const_cast<SQLCHAR*>(paramValue);
			}
		}

		SQLRETURN result = SQL_ERROR;
		result = (*m_initParamFuncPtr)(
				*m_sessionId,
				m_taskId,
				0,                // paramNumber
				m_paramName,
				m_paramNameString.length(),
				SQL_C_BINARY,
				paramSize,
				0,                  // decimalDigits
				expectedParamValue,
				strLenOrInd,
				1);                 // inputOutputType
		EXPECT_EQ(result, SQL_SUCCESS);

		// Do + 1 to skip the @ from the paramName
		//
		Rcpp::RawVector param = m_globalEnvironment[m_paramNameString.c_str() + 1];

		if (paramValue != nullptr)
		{
			// Always compare using strLenOrInd because
			// we copy only those many bytes into the raw parameter
			//
			for (SQLINTEGER i = 0; i < strLenOrInd; ++i)
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