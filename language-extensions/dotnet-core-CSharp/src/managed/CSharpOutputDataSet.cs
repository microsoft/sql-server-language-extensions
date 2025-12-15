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
        /// An array of int pointers containing strLenOrNullMap for each column.
        /// </summary>
        private unsafe int*[] _strLenOrNullMapPtrs;

        /// <summary>
        /// An array of void pointers containing data for each column.
        /// </summary>
        private unsafe void*[] _dataPtrs;

        /// <summary>
        /// This method extracts metadata and actual data for each column supplied
        /// by extracting data and information from every DataFrameColumn.
        /// </summary>
        /// <param name="CSharpDataFrame">The DataFrame containing the output data.</param>
        /// <param name="inputColumns">
        /// Optional dictionary of input column metadata (not used - C# extension always outputs ANSI strings
        /// like Python/R extensions).
        /// </param>
        public unsafe void ExtractColumns(DataFrame CSharpDataFrame, Dictionary<ushort, CSharpColumn> inputColumns = null)
        {
            Logging.Trace("CSharpOutputDataSet::ExtractColumns");
            _strLenOrNullMapPtrs = new int*[ColumnsNumber];
            _dataPtrs = new void*[ColumnsNumber];
            for(ushort columnNumber = 0; columnNumber < ColumnsNumber; ++columnNumber)
            {
                DataFrameColumn column = CSharpDataFrame.Columns[columnNumber];

                // Determine the SQL data type for this column
                // Prefer the incoming input column metadata when available so nvarchar metadata is preserved.
                // Default to ANSI mapping to keep existing behavior when no metadata is supplied.
                //
                SqlDataType dataType = DataTypeMap[column.DataType];
                ulong columnSize = (ulong)DataTypeSize[dataType];
                if(inputColumns != null && inputColumns.ContainsKey(columnNumber))
                {
                    dataType = inputColumns[columnNumber].DataType;
                    columnSize = inputColumns[columnNumber].Size;
                }

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
            _strLenOrNullMapPtrs[columnNumber] = (int*)colMapHandle.AddrOfPinnedObject();
            _handleList.Add(colMapHandle);

            // Use the data type already determined in ExtractColumns (which preserves nvarchar/varchar distinction)
            // instead of DataTypeMap[column.DataType] which always maps string to DotNetChar
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
                case SqlDataType.DotNetChar:
                    // Modify the size of the string column to be the max size of bytes.
                    //
                    int maxStrLen = colMap.Max();
                    if(maxStrLen > 0)
                    {
                        _columns[columnNumber].Size = (ulong)maxStrLen;
                    }

                    SetDataPtrs<byte>(columnNumber, GetStringArray(column));
                    break;
                case SqlDataType.DotNetWChar:
                    // Preserve nvarchar metadata by emitting UTF-16 data and byte counts.
                    //
                    int maxUnicodeStrLen = colMap.Max();
                    if(maxUnicodeStrLen > 0)
                    {
                        _columns[columnNumber].Size = (ulong)maxUnicodeStrLen;
                    }

                    SetDataPtrs<char>(columnNumber, GetUnicodeStringArray(column));
                    break;
                default:
                    throw new NotImplementedException("Parameter type for " + DataTypeMap[column.DataType].ToString() + " has not been implemented yet");
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
            _dataPtrs[columnNumber] = (void*)handle.AddrOfPinnedObject();
            _handleList.Add(handle);
        }

        /// <summary>
        /// This method gets the array from a DataFrameColumn Column for numeric types.
        /// </summary>
        private T[] GetArray<T>(DataFrameColumn column) where T : unmanaged
        {
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
        private byte[] GetStringArray(DataFrameColumn column)
        {
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
        private char[] GetUnicodeStringArray(DataFrameColumn column)
        {
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
                            // For nvarchar output, report UTF-16 byte length to match the emitted buffer.
                            //
                            colMap[rowNumber] = Encoding.Unicode.GetByteCount((string)column[rowNumber]);
                            Logging.Trace($"GetStrLenNullMap: Row {rowNumber}, Value='{column[rowNumber]}', ByteLen={colMap[rowNumber]}");
                            break;
                        default:
                            if(!DataTypeSize.ContainsKey(dataType))
                            {
                                throw new NotImplementedException("Parameter type for " + dataType.ToString() + " has not been implemented yet");
                            }
                            colMap[rowNumber] = DataTypeSize[dataType];
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
