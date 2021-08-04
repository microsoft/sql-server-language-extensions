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
using System.Linq;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using Microsoft.Data.Analysis;
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
    }

    /// <summary>
    /// This class loads and retrieves input dataset.
    /// </summary>
    public class CSharpInputDataSet: CSharpDataSet
    {
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
            if (columnName == null)
            {
                throw new ArgumentException("Invalid input column name supplied");
            }

            else if (columnNumber >= ColumnsNumber || columnNumber < 0)
            {
                throw new ArgumentException("Invalid input column id supplied: " + columnNumber.ToString());
            }

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

        /// <summary>
        /// This method adds data for each column supplied.
        /// </summary>
        public unsafe void AddColumns(
            ulong rowsNumber,
            void  **data,
            int   **strLenOrNullMap)
        {
            Logging.Trace("CSharpInputDataSet::AddColumns");
            CSharpDataFrame = new DataFrame();
            if(data != null)
            {
                for(ushort i = 0; i < ColumnsNumber; ++i)
                {
                    AddColumn(i, rowsNumber, data[i], strLenOrNullMap[i]);
                }
            }
        }

        /// <summary>
        /// This method adds column data to DataFrameColumn for each data type.
        /// </summary>
        private unsafe void AddColumn(
            ushort columnNumber,
            ulong  rowsNumber,
            void   *colData,
            int    *colMap)
        {
            Logging.Trace("CSharpInputDataSet::AddColumn");
            switch(_columns[columnNumber].DataType)
            {
                case SqlDataType.DotNetInteger:
                    AddDataFrameColumn<int>(columnNumber, rowsNumber, colData, colMap);
                    break;
                case SqlDataType.DotNetUInteger:
                    AddDataFrameColumn<uint>(columnNumber, rowsNumber, colData, colMap);
                    break;
                case SqlDataType.DotNetBigInt:
                    AddDataFrameColumn<long>(columnNumber, rowsNumber, colData, colMap);
                    break;
                case SqlDataType.DotNetUBigInt:
                    AddDataFrameColumn<ulong>(columnNumber, rowsNumber, colData, colMap);
                    break;
                case SqlDataType.DotNetTinyInt:
                    AddDataFrameColumn<sbyte>(columnNumber, rowsNumber, colData, colMap);
                    break;
                case SqlDataType.DotNetUTinyInt:
                    AddDataFrameColumn<byte>(columnNumber, rowsNumber, colData, colMap);
                    break;
                case SqlDataType.DotNetSmallInt:
                    AddDataFrameColumn<short>(columnNumber, rowsNumber, colData, colMap);
                    break;
                case SqlDataType.DotNetUSmallInt:
                    AddDataFrameColumn<ushort>(columnNumber, rowsNumber, colData, colMap);
                    break;
                case SqlDataType.DotNetBit:
                    AddDataFrameColumn<bool>(columnNumber, rowsNumber, colData, colMap);
                    break;
                case SqlDataType.DotNetFloat:
                    AddDataFrameColumn<double>(columnNumber, rowsNumber, colData, colMap);
                    break;
                case SqlDataType.DotNetDouble:
                    AddDataFrameColumn<double>(columnNumber, rowsNumber, colData, colMap);
                    break;
                case SqlDataType.DotNetReal:
                    AddDataFrameColumn<float>(columnNumber, rowsNumber, colData, colMap);
                    break;
                case SqlDataType.DotNetChar:
                    int[] strLens = new int[rowsNumber];
                    Interop.Copy((int*)colMap, strLens, 0, (int)rowsNumber);
                    CSharpDataFrame.Columns.Add(new StringDataFrameColumn(_columns[columnNumber].Name, DataSetUtils.StringSplitToArray(Interop.UTF8PtrToStr((char*)colData), strLens)));
                    break;
                default:
                    throw new NotImplementedException("Column type for " + _columns[columnNumber].DataType.ToString() + " has not been implemented yet");
            }
        }

        /// <summary>
        /// This method adds primitive column data to a PrimitiveDataFrameColumn
        /// and adds the PrimitiveDataFrameColumn to the DataFrameColumn array.
        /// </summary>
        private unsafe void AddDataFrameColumn<T>(
            ushort columnNumber,
            ulong  rowsNumber,
            void   *colData,
            int    *colMap
        ) where T : unmanaged
        {
            Span<T> colSpan = new Span<T>(colData, (int)rowsNumber);
            Span<int> nullSpan = new Span<int>(colMap, (int)rowsNumber);
            PrimitiveDataFrameColumn<T> colDataFrame = new PrimitiveDataFrameColumn<T>(_columns[columnNumber].Name, (int)rowsNumber);
            for(int i = 0; i < (int)rowsNumber; ++i)
            {
                if(_columns[columnNumber].Nullable == 0 || nullSpan[i] != SQL_NULL_DATA)
                {
                    colDataFrame[i] = colSpan[i];
                }
            }

            CSharpDataFrame.Columns.Add(colDataFrame);
        }
    }

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
        /// This method adds each column from the dataframe supplied to the _columns.
        /// </summary>
        public unsafe void AddColumnsMetadata(DataFrame CSharpDataFrame)
        {
            Logging.Trace("CSharpOutputDataSet::AddColumnsMetadata");
            for(ushort columnNumber = 0; columnNumber < ColumnsNumber; ++columnNumber)
            {
                DataFrameColumn column = CSharpDataFrame.Columns[columnNumber];
                _columns[columnNumber] = new CSharpColumn
                {
                    Name = column.Name,
                    DataType = DataTypeMap[column.DataType],
                    Nullable = (short)(column.NullCount > 0 ? 1 : 0),
                    DecimalDigits = 0,
                    Size = (ulong)DataTypeSize[DataTypeMap[column.DataType]],
                    Id = columnNumber
                };
            }
        }

        /// <summary>
        /// This method retrieves the DataFrame data from output DataSet.
        /// </summary>
        public unsafe void RetrieveColumns(
            void ***data,
            int  ***strLenOrNullMap
        )
        {
            Logging.Trace("CSharpOutputDataSet::RetrieveColumns");
            int*[] lens = new int*[ColumnsNumber];
            void*[] ptrs = new void*[ColumnsNumber];

            for(ushort columnNumber = 0; columnNumber < ColumnsNumber; ++columnNumber)
            {
                RetrieveColumn(columnNumber, ptrs, lens, CSharpDataFrame.Columns[columnNumber]);
            }

            fixed (void** ptrptr = ptrs)
            {
                _handleList.Add(GCHandle.Alloc(ptrs));
                *data = ptrptr;
            }

            fixed (int** ptrptr = lens)
            {
                _handleList.Add(GCHandle.Alloc(lens));
                *strLenOrNullMap = ptrptr;
            }
        }

        /// <summary>
        /// This method retrieves each column DataFrameColumn value from output DataSet.
        /// </summary>
        private unsafe void RetrieveColumn(
            ushort          columnNumber,
            void*[]         ptrs,
            int*[]          lens,
            DataFrameColumn column
        )
        {
            Logging.Trace("CSharpOutputDataSet::RetrieveColumn");
            fixed(int* len = GetMap(column))
            {
                lens[columnNumber] = len;
            }

            switch(DataTypeMap[column.DataType])
            {
                case SqlDataType.DotNetInteger:
                    fixed(void* arrayPtr = GetArray<int>(column))
                    {
                        ptrs[columnNumber] = arrayPtr;
                        break;
                    }
                case SqlDataType.DotNetUInteger:
                    fixed(void* arrayPtr = GetArray<uint>(column))
                    {
                        ptrs[columnNumber] = arrayPtr;
                        break;
                    }
                case SqlDataType.DotNetBigInt:
                    fixed(void* arrayPtr = GetArray<long>(column))
                    {
                        ptrs[columnNumber] = arrayPtr;
                        break;
                    }
                case SqlDataType.DotNetUBigInt:
                    fixed(void* arrayPtr = GetArray<ulong>(column))
                    {
                        ptrs[columnNumber] = arrayPtr;
                        break;
                    }
                case SqlDataType.DotNetSmallInt:
                    fixed(void* arrayPtr = GetArray<short>(column))
                    {
                        ptrs[columnNumber] = arrayPtr;
                        break;
                    }
                case SqlDataType.DotNetUSmallInt:
                    fixed(void* arrayPtr = GetArray<ushort>(column))
                    {
                        ptrs[columnNumber] = arrayPtr;
                        break;
                    }
                case SqlDataType.DotNetTinyInt:
                    fixed(void* arrayPtr = GetArray<sbyte>(column))
                    {
                        ptrs[columnNumber] = arrayPtr;
                        break;
                    }
                case SqlDataType.DotNetUTinyInt:
                    fixed(void* arrayPtr = GetArray<byte>(column))
                    {
                        ptrs[columnNumber] = arrayPtr;
                        break;
                    }
                case SqlDataType.DotNetBit:
                    fixed(void* arrayPtr = GetArray<bool>(column))
                    {
                        ptrs[columnNumber] = arrayPtr;
                        break;
                    }
                case SqlDataType.DotNetReal:
                    fixed(void* arrayPtr = GetArray<float>(column))
                    {
                        ptrs[columnNumber] = arrayPtr;
                        break;
                    }
                case SqlDataType.DotNetFloat:
                    fixed(void* arrayPtr = GetArray<double>(column))
                    {
                        ptrs[columnNumber] = arrayPtr;
                        break;
                    }
                case SqlDataType.DotNetDouble:
                    fixed(void* arrayPtr = GetArray<double>(column))
                    {
                        ptrs[columnNumber] = arrayPtr;
                        break;
                    }
            }
        }

        /// <summary>
        /// This method gets the array from a DataFrameColumn Column for numeric types.
        /// </summary>
        private T[] GetArray<T>(DataFrameColumn column) where T : unmanaged
        {
            T[] columnArray = new T[column.Length];
            _handleList.Add(GCHandle.Alloc(columnArray, GCHandleType.Pinned));
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
        /// This method gets the null map from a DataFrameColumn Column for numeric types.
        /// </summary>
        private int[] GetMap(DataFrameColumn column)
        {
            int[] colMap = new int[column.Length];
            _handleList.Add(GCHandle.Alloc(colMap, GCHandleType.Pinned));
            for(int rowNumber = 0; rowNumber < column.Length; ++rowNumber)
            {
                if(column[rowNumber] != null)
                {
                    colMap[rowNumber] = DataTypeSize[DataTypeMap[column.DataType]];
                }
                else
                {
                    colMap[rowNumber] = SQL_NULL_DATA;
                }
            }

            return colMap;
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
    }
}
