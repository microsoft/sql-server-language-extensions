//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: SqlNumericHelper.cs
//
// Purpose:
//  SQL NUMERIC/DECIMAL type support: ODBC-compatible struct definition
//  and bidirectional conversion between SQL_NUMERIC_STRUCT and C# decimal.
//
//*********************************************************************
using System;
using System.Linq;
using System.Runtime.InteropServices;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// Helper class for converting between SQL Server NUMERIC/DECIMAL types and C# decimal.
    /// Provides ODBC-compatible SQL_NUMERIC_STRUCT definition and conversion methods.
    /// </summary>
    public static class SqlNumericHelper
    {
        /// <summary>
        /// Maximum number of powers of 10 in the "PowersOf10" lookup table.
        /// C# decimal supports up to 28-29 significant digits, so we store 10^0 through 10^28 (29 entries).
        /// This covers all possible scale values (0-38) within C# decimal's precision range.
        /// Array index corresponds to the exponent: PowersOf10[n] = 10^n.
        /// </summary>
        private const int MaxPowersOf10Count = 29;

        // Powers of 10 lookup table for efficient decimal scaling (up to 10^28)
        // 
        // Use a lookup table instead of Math.Pow because:
        // - Math.Pow returns double, requiring conversion to decimal with potential precision loss.
        // - Repeated Math.Pow calls in tight loops have measurable performance impact.
        // - Pre-computed decimal constants give exact values with zero runtime overhead.
        // - C# decimal supports up to 28-29 significant digits, so 10^0 through 10^28 covers all cases.
        private static readonly decimal[] PowersOf10 = new decimal[MaxPowersOf10Count]
        {
            1m,                                  // 10^0
            10m,                                 // 10^1
            100m,                                // 10^2
            1000m,                               // 10^3
            10000m,                              // 10^4
            100000m,                             // 10^5
            1000000m,                            // 10^6
            10000000m,                           // 10^7
            100000000m,                          // 10^8
            1000000000m,                         // 10^9
            10000000000m,                        // 10^10
            100000000000m,                       // 10^11
            1000000000000m,                      // 10^12
            10000000000000m,                     // 10^13
            100000000000000m,                    // 10^14
            1000000000000000m,                   // 10^15
            10000000000000000m,                  // 10^16
            100000000000000000m,                 // 10^17
            1000000000000000000m,                // 10^18
            10000000000000000000m,               // 10^19
            100000000000000000000m,              // 10^20
            1000000000000000000000m,             // 10^21
            10000000000000000000000m,            // 10^22
            100000000000000000000000m,           // 10^23
            1000000000000000000000000m,          // 10^24
            10000000000000000000000000m,         // 10^25
            100000000000000000000000000m,        // 10^26
            1000000000000000000000000000m,       // 10^27
            10000000000000000000000000000m       // 10^28
        };

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
        /// Converts SQL_NUMERIC_STRUCT to C# decimal.
        /// </summary>
        /// <param name="numeric">The SQL numeric structure from ODBC.</param>
        /// <returns>The equivalent C# decimal value.</returns>
        /// <exception cref="OverflowException">Thrown when the value exceeds C# decimal range.</exception>
        public static decimal ToDecimal(SqlNumericStruct numeric)
        {
            decimal result;
            
            try
            {
                // Convert little-endian byte array (16 bytes) to a scaled integer value.
                // The val array contains the absolute value scaled by 10^scale.
                // For example, for numeric(10,2) value 123.45:
                //   scale = 2, val represents 12345 (123.45 * 10^2)
                //
                // Little-endian storage layout:
                // - val[0] = least significant byte (LSB)
                // - val[15] = most significant byte (MSB)
                // - Each byte represents one "digit" in base-256 representation
                // - Example: bytes [0x39, 0x30, 0x00, ...] = 0x39 + (0x30 * 256) = 57 + 12288 = 12345
                //
                decimal scaledValue = 0m;
                
                // Find the most significant non-zero byte (highest index) to optimize the conversion.
                // This avoids processing unnecessary high-order zero bytes and prevents potential
                // overflow when building large values. Most practical values use only 12-13 bytes.
                int lastNonZeroByte = -1;
                for (int i = 15; i >= 0; i--)
                {
                    if (numeric.GetVal(i) != 0)
                    {
                        lastNonZeroByte = i;
                        break;
                    }
                }
                
                // If all bytes are zero, result is 0
                if (lastNonZeroByte == -1)
                {
                    result = 0m;
                }
                else
                {
                    // Build the integer value by processing from MSB (highest index) to LSB (index 0).
                    // Algorithm: Start with MSB, then for each subsequent byte toward LSB,
                    // multiply current value by 256 and add the next byte.
                    // This approach avoids large intermediate multipliers that could overflow decimal.
                    for (int i = lastNonZeroByte; i >= 0; i--)
                    {
                        scaledValue = scaledValue * 256m + numeric.GetVal(i);
                    }

                    // Scale down by dividing by 10^scale to get the actual decimal value.
                    // The scaledValue contains the integer representation; we need to divide by 10^scale.
                    // For example, if scaledValue=12345 and scale=2, result = 12345 / 100 = 123.45
                    if (numeric.scale >= 0 && numeric.scale < PowersOf10.Length)
                    {
                        // Use pre-computed lookup table for scales 0-28 (fast path)
                        result = scaledValue / PowersOf10[numeric.scale];
                    }
                    else if (numeric.scale == 0)
                    {
                        // No scaling needed - value is already an integer
                        result = scaledValue;
                    }
                    else
                    {
                        // For scales beyond our lookup table (29-38), use repeated division by 10.
                        // We cannot use Math.Pow(10, scale) because:
                        // - Math.Pow returns double, and values > 10^28 overflow when converting double→decimal
                        // - Repeated division maintains decimal precision without overflow
                        result = scaledValue;
                        for (int i = 0; i < numeric.scale; i++)
                        {
                            result /= 10m;
                        }
                    }

                    // Apply sign: 1 = positive, 0 = negative
                    if (numeric.sign == 0)
                    {
                        result = -result;
                    }
                }
            }
            catch (OverflowException)
            {
                // SQL Server DECIMAL(38,scale) can represent values much larger than C# decimal's range.
                // C# decimal maximum: ±79,228,162,514,264,337,593,543,950,335 (approx ±7.9 × 10^28)
                // SQL DECIMAL(38,0) maximum: ±10^38 - 1
                //
                // This overflow typically occurs with DECIMAL(30+, scale) parameters containing values
                // that exceed 29 significant digits total.
                string valHex = string.Join("", Enumerable.Range(0, 16).Select(i => numeric.GetVal(i).ToString("X2")));
                throw new OverflowException(
                    $"SQL DECIMAL/NUMERIC value exceeds C# decimal range. " +
                    $"Precision={numeric.precision}, Scale={numeric.scale}, Sign={numeric.sign}, " +
                    $"Val={valHex}. " +
                    $"C# decimal supports up to 29 significant digits (±7.9×10^28). " +
                    $"Consider using lower precision parameters or handle large numerics differently.");
            }
            
            return result;
        }

        /// <summary>
        /// Converts C# decimal to SQL_NUMERIC_STRUCT.
        /// Follows the same conversion logic as Java extension's BigDecimalToNumericStruct.
        /// </summary>
        /// <param name="value">The C# decimal value to convert.</param>
        /// <param name="precision">Total number of digits (1-38).</param>
        /// <param name="scale">Number of digits after decimal point (0-precision).</param>
        /// <returns>The equivalent SQL numeric structure for ODBC.</returns>
        /// <exception cref="ArgumentException">Thrown when precision or scale are out of valid range.</exception>
        public static SqlNumericStruct FromDecimal(decimal value, byte precision, byte scale)
        {
            if (precision < 1 || precision > 38)
            {
                throw new ArgumentException($"Precision must be between 1 and 38, got {precision}");
            }
            if (scale > precision)
            {
                throw new ArgumentException($"Scale ({scale}) cannot exceed precision ({precision})");
            }

            SqlNumericStruct result = new SqlNumericStruct
            {
                precision = precision,
                scale = (sbyte)scale, // Safe cast: scale validated and the max is 38 < 127.
                sign = (byte)(value >= 0 ? 1 : 0)
            };

            // Work with absolute value
            decimal absValue = Math.Abs(value);

            // Scale up by multiplying by 10^scale to get an integer representation
            // For example, 123.45 with scale=2 becomes 12345
            decimal scaledValue;
            if (scale >= 0 && scale < PowersOf10.Length)
            {
                scaledValue = absValue * PowersOf10[scale];
            }
            else if (scale == 0)
            {
                scaledValue = absValue;
            }
            else
            {
                // For scales beyond our lookup table, use repeated multiplication by 10
                // Cannot use Math.Pow(10, scale) because values > 10^28 overflow when converting double→decimal
                scaledValue = absValue;
                for (int i = 0; i < scale; i++)
                {
                    scaledValue *= 10m;
                }
            }

            // Round to nearest integer (handles any remaining fractional part due to precision limits)
            scaledValue = Math.Round(scaledValue, 0, MidpointRounding.AwayFromZero);

            // Convert the scaled integer to little-endian byte array (16 bytes)
            // Each byte represents one position in base-256 representation
            for (int i = 0; i < 16; i++)
            {
                if (scaledValue > 0)
                {
                    decimal byteValue = scaledValue % 256m;
                    result.SetVal(i, (byte)byteValue);
                    scaledValue = Math.Floor(scaledValue / 256m);
                }
                else
                {
                    result.SetVal(i, 0);
                }
            }

            // If there's still value left after filling 16 bytes, we have overflow
            if (scaledValue > 0)
            {
                throw new OverflowException(
                    $"Value {value} with precision {precision} and scale {scale} exceeds SQL_NUMERIC_STRUCT capacity");
            }

            return result;
        }
    }
}
