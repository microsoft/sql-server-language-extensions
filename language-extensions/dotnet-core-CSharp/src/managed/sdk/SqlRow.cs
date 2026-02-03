//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: SqlRow.cs
//
// Purpose:
//  Represents a single row of SQL data with named column access.
//
//*********************************************************************
using System;
using System.Collections.Generic;

namespace Microsoft.SqlServer.CSharpExtension.SDK
{
    /// <summary>
    /// Represents a single row of SQL data with typed column access by name or index.
    /// </summary>
    /// <remarks>
    /// <para>
    /// SqlRow provides a lightweight, read-only view of a single row from a SQL result set.
    /// It supports both index-based and name-based (case-insensitive) column access.
    /// </para>
    /// <para>
    /// Use <see cref="IsNull(string)"/> to check for NULL values before calling typed accessors
    /// to avoid <see cref="InvalidCastException"/>.
    /// </para>
    /// </remarks>
    /// <example>
    /// <code>
    /// // Access values by column name
    /// int id = row.GetInt32("id");
    /// string name = row.GetString("name");
    /// 
    /// // Check for nulls before accessing
    /// if (!row.IsNull("description"))
    /// {
    ///     string desc = row.GetString("description");
    /// }
    /// 
    /// // Use generic accessor for any type
    /// DateTime created = row.Get&lt;DateTime&gt;("created_date");
    /// </code>
    /// </example>
    public class SqlRow
    {
        private readonly object[] _values;
        private readonly Dictionary<string, int> _columnIndexMap;

        internal SqlRow(object[] values, Dictionary<string, int> columnIndexMap)
        {
            _values = values;
            _columnIndexMap = columnIndexMap;
        }

        /// <summary>Gets the number of columns in this row.</summary>
        public int ColumnCount => _values.Length;

        /// <summary>
        /// Gets the value at the specified column index.
        /// </summary>
        /// <param name="index">Zero-based column index.</param>
        /// <returns>The value as <see cref="object"/>, or null if the value is NULL.</returns>
        /// <exception cref="IndexOutOfRangeException">The index is out of range.</exception>
        public object this[int index] => _values[index];

        /// <summary>
        /// Gets the value for the specified column name (case-insensitive).
        /// </summary>
        /// <param name="name">The column name.</param>
        /// <returns>The value as <see cref="object"/>, or null if the value is NULL.</returns>
        /// <exception cref="ArgumentException">The column name was not found.</exception>
        public object this[string name] => _columnIndexMap.TryGetValue(name, out int i) ? _values[i] : throw new ArgumentException($"Column '{name}' not found");

        /// <summary>
        /// Gets the value at the specified index, cast to the specified type.
        /// </summary>
        /// <typeparam name="T">The type to cast the value to.</typeparam>
        /// <param name="index">Zero-based column index.</param>
        /// <returns>The typed value.</returns>
        /// <exception cref="InvalidCastException">The value cannot be cast to type T, or the value is NULL.</exception>
        public T Get<T>(int index) => (T)_values[index];

        /// <summary>
        /// Gets the value for the specified column name, cast to the specified type.
        /// </summary>
        /// <typeparam name="T">The type to cast the value to.</typeparam>
        /// <param name="name">The column name (case-insensitive).</param>
        /// <returns>The typed value.</returns>
        /// <exception cref="ArgumentException">The column name was not found.</exception>
        /// <exception cref="InvalidCastException">The value cannot be cast to type T, or the value is NULL.</exception>
        public T Get<T>(string name) => (T)this[name];

        /// <summary>
        /// Determines whether the value at the specified index is NULL.
        /// </summary>
        /// <param name="index">Zero-based column index.</param>
        /// <returns><c>true</c> if the value is NULL; otherwise, <c>false</c>.</returns>
        public bool IsNull(int index) => _values[index] == null;

        /// <summary>
        /// Determines whether the value for the specified column name is NULL.
        /// </summary>
        /// <param name="name">The column name (case-insensitive).</param>
        /// <returns><c>true</c> if the value is NULL; otherwise, <c>false</c>.</returns>
        public bool IsNull(string name) => this[name] == null;

        /// <summary>Gets the value as a 32-bit integer.</summary>
        /// <param name="name">The column name (case-insensitive).</param>
        public int GetInt32(string name) => Get<int>(name);

        /// <summary>Gets the value as a 64-bit integer.</summary>
        /// <param name="name">The column name (case-insensitive).</param>
        public long GetInt64(string name) => Get<long>(name);

        /// <summary>Gets the value as a string.</summary>
        /// <param name="name">The column name (case-insensitive).</param>
        public string GetString(string name) => Get<string>(name);

        /// <summary>Gets the value as a double-precision floating-point number.</summary>
        /// <param name="name">The column name (case-insensitive).</param>
        public double GetDouble(string name) => Get<double>(name);

        /// <summary>Gets the value as a boolean.</summary>
        /// <param name="name">The column name (case-insensitive).</param>
        public bool GetBoolean(string name) => Get<bool>(name);
    }
}
