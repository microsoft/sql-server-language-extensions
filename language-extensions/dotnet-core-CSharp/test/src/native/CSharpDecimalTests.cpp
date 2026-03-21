//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpDecimalTests.cpp
//
// Purpose:
//  Test the .NET Core CSharp extension NUMERIC/DECIMAL support using the Extension API
//
//*********************************************************************
#include "CSharpExtensionApiTests.h"

using namespace std;

namespace ExtensionApiTest
{
    // SQL_NUMERIC_STRUCT size per ODBC specification
    //
    const SQLINTEGER SQL_NUMERIC_STRUCT_SIZE = 19;

    // SQL Server maximum NUMERIC/DECIMAL precision
    //
    const SQLINTEGER SQL_NUMERIC_MAX_PRECISION = 38;

    //----------------------------------------------------------------------------------------------
    // Name: InitNumericParamTest
    //
    // Description:
    //  Tests multiple SQL_NUMERIC_STRUCT values with various precision and scale combinations.
    //
    TEST_F(CSharpExtensionApiTests, InitNumericParamTest)
    {
        using TestHelpers::CreateNumericStruct;

        InitializeSession(
            0,  // inputSchemaColumnsNumber
            5); // parametersNumber

        // NUMERIC(10,2): 12345.67
        //
        SQL_NUMERIC_STRUCT param0 = CreateNumericStruct(1234567, 10, 2, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(
            0,       // paramNumber
            param0); // paramValue (12345.67)

        // NUMERIC(38,0): 999999999999 (max precision integer)
        //
        SQL_NUMERIC_STRUCT param1 = CreateNumericStruct(999999999999LL, 38, 0, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(
            1,       // paramNumber
            param1); // paramValue (999999999999)

        // NUMERIC(19,4): -123456789012.3456 (negative)
        //
        SQL_NUMERIC_STRUCT param2 = CreateNumericStruct(1234567890123456LL, 19, 4, true);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(
            2,       // paramNumber
            param2); // paramValue (-123456789012.3456)

        // NUMERIC(5,5): 0.12345 (scale = precision)
        //
        SQL_NUMERIC_STRUCT param3 = CreateNumericStruct(12345, 5, 5, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(
            3,       // paramNumber
            param3); // paramValue (0.12345)

        // NULL NUMERIC value
        //
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(
            4,                     // paramNumber
            SQL_NUMERIC_STRUCT(),  // paramValue (ignored due to isNull)
            true);                 // isNull

        // Invalid parameter number (5 out of bounds, valid: 0-4)
        //
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(
            5,                      // invalid paramNumber
            param0,                 // paramValue
            false,                  // isNull
            SQL_PARAM_INPUT_OUTPUT, // inputOutputType
            SQL_ERROR);             // expected error

        // Negative parameter number
        //
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(
            -1,                     // negative paramNumber
            param0,                 // paramValue
            false,                  // isNull
            SQL_PARAM_INPUT_OUTPUT, // inputOutputType
            SQL_ERROR);             // expected error
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetDecimalOutputParamTest
    //
    // Description:
    //  Tests C# SqlDecimal to SQL_NUMERIC_STRUCT output parameter conversion
    //
    TEST_F(CSharpExtensionApiTests, GetDecimalOutputParamTest)
    {
        int paramsNumber = 8;

        string userClassFullName = "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorDecimalParam";
        string scriptString = m_UserLibName + m_Separator + userClassFullName;

        InitializeSession(
            0,               // inputSchemaColumnsNumber
            paramsNumber,    // parametersNumber
            scriptString);   // scriptString

        for(int i = 0; i < paramsNumber; ++i)
        {
            InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(
                i,                       // paramNumber
                SQL_NUMERIC_STRUCT(),    // paramValue (will be set by C# executor)
                false,                   // isNull
                SQL_PARAM_INPUT_OUTPUT); // inputOutputType
        }

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

        // Expected strLenOrInd values: 19 bytes for valid, SQL_NULL_DATA for last param
        //
        vector<SQLINTEGER> strLenOrIndValues;
        
        // Non-null params: 19 bytes (sizeof SQL_NUMERIC_STRUCT)
        //
        for (int i = 0; i < paramsNumber - 1; ++i)
        {
            strLenOrIndValues.push_back(SQL_NUMERIC_STRUCT_SIZE);
        }

        // Last parameter is null - validates NULL handling in C# SqlDecimal to SQL_NUMERIC_STRUCT conversion
        //
        strLenOrIndValues.push_back(SQL_NULL_DATA);

        // Verify output parameters match expected values and structure
        //
        for (int i = 0; i < paramsNumber; ++i)
        {
            SQLPOINTER paramValue = nullptr;
            SQLINTEGER strLenOrInd = 0;

            SQLRETURN result = (*sm_getOutputParamFuncPtr)(
                *m_sessionId,
                m_taskId,
                i,
                &paramValue,
                &strLenOrInd);

            ASSERT_EQ(result, SQL_SUCCESS);
            EXPECT_EQ(strLenOrInd, strLenOrIndValues[i]);

            if (strLenOrInd != SQL_NULL_DATA)
            {
                ASSERT_NE(paramValue, nullptr);
                SQL_NUMERIC_STRUCT* numericValue = static_cast<SQL_NUMERIC_STRUCT*>(paramValue);
                
                // Validate precision/scale/sign integrity
                //
                EXPECT_GE(numericValue->precision, 1);
                EXPECT_LE(numericValue->precision, SQL_NUMERIC_MAX_PRECISION);
                EXPECT_GE(numericValue->scale, 0);
                EXPECT_LE(numericValue->scale, numericValue->precision);
                EXPECT_TRUE(numericValue->sign == 0 || numericValue->sign == 1);
            }
        }
    }

    //----------------------------------------------------------------------------------------------
    // Name: DecimalPrecisionScaleTest
    //
    // Description:
    //  Tests precision (1-38) and scale (0-38) combinations covering min/max, financial, scientific
    //
    TEST_F(CSharpExtensionApiTests, DecimalPrecisionScaleTest)
    {
        using TestHelpers::CreateNumericStruct;

        InitializeSession(
            0,   // inputSchemaColumnsNumber
            10); // parametersNumber

        // NUMERIC(1,0): 5 (min precision, no scale)
        //
        SQL_NUMERIC_STRUCT p0 = CreateNumericStruct(5, 1, 0, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(0, p0);

        // NUMERIC(1,1): 0.5 (min precision, scale = precision)
        //
        SQL_NUMERIC_STRUCT p1 = CreateNumericStruct(5, 1, 1, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(1, p1);

        // NUMERIC(38,0): 12345678901234567 (max precision, no scale)
        //
        SQL_NUMERIC_STRUCT p2 = CreateNumericStruct(12345678901234567LL, SQL_NUMERIC_MAX_PRECISION, 0, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(2, p2);

        // NUMERIC(38,38): 0.xxx (max precision, max scale)
        //
        SQL_NUMERIC_STRUCT p3 = CreateNumericStruct(123456789012345678LL, SQL_NUMERIC_MAX_PRECISION, SQL_NUMERIC_MAX_PRECISION, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(3, p3);

        // NUMERIC(19,4): SQL Server MONEY compatible
        //
        SQL_NUMERIC_STRUCT p4 = CreateNumericStruct(12345678901234567LL, 19, 4, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(4, p4);

        // NUMERIC(10,2): Common financial
        //
        SQL_NUMERIC_STRUCT p5 = CreateNumericStruct(1234567, 10, 2, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(5, p5);

        // NUMERIC(20,10): Balanced precision/scale
        //
        SQL_NUMERIC_STRUCT p6 = CreateNumericStruct(123456789012345678ULL, 20, 10, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(6, p6);

        // NUMERIC(20,15): Mostly fractional
        //
        SQL_NUMERIC_STRUCT p7 = CreateNumericStruct(12345123456789012345ULL, 20, 15, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(7, p7);

        // NUMERIC(28,10): Scientific notation
        //
        SQL_NUMERIC_STRUCT p8 = CreateNumericStruct(123456789012345678LL, 28, 10, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(8, p8);

        // NUMERIC(18,18): Scale = precision
        //
        SQL_NUMERIC_STRUCT p9 = CreateNumericStruct(123456789012345678LL, 18, 18, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(9, p9);
    }

    //----------------------------------------------------------------------------------------------
    // Name: DecimalBoundaryValuesTest
    //
    // Description:
    //  Tests boundary values: zero, very small, very large, negative
    //
    TEST_F(CSharpExtensionApiTests, DecimalBoundaryValuesTest)
    {
        using TestHelpers::CreateNumericStruct;

        InitializeSession(
            0,  // inputSchemaColumnsNumber
            6); // parametersNumber

        // Zero: 0.00
        //
        SQL_NUMERIC_STRUCT zero = CreateNumericStruct(0, 10, 2, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(0, zero);

        // Very small positive: 0.01
        //
        SQL_NUMERIC_STRUCT smallPos = CreateNumericStruct(1, 10, 2, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(1, smallPos);

        // Very small negative: -0.01
        //
        SQL_NUMERIC_STRUCT smallNeg = CreateNumericStruct(1, 10, 2, true);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(2, smallNeg);

        // Large positive: 999999999999999999 (near NUMERIC(38) max)
        //
        SQL_NUMERIC_STRUCT largePos = CreateNumericStruct(999999999999999999LL, SQL_NUMERIC_MAX_PRECISION, 0, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(3, largePos);

        // Large negative: -999999999999999999
        //
        SQL_NUMERIC_STRUCT largeNeg = CreateNumericStruct(999999999999999999LL, SQL_NUMERIC_MAX_PRECISION, 0, true);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(4, largeNeg);

        // Maximum scale: 0.000000000000000001 (10^-18)
        //
        SQL_NUMERIC_STRUCT maxScale = CreateNumericStruct(1, 18, 18, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(5, maxScale);
    }

    //----------------------------------------------------------------------------------------------
    // Name: DecimalStructLayoutTest
    //
    // Description:
    //  Verifies SQL_NUMERIC_STRUCT ODBC binary layout: 19 bytes, field offsets
    //
    TEST_F(CSharpExtensionApiTests, DecimalStructLayoutTest)
    {
        // ODBC spec: struct size = 19 bytes
        //
        EXPECT_EQ(sizeof(SQL_NUMERIC_STRUCT), 19);

        // Verify field offsets for binary compatibility
        //
        SQL_NUMERIC_STRUCT test;
        
        EXPECT_EQ((size_t)&test.precision - (size_t)&test, 0);  // precision at offset 0
        EXPECT_EQ((size_t)&test.scale - (size_t)&test, 1);       // scale at offset 1
        EXPECT_EQ((size_t)&test.sign - (size_t)&test, 2);        // sign at offset 2
        EXPECT_EQ((size_t)&test.val[0] - (size_t)&test, 3);      // val array at offset 3
        EXPECT_EQ(sizeof(test.val), 16);                          // val array = 16 bytes

        // Verify struct initialization and field access
        //
        test.precision = 38;
        test.scale = 10;
        test.sign = 1;
        memset(test.val, 0, 16);
        test.val[0] = 0x39;  // 12345 in little-endian
        test.val[1] = 0x30;
        
        EXPECT_EQ(test.precision, 38);
        EXPECT_EQ(test.scale, 10);
        EXPECT_EQ(test.sign, 1);
        EXPECT_EQ(test.val[0], 0x39);
        EXPECT_EQ(test.val[1], 0x30);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetDecimalInputColumnsTest
    //
    // Description:
    //  Tests SQL_NUMERIC_STRUCT input columns with mixed precision/scale and NULL values.
    //  E2E tests had coverage, but unit tests had zero decimal column coverage until this test.
    //
    TEST_F(CSharpExtensionApiTests, GetDecimalInputColumnsTest)
    {
        using TestHelpers::CreateNumericStruct;

        // Column 1: Non-nullable NUMERIC(19,4)
        // Values: 12345.6789, 9876543.2100, 0.1234, -555.5000, 999999999.9999
        //
        vector<SQL_NUMERIC_STRUCT> column1Data = {
            CreateNumericStruct(123456789, 19, 4, false),      // 12345.6789
            CreateNumericStruct(98765432100LL, 19, 4, false),  // 9876543.2100
            CreateNumericStruct(1234, 19, 4, false),           // 0.1234
            CreateNumericStruct(5555000, 19, 4, true),         // -555.5000
            CreateNumericStruct(9999999999999LL, 19, 4, false) // 999999999.9999
        };

        // Column 2: Nullable NUMERIC(38,10) with NULL values
        // Values: 1234567890.1234567890, NULL, 0.0000000001, NULL, -9999.9999999999
        //
        vector<SQL_NUMERIC_STRUCT> column2Data = {
            CreateNumericStruct(12345678901234567890ULL, 38, 10, false), // 1234567890.1234567890
            SQL_NUMERIC_STRUCT(),                                         // NULL
            CreateNumericStruct(1, 38, 10, false),                        // 0.0000000001
            SQL_NUMERIC_STRUCT(),                                         // NULL
            CreateNumericStruct(99999999999999ULL, 38, 10, true)          // -9999.9999999999
        };

        // Column 1: All non-null
        //
        vector<SQLINTEGER> col1StrLenOrInd(5, SQL_NUMERIC_STRUCT_SIZE);

        // Column 2: Rows 1 and 3 are NULL
        //
        vector<SQLINTEGER> col2StrLenOrInd = {
            SQL_NUMERIC_STRUCT_SIZE,  // Row 0: valid
            SQL_NULL_DATA,            // Row 1: NULL
            SQL_NUMERIC_STRUCT_SIZE,  // Row 2: valid
            SQL_NULL_DATA,            // Row 3: NULL
            SQL_NUMERIC_STRUCT_SIZE   // Row 4: valid
        };

        // Create ColumnInfo with decimal data
        //
        ColumnInfo<SQL_NUMERIC_STRUCT> decimalInfo(
            "DecimalColumn1",
            column1Data,
            col1StrLenOrInd,
            "DecimalColumn2",
            column2Data,
            col2StrLenOrInd,
            vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });

        InitializeSession(
            decimalInfo.GetColumnsNumber(),
            0,
            m_scriptString);

        InitializeColumns<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(&decimalInfo);

        // Execute with decimal input columns (tests native to C# DataFrame conversion)
        //
        Execute<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(
            ColumnInfo<SQL_NUMERIC_STRUCT>::sm_rowsNumber,
            decimalInfo.m_dataSet.data(),
            decimalInfo.m_strLen_or_Ind.data(),
            decimalInfo.m_columnNames);

        // Verify column metadata matches input (SqlDecimal preserves precision/scale)
        //
        GetResultColumn(
            0,                  // columnNumber
            SQL_C_NUMERIC,      // dataType
            19,                 // columnSize (precision from input)
            4,                  // decimalDigits (scale)
            SQL_NO_NULLS);      // nullable

        GetResultColumn(
            1,                  // columnNumber
            SQL_C_NUMERIC,      // dataType
            38,                 // columnSize (precision from input)
            10,                 // decimalDigits (scale)
            SQL_NULLABLE);      // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetDecimalResultColumnsTest
    //
    // Description:
    //  Tests decimal result column conversion preserves precision/scale
    //  and proper NULL handling of nullable columns.
    //
    TEST_F(CSharpExtensionApiTests, GetDecimalResultColumnsTest)
    {
        using TestHelpers::CreateNumericStruct;

        // Result Column 1: NUMERIC(18,2) - typical financial data
        // Max value: 999999999999999.99 requires precision 18
        //
        vector<SQL_NUMERIC_STRUCT> resultCol1 = {
            CreateNumericStruct(123456789, 18, 2, false),         // 1234567.89
            CreateNumericStruct(99999999999999999LL, 18, 2, false), // 999999999999999.99
            CreateNumericStruct(1050, 18, 2, false),              // 10.50
            CreateNumericStruct(100, 18, 2, true),                // -1.00
            CreateNumericStruct(0, 18, 2, false)                  // 0.00
        };

        // Result Column 2: NUMERIC(10,5) - high precision with NULLs
        // Max value: 12345.67891 requires precision 10
        //
        vector<SQL_NUMERIC_STRUCT> resultCol2 = {
            CreateNumericStruct(1234567891, 10, 5, false),  // 12345.67891
            SQL_NUMERIC_STRUCT(),                           // NULL
            CreateNumericStruct(1, 10, 5, false),           // 0.00001
            SQL_NUMERIC_STRUCT(),                           // NULL
            CreateNumericStruct(9999999999LL, 10, 5, true)  // -99999.99999
        };

        vector<SQLINTEGER> col1StrLenOrInd(5, SQL_NUMERIC_STRUCT_SIZE);
        vector<SQLINTEGER> col2StrLenOrInd = {
            SQL_NUMERIC_STRUCT_SIZE,
            SQL_NULL_DATA,
            SQL_NUMERIC_STRUCT_SIZE,
            SQL_NULL_DATA,
            SQL_NUMERIC_STRUCT_SIZE
        };

        ColumnInfo<SQL_NUMERIC_STRUCT> decimalResultInfo(
            "AmountColumn",
            resultCol1,
            col1StrLenOrInd,
            "PrecisionColumn",
            resultCol2,
            col2StrLenOrInd,
            vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });

        InitializeSession(
            decimalResultInfo.GetColumnsNumber(),
            0,
            m_scriptString);

        InitializeColumns<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(&decimalResultInfo);

        Execute<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(
            ColumnInfo<SQL_NUMERIC_STRUCT>::sm_rowsNumber,
            decimalResultInfo.m_dataSet.data(),
            decimalResultInfo.m_strLen_or_Ind.data(),
            decimalResultInfo.m_columnNames);

        // Verify result column metadata preserved from input
        // CSharpOutputDataSet.ExtractNumericColumn() preserves SqlDecimal precision/scale
        //
        GetResultColumn(
            0,                  // columnNumber
            SQL_C_NUMERIC,      // dataType
            18,                 // columnSize (declared precision from input)
            2,                  // decimalDigits (scale)
            SQL_NO_NULLS);      // nullable

        GetResultColumn(
            1,                  // columnNumber
            SQL_C_NUMERIC,      // dataType
            10,                 // columnSize (declared precision from input)
            5,                  // decimalDigits (scale)
            SQL_NULLABLE);      // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: DecimalColumnsWithNullsTest
    //
    // Description:
    //  Tests decimal columns with mixed NULL and non-NULL values. SQL_NUMERIC_STRUCT doesn't
    //  have NULL indicator - NULL tracked via strLenOrInd = SQL_NULL_DATA separately.
    //
    TEST_F(CSharpExtensionApiTests, DecimalColumnsWithNullsTest)
    {
        using TestHelpers::CreateNumericStruct;

        // Column 1: First and last NULL - NUMERIC(28,6)
        // Pattern: NULL, 12345.678900, 98765.432100, 0.000001, NULL
        //
        vector<SQL_NUMERIC_STRUCT> col1Data = {
            SQL_NUMERIC_STRUCT(),                          // NULL
            CreateNumericStruct(12345678900LL, 28, 6, false), // 12345.678900
            CreateNumericStruct(98765432100LL, 28, 6, false), // 98765.432100
            CreateNumericStruct(1, 28, 6, false),             // 0.000001
            SQL_NUMERIC_STRUCT()                              // NULL
        };

        // Column 2: Middle rows NULL - NUMERIC(15,3)
        // Pattern: 999999.999, NULL, NULL, -123.456, 0.001
        //
        vector<SQL_NUMERIC_STRUCT> col2Data = {
            CreateNumericStruct(999999999, 15, 3, false),  // 999999.999
            SQL_NUMERIC_STRUCT(),                          // NULL
            SQL_NUMERIC_STRUCT(),                          // NULL
            CreateNumericStruct(123456, 15, 3, true),      // -123.456
            CreateNumericStruct(1, 15, 3, false)           // 0.001
        };

        // Rows 0 and 4 NULL
        //
        vector<SQLINTEGER> col1StrLenOrInd = {
            SQL_NULL_DATA,
            SQL_NUMERIC_STRUCT_SIZE,
            SQL_NUMERIC_STRUCT_SIZE,
            SQL_NUMERIC_STRUCT_SIZE,
            SQL_NULL_DATA
        };

        // Rows 1 and 2 NULL
        //
        vector<SQLINTEGER> col2StrLenOrInd = {
            SQL_NUMERIC_STRUCT_SIZE,
            SQL_NULL_DATA,
            SQL_NULL_DATA,
            SQL_NUMERIC_STRUCT_SIZE,
            SQL_NUMERIC_STRUCT_SIZE
        };

        ColumnInfo<SQL_NUMERIC_STRUCT> nullDecimalInfo(
            "SparseColumn1",
            col1Data,
            col1StrLenOrInd,
            "SparseColumn2",
            col2Data,
            col2StrLenOrInd,
            vector<SQLSMALLINT>{ SQL_NULLABLE, SQL_NULLABLE });

        InitializeSession(
            nullDecimalInfo.GetColumnsNumber(),
            0,
            m_scriptString);

        InitializeColumns<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(&nullDecimalInfo);

        Execute<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(
            ColumnInfo<SQL_NUMERIC_STRUCT>::sm_rowsNumber,
            nullDecimalInfo.m_dataSet.data(),
            nullDecimalInfo.m_strLen_or_Ind.data(),
            nullDecimalInfo.m_columnNames);

        // Verify metadata: both columns nullable, precision preserved despite NULLs
        //
        GetResultColumn(
            0,                  // columnNumber
            SQL_C_NUMERIC,      // dataType
            28,                 // columnSize (declared precision from input NUMERIC(28,6))
            6,                  // decimalDigits (scale)
            SQL_NULLABLE);      // nullable (contains NULLs)

        GetResultColumn(
            1,                  // columnNumber
            SQL_C_NUMERIC,      // dataType
            15,                 // columnSize (declared precision from input NUMERIC(15,3))
            3,                  // decimalDigits (scale)
            SQL_NULLABLE);      // nullable (contains NULLs)
    }

    //----------------------------------------------------------------------------------------------
    // Name: DecimalHighScaleTest
    //
    // Description:
    //  Tests high scale (29-38) decimal values. 
    //  These are valid SQL Server types and must be handled correctly.
    //
    TEST_F(CSharpExtensionApiTests, DecimalHighScaleTest)
    {
        using TestHelpers::CreateNumericStruct;

        InitializeSession(
            0,  // inputSchemaColumnsNumber
            6); // parametersNumber

        // NUMERIC(38,29): Boundary at scale = 29
        //
        SQL_NUMERIC_STRUCT p0 = CreateNumericStruct(1, SQL_NUMERIC_MAX_PRECISION, 29, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(0, p0);

        // NUMERIC(38,30): 123 at scale 30
        //
        SQL_NUMERIC_STRUCT p1 = CreateNumericStruct(123, SQL_NUMERIC_MAX_PRECISION, 30, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(1, p1);

        // NUMERIC(38,35): Very high scale (3 significant digits)
        //
        SQL_NUMERIC_STRUCT p2 = CreateNumericStruct(123, SQL_NUMERIC_MAX_PRECISION, 35, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(2, p2);

        // NUMERIC(38,38): Maximum scale (smallest non-zero value)
        //
        SQL_NUMERIC_STRUCT p3 = CreateNumericStruct(1, SQL_NUMERIC_MAX_PRECISION, SQL_NUMERIC_MAX_PRECISION, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(3, p3);

        // NUMERIC(38,31): Negative with high scale
        //
        SQL_NUMERIC_STRUCT p4 = CreateNumericStruct(1, SQL_NUMERIC_MAX_PRECISION, 31, true);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(4, p4);

        // NUMERIC(38,32): Zero with high scale (remains zero)
        //
        SQL_NUMERIC_STRUCT p5 = CreateNumericStruct(0, SQL_NUMERIC_MAX_PRECISION, 32, false);
        InitParam<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>(5, p5);
    }
}
