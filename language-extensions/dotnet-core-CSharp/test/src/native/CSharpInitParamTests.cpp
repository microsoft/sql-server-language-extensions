//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpInitParamTests.cpp
//
// Purpose:
//  Test the .NET Core CSharp extension parameters using the Extension API
//
//*********************************************************************
#include "CSharpExtensionApiTests.h"

using namespace std;

namespace ExtensionApiTest
{
    //----------------------------------------------------------------------------------------------
    // Name: InitIntegerParamTest
    //
    // Description:
    //  Tests multiple SQLINTEGER values.
    //
    TEST_F(CSharpExtensionApiTests, InitIntegerParamTest)
    {
        InitializeSession(
            0,  // inputSchemaColumnsNumber
            4); // parametersNumber

        // Test max INT value
        //
        InitParam<SQLINTEGER, SQL_C_SLONG>(
            0,           // paramNumber
            2147483647); // max INT value

        // Test min INT value
        //
        InitParam<SQLINTEGER, SQL_C_SLONG>(
            1,            // paramNumber
            -2147483647); // min INT value

        // Test a normal value
        //
        InitParam<SQLINTEGER, SQL_C_SLONG>(
            2,  // paramNumber
            4); // normal int value

        // Test null INT value
        //
        InitParam<SQLINTEGER, SQL_C_SLONG>(
            3,     // paramNumber
            0,     // paramValue
            true); // isNull

        // Test invalid parameter number
        //
        InitParam<SQLINTEGER, SQL_C_SLONG>(
            4,                      // invalid paramNumber
            0,                      // paramValue
            false,                  // isNULL
            SQL_PARAM_INPUT_OUTPUT, // inputOutputType
            SQL_ERROR);             // SQLReturn

        // Test negative parameter number
        //
        InitParam<SQLINTEGER, SQL_C_SLONG>(
            -1,                     // negative paramNumber
            0,                      // paramValue
            false,                  // isNULL
            SQL_PARAM_INPUT_OUTPUT, // inputOutputType
            SQL_ERROR);             // SQLReturn
    }

    //----------------------------------------------------------------------------------------------
    // Name: InitBitParamTest
    //
    // Description:
    //  Test multiple BIT values
    //
    TEST_F(CSharpExtensionApiTests, InitBitParamTest)
    {
        InitializeSession(
            0,  // inputSchemaColumnsNumber
            1); // parametersNumber

        // Test '1' BIT value
        //
        InitParam<SQLCHAR, SQL_C_BIT>(
            0,    // paramNumber
            '1'); // paramValue

        // Test '0' BIT value
        //
        InitParam<SQLCHAR, SQL_C_BIT>(
            0,    // paramNumber
            '0'); // paramValue

        // Test 1 BIT value
        //
        InitParam<SQLCHAR, SQL_C_BIT>(
            0,  // paramNumber
            1); // paramValue

        // Test 0 BIT value
        //
        InitParam<SQLCHAR, SQL_C_BIT>(
            0,  // paramNumber
            0); // paramValue

        // Test null BIT value
        //
        InitParam<SQLCHAR, SQL_C_BIT>(
            0,     // paramNumber
            0,     // paramValue
            true); // isNull

        // Test > 1 BIT value, should be True
        //
        InitParam<SQLCHAR, SQL_C_BIT>(
            0,  // paramNumber
            2); // paramValue

        // Test != '1' and != '0' BIT value, should be True
        //
        InitParam<SQLCHAR, SQL_C_BIT>(
            0,    // paramNumber
            '3'); // paramValue
    }

    //----------------------------------------------------------------------------------------------
    // Name: InitFloatParamTest
    //
    // Description:
    //  Test multiple FLOAT(24) (SQLREAL) values
    //
    TEST_F(CSharpExtensionApiTests, InitFloatParamTest)
    {
        InitializeSession(
            0,  // inputSchemaColumnsNumber
            1); // parametersNumber

        // Test max FLOAT(24) i.e. REAL value
        //
        InitParam<SQLREAL, SQL_C_FLOAT>(
            0,        // paramNumber
            3.4e38F); // max FLOAT value

        // Test min FLOAT(24) i.e. REAL value
        //
        InitParam<SQLREAL, SQL_C_FLOAT>(
            0,         // paramNumber
            -3.4e38F); // min FLOAT value

        // Test a normal FLOAT(24) value
        //
        InitParam<SQLREAL, SQL_C_FLOAT>(
            0,       // paramNumber
            2.3e4F); //normal FLOAT value

        // Test null FLOAT(24) i.e. REAL value
        //
        InitParam<SQLREAL, SQL_C_FLOAT>(
            0,     // paramNumber
            0,     // paramValue
            true); // isNull
    }

    //----------------------------------------------------------------------------------------------
    // Name: InitDoubleParamTest
    //
    // Description:
    //  Test multiple FLOAT(53) (DOUBLE) values
    //
    TEST_F(CSharpExtensionApiTests, InitDoubleParamTest)
    {
        InitializeSession(
            0,  // inputSchemaColumnsNumber
            1); // parametersNumber

        // Test max FLOAT(53) i.e. DOUBLE PRECISION value
        //
        InitParam<SQLDOUBLE, SQL_C_DOUBLE>(
            0,         // paramNumber
            1.79e308); // max DOUBLE value

        // Test min FLOAT(53) i.e. DOUBLE PRECISION value
        //
        InitParam<SQLDOUBLE, SQL_C_DOUBLE>(
            0,          // paramNumber
            -1.79e308); // min DOUBLE value

        // Test normal FLOAT(53) i.e. DOUBLE PRECISION value
        //
        InitParam<SQLDOUBLE, SQL_C_DOUBLE>(
            0,        // paramNumber
            1.45e38); // normal DOUBLE value

        // Test null FLOAT(53) value
        //
        InitParam<SQLDOUBLE, SQL_C_DOUBLE>(
            0,     // paramNumber
            0,     // paramValue
            true); // isNull
    }

    //----------------------------------------------------------------------------------------------
    // Name: InitBigIntParamTest
    //
    // Description:
    //  Test multiple SQLBIGINT values
    //
    TEST_F(CSharpExtensionApiTests, InitBigIntParamTest)
    {
        InitializeSession(
            0,  // inputSchemaColumnsNumber
            1); // parametersNumber

        // Test max BIGINT value
        //
        InitParam<SQLBIGINT, SQL_C_SBIGINT>(
            0,                      // paramNumber
            9223372036854775807LL); // max BIGINT value

        // Test min BIGINT value : -9223372036854775808 gives compiler error
        //
        InitParam<SQLBIGINT, SQL_C_SBIGINT>(
            0,                       // paramNumber
            -9223372036854775807LL); // min BIGINT value

        // Test normal BIGINT value
        //
        InitParam<SQLBIGINT, SQL_C_SBIGINT>(
            0,                    // paramNumber
            9'372'036'854'775LL); // normal BIGINT value

        // Test null BIGINT value
        //
        InitParam<SQLBIGINT, SQL_C_SBIGINT>(
            0,     // paramNumber
            0,     // paramValue
            true); // isNull
    }

    //----------------------------------------------------------------------------------------------
    // Name: InitTinyIntParamTest
    //
    // Description:
    //  Test multiple TINYINT values
    //
    TEST_F(CSharpExtensionApiTests, InitTinyIntParamTest)
    {
        InitializeSession(
            0,  // inputSchemaColumnsNumber
            1); // parametersNumber

        // Test max TINYINT value
        //
        InitParam<SQLCHAR, SQL_C_UTINYINT>(0, // paramNumber
            (numeric_limits<SQLCHAR>::max)());    // max TINYINT value

        // Test min TINYINT value
        //
        InitParam<SQLCHAR, SQL_C_UTINYINT>(0, // paramNumber
            (numeric_limits<SQLCHAR>::min)());    // min TINYINT value

        // Test normal TINYINT value
        //
        InitParam<SQLCHAR, SQL_C_UTINYINT>(0, // paramNumber
            123);                                 // normal TINYINT value

        // Test null TINYINT value
        //
        InitParam<SQLCHAR, SQL_C_UTINYINT>(0, // paramNumber
            0,                                    // paramValue
            true);                                // isNull

        // Test -1 TINYINT value underflows to Max TinyInt
        //
        InitParam<SQLCHAR, SQL_C_UTINYINT>(0, // paramNumber
            -1);                                  // paramValue
    }

    //----------------------------------------------------------------------------------------------
    // Name: InitSmallIntParamTest
    //
    // Description:
    //  Test multiple SMALLINT values
    //
    TEST_F(CSharpExtensionApiTests, InitSmallIntParamTest)
    {
        InitializeSession(
            0,  // inputSchemaColumnsNumber
            1); // parametersNumber

        // Test max SMALLINT value
        //
        InitParam<SQLSMALLINT, SQL_C_SSHORT>(0, // paramNumber
            32767);                                 // max SMALLINT value

        // Test min SMALLINT value
        //
        InitParam<SQLSMALLINT, SQL_C_SSHORT>(0, // paramNumber
            -32768);                                // min SMALLINT value

        // Test normal SMALLINT value
        //
        InitParam<SQLSMALLINT, SQL_C_SSHORT>(0, // paramNumber
            3'007);                                 // normal SMALLINT value

        // Test null SMALLINT value
        //
        InitParam<SQLSMALLINT, SQL_C_SSHORT>(0, // paramNumber
            0,                                      // paramValue
            true);                                  // isNull
    }

    //----------------------------------------------------------------------------------------------
    // Name: InitStringParamTest
    //
    // Description:
    //  Test multiple CHAR and VARCHAR values
    //
    TEST_F(CSharpExtensionApiTests, InitStringParamTest)
    {
        InitializeSession(
            0,  // inputSchemaColumnsNumber
            1); // parametersNumber

        // Test simple CHAR(5) value
        //
        InitStringParameter(
            0,       // paramNumber
            "HELLO", // paramValue
            5,       // paramSize
            true);   // isFixedType

        // Test simple CHAR(6) value with parameter length less than size - should be padded.
        //
        InitStringParameter(
            0,       // paramNumber
            "WORLD", // paramValue
            6,       // paramSize
            true);   // isFixedType

        // Test CHAR(6) value with parameter length more than size - should be truncated.
        //
        InitStringParameter(
            0,             // paramNumber
            "DOTNETEXTENSION", // paramValue
            6,             // paramSize
            true);         // isFixedType

        // Test null CHAR(5) value
        //
        InitStringParameter(
            0,       // paramNumber
            nullptr, // paramValue
            5,       // paramSize
            true);   // isFixedType

        // Test simple VARCHAR(6) value
        //
        InitStringParameter(
            0,        // paramNumber
            "WORLD!", // paramValue
            6,        // paramSize
            false);   // isFixedType

        // Test simple VARCHAR(8) value with parameter length less than size - NO padding.
        //
        InitStringParameter(
            0,       // paramNumber
            "WORLD", // paramValue
            8,       // paramSize
            false);  // isFixedType

        // Test VARCHAR(6) value with parameter length more than size - should be truncated.
        //
        InitStringParameter(
            0,             // paramNumber
            "DOTNETEXTENSION", // paramValue
            6,             // paramSize
            false);        // isFixedType

        // Test CHAR value with UTF-8 encoded string (with chinese)
        //
        string utfstring = u8"中文编码";
        InitStringParameter(
            0,                  // paramNumber
            utfstring.c_str(),  // paramValue
            utfstring.length(), // paramSize
            true);              // isFixedType

        // Test VARCHAR value with UTF-8 encoded string (with cyrillic)
        //
        utfstring = u8"абвг";
        InitStringParameter(
            0,                  // paramNumber
            utfstring.c_str(),  // paramValue
            utfstring.length(), // paramSize
            false);             // isFixedType

        // Test null VARCHAR(5) value
        //
        InitStringParameter(
            0,       // paramNumber
            nullptr, // paramValue
            5,       // paramSize
            false);  // isFixedType

        // Test CHAR value with UTF-8 self-constructed encoded character (Euro sign)
        // https://en.wikipedia.org/wiki/UTF-8#Examples
        //
        string goodUTF8 = string("a") + "\xE2" + "\x82" + "\xAC";
        InitStringParameter(
            0,                 // paramNumber
            goodUTF8.c_str(),  // paramValue
            goodUTF8.length(), // paramSize
            true);             // isFixedType
    }

    //----------------------------------------------------------------------------------------------
    // Name: InitWStringParamTest
    //
    // Description:
    //  Test multiple WCHAR and WVARCHAR values
    //
    TEST_F(CSharpExtensionApiTests, InitWStringParamTest)
    {
        InitializeSession(
            0,  // inputSchemaColumnsNumber
            1); // parametersNumber

        // Test simple NCHAR(5) value
        //
        InitWStringParameter(
            0,        // paramNumber
            L"HELLO", // paramValue
            5,        // paramSize
            true);    // isFixedType

        // Test simple NCHAR(6) value with parameter length less than size - should be padded.
        //
        InitWStringParameter(
            0,        // paramNumber
            L"WORLD", // paramValue
            6,        // paramSize
            true);    // isFixedType

        // Test NCHAR(6) value with parameter length more than size - should be truncated.
        //
        InitWStringParameter(
            0,                  // paramNumber
            L"DOTNETEXTENSION", // paramValue
            6,                  // paramSize
            true);              // isFixedType

        // Test null NCHAR(5) value
        //
        InitWStringParameter(
            0,        // paramNumber
            nullptr,  // paramValue
            5,        // paramSize
            true);    // isFixedType

        // Test simple NVARCHAR(6) value
        //
        InitWStringParameter(
            0,         // paramNumber
            L"WORLD!", // paramValue
            6,         // paramSize
            false);    // isFixedType

        // Test simple NVARCHAR(8) value with parameter length less than size - NO padding.
        //
        InitWStringParameter(
            0,        // paramNumber
            L"WORLD", // paramValue
            8,        // paramSize
            false);   // isFixedType

        // Test NVARCHAR(6) value with parameter length more than size - should be truncated.
        //
        InitWStringParameter(
            0,                  // paramNumber
            L"DOTNETEXTENSION", // paramValue
            6,                  // paramSize
            false);             // isFixedType

        // Test null NVARCHAR(5) value
        //
        InitWStringParameter(
            0,        // paramNumber
            nullptr,  // paramValue
            5,        // paramSize
            false);   // isFixedType

        // Test NCHAR value with Unicode characters (Chinese)
        //
        InitWStringParameter(
            0,            // paramNumber
            L"中文编码",  // paramValue
            4,            // paramSize
            true);        // isFixedType

        // Test NVARCHAR value with Unicode characters (Cyrillic)
        //
        InitWStringParameter(
            0,        // paramNumber
            L"абвг",  // paramValue
            4,        // paramSize
            false);   // isFixedType

        // Test NVARCHAR value with Euro sign and currency symbols
        //
        InitWStringParameter(
            0,            // paramNumber
            L"€100£50¥",  // paramValue - Euro, Pound, Yen
            8,            // paramSize
            false);       // isFixedType

        // Test NVARCHAR value with mixed languages (English, Chinese, Japanese, Korean)
        //
        InitWStringParameter(
            0,                    // paramNumber
            L"Hello世界こんにちは안녕",  // paramValue - English, Chinese, Japanese, Korean
            14,                   // paramSize
            false);               // isFixedType

        // Test NVARCHAR value with emoji (Note: Some emoji are surrogate pairs in UTF-16)
        //
        InitWStringParameter(
            0,              // paramNumber
            L"Hi\U0001F600\U0001F44D",  // paramValue - "Hi" + grinning face + thumbs up emoji
            6,              // paramSize (2 chars + 2 surrogate pairs = 6 UTF-16 code units)
            false);         // isFixedType

        // Test NVARCHAR value with accented characters (European languages)
        //
        InitWStringParameter(
            0,                  // paramNumber
            L"àéîõüñçß",        // paramValue - Various accented chars
            8,                  // paramSize
            false);             // isFixedType

        // Test NVARCHAR value with leading, trailing, and embedded spaces
        //
        InitWStringParameter(
            0,                  // paramNumber
            L" with spaces ",   // paramValue - spaces at start, middle, and end
            13,                 // paramSize
            false);             // isFixedType

        // Test NCHAR value with spaces (should be preserved, not trimmed)
        //
        InitWStringParameter(
            0,                  // paramNumber
            L"  padded  ",      // paramValue - multiple spaces
            10,                 // paramSize
            true);              // isFixedType
    }

    //----------------------------------------------------------------------------------------------
    // Name: InitParam
    //
    // Description:
    // Templatized function to call InitParam for the given paramValue and dataType.
    // Testing if InitParam is implemented correctly for integer/numeric/boolean dataTypes.
    //
    template<class SQLType, SQLSMALLINT dataType>
    void CSharpExtensionApiTests::InitParam(
        int         paramNumber,
        SQLType     paramValue,
        bool        isNull,
        SQLSMALLINT inputOutputType,
        SQLRETURN   SQLResult)
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

        SQLRETURN result = (*sm_initParamFuncPtr)(
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
            1);                                         // inputOutputType

        EXPECT_EQ(result, SQLResult);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InitStringParameter
    //
    // Description:
    // Testing if InitParam is implemented correctly for the char/varchar dataType.
    //
    void CSharpExtensionApiTests::InitStringParameter(
        int           paramNumber,
        const char    *paramValue,
        const SQLULEN paramSize,
        bool          isFixedType,
        SQLSMALLINT   inputOutputType,
        SQLRETURN     SQLResult)
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

        SQLRETURN result = (*sm_initParamFuncPtr)(
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

        EXPECT_EQ(result, SQLResult);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InitWStringParameter
    //
    // Description:
    // Testing if InitParam is implemented correctly for the nchar/nvarchar dataType.
    //
    void CSharpExtensionApiTests::InitWStringParameter(
        int              paramNumber,
        const wchar_t    *paramValue,
        const SQLINTEGER paramSize,
        bool             isFixedType,
        SQLSMALLINT      inputOutputType,
        SQLRETURN        SQLResult)
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

        result = (*sm_initParamFuncPtr)(
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

        EXPECT_EQ(result, SQLResult);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetWStringLength
    //
    // Description:
    //  Utility function to get the length of a wchar_t *.
    //  wcslen does not work in Linux with -fshort-wchar, so we use this function instead.
    //
    SQLINTEGER CSharpExtensionApiTests::GetWStringLength(const wchar_t *wstr)
    {
        SQLINTEGER distance = -1;

        // If nullptr, return
        //
        if (wstr)
        {
            // Get distance from end of string to beginning
            //
            const wchar_t *newstr = wstr;
            while (*newstr)
            {
                ++newstr;
            }

            distance = newstr - wstr;
        }

        return distance;
    }
}
