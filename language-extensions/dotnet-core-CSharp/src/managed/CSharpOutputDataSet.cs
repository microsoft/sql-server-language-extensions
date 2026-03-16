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
        public unsafe void ExtractColumns(DataFrame dataFrame)
        {
            Logging.Trace("CSharpOutputDataSet::ExtractColumns");
            _strLenOrNullMapPtrs = new IntPtr[ColumnsNumber];
            _dataPtrs = new IntPtr[ColumnsNumber];
            for(ushort columnNumber = 0; columnNumber < ColumnsNumber; ++columnNumber)
            {
                DataFrameColumn column = dataFrame.Columns[columnNumber];

                // Determine the SQL data type for this column.
                // All .NET strings are output as DotNetChar (varchar/UTF-8).
                //
                SqlDataType dataType = DataTypeMap[column.DataType];
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
                    // columnSize = max character count (UTF-16 byte length / 2).
                    // Minimum size is 1 character (nchar(0) is illegal in SQL).
                    //
                    int maxUnicodeByteLen = colMap.Length > 0 ? colMap.Where(x => x > 0).DefaultIfEmpty(0).Max() : 0;
                    int maxCharCount = maxUnicodeByteLen / sizeof(char);
                    _columns[columnNumber].Size = (ulong)Math.Max(maxCharCount, MinUtf16CharSize);

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
        /// This method extracts NUMERIC/DECIMAL column data by converting C# decimal values
        /// to SQL_NUMERIC_STRUCT array, pinning it, and storing the pointer.
        /// Follows the same pattern as Java extension's numeric handling.
        /// </summary>
        /// <param name="columnNumber">The column index.</param>
        /// <param name="column">The DataFrameColumn containing decimal values.</param>
        private unsafe void ExtractNumericColumn(
            ushort          columnNumber,
            DataFrameColumn column)
        {
            if (column == null)
            {
                SetDataPtrs<SqlNumericStruct>(columnNumber, Array.Empty<SqlNumericStruct>());
                return;
            }

            // For NUMERIC/DECIMAL, we need to determine appropriate precision and scale from the data.
            // SQL Server supports precision 1-38 and scale 0-precision.
            // We'll calculate both precision and scale by examining the actual decimal values.
            //
            // WHY calculate from data instead of hardcoding?
            // - The extension doesn't have access to the input column's original precision
            // - SQL Server validates returned precision against WITH RESULT SETS declaration
            // - Using precision=38 for all values causes "Invalid data for type numeric" errors
            // - We must calculate the minimum precision needed to represent the data
            //
            byte precision = 0;
            byte scale = (byte)_columns[columnNumber].DecimalDigits;
            
            // Calculate precision and scale by examining all non-null values
            // We need to find the maximum precision and scale to ensure no data loss
            //
            // WHY examine ALL rows instead of just sampling?
            // - A previous implementation only checked first 10 rows (optimization attempt)
            // - This caused data loss when higher-precision values appeared later in the dataset
            // - Example: rows 1-10 need precision 6, but row 100 needs precision 14
            // - If we use precision=6 for the entire column, row 100 gets truncated (data loss!)
            // - Must examine ALL rows to find maximum precision and scale
            //
            for (int rowNumber = 0; rowNumber < column.Length; ++rowNumber)
            {
                if (column[rowNumber] != null)
                {
                    decimal value = (decimal)column[rowNumber];
                    
                    // Get the scale from the decimal value itself
                    // Scale is in bits 16-23 of flags field (bits[3])
                    int[] bits = decimal.GetBits(value);
                    byte valueScale = (byte)((bits[3] >> 16) & 0x7F);
                    scale = Math.Max(scale, valueScale);
                    
                    // Calculate precision by counting significant digits
                    // Remove the scale (decimal places) to get the integer part,
                    // then count digits in both parts
                    decimal absValue = Math.Abs(value);
                    decimal integerPart = Math.Truncate(absValue);
                    
                    // Count digits in integer part (or 1 if zero)
                    byte integerDigits;
                    if (integerPart == 0)
                    {
                        integerDigits = 1;
                    }
                    else
                    {
                        // Log10 gives us the magnitude, +1 for digit count
                        integerDigits = (byte)(Math.Floor(Math.Log10((double)integerPart)) + 1);
                    }
                    
                    // Precision = digits before decimal + digits after decimal
                    byte valuePrecision = (byte)(integerDigits + valueScale);
                    precision = Math.Max(precision, valuePrecision);
                }
            }
            
            // Ensure minimum precision of 1 and maximum of 38
            precision = Math.Max(precision, (byte)1);
            precision = Math.Min(precision, (byte)38);
            
            // Ensure scale doesn't exceed precision
            if (scale > precision)
            {
                precision = scale;
            }

            // Update column metadata with calculated precision and scale
            // Size contains the precision for DECIMAL/NUMERIC types (not bytes)
            // DecimalDigits contains the scale
            _columns[columnNumber].Size = precision;
            _columns[columnNumber].DecimalDigits = scale;

            Logging.Trace($"ExtractNumericColumn: Column {columnNumber}, Precision={precision}, Scale={scale}, RowCount={column.Length}");

            // Convert each decimal value to SQL_NUMERIC_STRUCT
            SqlNumericStruct[] numericArray = new SqlNumericStruct[column.Length];
            for (int rowNumber = 0; rowNumber < column.Length; ++rowNumber)
            {
                if (column[rowNumber] != null)
                {
                    decimal value = (decimal)column[rowNumber];
                    numericArray[rowNumber] = FromDecimal(value, precision, scale);
                    Logging.Trace($"ExtractNumericColumn: Row {rowNumber}, Value={value} converted to SqlNumericStruct");
                }
                else
                {
                    // For null values, create a zero-initialized struct
                    // The null indicator in strLenOrNullMap will mark this as SQL_NULL_DATA
                    //
                    // WHY create a struct for NULL values instead of leaving uninitialized?
                    // - ODBC requires a valid struct pointer even for NULL values
                    // - The strLenOrNullMap array separately tracks which values are NULL
                    // - Native code reads from the struct pointer, so it must be valid memory
                    // - We use sign=1 (positive) by convention for NULL placeholders
                    numericArray[rowNumber] = new SqlNumericStruct
                    {
                        precision = precision,
                        scale = (sbyte)scale,
                        sign = 1  // Positive sign convention for NULL placeholders
                    };
                    Logging.Trace($"ExtractNumericColumn: Row {rowNumber} is NULL");
                }
            }

            // Pin the SqlNumericStruct array and store pointer
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
