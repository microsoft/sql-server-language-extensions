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
        /// <summary>
        /// An integer identifying the index of this column.
        /// Columns are numbered sequentially in increasing order starting at 0.
        /// </summary>
        public ushort Id { get; set; }

        /// <summary>
        /// The name of this column.
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// The maximum size in bytes of the underlying data in this column.
        /// </summary>
        public ulong Size { get; set; }

        /// <summary>
        /// The Sql data type of this column.
        /// </summary>
        public SqlDataType DataType { get; set; }

        /// <summary>
        /// A value that indicates whether this column may contain NULL values.
        /// SQL_NO_NULLS(0): The column cannot contain NULL values.
        /// SQL_NULLABLE(1): The column may contain NULL values.
        /// </summary>
        public short Nullable { get; set; }

        /// <summary>
        /// The decimal digits of underlying data in this column.
        /// </summary>
        public short DecimalDigits { get; set; }
    }
}
