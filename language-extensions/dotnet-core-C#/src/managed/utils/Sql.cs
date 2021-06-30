//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: Sql.cs
//
// Purpose:
//  This is the the main include for SqlDataType and Sql return values
//
//*********************************************************************
using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class includes SqlDataType and Sql related values
    /// </summary>
    public class Sql
    {
        public static readonly short SQL_SUCCESS = 0;
        public static readonly short SQL_ERROR = -1;
    }
}
