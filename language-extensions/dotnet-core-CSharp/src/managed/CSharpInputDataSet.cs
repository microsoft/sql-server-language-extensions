//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpInputDataSet.cs
//
// Purpose:
//  Class loading data to a DataFrame.
//
//*********************************************************************
using System;
using Microsoft.Data.Analysis;
using static Microsoft.SqlServer.CSharpExtension.Sql;

namespace Microsoft.SqlServer.CSharpExtension
{
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
            else
            {
                for(ushort i = 0; i < ColumnsNumber; ++i)
                {
                    AddColumn(i, rowsNumber, null, null);
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
}
