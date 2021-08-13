//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: ExceptionUtils.cs
//
// Purpose:
//  Handle the exceptions from managed language extensions APIs
//
//*********************************************************************
using System;
using static Microsoft.SqlServer.CSharpExtension.Sql;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class handles the exceptions from managed language extensions APIs and
    /// logs the error messages to the stderr.
    /// </summary>
    class ExceptionUtils
    {
        /// <summary>
        /// This method calls the APIs and handles exceptions.
        /// </summary>
        /// <param name="func">
        /// Managed language extensions APIs fucntion
        /// </param>
        /// <returns>
        /// SQL_SUCCESS(0), SQL_ERROR(-1)
        /// </returns>
        public static short WrapError(Action func)
        {
            try
            {
                func();
                return SQL_SUCCESS;
            }
            catch (Exception e)
            {
                Logging.Error(e.StackTrace + "Error: " + e.Message);
                return SQL_ERROR;
            }
        }
    }
}
