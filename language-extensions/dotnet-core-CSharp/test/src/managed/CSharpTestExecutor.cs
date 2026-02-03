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

    /// <summary>
    /// Test executor that explicitly converts VARCHAR input to NVARCHAR output
    /// using OutputColumnTypes. Input is UTF-8, output should be UTF-16.
    /// </summary>
    public class CSharpTestExecutorVarcharToNvarchar : AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            Console.WriteLine("Hello .NET Core CSharpExtension!");

            // Explicitly request NVARCHAR (UTF-16) output for string columns
            // even though input was VARCHAR (UTF-8)
            foreach (DataFrameColumn column in input.Columns)
            {
                if (column.DataType == typeof(string))
                {
                    OutputColumnTypes[column.Name] = SqlTypes.NVARCHAR;
                }
            }

            return input;
        }
    }

    /// <summary>
    /// Test executor that explicitly converts NVARCHAR input to VARCHAR output
    /// using OutputColumnTypes. Input is UTF-16, output should be UTF-8.
    /// </summary>
    public class CSharpTestExecutorNvarcharToVarchar : AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            Console.WriteLine("Hello .NET Core CSharpExtension!");

            // Explicitly request VARCHAR (UTF-8) output for string columns
            // even though input was NVARCHAR (UTF-16)
            foreach (DataFrameColumn column in input.Columns)
            {
                if (column.DataType == typeof(string))
                {
                    OutputColumnTypes[column.Name] = SqlTypes.VARCHAR;
                }
            }

            return input;
        }
    }

    /// <summary>
    /// Test executor that outputs mixed types: some columns as NVARCHAR, some as VARCHAR.
    /// First column is NVARCHAR, second column is VARCHAR.
    /// </summary>
    public class CSharpTestExecutorMixedStringTypes : AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            Console.WriteLine("Hello .NET Core CSharpExtension!");

            int columnIndex = 0;
            foreach (DataFrameColumn column in input.Columns)
            {
                if (column.DataType == typeof(string))
                {
                    // Alternate: first string column = NVARCHAR, second = VARCHAR
                    OutputColumnTypes[column.Name] = (columnIndex % 2 == 0)
                        ? SqlTypes.NVARCHAR
                        : SqlTypes.VARCHAR;
                    columnIndex++;
                }
            }

            return input;
        }
    }

    /// <summary>
    /// Test executor that uses PrimitiveDataset with explicit NVARCHAR column types.
    /// Creates new output columns not in input schema.
    /// </summary>
    public class CSharpTestExecutorPrimitiveDatasetNvarchar : AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            PrimitiveDataset output = new PrimitiveDataset();

            // Add column metadata with explicit NVARCHAR type
            output.AddColumnMetadata(0, "UnicodeOutput", SqlTypes.NVARCHAR);

            // Add data rows
            output.AddColumn<string>(0, new string[] { "Hello世界", "Café", "日本語", null, "Test" });

            return output.ToDataFrame(this);
        }
    }

    /// <summary>
    /// Test executor that uses PrimitiveDataset with explicit VARCHAR column types.
    /// Creates new output columns not in input schema.
    /// </summary>
    public class CSharpTestExecutorPrimitiveDatasetVarchar : AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            PrimitiveDataset output = new PrimitiveDataset();

            // Add column metadata with explicit VARCHAR type (UTF-8)
            output.AddColumnMetadata(0, "AsciiOutput", SqlTypes.VARCHAR);

            // Add data rows - ASCII only for VARCHAR
            output.AddColumn<string>(0, new string[] { "Hello", "World", "Test", null, "Data" });

            return output.ToDataFrame(this);
        }
    }

    /// <summary>
    /// Test executor that processes input with Unicode characters and outputs NVARCHAR.
    /// This tests that Unicode is preserved correctly through the pipeline.
    /// </summary>
    public class CSharpTestExecutorUnicodePreservation : AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            // Ensure all string columns are output as NVARCHAR to preserve Unicode
            foreach (DataFrameColumn column in input.Columns)
            {
                if (column.DataType == typeof(string))
                {
                    OutputColumnTypes[column.Name] = SqlTypes.NVARCHAR;
                }
            }

            return input;
        }
    }
}
