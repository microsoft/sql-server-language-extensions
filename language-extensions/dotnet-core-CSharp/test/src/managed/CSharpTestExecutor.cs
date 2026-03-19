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

    public class CSharpTestExecutorDecimalParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            // Test maximum SqlDecimal value (DECIMAL(38,0) max = 99999999999999999999999999999999999999)
            // Note: SqlDecimal supports full 38 digits, unlike C# decimal which is limited to ~29 digits
            sqlParams["@param0"] = SqlDecimal.Parse("99999999999999999999999999999999999999");
            
            // Test minimum value (negative max)
            sqlParams["@param1"] = SqlDecimal.Parse("-99999999999999999999999999999999999999");
            
            // Test high scale value (DECIMAL(38, 10)) - full 38-digit precision
            sqlParams["@param2"] = SqlDecimal.Parse("1234567890123456789012345678.1234567890");
            
            // Test zero
            sqlParams["@param3"] = new SqlDecimal(0);
            
            // Test small value with high precision (DECIMAL(38, 28))
            sqlParams["@param4"] = SqlDecimal.Parse("1234567890.1234567890123456789012345678");
            
            // Test typical financial value (DECIMAL(19, 4))
            sqlParams["@param5"] = SqlDecimal.Parse("123456789012345.6789");
            
            // Test negative financial value
            sqlParams["@param6"] = SqlDecimal.Parse("-123456789012345.6789");
            
            // Test null (last parameter)
            sqlParams["@param7"] = null;
            
            return null;
        }
    }

    /// <summary>
    /// Test executor for SqlDecimal OUTPUT parameters with maximum precision (38 digits).
    /// Tests the FromSqlDecimal() conversion for values at the edge of SQL Server DECIMAL's capability.
    /// 
    /// Note: SqlDecimal supports up to 38 digits of precision, matching SQL Server's DECIMAL/NUMERIC.
    /// </summary>
    public class CSharpTestExecutorDecimalHighScaleParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            // Set high-precision SqlDecimal values (38 significant digits total)
            // These exercise the FromSqlDecimal() conversion for SQL Server's maximum capability
            // SqlDecimal can represent values with up to 38 significant digits
            
            // param0: Maximum precision with integer and fractional parts (DECIMAL(38, 10))
            sqlParams["@param0"] = SqlDecimal.Parse("1234567890123456789012345678.9012345678");
            
            // param1: Large fractional precision (DECIMAL(38, 28))
            sqlParams["@param1"] = SqlDecimal.Parse("1234567890.1234567890123456789012345678");
            
            // param2: Different high-precision pattern (DECIMAL(38, 20))
            sqlParams["@param2"] = SqlDecimal.Parse("123456789012345678.12345678901234567890");
            
            // param3: Maximum fractional precision (DECIMAL(38, 38))
            sqlParams["@param3"] = SqlDecimal.Parse("0.12345678901234567890123456789012345678");
            
            // param4: Negative high-precision value (DECIMAL(38, 18))
            sqlParams["@param4"] = SqlDecimal.Parse("-12345678901234567890.123456789012345678");
            
            // param5: Zero value for validation
            sqlParams["@param5"] = new SqlDecimal(0);
            
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
