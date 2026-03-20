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
        /// SQL Server maximum precision for DECIMAL/NUMERIC types (digits).
        /// </summary>
        public const byte SQL_MAX_PRECISION = 38;

        /// <summary>
        /// Minimum precision for DECIMAL/NUMERIC types (digits).
        /// </summary>
        public const byte SQL_MIN_PRECISION = 1;

        /// <summary>
        /// Maximum scale for DECIMAL/NUMERIC types (digits after decimal point).
        /// Scale cannot exceed precision.
        /// </summary>
        public const byte SQL_MAX_SCALE = 38;

        /// <summary>
        /// Minimum scale for DECIMAL/NUMERIC types (digits after decimal point).
        /// </summary>
        public const byte SQL_MIN_SCALE = 0;

        /// <summary>
        /// Size of SQL_NUMERIC_STRUCT value array in bytes.
        /// Defined as SQL_MAX_NUMERIC_LEN in ODBC specification (sql.h/sqltypes.h).
        /// </summary>
        public const int SQL_NUMERIC_VALUE_SIZE = 16;

        /// <summary>
        /// Total size of SQL_NUMERIC_STRUCT in bytes: precision(1) + scale(1) + sign(1) + val(16) = 19.
        /// </summary>
        public const int SQL_NUMERIC_STRUCT_SIZE = 19;

        /// <summary>
        /// SQL_NUMERIC_STRUCT structure matching ODBC's SQL_NUMERIC_STRUCT.
        /// Used for transferring NUMERIC/DECIMAL data between SQL Server and C#.
        /// 
        /// Binary Layout (19 bytes total, Pack=1 for no padding):
        ///   Offset 0: precision (SQLCHAR / byte)         - Total digits (1-38)
        ///   Offset 1: scale (SQLSCHAR / sbyte)           - Digits after decimal point (0-precision)
        ///   Offset 2: sign (SQLCHAR / byte)              - 1=positive, 0=negative
        ///   Offset 3-18: val (SQLCHAR[16] / byte[16])    - Little-endian 128-bit scaled integer
        /// 
        /// References:
        /// - ODBC Programmer's Reference: https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types
        /// - SQL_NUMERIC_STRUCT definition: https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/sql-numeric-structure
        /// - sqltypes.h header: SQL_MAX_NUMERIC_LEN = 16
        /// 
        /// CRITICAL: This struct must be binary-compatible with ODBC's SQL_NUMERIC_STRUCT.
        /// Any layout mismatch will cause data corruption when marshaling to/from native code.
        /// </summary>
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public unsafe struct SqlNumericStruct
        {
            /// <summary>
            /// Total number of decimal digits (1-38).
            /// In T-SQL terms: DECIMAL(precision, scale) - this is the 'precision' part.
            /// Example: DECIMAL(10,2) has precision=10 (up to 10 total digits).
            /// Maps to SQLCHAR (unsigned byte) in ODBC specification.
            /// </summary>
            public byte precision;

            /// <summary>
            /// Number of digits after the decimal point (0-precision).
            /// In T-SQL terms: DECIMAL(precision, scale) - this is the 'scale' part.
            /// Example: DECIMAL(10,2) has scale=2 (2 digits after decimal point).
            /// 
            /// CRITICAL: Maps to SQLSCHAR (signed char) in ODBC specification.
            /// We must use sbyte (not byte) for exact binary layout compatibility.
            /// Although scale is always non-negative in T-SQL, ODBC defines it as signed.
            /// </summary>
            public sbyte scale;

            /// <summary>
            /// Sign indicator: 1 = positive/zero, 0 = negative.
            /// Maps to SQLCHAR (unsigned byte) in ODBC specification.
            /// </summary>
            public byte sign;

            /// <summary>
            /// Little-endian 128-bit integer representing the scaled value.
            /// The actual numeric value = (val as 128-bit integer) * 10^(-scale) * sign.
            /// 
            /// Fixed buffer provides direct memory access without helper methods.
            /// Maps to SQLCHAR val[SQL_MAX_NUMERIC_LEN] where SQL_MAX_NUMERIC_LEN=16.
            /// 
            /// Note: Requires unsafe context to access fixed buffer.
            /// Use: fixed (byte* ptr = numericStruct.val) { ... }
            /// Or: byte b = numericStruct.val[i]; // Direct indexing in unsafe context
            /// </summary>
            public fixed byte val[SQL_NUMERIC_VALUE_SIZE];
        }

        /// <summary>
        /// Validates precision and scale parameters for SQL Server DECIMAL/NUMERIC types.
        /// </summary>
        /// <param name="precision">Total number of digits (1-38).</param>
        /// <param name="scale">Number of digits after decimal point (0-precision).</param>
        /// <param name="parameterName">Parameter name for error messages (e.g., "precision", "scale").</param>
        /// <exception cref="ArgumentException">Thrown when precision or scale are out of valid range.</exception>
        private static void ValidatePrecisionAndScale(byte precision, sbyte scale, string parameterName = "value")
        {
            if (precision < SQL_MIN_PRECISION || precision > SQL_MAX_PRECISION)
            {
                throw new ArgumentException(
                    $"Precision must be between {SQL_MIN_PRECISION} and {SQL_MAX_PRECISION} (T-SQL DECIMAL(p,s) constraint), got {precision}",
                    parameterName);
            }

            if (scale < SQL_MIN_SCALE)
            {
                throw new ArgumentException(
                    $"Scale must be non-negative (T-SQL DECIMAL(p,s) constraint), got {scale}",
                    parameterName);
            }

            if (scale > precision)
            {
                throw new ArgumentException(
                    $"Scale ({scale}) cannot exceed precision ({precision}) (T-SQL DECIMAL(p,s) constraint)",
                    parameterName);
            }
        }

        /// <summary>
        /// Converts SQL_NUMERIC_STRUCT to SqlDecimal with full 38-digit precision support.
        /// This method supports the complete SQL Server DECIMAL/NUMERIC range without data loss.
        /// </summary>
        /// <param name="numeric">The SQL numeric structure from ODBC.</param>
        /// <returns>The equivalent SqlDecimal value.</returns>
        /// <exception cref="ArgumentException">
        /// Thrown when precision or scale are out of valid T-SQL range:
        /// - Precision must be 1-38
        /// - Scale must be 0 to precision
        /// </exception>
        /// <remarks>
        /// SqlDecimal provides full SQL Server precision (38 digits) compared to C# decimal (28-29 digits).
        /// Use this method when working with high-precision values to avoid data loss.
        /// </remarks>
        public static unsafe SqlDecimal ToSqlDecimal(SqlNumericStruct numeric)
        {
            // Validate precision and scale before creating SqlDecimal
            ValidatePrecisionAndScale(numeric.precision, numeric.scale, nameof(numeric));
            
            // SqlDecimal constructor requires int[] array (not byte[])
            // The val array in SqlNumericStruct is 16 bytes = 128 bits
            // We need to convert to 4 int32s (4 x 32 bits = 128 bits)
            
            int[] data = new int[4];
            // Fixed buffers are already fixed - access directly via pointer
            byte* valPtr = numeric.val;
            for (int i = 0; i < 4; i++)
            {
                // Convert each group of 4 bytes to an int32 (little-endian)
                int offset = i * 4;
                data[i] = valPtr[offset] |
                         (valPtr[offset + 1] << 8) |
                         (valPtr[offset + 2] << 16) |
                         (valPtr[offset + 3] << 24);
            }
            
            // SqlDecimal constructor: SqlDecimal(byte precision, byte scale, bool positive, int[] data)
            bool isPositive = numeric.sign == 1;
            
            // Note: SqlDecimal scale parameter is byte (unsigned), but SqlNumericStruct.scale is sbyte (signed)
            // SQL Server scale is always non-negative (0-38), so this cast is safe after validation
            byte scale = (byte)numeric.scale;
            
            return new SqlDecimal(numeric.precision, scale, isPositive, data);
        }

        /// <summary>
        /// Converts SqlDecimal to SQL_NUMERIC_STRUCT for transfer to SQL Server.
        /// This method handles the full 38-digit precision range without data loss.
        /// </summary>
        /// <param name="value">The SqlDecimal value to convert.</param>
        /// <param name="precision">
        /// Total number of digits (1-38) in T-SQL DECIMAL(precision, scale) terms.
        /// If null, uses SqlDecimal's intrinsic precision.
        /// </param>
        /// <param name="scale">
        /// Number of digits after decimal point (0-precision) in T-SQL DECIMAL(precision, scale) terms.
        /// If null, uses SqlDecimal's intrinsic scale.
        /// </param>
        /// <returns>The equivalent SQL numeric structure for ODBC transfer.</returns>
        /// <exception cref="ArgumentException">
        /// Thrown when precision or scale are out of valid T-SQL range:
        /// - Precision must be 1-38
        /// - Scale must be 0 to precision
        /// </exception>
        /// <exception cref="OverflowException">
        /// Thrown when scale adjustment causes data loss (e.g., reducing scale removes non-zero decimal places).
        /// </exception>
        /// <remarks>
        /// When converting SqlDecimal.Null, returns a zero-initialized struct.
        /// Caller must set the null indicator separately (e.g., strLenOrNullMap = SQL_NULL_DATA).
        /// 
        /// Scale Adjustment:
        /// - If targetScale > value.Scale: Adds trailing decimal zeros (no data loss).
        /// - If targetScale &lt; value.Scale: Truncates decimal places (may lose data, throws OverflowException).
        /// - Use AdjustScale(value, scaleShift, round=false) for exact truncation behavior.
        /// </remarks>
        public static unsafe SqlNumericStruct FromSqlDecimal(SqlDecimal value, byte? precision = null, byte? scale = null)
        {
            // Use SqlDecimal's intrinsic precision/scale if not specified
            byte targetPrecision = precision ?? value.Precision;
            byte targetScale = scale ?? value.Scale;
            
            // Validate target precision and scale
            ValidatePrecisionAndScale(targetPrecision, (sbyte)targetScale, nameof(value));
            
            // Handle SqlDecimal.Null
            if (value.IsNull)
            {
                // Return a zero-initialized struct - caller should set null indicator separately
                SqlNumericStruct nullStruct = new SqlNumericStruct
                {
                    precision = targetPrecision,
                    scale = (sbyte)targetScale,
                    sign = 1  // Positive sign convention for NULL placeholders
                };
                
                // Zero out the val array (fixed buffer is already fixed - access directly)
                byte* nullValPtr = nullStruct.val;
                for (int i = 0; i < SQL_NUMERIC_VALUE_SIZE; i++)
                {
                    nullValPtr[i] = 0;
                }
                
                return nullStruct;
            }
            
            // Adjust scale if needed (SqlDecimal has AdjustScale method)
            SqlDecimal adjustedValue = value;
            if (targetScale != value.Scale)
            {
                // AdjustScale returns a new SqlDecimal with the specified scale
                // positive scaleShift adds decimal places, negative removes them
                int scaleShift = targetScale - value.Scale;
                
                try
                {
                    adjustedValue = SqlDecimal.AdjustScale(value, scaleShift, fRound: false);
                }
                catch (OverflowException ex)
                {
                    throw new OverflowException(
                        $"Cannot adjust SqlDecimal scale from {value.Scale} to {targetScale} without data loss. " +
                        $"Original value: {value}", ex);
                }
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
            
            // Fixed buffer is already fixed - access directly via pointer
            byte* valPtr = result.val;
            for (int i = 0; i < 4 && i < data.Length; i++)
            {
                // Convert each int32 to 4 bytes (little-endian)
                int offset = i * 4;
                int value32 = data[i];
                valPtr[offset] = (byte)(value32 & 0xFF);
                valPtr[offset + 1] = (byte)((value32 >> 8) & 0xFF);
                valPtr[offset + 2] = (byte)((value32 >> 16) & 0xFF);
                valPtr[offset + 3] = (byte)((value32 >> 24) & 0xFF);
            }
            
            return result;
        }
    }
}
