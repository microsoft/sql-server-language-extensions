//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpParam.cs
//
// Purpose:
//  Classes storing information about input/output parameter.
//
//*********************************************************************
using System;
using System.Collections.Generic;
using static Microsoft.SqlServer.CSharpExtension.Sql;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class stores input/output parameter.
    /// </summary>
    public class CSharpParam
    {
        /// <summary>
        /// An integer identifying the index of this parameter.
        /// </summary>
        public ushort Number { get; set; }

        /// <summary>
        /// Null-terminated UTF-8 string containing the parameter's name.
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// The maximum size in bytes of the underlying data in this parameter.
        /// </summary>
        public ulong Size { get; set; }

        /// <summary>
        /// The Sql data type identifying this parameter's data type.
        /// </summary>
        public SqlDataType DataType { get; set; }

        /// <summary>
        /// The parameter's value.
        /// </summary>
        public dynamic Value { get; set; }

        /// <summary>
        /// The decimal digits of underlying data in this parameter
        /// </summary>
        public short DecimalDigits { get; set; }

        /// <summary>
        /// An integer value indicating the length in bytes
        /// of ParamValue, or SQL_NULL_DATA(-1) to indicate that the data is NULL.
        /// </summary>
        public int StrLenOrNullMap { get; set; }

        /// <summary>
        /// The type of the parameter.
        /// </summary>
        public short InputOutputType { get; set; }
    }
}
