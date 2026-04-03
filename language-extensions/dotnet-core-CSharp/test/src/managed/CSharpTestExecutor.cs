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

    /// <summary>
    /// Test executor that returns a DataFrame with mixed-scale SqlDecimal values in the same column.
    /// This exercises the precision clamping fix in ExtractNumericColumn: when maxIntDigits + maxScale > 38,
    /// scale is reduced to (38 - maxIntDigits) to preserve integer digits.
    /// </summary>
    public class CSharpTestExecutorMixedScaleDecimalOutput : AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            // Create output DataFrame with one SqlDecimal column containing mixed scales
            //
            var column = new PrimitiveDataFrameColumn<SqlDecimal>("MixedScaleCol", 3);
            column[0] = SqlDecimal.Parse("999999999999999999");                       // 18 int digits, scale=0
            column[1] = SqlDecimal.Parse("0.000000000000000000000000000001");           // 0 int digits, scale=30
            column[2] = SqlDecimal.Parse("42");                                        // 2 int digits, scale=0
            // maxIntDigits=18, maxScale=30, sum=48 > 38 → clamp: precision=38, scale=20

            return new DataFrame(column);
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
