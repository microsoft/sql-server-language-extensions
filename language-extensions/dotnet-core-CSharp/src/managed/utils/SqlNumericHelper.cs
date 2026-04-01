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
using System.Runtime.InteropServices;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// Helper class for converting between SQL Server NUMERIC/DECIMAL types and SqlDecimal.
    /// Provides ODBC-compatible SQL_NUMERIC_STRUCT definition and conversion methods.
    ///
    /// IMPORTANT: We use SqlDecimal from Microsoft.Data.SqlClient which supports
    /// full SQL Server precision (38 digits). 
    /// C# native decimal is NOT used as it has 28-digit limitations.
    /// </summary>
    public static class SqlNumericHelper
    {
        // Precision and scale constraints from SqlDecimal (Microsoft.Data.SqlClient)
        // These are the canonical SQL Server DECIMAL/NUMERIC limits
        
        /// <summary>
        /// SQL Server maximum precision for DECIMAL/NUMERIC types (digits).
        /// </summary>
        public static readonly byte SQL_MAX_PRECISION = SqlDecimal.MaxPrecision;

        /// <summary>
        /// Minimum precision for DECIMAL/NUMERIC types (digits).
        /// SQL Server requires at least 1 digit of precision.
        /// </summary>
        public const byte SQL_MIN_PRECISION = 1;

        /// <summary>
        /// Maximum scale for DECIMAL/NUMERIC types (digits after decimal point).
        /// Scale cannot exceed precision.
        /// </summary>
        public static readonly byte SQL_MAX_SCALE = SqlDecimal.MaxScale;

        /// <summary>
        /// Minimum scale for DECIMAL/NUMERIC types (digits after decimal point).
        /// SQL Server allows scale of 0 (integers).
        /// </summary>
        public const byte SQL_MIN_SCALE = 0;

        /// <summary>
        /// Size of SQL_NUMERIC_STRUCT value array in bytes.
        /// Defined as SQL_MAX_NUMERIC_LEN in ODBC specification (sql.h/sqltypes.h).
        /// </summary>
        public const int SQL_NUMERIC_VALUE_SIZE = 16;

        /// <summary>
        /// Number of Int32 values needed to represent the SQL_NUMERIC_STRUCT value array.
        /// Calculated as: 16 bytes / 4 bytes per Int32 = 4 Int32s.
        /// </summary>
        private const int INT32_ARRAY_SIZE = 4;

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
        /// - SQL_NUMERIC_STRUCT definition: https://learn.microsoft.com/en-us/sql/odbc/reference/appendixes/retrieve-numeric-data-sql-numeric-struct-kb222831
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
            /// Maps to SQLSCHAR (signed char) in ODBC specification.
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
            /// Fixed buffer provides direct memory access.
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
        /// Converts SQL_NUMERIC_STRUCT to SqlDecimal.
        /// </summary>
        /// <param name="numeric">The SQL numeric structure from ODBC.</param>
        /// <returns>The equivalent SqlDecimal value.</returns>
        /// <exception cref="ArgumentException">
        /// Thrown when precision or scale are out of valid T-SQL range:
        /// - Precision must be 1-38
        /// - Scale must be between 0 and precision
        /// </exception>
        /// <remarks>
        /// SqlDecimal provides full SQL Server precision (38 digits) compared to the native C# decimal (28-29 digits).
        /// </remarks>
        public static unsafe SqlDecimal ToSqlDecimal(SqlNumericStruct numeric)
        {
            // Validate precision and scale before creating SqlDecimal
            //
            ValidatePrecisionAndScale(numeric.precision, numeric.scale, nameof(numeric));
            
            // SqlDecimal constructor requires int[] array (not byte[])
            // The val array in SqlNumericStruct is 16 bytes = 128 bits
            // We need to convert to 4 int32s (4 x 32 bits = 128 bits)
            //
            int[] data = new int[INT32_ARRAY_SIZE];

            // Fixed buffers are already fixed - access directly via pointer
            //
            byte* valPtr = numeric.val;
            for (int i = 0; i < INT32_ARRAY_SIZE; i++)
            {
                // Convert each group of 4 bytes to an int32 (little-endian)
                //
                int offset = i * 4;
                data[i] = valPtr[offset] |
                         (valPtr[offset + 1] << 8) |
                         (valPtr[offset + 2] << 16) |
                         (valPtr[offset + 3] << 24);
            }
            
            // SqlDecimal constructor: 
            // SqlDecimal(byte precision, byte scale, bool positive, int[] data)
            //
            bool isPositive = numeric.sign == 1;
            
            // Note: SqlDecimal scale parameter is byte (unsigned), but SqlNumericStruct.scale is sbyte (signed)
            // SQL Server scale is always non-negative (0-38), so this cast is safe after validation
            //
            byte scale = (byte)numeric.scale;
            
            return new SqlDecimal(numeric.precision, scale, isPositive, data);
        }

        /// <summary>
        /// Converts SqlDecimal to SQL_NUMERIC_STRUCT for ODBC transfer.
        /// </summary>
        /// <param name="value">The SqlDecimal value to convert.</param>
        /// <param name="precision">Target precision (1-38). If null, uses value's intrinsic precision.</param>
        /// <param name="scale">Target scale (0-precision). If null, uses value's intrinsic scale.</param>
        /// <returns>ODBC-compatible SQL_NUMERIC_STRUCT.</returns>
        /// <exception cref="ArgumentException">Thrown when precision or scale constraints violated.</exception>
        /// <exception cref="OverflowException">Thrown when scale adjustment loses data or value exceeds target precision.</exception>
        /// <remarks>
        /// NULL values return zero-initialized struct; caller must set null indicator (e.g., strLenOrNullMap = SQL_NULL_DATA).
        /// Scale adjustment uses SqlDecimal.AdjustScale with fRound=false to prevent silent data loss.
        /// </remarks>
        public static unsafe SqlNumericStruct FromSqlDecimal(SqlDecimal value, byte? precision = null, byte? scale = null)
        {
            // Use SqlDecimal's intrinsic precision/scale if not specified
            //
            byte targetPrecision = precision ?? value.Precision;
            byte targetScale = scale ?? value.Scale;
            
            // Validate target precision and scale constraints
            //
            ValidatePrecisionAndScale(targetPrecision, (sbyte)targetScale, nameof(value));
            
            // NULL values return zero-initialized struct; caller sets null indicator separately
            //
            if (value.IsNull)
            {
                return new SqlNumericStruct
                {
                    precision = targetPrecision,
                    scale = (sbyte)targetScale,
                    sign = 1
                };
            }
            
            // Adjust scale if needed to match target
            //
            SqlDecimal adjustedValue = value;
            if (targetScale != value.Scale)
            {
                int scaleShift = targetScale - value.Scale;
                
                try
                {
                    // fRound=false ensures no silent data loss when reducing scale
                    //
                    adjustedValue = SqlDecimal.AdjustScale(value, scaleShift, fRound: false);
                }
                catch (OverflowException ex)
                {
                    throw new OverflowException(
                        $"Cannot adjust scale from {value.Scale} to {targetScale} without data loss. Value: {value}", ex);
                }
            }
            
            // Validate adjusted value fits within target precision
            //
            if (adjustedValue.Precision > targetPrecision)
            {
                throw new OverflowException(
                    $"Value requires {adjustedValue.Precision} digits but target DECIMAL({targetPrecision},{targetScale}) allows only {targetPrecision}.");
            }
            
            SqlNumericStruct result = new SqlNumericStruct
            {
                precision = targetPrecision,
                scale = (sbyte)targetScale,
                sign = (byte)(adjustedValue.IsPositive ? 1 : 0)
            };
            
            // Convert SqlDecimal's int[4] data to byte[16] for ODBC struct (little-endian)
            //
            int[] data = adjustedValue.Data;
            byte* valPtr = result.val;
            
            for (int i = 0; i < INT32_ARRAY_SIZE; i++)
            {
                int value32 = data[i];
                int offset = i * 4;
                valPtr[offset] = (byte)value32;
                valPtr[offset + 1] = (byte)(value32 >> 8);
                valPtr[offset + 2] = (byte)(value32 >> 16);
                valPtr[offset + 3] = (byte)(value32 >> 24);
            }
            
            return result;
        }

        /// <summary>
        /// Converts SQL_NUMERIC_STRUCT pointer to SqlDecimal, handling OUTPUT parameter convention.
        /// </summary>
        /// <param name="numericPtr">Pointer to SQL_NUMERIC_STRUCT from ODBC.</param>
        /// <returns>SqlDecimal value, or SqlDecimal.Null for OUTPUT parameters (precision=0 sentinel).</returns>
        /// <exception cref="ArgumentNullException">Thrown when pointer is null.</exception>
        /// <exception cref="ArgumentException">Thrown when precision or scale violate constraints.</exception>
        /// <remarks>
        /// OUTPUT Parameter Convention: SQL Server passes uninitialized structs with precision=0 as a sentinel
        /// indicating "output only, no input value". This violates ODBC spec (requires precision 1-38) but is
        /// safe to detect. Returns SqlDecimal.Null in this case; caller will assign the actual output value.
        /// </remarks>
        public static unsafe SqlDecimal ToSqlDecimalFromPointer(SqlNumericStruct* numericPtr)
        {
            if (numericPtr == null)
            {
                throw new ArgumentNullException(nameof(numericPtr));
            }

            // precision=0 is the OUTPUT parameter sentinel (uninitialized struct)
            //
            if (numericPtr->precision == 0)
            {
                return SqlDecimal.Null;
            }

            return ToSqlDecimal(*numericPtr);
        }
    }
}
