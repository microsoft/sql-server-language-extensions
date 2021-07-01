//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpColumn.cs
//
// Purpose:
//  Encapsulate data column attributes
//
//*********************************************************************
using System;
using System.Collections.Generic;
using static Microsoft.SqlServer.CSharpExtension.Sql;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class stores column meta data
    /// </summary>
    public class CSharpColumn
    {
        public ushort Id { get; set; }
        public string Name { get; set; }
        public ulong Size { get; set; }
        public SqlDataType DataType { get; set; }
        public short Nullable { get; set; }
        public short DecimalDigits { get; set; }
    }
}
