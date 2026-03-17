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

    public class CSharpTestExecutorDecimalParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            // Test maximum C# decimal value (decimal.MaxValue = 79228162514264337593543950335)
            // Note: C# decimal supports ~29 digits, even though SQL NUMERIC can support up to 38 digits
            sqlParams["@param0"] = decimal.MaxValue;
            
            // Test minimum value (negative max)
            sqlParams["@param1"] = decimal.MinValue;
            
            // Test high scale value (DECIMAL(38, 10))
            // Using 18 significant digits to stay within C# decimal range
            sqlParams["@param2"] = 12345678.1234567890m;
            
            // Test zero
            sqlParams["@param3"] = 0m;
            
            // Test small value with high precision (28 decimal places, max for C# decimal)
            sqlParams["@param4"] = 0.1234567890123456789012345678m;
            
            // Test typical financial value (DECIMAL(19, 4))
            sqlParams["@param5"] = 123456789012345.6789m;
            
            // Test negative financial value
            sqlParams["@param6"] = -123456789012345.6789m;
            
            // Test null (last parameter)
            sqlParams["@param7"] = null;
            
            return null;
        }
    }

    /// <summary>
    /// Test executor for decimal OUTPUT parameters with maximum precision (29 digits).
    /// Tests the FromDecimal() conversion for values at the edge of C# decimal's capability.
    /// 
    /// Note: C# decimal normalizes values - the scale is determined by the value's actual
    /// precision requirements, not by a declared scale. This tests high-precision conversions.
    /// </summary>
    public class CSharpTestExecutorDecimalHighScaleParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            // Set high-precision decimal values (29 significant digits total)
            // These exercise the FromDecimal() conversion for C# decimal's maximum capability
            // C# decimal can represent values with up to 29 significant digits
            
            // param0: Maximum precision with integer and fractional parts
            sqlParams["@param0"] = 12345678901234567.890123456789m; // 29 total digits
            
            // param1: Large fractional precision
            sqlParams["@param1"] = 1.2345678901234567890123456789m; // 29 total digits
            
            // param2: Different high-precision pattern
            sqlParams["@param2"] = 123.45678901234567890123456789m; // 29 total digits
            
            // param3: Maximum fractional precision
            sqlParams["@param3"] = 0.12345678901234567890123456789m; // 29 total digits
            
            // param4: Negative high-precision value
            sqlParams["@param4"] = -987.65432109876543210987654321m; // 29 total digits
            
            // param5: Zero value for validation
            sqlParams["@param5"] = 0.0m;
            
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
