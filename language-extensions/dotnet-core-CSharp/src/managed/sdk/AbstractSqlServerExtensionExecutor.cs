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
    /// Abstract class containing interface used by the .NET Core C# extension
    /// </summary>
    public abstract class AbstractSqlServerExtensionExecutor
    {
        /// <summary>
        /// Supported versions of the .NET Core C# extension
        /// </summary>
        public readonly int SQLSERVER_DOTNET_LANG_EXTENSION_V1 = 1;

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
