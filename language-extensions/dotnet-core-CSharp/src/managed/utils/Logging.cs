//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: Logging.cs
//
// Purpose:
//  Implementation of logging for the .NET Core C# extension.
//
//*********************************************************************
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class implements logging for the .NET Core C# extension.
    /// </summary>
    class Logging
    {
        /// <summary>
        /// This method logs a message to stdout.
        /// </summary>
        /// <param name="message">
        /// Trace message
        /// </param>
        public static void Trace(string message)
        {
        #if DEBUG
            Console.WriteLine(message);
        #endif
        }

        /// <summary>
        /// This method logs an error to stderr.
        /// </summary>
        /// <param name="message">
        /// Error message
        /// </param>
        public static void Error(string message)
        {
            Console.Error.WriteLine(message);
        }
    }
}
