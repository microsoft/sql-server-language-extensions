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
            for(ushort i = 0; i < ColumnsNumber; ++i)
            {
                AddColumn(i, rowsNumber, data[i], strLenOrNullMap[i]);
            }
        }

        /// <summary>
        /// This method adds column data to DataFrameColumn for each data type.
        /// </summary>
        private unsafe void AddColumn(
            ushort            columnNumber,
            ulong             rowsNumber,
            void              *colData,
            int               *colMap)
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
            ushort            columnNumber,
            ulong             rowsNumber,
            void              *colData,
            int               *colMap
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
        /// This method adds each column from the dataframe supplied to the _columns.
        /// </summary>
        public unsafe void AddColumnsMetadata(DataFrame CSharpDataFrame)
        {
            Logging.Trace("CSharpOutputDataSet::AddColumnsMetadata");
            for(ushort columnNumber = 0; columnNumber < CSharpDataFrame.Columns.Count; ++columnNumber)
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
    }
}
