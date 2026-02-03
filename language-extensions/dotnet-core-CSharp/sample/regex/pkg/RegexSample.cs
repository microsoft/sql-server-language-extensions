//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: RegexSample.cs
//
// Purpose:
//  Regex sample class using a regular expression that checks if a text 
//  contains the word "C#" or "c#". Demonstrates PrimitiveDataset pattern
//  with explicit column type control (like Java's addColumnMetadata).
//
//*********************************************************************
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using Microsoft.Data.Analysis;
using Microsoft.SqlServer.CSharpExtension.SDK;

namespace UserExecutor
{
    /// <summary>
    /// This class extends the AbstractSqlServerExtensionExecutor and uses
    /// a regular expression that checks if a text contains the word "C#" or "c#".
    /// Demonstrates the PrimitiveDataset pattern for explicit output type control.
    /// </summary>
    public class CSharpRegexExecutor : AbstractSqlServerExtensionExecutor
    {
        /// <summary>
        /// This method overrides the Execute method from AbstractSqlServerExtensionExecutor.
        /// Uses PrimitiveDataset for explicit column type control (similar to Java SDK).
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
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            // Use SqlInputDataset for LINQ-style access to input rows
            //
            List<SqlRow> rows = input.AsSqlDataset()
                .Where(row => !row.IsNull("text") && !row.IsNull("id"))
                .Where(row => Regex.IsMatch(row.GetString("text"), sqlParams["@regexExpr"]))
                .OrderBy(row => row.GetInt32("id"))
                .ToList();

            // Create output using PrimitiveDataset with explicit column types
            // This is similar to Java's addColumnMetadata + addColumn pattern
            //
            PrimitiveDataset output = new PrimitiveDataset();

            // Define columns with explicit SQL types:
            // - Column 0: id as INTEGER
            // - Column 1: text as NVARCHAR (UTF-16)
            // - Column 2: text_upper as VARCHAR (UTF-8)
            //
            output.AddColumnMetadata(0, "id", SqlTypes.INTEGER);
            output.AddColumnMetadata(1, "text", SqlTypes.NVARCHAR);
            output.AddColumnMetadata(2, "text_upper", SqlTypes.VARCHAR);

            // Add data columns
            //
            output.AddColumn(0, rows.Select(r => r.GetInt32("id")).ToArray());
            output.AddColumn(1, rows.Select(r => r.GetString("text")).ToArray());
            output.AddColumn(2, rows.Select(r => r.GetString("text").ToUpperInvariant()).ToArray());

            // Modify the output parameters
            //
            sqlParams["@rowsCount"] = rows.Count;
            sqlParams["@regexExpr"] = "Success!";

            // Convert to DataFrame - this overload also copies column types to OutputColumnTypes
            //
            return output.ToDataFrame(this);
        }
    }
}
