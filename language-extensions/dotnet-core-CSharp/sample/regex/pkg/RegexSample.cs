//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: RegexSample.cs
//
// Purpose:
//  Regex sample class using a regular expression that checks if a text contains the word "C#" or "c#"
//
//*********************************************************************
using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using Microsoft.Data.Analysis;
using Microsoft.SqlServer.CSharpExtension.SDK;
using System.Text.RegularExpressions;
using static Microsoft.SqlServer.CSharpExtension.Sql;

namespace UserExecutor
{
    /// <summary>
    /// This class extends the AbstractSqlServerExtensionExecutor and uses
    /// a regular expression that checks if a text contains the word "C#" or "c#"
    /// </summary>
    public class CSharpRegexExecutor: AbstractSqlServerExtensionExecutor
    {
        /// <summary>
        /// This method overrides the Execute method from AbstractSqlServerExtensionExecutor.
        /// </summary>
        /// <param name="input">
        /// A C# DataFrame contains the input dataset.
        /// </param>
        /// <param name="sqlParams">
        /// A Dictionary contains the parameters from SQL server with name as the key.
        /// </param>
        /// <returns>
        /// A C# DataFrame contains the output dataset.
        /// </returns>
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            // Drop NULL values and sort by id
            //
            input = input.DropNulls().OrderBy("id");

            // Create empty output DataFrame with two columns
            //
            DataFrame output = new DataFrame(new PrimitiveDataFrameColumn<int>("id", 0), new StringDataFrameColumn("text", 0));

            // Filter text containing specific substring using regex expression
            //
            DataFrameColumn texts = input.Columns["text"];
            for(int i = 0; i < texts.Length; ++i)
            {
                if(Regex.IsMatch((string)texts[i], sqlParams["@regexExpr"]))
                {
                    output.Append(input.Rows[i], true);
                }
            }

            // Modify the parameters
            //
            sqlParams["@rowsCount"]  = output.Rows.Count;
            sqlParams["@regexExpr"] = "Success!";

            // Optionally, specify that "text" column should be output as NVARCHAR (UTF-16)
            // instead of the default VARCHAR (UTF-8). Uncomment the line below to enable:
            //
            // OutputColumnDataTypes["text"] = SqlDataType.DotNetWChar;

            // Return output dataset as a DataFrame
            //
            return output;
        }
    }
}
