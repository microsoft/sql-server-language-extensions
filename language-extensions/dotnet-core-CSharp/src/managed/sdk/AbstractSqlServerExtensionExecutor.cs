//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: AbstractSqlServerExtensionExecutor.cs
//
// Purpose:
//  Abstract class containing interface used by the .NET Core C# extension
//
//*********************************************************************
using System.Collections.Generic;
using Microsoft.Data.Analysis;

namespace Microsoft.SqlServer.CSharpExtension.SDK
{
    /// <summary>
    /// SQL data type constants for specifying output column types.
    /// </summary>
    /// <remarks>
    /// <para>
    /// These values correspond to java.sql.Types and are used to explicitly specify
    /// the SQL type for output columns, particularly to distinguish between
    /// VARCHAR (UTF-8) and NVARCHAR (UTF-16) string encodings.
    /// </para>
    /// <para>
    /// Use these constants with <see cref="AbstractSqlServerExtensionExecutor.OutputColumnTypes"/>
    /// or <see cref="PrimitiveDataset.AddColumnMetadata"/>.
    /// </para>
    /// </remarks>
    /// <example>
    /// <code>
    /// // Specify output types explicitly
    /// OutputColumnTypes["unicodeColumn"] = SqlTypes.NVARCHAR;  // UTF-16
    /// OutputColumnTypes["asciiColumn"] = SqlTypes.VARCHAR;     // UTF-8
    /// </code>
    /// </example>
    public static class SqlTypes
    {
        /// <summary>VARCHAR - Variable-length UTF-8 encoded string. Maps to SQL Server varchar.</summary>
        public const int VARCHAR = 12;

        /// <summary>NVARCHAR - Variable-length UTF-16 encoded string. Maps to SQL Server nvarchar.</summary>
        public const int NVARCHAR = -9;

        /// <summary>CHAR - Fixed-length UTF-8 encoded string. Maps to SQL Server char.</summary>
        public const int CHAR = 1;

        /// <summary>NCHAR - Fixed-length UTF-16 encoded string. Maps to SQL Server nchar.</summary>
        public const int NCHAR = -15;

        /// <summary>INTEGER - 32-bit signed integer. Maps to SQL Server int.</summary>
        public const int INTEGER = 4;

        /// <summary>BIGINT - 64-bit signed integer. Maps to SQL Server bigint.</summary>
        public const int BIGINT = -5;

        /// <summary>SMALLINT - 16-bit signed integer. Maps to SQL Server smallint.</summary>
        public const int SMALLINT = 5;

        /// <summary>TINYINT - 8-bit unsigned integer. Maps to SQL Server tinyint.</summary>
        public const int TINYINT = -6;

        /// <summary>BIT - Boolean value. Maps to SQL Server bit.</summary>
        public const int BIT = -7;

        /// <summary>REAL - 32-bit floating-point number. Maps to SQL Server real.</summary>
        public const int REAL = 7;

        /// <summary>DOUBLE - 64-bit floating-point number. Maps to SQL Server float.</summary>
        public const int DOUBLE = 8;

        /// <summary>FLOAT - 64-bit floating-point number (alias for DOUBLE). Maps to SQL Server float.</summary>
        public const int FLOAT = 6;
    }

    /// <summary>
    /// Base class for SQL Server Language Extension executors.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Derive from this class to implement custom C# logic that can be called from SQL Server
    /// using sp_execute_external_script with @language = N'CSharp'.
    /// </para>
    /// <para>
    /// Override <see cref="Execute"/> to implement your data processing logic.
    /// Optionally override <see cref="Init"/> and <see cref="Cleanup"/> for setup and teardown.
    /// </para>
    /// </remarks>
    /// <example>
    /// <code>
    /// public class MyExecutor : AbstractSqlServerExtensionExecutor
    /// {
    ///     public override DataFrame Execute(DataFrame input, Dictionary&lt;string, dynamic&gt; sqlParams)
    ///     {
    ///         // Process input data
    ///         var output = new PrimitiveDataset();
    ///         output.AddColumnMetadata(0, "result", SqlTypes.NVARCHAR);
    ///         output.AddColumn(0, new string[] { "Hello, World!" });
    ///         return output.ToDataFrame(this);
    ///     }
    /// }
    /// </code>
    /// </example>
    public abstract class AbstractSqlServerExtensionExecutor
    {
        /// <summary>
        /// Version identifier for the .NET Core C# language extension.
        /// </summary>
        public readonly int SQLSERVER_DOTNET_LANG_EXTENSION_V1 = 1;

        /// <summary>
        /// Dictionary specifying SQL types for output columns.
        /// </summary>
        /// <remarks>
        /// <para>
        /// Use this property to explicitly specify the SQL type for string output columns.
        /// This is important for controlling whether strings are encoded as UTF-8 (VARCHAR)
        /// or UTF-16 (NVARCHAR).
        /// </para>
        /// <para>
        /// Column names are matched case-insensitively. If a column is not specified here,
        /// the extension will attempt to preserve the input column type for columns with
        /// matching names, or default to VARCHAR (UTF-8).
        /// </para>
        /// </remarks>
        /// <example>
        /// <code>
        /// public override DataFrame Execute(DataFrame input, Dictionary&lt;string, dynamic&gt; sqlParams)
        /// {
        ///     OutputColumnTypes["unicodeName"] = SqlTypes.NVARCHAR;
        ///     OutputColumnTypes["asciiCode"] = SqlTypes.VARCHAR;
        ///     // ... create and return DataFrame
        /// }
        /// </code>
        /// </example>
        public Dictionary<string, int> OutputColumnTypes { get; } = new Dictionary<string, int>(System.StringComparer.OrdinalIgnoreCase);

        /// <summary>
        /// Initializes a new instance of the executor.
        /// </summary>
        public AbstractSqlServerExtensionExecutor() {}

        /// <summary>
        /// Called once before <see cref="Execute"/> to initialize the session.
        /// </summary>
        /// <param name="sessionId">Unique identifier for the SQL Server session.</param>
        /// <param name="taskId">The task ID for parallel execution (0-based).</param>
        /// <param name="numTasks">Total number of parallel tasks.</param>
        /// <remarks>
        /// Override this method to perform one-time initialization such as loading
        /// configuration or establishing connections.
        /// </remarks>
        public virtual void Init(string sessionId, int taskId, int numTasks) {}

        /// <summary>
        /// Processes input data and returns output data.
        /// </summary>
        /// <param name="input">A DataFrame containing all input columns from SQL Server.</param>
        /// <param name="sqlParams">A dictionary of SQL parameters keyed by parameter name (including @ prefix).</param>
        /// <returns>A DataFrame containing the output data to return to SQL Server.</returns>
        /// <remarks>
        /// <para>
        /// This is the main entry point for your C# logic. The input DataFrame contains
        /// the data passed from SQL Server, and the returned DataFrame will be sent back.
        /// </para>
        /// <para>
        /// Modify sqlParams values to return updated parameter values to SQL Server.
        /// Use <see cref="OutputColumnTypes"/> or <see cref="PrimitiveDataset"/> to control
        /// output column types.
        /// </para>
        /// </remarks>
        public abstract DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams);

        /// <summary>
        /// Called once after <see cref="Execute"/> to clean up resources.
        /// </summary>
        /// <remarks>
        /// Override this method to release resources such as connections or temporary files.
        /// </remarks>
        public virtual void Cleanup() {}
    }
}
