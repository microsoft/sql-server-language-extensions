//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: SqlNumericHelper.cs
//
// Purpose:
//  SQL NUMERIC/DECIMAL type support: ODBC-compatible struct definition
//  and bidirectional conversion between SQL_NUMERIC_STRUCT and SqlDecimal.
//
//*********************************************************************
using System;
using System.Data.SqlTypes;
using System.Linq;
using System.Runtime.InteropServices;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// Helper class for converting between SQL Server NUMERIC/DECIMAL types and SqlDecimal.
    /// Provides ODBC-compatible SQL_NUMERIC_STRUCT definition and conversion methods.
    ///
    /// IMPORTANT: This implementation uses SqlDecimal from Microsoft.Data.SqlClient which supports
    /// full SQL Server precision (38 digits). C# decimal is NOT used to avoid 28-digit limitations.
    /// </summary>
    public static class SqlNumericHelper
    {

        /// <summary>
        /// SQL_NUMERIC_STRUCT structure matching ODBC's SQL_NUMERIC_STRUCT.
        /// Used for transferring NUMERIC/DECIMAL data between SQL Server and C#.
        /// IMPORTANT: This struct must be binary-compatible with ODBC's SQL_NUMERIC_STRUCT
        /// defined in sql.h/sqltypes.h on the native side.
        /// 
        /// Why individual byte fields instead of byte[] array?
        /// - Using byte[] would make this a managed type (reference type), violating the unmanaged constraint
        /// - Fixed buffers (fixed byte val[16]) require unsafe code, which we want to avoid for safety.
        /// - Individual fields keep this as a pure value type (unmanaged) with memory safety.
        /// - The compiler will optimize access patterns, so there's no performance penalty.
        /// </summary>
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct SqlNumericStruct
        {
            /// <summary>
            /// Total number of digits (e.g., 1-38) - SQLCHAR (unsigned byte)
            /// </summary>
            public byte precision;

            /// <summary>
            /// Number of digits after decimal point - SQLSCHAR (signed byte)
            /// 
            /// ODBC specification defines scale as SQLSCHAR (signed char) in SQL_NUMERIC_STRUCT.
            /// We must use sbyte for exact binary layout compatibility with native ODBC code.
            /// Mismatch would cause struct layout corruption when marshaling to/from native code.
            /// </summary>
            public sbyte scale;

            /// <summary>
            /// Sign indicator: 1 = positive, 0 = negative - SQLCHAR (unsigned byte)
            /// </summary>
            public byte sign;

            /// <summary>
            /// Little-endian byte array (16 bytes) representing the scaled integer value.
            /// The actual numeric value = (val as integer) * 10^(-scale), adjusted for sign.
            /// Corresponds to SQLCHAR val[SQL_MAX_NUMERIC_LEN] where SQL_MAX_NUMERIC_LEN = 16.
            /// 
            /// Why 16 separate fields instead of an array?
            /// - See struct-level comment: arrays would make this managed, violating unmanaged constraint.
            /// - This verbose approach maintains binary compatibility without requiring unsafe code or /unsafe compiler flag.
            /// </summary>
            public byte val0;
            public byte val1;
            public byte val2;
            public byte val3;
            public byte val4;
            public byte val5;
            public byte val6;
            public byte val7;
            public byte val8;
            public byte val9;
            public byte val10;
            public byte val11;
            public byte val12;
            public byte val13;
            public byte val14;
            public byte val15;

            /// <summary>
            /// Helper method to get val byte at specified index (0-15).
            /// 
            /// We use switch expression instead of array indexing:
            /// - Since we can't use arrays (would make struct managed), we need field access.
            /// - Switch expressions are optimized by the compiler to efficient jump tables.
            /// - Modern Just-In-Time compiler will inline this for zero overhead compared to array access.
            /// </summary>
            public byte GetVal(int index)
            {
                return index switch
                {
                    0 => val0,
                    1 => val1,
                    2 => val2,
                    3 => val3,
                    4 => val4,
                    5 => val5,
                    6 => val6,
                    7 => val7,
                    8 => val8,
                    9 => val9,
                    10 => val10,
                    11 => val11,
                    12 => val12,
                    13 => val13,
                    14 => val14,
                    15 => val15,
                    _ => throw new ArgumentOutOfRangeException(nameof(index), "Index must be 0-15")
                };
            }

            /// <summary>
            /// Helper method to set val byte at specified index (0-15).
            /// 
            /// We use switch statement instead of array indexing:
            /// - Same reason as GetVal: can't use arrays without making struct managed.
            /// - Switch statement compiles to efficient code without runtime overhead.
            /// </summary>
            public void SetVal(int index, byte value)
            {
                switch (index)
                {
                    case 0: val0 = value; break;
                    case 1: val1 = value; break;
                    case 2: val2 = value; break;
                    case 3: val3 = value; break;
                    case 4: val4 = value; break;
                    case 5: val5 = value; break;
                    case 6: val6 = value; break;
                    case 7: val7 = value; break;
                    case 8: val8 = value; break;
                    case 9: val9 = value; break;
                    case 10: val10 = value; break;
                    case 11: val11 = value; break;
                    case 12: val12 = value; break;
                    case 13: val13 = value; break;
                    case 14: val14 = value; break;
                    case 15: val15 = value; break;
                    default: throw new ArgumentOutOfRangeException(nameof(index), "Index must be 0-15");
                }
            }
        }

        /// <summary>
        /// Converts SQL_NUMERIC_STRUCT to SqlDecimal with full 38-digit precision support.
        /// This method supports the complete SQL Server DECIMAL/NUMERIC range without data loss.
        /// </summary>
        /// <param name="numeric">The SQL numeric structure from ODBC.</param>
        /// <returns>The equivalent SqlDecimal value.</returns>
        /// <remarks>
        /// SqlDecimal provides full SQL Server precision (38 digits) compared to C# decimal (28-29 digits).
        /// Use this method when working with high-precision values to avoid data loss.
        /// </remarks>
        public static SqlDecimal ToSqlDecimal(SqlNumericStruct numeric)
        {
            // Validate precision and scale before creating SqlDecimal
            if (numeric.precision < 1 || numeric.precision > 38)
            {
                throw new ArgumentException($"Precision must be between 1 and 38, got {numeric.precision}");
            }
            if (numeric.scale < 0 || numeric.scale > numeric.precision)
            {
                throw new ArgumentException($"Scale ({numeric.scale}) must be between 0 and precision ({numeric.precision})");
            }
            
            // SqlDecimal constructor requires int[] array (not byte[])
            // The val array in SqlNumericStruct is 16 bytes = 128 bits
            // We need to convert to 4 int32s (4 x 32 bits = 128 bits)
            
            int[] data = new int[4];
            for (int i = 0; i < 4; i++)
            {
                // Convert each group of 4 bytes to an int32 (little-endian)
                int offset = i * 4;
                data[i] = numeric.GetVal(offset) |
                         (numeric.GetVal(offset + 1) << 8) |
                         (numeric.GetVal(offset + 2) << 16) |
                         (numeric.GetVal(offset + 3) << 24);
            }
            
            // SqlDecimal constructor: SqlDecimal(byte precision, byte scale, bool positive, int[] data)
            bool isPositive = numeric.sign == 1;
            
            // Note: SqlDecimal scale parameter is byte (unsigned), but SqlNumericStruct.scale is sbyte (signed)
            // SQL Server scale is always non-negative (0-38), so this cast is safe
            byte scale = (byte)Math.Max((sbyte)0, numeric.scale);
            
            return new SqlDecimal(numeric.precision, scale, isPositive, data);
        }

        /// <summary>
        /// Converts SqlDecimal to SQL_NUMERIC_STRUCT for transfer to SQL Server.
        /// This method handles the full 38-digit precision range without data loss.
        /// </summary>
        /// <param name="value">The SqlDecimal value to convert.</param>
        /// <param name="precision">Total number of digits (1-38). If null, uses SqlDecimal's precision.</param>
        /// <param name="scale">Number of digits after decimal point (0-precision). If null, uses SqlDecimal's scale.</param>
        /// <returns>The equivalent SQL numeric structure for ODBC.</returns>
        /// <exception cref="ArgumentException">Thrown when precision or scale are out of valid range.</exception>
        public static SqlNumericStruct FromSqlDecimal(SqlDecimal value, byte? precision = null, byte? scale = null)
        {
            // Handle SqlDecimal.Null
            if (value.IsNull)
            {
                // Return a zero-initialized struct - caller should set null indicator separately
                return new SqlNumericStruct
                {
                    precision = precision ?? 1,
                    scale = (sbyte)(scale ?? 0),
                    sign = 1
                };
            }
            
            // Use SqlDecimal's own precision/scale if not specified
            byte targetPrecision = precision ?? value.Precision;
            byte targetScale = scale ?? value.Scale;
            
            if (targetPrecision < 1 || targetPrecision > 38)
            {
                throw new ArgumentException($"Precision must be between 1 and 38, got {targetPrecision}");
            }
            if (targetScale > targetPrecision)
            {
                throw new ArgumentException($"Scale ({targetScale}) cannot exceed precision ({targetPrecision})");
            }
            
            // Adjust scale if needed (SqlDecimal has AdjustScale method)
            SqlDecimal adjustedValue = value;
            if (targetScale != value.Scale)
            {
                // AdjustScale returns a new SqlDecimal with the specified scale
                // positive scaleShift adds decimal places, negative removes them
                int scaleShift = targetScale - value.Scale;
                adjustedValue = SqlDecimal.AdjustScale(value, scaleShift, false);
            }
            
            SqlNumericStruct result = new SqlNumericStruct
            {
                precision = targetPrecision,
                scale = (sbyte)targetScale,
                sign = (byte)(adjustedValue.IsPositive ? 1 : 0)
            };
            
            // SqlDecimal stores data as int[4] array (128 bits total)
            // We need to convert to byte[16] for SqlNumericStruct
            int[] data = adjustedValue.Data;
            
            for (int i = 0; i < 4 && i < data.Length; i++)
            {
                // Convert each int32 to 4 bytes (little-endian)
                int offset = i * 4;
                int value32 = data[i];
                result.SetVal(offset, (byte)(value32 & 0xFF));
                result.SetVal(offset + 1, (byte)((value32 >> 8) & 0xFF));
                result.SetVal(offset + 2, (byte)((value32 >> 16) & 0xFF));
                result.SetVal(offset + 3, (byte)((value32 >> 24) & 0xFF));
            }
            
            return result;
        }
    }
}
