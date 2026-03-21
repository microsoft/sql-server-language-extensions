//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpTestExecutor.cs
//
// Purpose:
//  Test classes extending the AbstractSqlServerExtensionExecutor.
//
//*********************************************************************
using System;
using System.Data.SqlTypes;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using Microsoft.Data.Analysis;
using Microsoft.SqlServer.CSharpExtension.SDK;

namespace Microsoft.SqlServer.CSharpExtensionTest
{
    public class CSharpTestExecutor: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            Console.WriteLine("Hello .NET Core CSharpExtension!");
            return input;
        }
    }

    public class CSharpTestExecutorIntParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = Int32.MaxValue;
            sqlParams["@param1"] = Int32.MinValue;
            sqlParams["@param2"] = 4;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorBitParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = true;
            sqlParams["@param1"] = false;
            sqlParams["@param2"] = 3;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorRealParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = 3.4e38F;
            sqlParams["@param1"] = -3.4e38F;
            sqlParams["@param2"] = 2.3e4;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorDoubleParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = 1.79e308;
            sqlParams["@param1"] = -1.79e308;
            sqlParams["@param2"] = 1.45e38;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorBigIntParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = Int64.MaxValue;
            sqlParams["@param1"] = Int64.MinValue;
            sqlParams["@param2"] = 9372036854775;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorSmallIntParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = Int16.MaxValue;
            sqlParams["@param1"] = Int16.MinValue;
            sqlParams["@param2"] = 3007;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorTinyIntParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = 255;
            sqlParams["@param1"] = 0;
            sqlParams["@param2"] = 123;
            sqlParams["@param3"] = -1;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    /// <summary>
    /// Comprehensive test executor for DECIMAL/NUMERIC OUTPUT parameters.
    /// Covers: max/min values, high precision/scale, financial values, zero, nulls.
    /// Consolidated from CSharpTestExecutorDecimalParam + CSharpTestExecutorDecimalHighScaleParam.
    /// </summary>
    public class CSharpTestExecutorDecimalParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            // Maximum value: DECIMAL(38,0) max = 10^38 - 1
            sqlParams["@param0"] = SqlDecimal.Parse("99999999999999999999999999999999999999");
            
            // Minimum value (negative max)
            sqlParams["@param1"] = SqlDecimal.Parse("-99999999999999999999999999999999999999");
            
            // High scale: DECIMAL(38,10) - 38 digits with 10 fractional
            sqlParams["@param2"] = SqlDecimal.Parse("1234567890123456789012345678.1234567890");
            
            // Zero
            sqlParams["@param3"] = new SqlDecimal(0);
            
            // High fractional precision: DECIMAL(38,28) - 10 integer + 28 fractional
            sqlParams["@param4"] = SqlDecimal.Parse("1234567890.1234567890123456789012345678");
            
            // Typical financial: DECIMAL(19,4)
            sqlParams["@param5"] = SqlDecimal.Parse("123456789012345.6789");
            
            // Negative financial
            sqlParams["@param6"] = SqlDecimal.Parse("-123456789012345.6789");
            
            // Null
            sqlParams["@param7"] = null;
            
            return null;
        }
    }

    /// <summary>
    /// Test executor for DECIMAL precision overflow validation.
    /// This executor deliberately sets values that exceed the target precision after scale adjustment.
    /// This tests that FromSqlDecimal properly validates precision overflow.
    /// 
    /// Bug scenario: Value 12345678.99 (10 digits) converted to DECIMAL(10,4) becomes 12345678.9900
    /// which requires 12 significant digits, exceeding the declared precision of 10.
    /// </summary>
    public class CSharpTestExecutorDecimalPrecisionOverflow: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            // param0: DECIMAL(10,4) max is 999999.9999 (6 before + 4 after = 10 total digits)
            // Using value 12345678.99 has 10 significant digits (precision=10), scale=2
            // When adjusted to scale=4, would need precision=12 (12345678.9900), exceeding DECIMAL(10,4)
            decimal dec0 = 12345678.99m;
            sqlParams["@param0"] = new SqlDecimal(dec0);
            
            // param1: Using 999999999.999 has 12 significant digits (precision=12), scale=3
            // When adjusted to scale=4, would need precision=13 (999999999.9990), exceeding DECIMAL(10,4)
            decimal dec1 = 999999999.999m;
            sqlParams["@param1"] = new SqlDecimal(dec1);
            
            // param2: Value that fits in DECIMAL(8,3)
            // Using 12345.67 has 7 significant digits (precision=7), scale=2
            // When adjusted to scale=3, would need precision=8 (12345.670), fits in DECIMAL(8,3)
            decimal dec2 = 12345.67m;
            sqlParams["@param2"] = new SqlDecimal(dec2);
            
            return null;
        }
    }

    public class CSharpTestExecutorStringParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = "HELLO";
            sqlParams["@param1"] = "C#Extension";
            sqlParams["@param2"] = "";
            sqlParams["@param3"] = "WORLD";
            sqlParams["@param4"] = null;
            sqlParams["@param5"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorWStringParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = "HELLO";
            sqlParams["@param1"] = "C#Extension";
            sqlParams["@param2"] = string.Empty;
            sqlParams["@param3"] = "WORLD";
            sqlParams["@param4"] = null;  // null NVARCHAR (variable-length)
            sqlParams["@param5"] = null;  // null NCHAR (fixed-length)
            return null;
        }
    }

    /// <summary>
    /// Test executor for NVARCHAR(MAX) scenarios with large strings.
    /// </summary>
    public class CSharpTestExecutorWStringMaxParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            // Generate a large string (10,000 characters) to test NVARCHAR(MAX) handling
            //
            sqlParams["@param0"] = new string('A', 10000);

            // Generate a string with repeating Unicode pattern (5,000 characters)
            //
            string unicodePattern = "你好世界€";  // 5 characters
            var sb = new System.Text.StringBuilder();
            for (int i = 0; i < 1000; i++)
            {
                sb.Append(unicodePattern);
            }
            sqlParams["@param1"] = sb.ToString();

            // Test null for NVARCHAR(MAX)
            //
            sqlParams["@param2"] = null;

            return null;
        }
    }

    /// <summary>
    /// Test executor for Unicode variety: emojis, accented characters, mixed scripts.
    /// </summary>
    public class CSharpTestExecutorWStringUnicodeParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            // Test emoji characters (surrogate pairs in UTF-16)
            //
            sqlParams["@param0"] = "Hi\U0001F600\U0001F44D";  // "Hi" + grinning face + thumbs up

            // Test accented/European characters
            //
            sqlParams["@param1"] = "Café résumé naïve";

            // Test mixed scripts (Latin, Chinese, Japanese, Korean)
            //
            sqlParams["@param2"] = "Hello世界こんにちは";

            // Test currency and special symbols
            //
            sqlParams["@param3"] = "€100 £50 ¥1000 ©®™";

            return null;
        }
    }
}
