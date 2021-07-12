//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpDataSet.cs
//
// Purpose:
//  Class handling loading and retrieving data from a DataFrame.
//
//*********************************************************************
using System;
using System.Collections.Generic;
using static Microsoft.SqlServer.CSharpExtension.Sql;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class loads and retrieves input/output dataset.
    /// </summary>
    public class CSharpDataSet
    {
        public string Name { get; set; }
        public ushort ColumnsNumber { get; set; }
    }

    /// <summary>
    /// This class loads and retrieves input dataset.
    /// </summary>
    public class CSharpInputDataSet: CSharpDataSet
    {
        /// <summary>
        /// This dictionary contains all the columns as CSharpColumn objects.
        /// </summary>
        private Dictionary<ushort, CSharpColumn> _columns = new Dictionary<ushort, CSharpColumn>();

        /// <summary>
        /// This method appends each column as a CSharpColumn object to a dictionary.
        /// </summary>
        public void InitColumn(
            ushort      columnNumber,
            string      columnName,
            ulong       columnSize,
            SqlDataType dataType,
            short       nullable,
            short       decimalDigits)
        {
            Logging.Trace("CSharpInputDataSet::InitColumn");
            _columns[columnNumber] = new CSharpColumn
            {
                Name = columnName,
                DataType = dataType,
                DecimalDigits = decimalDigits,
                Nullable = nullable,
                Size = columnSize,
                Id = columnNumber
            };
        }
    }
}