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
using System.Collections.Generic;
using Microsoft.Data.Analysis;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class loads and retrieves input/output dataset.
    /// </summary>
    public abstract class CSharpDataSet
    {
        public string Name { get; set; }
        public ushort ColumnsNumber { get; set; }

        /// <summary>
        /// This dataframe contains input/output dataset.
        /// </summary>
        public DataFrame CSharpDataFrame { get; set; }

        /// <summary>
        /// This property defines [] operator for CSharpDataSet.
        /// </summary>
        public CSharpColumn this[ushort columnNumber]
        {
            get
            {
                return _columns[columnNumber];
            }

            private set
            {
                _columns[columnNumber] = value;
            }
        }

        /// <summary>
        /// This dictionary contains all the columns metadata as CSharpColumn objects.
        /// </summary>
        protected Dictionary<ushort, CSharpColumn> _columns = new Dictionary<ushort, CSharpColumn>();

        /// <summary>
        /// Gets the columns metadata dictionary.
        /// </summary>
        public Dictionary<ushort, CSharpColumn> Columns => _columns;
    }
}
