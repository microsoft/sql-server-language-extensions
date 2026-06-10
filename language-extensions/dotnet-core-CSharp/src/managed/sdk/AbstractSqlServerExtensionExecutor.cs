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
    /// SQL output type for a string result-set column. Used with
    /// <see cref="AbstractSqlServerExtensionExecutor.StringOutputColumnTypes"/> to control
    /// whether a string column is emitted as VARCHAR or NVARCHAR. Because these are the only
    /// two expressible values, invalid overrides cannot be requested.
    /// </summary>
    public enum StringOutputType
    {
        /// <summary>
        /// VARCHAR (UTF-8). This is the default for string columns.
        /// </summary>
        VarChar,

        /// <summary>
        /// NVARCHAR (UTF-16). Use for native Unicode output.
        /// </summary>
        NVarChar
    }

    /// <summary>
    /// Abstract class containing interface used by the .NET Core C# extension
    /// </summary>
    public abstract class AbstractSqlServerExtensionExecutor
    {
        /// <summary>
        /// Supported versions of the .NET Core C# extension
        /// </summary>
        public readonly int SQLSERVER_DOTNET_LANG_EXTENSION_V1 = 1;

        /// <summary>
        /// Optional: override the SQL output type of individual string result-set columns by name.
        /// Only string columns are affected; entries whose key is a non-string column — or matches
        /// no output column at all — are ignored (a trace message is logged to aid debugging).
        /// String columns default to VARCHAR (UTF-8); map a column name to
        /// <see cref="StringOutputType.NVarChar"/> to emit that column as NVARCHAR (UTF-16).
        ///
        /// Populate this dictionary in place inside <see cref="Execute"/>, e.g.
        /// <c>StringOutputColumnTypes["text"] = StringOutputType.NVarChar;</c>. The property is
        /// get-only (the dictionary cannot be replaced) and is read once, after Execute() returns,
        /// when the output schema is built.
        /// </summary>
        public Dictionary<string, StringOutputType> StringOutputColumnTypes { get; } = new Dictionary<string, StringOutputType>();

        /// <summary>
        /// Default constructor for AbstractSqlServerExtensionExecutor
        /// </summary>
        public AbstractSqlServerExtensionExecutor() {}

        /// <summary>
        /// Default implementation of init() is no-op
        /// </summary>
        public virtual void Init(string sessionId, int taskId, int numTasks) {}

        /// <summary>
        /// User execute method
        /// </summary>
        /// <param name="input">
        /// A Dataframe containing all the input columns
        /// </param>
        /// <param name="sqlParams">
        /// A Dictionary containing all the parameters with name as the key
        /// </param>
        public abstract DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams);

        /// <summary>
        /// Default implementation of cleanup() is no-op
        /// </summary>
        public virtual void Cleanup() {}
    }
}
