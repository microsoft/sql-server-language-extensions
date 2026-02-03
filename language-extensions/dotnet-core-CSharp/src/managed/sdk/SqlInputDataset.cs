//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: SqlInputDataset.cs
//
// Purpose:
//  Provides IEnumerable access to SQL input data with SqlRow objects.
//
//*********************************************************************
using System;
using System.Collections;
using System.Collections.Generic;
using Microsoft.Data.Analysis;

namespace Microsoft.SqlServer.CSharpExtension.SDK
{
    /// <summary>
    /// Provides LINQ-compatible enumerable access to SQL input data as <see cref="SqlRow"/> objects.
    /// </summary>
    /// <remarks>
    /// <para>
    /// SqlInputDataset wraps a <see cref="DataFrame"/> and exposes each row as a <see cref="SqlRow"/>,
    /// enabling idiomatic C# LINQ queries over SQL input data.
    /// </para>
    /// <para>
    /// Create an instance using the <see cref="DataFrameExtensions.AsSqlDataset"/> extension method.
    /// </para>
    /// </remarks>
    /// <example>
    /// <code>
    /// public override DataFrame Execute(DataFrame input, Dictionary&lt;string, dynamic&gt; sqlParams)
    /// {
    ///     // Filter and transform using LINQ
    ///     var results = input.AsSqlDataset()
    ///         .Where(row => !row.IsNull("name"))
    ///         .Where(row => row.GetInt32("age") >= 18)
    ///         .OrderBy(row => row.GetString("name"))
    ///         .ToList();
    ///     
    ///     // Process results...
    /// }
    /// </code>
    /// </example>
    public class SqlInputDataset : IEnumerable<SqlRow>
    {
        private readonly DataFrame _dataFrame;
        private readonly Dictionary<string, int> _columnIndexMap;

        internal SqlInputDataset(DataFrame dataFrame)
        {
            _dataFrame = dataFrame ?? throw new ArgumentNullException(nameof(dataFrame));
            _columnIndexMap = new Dictionary<string, int>(StringComparer.OrdinalIgnoreCase);
            
            for (int i = 0; i < dataFrame.Columns.Count; i++)
                _columnIndexMap[dataFrame.Columns[i].Name] = i;
        }

        /// <summary>Gets the total number of rows in the dataset.</summary>
        public long RowCount => _dataFrame.Rows.Count;

        /// <summary>Gets the number of columns in the dataset.</summary>
        public int ColumnCount => _dataFrame.Columns.Count;

        /// <summary>
        /// Returns an enumerator that iterates through rows as <see cref="SqlRow"/> objects.
        /// </summary>
        /// <returns>An enumerator for the rows.</returns>
        public IEnumerator<SqlRow> GetEnumerator()
        {
            int colCount = _dataFrame.Columns.Count;
            for (long i = 0; i < _dataFrame.Rows.Count; i++)
            {
                DataFrameRow dfRow = _dataFrame.Rows[i];
                object[] values = new object[colCount];
                for (int j = 0; j < colCount; j++)
                    values[j] = dfRow[j];
                yield return new SqlRow(values, _columnIndexMap);
            }
        }

        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

        /// <summary>
        /// Gets the underlying DataFrame for backward compatibility or advanced operations.
        /// </summary>
        /// <returns>The wrapped <see cref="DataFrame"/>.</returns>
        public DataFrame ToDataFrame() => _dataFrame;
    }

    /// <summary>
    /// Extension methods for converting <see cref="DataFrame"/> to SDK types.
    /// </summary>
    public static class DataFrameExtensions
    {
        /// <summary>
        /// Converts a DataFrame to a <see cref="SqlInputDataset"/> for LINQ-style row access.
        /// </summary>
        /// <param name="dataFrame">The DataFrame to wrap.</param>
        /// <returns>A <see cref="SqlInputDataset"/> that enumerates rows as <see cref="SqlRow"/> objects.</returns>
        /// <exception cref="ArgumentNullException">The dataFrame is null.</exception>
        /// <example>
        /// <code>
        /// var filtered = input.AsSqlDataset()
        ///     .Where(row => row.GetString("status") == "active")
        ///     .ToList();
        /// </code>
        /// </example>
        public static SqlInputDataset AsSqlDataset(this DataFrame dataFrame) => new SqlInputDataset(dataFrame);
    }
}
