//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: PrimitiveDataset.cs
//
// Purpose:
//  Dataset with explicit SQL type metadata, similar to Java's PrimitiveDataset.
//
//*********************************************************************
using System;
using System.Collections.Generic;
using Microsoft.Data.Analysis;

namespace Microsoft.SqlServer.CSharpExtension.SDK
{
    /// <summary>
    /// A dataset that stores typed arrays with explicit SQL type metadata for output.
    /// </summary>
    /// <remarks>
    /// <para>
    /// PrimitiveDataset provides a Java SDK-like pattern for building output data with
    /// explicit control over SQL types. This is especially useful for distinguishing
    /// between VARCHAR (UTF-8) and NVARCHAR (UTF-16) string columns.
    /// </para>
    /// <para>
    /// Usage pattern:
    /// <list type="number">
    ///   <item>Define column metadata using <see cref="AddColumnMetadata"/></item>
    ///   <item>Add column data using <see cref="AddColumn{T}"/></item>
    ///   <item>Return using <see cref="ToDataFrame(AbstractSqlServerExtensionExecutor)"/></item>
    /// </list>
    /// </para>
    /// </remarks>
    /// <example>
    /// <code>
    /// public override DataFrame Execute(DataFrame input, Dictionary&lt;string, dynamic&gt; sqlParams)
    /// {
    ///     var output = new PrimitiveDataset();
    ///     
    ///     // Define columns with explicit SQL types
    ///     output.AddColumnMetadata(0, "id", SqlTypes.INTEGER);
    ///     output.AddColumnMetadata(1, "name", SqlTypes.NVARCHAR);  // UTF-16
    ///     output.AddColumnMetadata(2, "code", SqlTypes.VARCHAR);   // UTF-8
    ///     
    ///     // Add data
    ///     output.AddColumn(0, new int[] { 1, 2, 3 });
    ///     output.AddColumn(1, new string[] { "Alice", "Bob", "Charlie" });
    ///     output.AddColumn(2, new string[] { "A01", "B02", "C03" });
    ///     
    ///     // Return - this copies types to OutputColumnTypes automatically
    ///     return output.ToDataFrame(this);
    /// }
    /// </code>
    /// </example>
    public class PrimitiveDataset
    {
        private readonly List<(string Name, int SqlType, Array Data)> _columns = new List<(string, int, Array)>();

        /// <summary>
        /// Adds metadata for a column. Must be called before <see cref="AddColumn{T}"/>.
        /// </summary>
        /// <param name="columnId">Zero-based column index.</param>
        /// <param name="columnName">The column name.</param>
        /// <param name="sqlType">The SQL type from <see cref="SqlTypes"/> (e.g., SqlTypes.NVARCHAR).</param>
        /// <exception cref="ArgumentException">Column metadata already exists for this columnId.</exception>
        public void AddColumnMetadata(int columnId, string columnName, int sqlType)
        {
            while (_columns.Count <= columnId)
                _columns.Add((null, 0, null));
            
            if (_columns[columnId].Name != null)
                throw new ArgumentException($"Column {columnId} already defined");
            
            _columns[columnId] = (columnName, sqlType, null);
        }

        /// <summary>
        /// Adds data for a column. <see cref="AddColumnMetadata"/> must be called first.
        /// </summary>
        /// <typeparam name="T">The element type of the data array.</typeparam>
        /// <param name="columnId">Zero-based column index.</param>
        /// <param name="data">The column data as an array.</param>
        /// <exception cref="ArgumentException">Column metadata not defined for this columnId.</exception>
        public void AddColumn<T>(int columnId, T[] data)
        {
            if (columnId >= _columns.Count || _columns[columnId].Name == null)
                throw new ArgumentException($"Column {columnId} metadata not defined. Call AddColumnMetadata first.");
            
            (string Name, int SqlType, Array Data) col = _columns[columnId];
            _columns[columnId] = (col.Name, col.SqlType, data);
        }

        /// <summary>Gets the number of columns defined.</summary>
        public int ColumnCount => _columns.Count;

        /// <summary>
        /// Gets metadata for a column.
        /// </summary>
        /// <param name="columnId">Zero-based column index.</param>
        /// <returns>A tuple containing the column name and SQL type.</returns>
        public (string Name, int SqlType) GetColumnInfo(int columnId) => (_columns[columnId].Name, _columns[columnId].SqlType);

        /// <summary>
        /// Gets the data array for a column.
        /// </summary>
        /// <typeparam name="T">The expected element type.</typeparam>
        /// <param name="columnId">Zero-based column index.</param>
        /// <returns>The column data as a typed array.</returns>
        public T[] GetColumn<T>(int columnId) => (T[])_columns[columnId].Data;

        /// <summary>
        /// Converts this dataset to a DataFrame.
        /// </summary>
        /// <returns>A DataFrame containing all columns.</returns>
        /// <remarks>
        /// This overload does not copy column types to OutputColumnTypes.
        /// Use <see cref="ToDataFrame(AbstractSqlServerExtensionExecutor)"/> instead
        /// to ensure proper type handling for string columns.
        /// </remarks>
        public DataFrame ToDataFrame()
        {
            List<DataFrameColumn> dfColumns = new List<DataFrameColumn>();
            
            for (int i = 0; i < _columns.Count; i++)
            {
                (string name, int sqlType, Array data) = _columns[i];
                if (data == null) continue;

                DataFrameColumn col = data switch
                {
                    int[] arr => new Int32DataFrameColumn(name, arr),
                    long[] arr => new Int64DataFrameColumn(name, arr),
                    short[] arr => new Int16DataFrameColumn(name, arr),
                    byte[] arr => new ByteDataFrameColumn(name, arr),
                    float[] arr => new SingleDataFrameColumn(name, arr),
                    double[] arr => new DoubleDataFrameColumn(name, arr),
                    bool[] arr => new BooleanDataFrameColumn(name, arr),
                    string[] arr => new StringDataFrameColumn(name, arr),
                    _ => throw new NotSupportedException($"Unsupported array type: {data.GetType()}")
                };
                dfColumns.Add(col);
            }

            return new DataFrame(dfColumns);
        }

        /// <summary>
        /// Gets a dictionary mapping column names to SQL types.
        /// </summary>
        /// <returns>A case-insensitive dictionary of column name to <see cref="SqlTypes"/> value.</returns>
        public Dictionary<string, int> GetColumnTypes()
        {
            Dictionary<string, int> types = new Dictionary<string, int>(StringComparer.OrdinalIgnoreCase);
            foreach ((string name, int sqlType, Array _) in _columns)
            {
                if (name != null) types[name] = sqlType;
            }
            return types;
        }

        /// <summary>
        /// Copies column types to the executor and returns a DataFrame.
        /// This is the recommended way to return a PrimitiveDataset from Execute().
        /// </summary>
        /// <param name="executor">The executor instance (typically <c>this</c>).</param>
        /// <returns>A DataFrame containing all columns.</returns>
        /// <remarks>
        /// This method automatically populates <see cref="AbstractSqlServerExtensionExecutor.OutputColumnTypes"/>
        /// with the SQL types defined via <see cref="AddColumnMetadata"/>, ensuring proper
        /// encoding (UTF-8 vs UTF-16) for string columns.
        /// </remarks>
        /// <example>
        /// <code>
        /// return output.ToDataFrame(this);  // Pass 'this' from Execute()
        /// </code>
        /// </example>
        public DataFrame ToDataFrame(AbstractSqlServerExtensionExecutor executor)
        {
            foreach (KeyValuePair<string, int> kvp in GetColumnTypes())
                executor.OutputColumnTypes[kvp.Key] = kvp.Value;
            return ToDataFrame();
        }
    }
}
