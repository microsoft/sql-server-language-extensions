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
using static Microsoft.SqlServer.CSharpExtension.SqlNumericHelper;

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
                case SqlDataType.DotNetNumeric:
                    AddNumericDataFrameColumn(columnNumber, rowsNumber, colData, colMap);
                    break;
                case SqlDataType.DotNetChar:
                    int[] strLens = new int[rowsNumber];
                    Interop.Copy((int*)colMap, strLens, 0, (int)rowsNumber);

                    // Calculate total buffer size from the sum of all positive (non-null) byte lengths.
                    // This enables bounds checking in UTF8ByteSplitToArray to guard against corrupted length data.
                    //
                    int charTotalBufferSize = DataSetUtils.CalculateTotalBufferSize(strLens);

                    // SQL Server sends UTF-8 encoded strings with byte lengths in strLenOrNullMap.
                    // We decode each segment directly from the byte buffer to properly handle
                    // multi-byte UTF-8 characters where byte count != character count.
                    //
                    CSharpDataFrame.Columns.Add(new StringDataFrameColumn(_columns[columnNumber].Name, DataSetUtils.UTF8ByteSplitToArray((byte*)colData, strLens, charTotalBufferSize)));
                    break;
                case SqlDataType.DotNetWChar:
                    int[] wcharByteLens = new int[rowsNumber];
                    Interop.Copy((int*)colMap, wcharByteLens, 0, (int)rowsNumber);

                    // Calculate total buffer size from the sum of all positive (non-null) byte lengths.
                    // This enables bounds checking in UTF16ByteSplitToArray to guard against corrupted length data.
                    //
                    int wcharTotalBufferSize = DataSetUtils.CalculateTotalBufferSize(wcharByteLens);

                    // SQL Server sends UTF-16 (nvarchar/nchar) encoded strings with byte lengths in strLenOrNullMap.
                    // We decode each segment directly from the byte buffer to properly handle
                    // multi-string buffers where strings are concatenated without null terminators.
                    //
                    CSharpDataFrame.Columns.Add(new StringDataFrameColumn(_columns[columnNumber].Name, DataSetUtils.UTF16ByteSplitToArray((byte*)colData, wcharByteLens, wcharTotalBufferSize)));
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

        /// <summary>
        /// This method adds NUMERIC/DECIMAL column data by converting from SQL_NUMERIC_STRUCT
        /// to C# decimal values, creating a PrimitiveDataFrameColumn<decimal>, and adding it to the DataFrame.
        /// Follows the same pattern as Java extension's numeric handling.
        /// </summary>
        /// <param name="columnNumber">The column index.</param>
        /// <param name="rowsNumber">Number of rows in this column.</param>
        /// <param name="colData">Pointer to array of SQL_NUMERIC_STRUCT structures (19 bytes each).</param>
        /// <param name="colMap">Pointer to null indicator array (SQL_NULL_DATA for null values).</param>
        private unsafe void AddNumericDataFrameColumn(
            ushort columnNumber,
            ulong  rowsNumber,
            void   *colData,
            int    *colMap)
        {
            // Cast the raw pointer to SQL_NUMERIC_STRUCT array
            SqlNumericStruct* numericArray = (SqlNumericStruct*)colData;
            
            // Create a DataFrame column for decimal values
            PrimitiveDataFrameColumn<decimal> colDataFrame = 
                new PrimitiveDataFrameColumn<decimal>(_columns[columnNumber].Name, (int)rowsNumber);
            
            // Convert each SQL_NUMERIC_STRUCT to decimal, handling nulls
            Span<int> nullSpan = new Span<int>(colMap, (int)rowsNumber);
            for (int i = 0; i < (int)rowsNumber; ++i)
            {
                // Check if this row has a null value
                // 
                // WHY check both Nullable == 0 and SQL_NULL_DATA?
                // - Nullable == 0 means column is declared NOT NULL (cannot contain nulls)
                // - For NOT NULL columns, skip null checking for performance (nullSpan[i] is undefined)
                // - For nullable columns (Nullable != 0), check if nullSpan[i] == SQL_NULL_DATA (-1)
                // - This matches the pattern used by other numeric types in the codebase
                if (_columns[columnNumber].Nullable == 0 || nullSpan[i] != SQL_NULL_DATA)
                {
                    // Convert SQL_NUMERIC_STRUCT to C# decimal
                    // The conversion handles precision, scale, sign, and the 16-byte integer value
                    colDataFrame[i] = ToDecimal(numericArray[i]);
                }
                // If null, the PrimitiveDataFrameColumn<decimal> slot remains as null
            }

            CSharpDataFrame.Columns.Add(colDataFrame);
        }
    }
}
