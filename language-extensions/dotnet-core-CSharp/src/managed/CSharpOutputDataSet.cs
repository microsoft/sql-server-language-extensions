//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpOutputDataSet.cs
//
// Purpose:
//  Class retrieving data from a DataFrame.
//
//*********************************************************************
using System;
using System.Data.SqlTypes;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using Microsoft.Data.Analysis;
using static Microsoft.SqlServer.CSharpExtension.Sql;
using static Microsoft.SqlServer.CSharpExtension.SqlNumericHelper;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class loads and retrieves output dataset.
    /// </summary>
    public class CSharpOutputDataSet: CSharpDataSet
    {
        /// <summary>
        /// A list of data array and null map handles to prevent garbage collector releasing the memory before the objects are used.
        /// </summary>
        private List<GCHandle> _handleList = new List<GCHandle>();

        /// <summary>
        /// An array of IntPtr containing strLenOrNullMap pointers for each column.
        /// IntPtr is blittable and can be pinned with GCHandle.
        /// </summary>
        private IntPtr[] _strLenOrNullMapPtrs;

        /// <summary>
        /// An array of IntPtr containing data pointers for each column.
        /// IntPtr is blittable and can be pinned with GCHandle.
        /// </summary>
        private IntPtr[] _dataPtrs;

        /// <summary>
        /// This method extracts metadata and actual data for each column supplied
        /// by extracting data and information from every DataFrameColumn.
        /// </summary>
        /// <param name="dataFrame">The DataFrame containing the output data.</param>
        /// <param name="outputColumnDataTypes">Optional user-specified column data types by name.</param>
        public unsafe void ExtractColumns(
            DataFrame dataFrame,
            Dictionary<string, SqlDataType> outputColumnDataTypes = null)
        {
            Logging.Trace("CSharpOutputDataSet::ExtractColumns");
            _strLenOrNullMapPtrs = new IntPtr[ColumnsNumber];
            _dataPtrs = new IntPtr[ColumnsNumber];

            for(ushort columnNumber = 0; columnNumber < ColumnsNumber; ++columnNumber)
            {
                DataFrameColumn column = dataFrame.Columns[columnNumber];

                // Determine the SQL data type for this column.
                // Default behavior: map .NET types to SQL types (strings -> DotNetChar/varchar).
                //
                SqlDataType dataType = DataTypeMap[column.DataType];

                // For string columns, check for user-specified type override
                //
                if (column.DataType == typeof(string) && outputColumnDataTypes != null)
                {
                    if (outputColumnDataTypes.TryGetValue(column.Name, out var userType))
                    {
                        if (userType != SqlDataType.DotNetChar && userType != SqlDataType.DotNetWChar)
                        {
                            throw new ArgumentException(
                                $"Invalid type override '{userType}' for string column '{column.Name}'. " +
                                $"Only DotNetChar and DotNetWChar are supported for string columns.");
                        }

                        dataType = userType;
                        Logging.Trace($"ExtractColumns: Column '{column.Name}' using user-specified type: {dataType}");
                    }
                }

                ulong columnSize = (ulong)DataTypeSize[dataType];

                // Add column metadata to a CSharpColumn dictionary
                //
                _columns[columnNumber] = new CSharpColumn
                {
                    Name = column.Name,
                    DataType = dataType,
                    Nullable = (short)(column.NullCount > 0 ? 1 : 0),
                    DecimalDigits = 0,
                    Size = columnSize,
                    Id = columnNumber
                };

                // Extract actual column data from a DataFrameColumn and store into a void pointers array
                //
                ExtractColumn(columnNumber, column);
            }
        }

        /// <summary>
        /// This method retrieves the DataFrame data from output DataSet by assigning
        /// pointers to the data pointers array and strLenOrNullMap pointers array
        /// </summary>
        public unsafe void RetrieveColumns(
            void ***data,
            int  ***strLenOrNullMap
        )
        {
            Logging.Trace("CSharpOutputDataSet::RetrieveColumns");
            
            // Pin the pointer arrays and get their addresses
            //
            GCHandle dataHandle = GCHandle.Alloc(_dataPtrs, GCHandleType.Pinned);
            *data = (void**)dataHandle.AddrOfPinnedObject();
            _handleList.Add(dataHandle);

            GCHandle strLenHandle = GCHandle.Alloc(_strLenOrNullMapPtrs, GCHandleType.Pinned);
            *strLenOrNullMap = (int**)strLenHandle.AddrOfPinnedObject();
            _handleList.Add(strLenHandle);
        }

        /// <summary>
        /// This method cleans up all the handles for garbage collection.
        /// </summary>
        public void HandleCleanup()
        {
            Logging.Trace("CSharpOutputDataSet::HandleCleanup");
            if(_handleList != null)
            {
                foreach(GCHandle handle in _handleList)
                {
                    handle.Free();
                }
            }

            _handleList = null;
        }

        /// <summary>
        /// This method retrieves each column DataFrameColumn value from output DataSet
        /// by getting an array of the DataFrameColumn values, assigning a pointer to that
        /// array, and appending the pointer to the data pointers array. Same process for
        /// retrieving the StrLenNullMap.
        /// </summary>
        private unsafe void ExtractColumn(
            ushort          columnNumber,
            DataFrameColumn column
        )
        {
            Logging.Trace("CSharpOutputDataSet::ExtractColumn");
            int[] colMap = GetStrLenNullMap(columnNumber, column);
            GCHandle colMapHandle = GCHandle.Alloc(colMap, GCHandleType.Pinned);
            _strLenOrNullMapPtrs[columnNumber] = colMapHandle.AddrOfPinnedObject();
            _handleList.Add(colMapHandle);

            // Process column based on its data type
            //
            switch(_columns[columnNumber].DataType)
            {
                case SqlDataType.DotNetInteger:
                    SetDataPtrs<int>(columnNumber, GetArray<int>(column));
                    break;
                case SqlDataType.DotNetUInteger:
                    SetDataPtrs<uint>(columnNumber, GetArray<uint>(column));
                    break;
                case SqlDataType.DotNetBigInt:
                    SetDataPtrs<long>(columnNumber, GetArray<long>(column));
                    break;
                case SqlDataType.DotNetUBigInt:
                    SetDataPtrs<ulong>(columnNumber, GetArray<ulong>(column));
                    break;
                case SqlDataType.DotNetSmallInt:
                    SetDataPtrs<short>(columnNumber, GetArray<short>(column));
                    break;
                case SqlDataType.DotNetUSmallInt:
                    SetDataPtrs<ushort>(columnNumber, GetArray<ushort>(column));
                    break;
                case SqlDataType.DotNetTinyInt:
                    SetDataPtrs<sbyte>(columnNumber, GetArray<sbyte>(column));
                    break;
                case SqlDataType.DotNetUTinyInt:
                    SetDataPtrs<byte>(columnNumber, GetArray<byte>(column));
                    break;
                case SqlDataType.DotNetBit:
                    SetDataPtrs<bool>(columnNumber, GetArray<bool>(column));
                    break;
                case SqlDataType.DotNetReal:
                    SetDataPtrs<float>(columnNumber, GetArray<float>(column));
                    break;
                case SqlDataType.DotNetFloat:
                    SetDataPtrs<double>(columnNumber, GetArray<double>(column));
                    break;
                case SqlDataType.DotNetDouble:
                    SetDataPtrs<double>(columnNumber, GetArray<double>(column));
                    break;
                case SqlDataType.DotNetNumeric:
                    ExtractNumericColumn(columnNumber, column);
                    break;
                case SqlDataType.DotNetChar:
                    // Calculate column size from actual data.
                    // columnSize = max UTF-8 byte length across all rows.
                    // Minimum size is 1 byte (char(0) is illegal in SQL).
                    //
                    int maxStrLen = colMap.Length > 0 ? colMap.Where(x => x > 0).DefaultIfEmpty(0).Max() : 0;
                    _columns[columnNumber].Size = (ulong)Math.Max(maxStrLen, MinUtf8CharSize);

                    SetDataPtrs<byte>(columnNumber, GetStringArray(column));
                    break;
                case SqlDataType.DotNetWChar:
                    // Calculate column size from actual data.
                    // For WCHAR types, column size should be the max byte length.
                    // Minimum size is 2 bytes (1 UTF-16 character).
                    //
                    int maxUnicodeByteLen = colMap.Length > 0 ? colMap.Where(x => x > 0).DefaultIfEmpty(0).Max() : 0;
                    _columns[columnNumber].Size = (ulong)Math.Max(maxUnicodeByteLen, MinUtf16CharSize);

                    SetDataPtrs<char>(columnNumber, GetUnicodeStringArray(column));
                    break;
                default:
                    throw new NotImplementedException("Parameter type for " + DataTypeMap[column.DataType] + " has not been implemented yet");
            }
        }

        /// <summary>
        /// This method sets data pointer for the column and append the array to the handle list.
        /// </summary>
        private unsafe void SetDataPtrs<T>(
            ushort  columnNumber,
            T[]     array
        ) where T : unmanaged
        {
            GCHandle handle = GCHandle.Alloc(array, GCHandleType.Pinned);
            _dataPtrs[columnNumber] = handle.AddrOfPinnedObject();
            _handleList.Add(handle);
        }

        /// <summary>
        /// Extracts NUMERIC/DECIMAL column data by converting SqlDecimal values to ODBC-compatible SQL_NUMERIC_STRUCT array.
        /// </summary>
        /// <param name="columnNumber">The column index.</param>
        /// <param name="column">The DataFrameColumn containing SqlDecimal values.</param>
        private unsafe void ExtractNumericColumn(
            ushort          columnNumber,
            DataFrameColumn column)
        {
            if (column == null)
            {
                SetDataPtrs<SqlNumericStruct>(columnNumber, Array.Empty<SqlNumericStruct>());
                return;
            }

            // Determine target precision/scale from max values across all rows.
            // Track integer digits and scale independently so that after scale normalization,
            // precision = maxIntDigits + 'target scale' covers all values without overflow.
            // (e.g., 123 (p=3,s=0) and 0.99 (p=2,s=2) → intDigits=3, scale=2 → precision=5)
            //
            byte maxIntDigits = 0;
            byte scale = (byte)_columns[columnNumber].DecimalDigits;
            
            for (int rowNumber = 0; rowNumber < column.Length; ++rowNumber)
            {
                if (column[rowNumber] != null)
                {
                    SqlDecimal value = (SqlDecimal)column[rowNumber];
                    if (!value.IsNull)
                    {
                        byte intDigits = (byte)(value.Precision - value.Scale);
                        maxIntDigits = Math.Max(maxIntDigits, intDigits);
                        scale = Math.Max(scale, value.Scale);
                    }
                }
            }
            
            // Derive precision from max integer digits + target scale.
            // When the sum exceeds SQL_MAX_PRECISION (38), we must reduce scale to preserve
            // integer digits. This matches SQL Server's own behavior for DECIMAL arithmetic:
            // when computed precision exceeds 38, SQL Server clamps to 38 and reduces scale
            // to preserve integer digits, since losing integer digits would corrupt the value.
            // See: https://learn.microsoft.com/en-us/sql/t-sql/data-types/precision-scale-and-length-transact-sql
            //
            // This scenario occurs when a user's C# executor returns SqlDecimal values with
            // widely different scales in the same column (e.g., an 18-digit integer alongside
            // a value with scale=30). The computed precision (18+30=48) exceeds 38, so we
            // clamp precision to 38 and reduce scale to (38-18=20) to fit all integer digits.
            //
            byte precision = (byte)(maxIntDigits + scale);
            precision = Math.Max(precision, SqlNumericHelper.SQL_MIN_PRECISION);
            if (precision > SqlNumericHelper.SQL_MAX_PRECISION)
            {
                precision = SqlNumericHelper.SQL_MAX_PRECISION;
                scale = (byte)(SqlNumericHelper.SQL_MAX_PRECISION - maxIntDigits);
            }

            // Update metadata: Size = precision (total digits), DecimalDigits = scale (fractional digits)
            //
            _columns[columnNumber].Size = precision;
            _columns[columnNumber].DecimalDigits = scale;

            Logging.Trace($"ExtractNumericColumn: Column {columnNumber}, T-SQL type=DECIMAL({precision},{scale}), RowCount={column.Length}");

            // Convert all values (including NULLs) to SQL_NUMERIC_STRUCT using FromSqlDecimal
            //
            SqlNumericStruct[] numericArray = new SqlNumericStruct[column.Length];
            for (int rowNumber = 0; rowNumber < column.Length; ++rowNumber)
            {
                SqlDecimal value = (column[rowNumber] != null) ? (SqlDecimal)column[rowNumber] : SqlDecimal.Null;
                numericArray[rowNumber] = FromSqlDecimal(value, precision, scale);
                Logging.Trace($"ExtractNumericColumn: Row {rowNumber}, Value={value}");
            }

            SetDataPtrs<SqlNumericStruct>(columnNumber, numericArray);
        }

        /// <summary>
        /// This method gets the array from a DataFrameColumn Column for numeric types.
        /// </summary>
        private T[] GetArray<T>(DataFrameColumn column) where T : unmanaged
        {
            if (column == null)
            {
                return Array.Empty<T>();
            }

            T[] columnArray = new T[column.Length];
            for(int rowNumber = 0; rowNumber < column.Length; ++rowNumber)
            {
                if(column[rowNumber] != null)
                {
                    columnArray[rowNumber] = (T)column[rowNumber];
                }
                else if(typeof(T) == typeof(double))
                {
                    columnArray[rowNumber] = (T)Convert.ChangeType(Double.NaN, typeof(T));
                }
                else if(typeof(T) == typeof(float))
                {
                    columnArray[rowNumber] = (T)Convert.ChangeType(Single.NaN, typeof(T));
                }
            }

            return columnArray;
        }

        /// <summary>
        /// This method gets the array from a DataFrameColumn Column for string types by
        /// building a long string from the column and returning the underlying bytes as an array.
        /// </summary>
        /// <param name="column">The DataFrameColumn containing string data.</param>
        /// <returns>A byte array containing all non-null string values as UTF-8 encoded bytes.</returns>
        private byte[] GetStringArray(DataFrameColumn column)
        {
            if (column == null)
            {
                return Array.Empty<byte>();
            }

            StringBuilder builder = new StringBuilder();
            int totalBytes = 0;
            for(int rowNumber = 0; rowNumber < column.Length; ++rowNumber)
            {
                // In case of null strings, nothing will be added to the returned data.
                //
                if(column[rowNumber] != null)
                {
                    string value = (string)column[rowNumber];
                    int byteLen = Encoding.UTF8.GetByteCount(value);
                    Logging.Trace($"GetStringArray: Row {rowNumber}, Value='{value}', ByteLen={byteLen}, CurrentOffset={totalBytes}");
                    builder.Append(value);
                    totalBytes += byteLen;
                }
                else
                {
                    Logging.Trace($"GetStringArray: Row {rowNumber} is NULL");
                }
            }

            byte[] result = Encoding.UTF8.GetBytes(builder.ToString());
            Logging.Trace($"GetStringArray: Total buffer size={result.Length}, TotalBytesCalculated={totalBytes}");
            return result;
        }

        /// <summary>
        /// This method builds a contiguous UTF-16 buffer for string types (nvarchar/nchar).
        /// </summary>
        /// <param name="column">The DataFrameColumn containing string data.</param>
        /// <returns>A char array containing all non-null string values concatenated.</returns>
        private char[] GetUnicodeStringArray(DataFrameColumn column)
        {
            if (column == null)
            {
                return Array.Empty<char>();
            }

            StringBuilder builder = new StringBuilder();
            int totalBytes = 0;
            for(int rowNumber = 0; rowNumber < column.Length; ++rowNumber)
            {
                if(column[rowNumber] != null)
                {
                    string value = (string)column[rowNumber];
                    int byteLen = Encoding.Unicode.GetByteCount(value);
                    Logging.Trace($"GetUnicodeStringArray: Row {rowNumber}, Value='{value}', ByteLen={byteLen}, CurrentOffset={totalBytes}");
                    builder.Append(value);
                    totalBytes += byteLen;
                }
                else
                {
                    Logging.Trace($"GetUnicodeStringArray: Row {rowNumber} is NULL");
                }
            }

            char[] result = builder.ToString().ToCharArray();
            Logging.Trace($"GetUnicodeStringArray: Total buffer size={result.Length * sizeof(char)}, TotalBytesCalculated={totalBytes}");
            return result;
        }

        /// <summary>
        /// This method gets the StrLenNullMap from a DataFrameColumn Column.
        /// </summary>
        /// <param name="columnNumber">The column index, used to look up the correct data type from _columns</param>
        /// <param name="column">The DataFrameColumn containing the data</param>
        /// <returns>Array of string lengths or null indicators for each row</returns>
        private int[] GetStrLenNullMap(ushort columnNumber, DataFrameColumn column)
        {
            if (column == null)
            {
                return Array.Empty<int>();
            }

            int[] colMap = new int[column.Length];
            
            SqlDataType dataType = _columns[columnNumber].DataType;
            Logging.Trace($"GetStrLenNullMap: Column {columnNumber}, DataType={dataType}, RowCount={column.Length}");
            
            for(int rowNumber = 0; rowNumber < column.Length; ++rowNumber)
            {
                if(column[rowNumber] != null)
                {
                    switch(dataType)
                    {
                        case SqlDataType.DotNetChar:
                            // Must match the actual byte count from Encoding.UTF8.GetBytes()
                            //
                            colMap[rowNumber] = Encoding.UTF8.GetByteCount((string)column[rowNumber]);
                            Logging.Trace($"GetStrLenNullMap: Row {rowNumber}, Value='{column[rowNumber]}', ByteLen={colMap[rowNumber]}");
                            break;
                        case SqlDataType.DotNetWChar:
                            // Report the byte length of the UTF-16 encoded string (2 bytes per code unit).
                            // This must match the buffer size emitted by GetUnicodeStringArray().
                            //
                            colMap[rowNumber] = Encoding.Unicode.GetByteCount((string)column[rowNumber]);
                            Logging.Trace($"GetStrLenNullMap: Row {rowNumber}, Value='{column[rowNumber]}', ByteLen={colMap[rowNumber]}");
                            break;
                        default:
                            if(!DataTypeSize.TryGetValue(dataType, out short size))
                            {
                                throw new NotImplementedException("Parameter type for " + dataType + " has not been implemented yet");
                            }
                            colMap[rowNumber] = size;
                            break;
                    }
                }
                else
                {
                    colMap[rowNumber] = SQL_NULL_DATA;
                    Logging.Trace($"GetStrLenNullMap: Row {rowNumber} is NULL");
                }
            }

            return colMap;
        }
    }
}
