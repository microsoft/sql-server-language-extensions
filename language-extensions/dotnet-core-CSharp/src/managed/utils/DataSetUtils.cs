//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: DataSetUtils.cs
//
// Purpose:
//  Provides methods needed for Input/Output DataSet
//
//*********************************************************************
using System;
using System.Text;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class contains methods needed for Input/Output DataSet
    /// </summary>
    internal class DataSetUtils
    {
        /// <summary>
        /// Value indicating a null string in the strLens/byteLens array.
        /// </summary>
        public const int NullStringIndicator = -1;

        /// <summary>
        /// Calculates the total buffer size from an array of byte lengths.
        /// This is used to enable bounds checking when splitting string buffers.
        /// </summary>
        /// <param name="byteLens">An array of byte lengths, where negative values indicate null entries.</param>
        /// <returns>The sum of all positive (non-null) byte lengths.</returns>
        public static int CalculateTotalBufferSize(int[] byteLens)
        {
            if (byteLens == null || byteLens.Length == 0)
            {
                return 0;
            }

            int total = 0;
            for (int i = 0; i < byteLens.Length; ++i)
            {
                if (byteLens[i] > 0)
                {
                    total += byteLens[i];
                }
            }
            return total;
        }

        /// <summary>
        /// This method splits a string to an array of substring according to lengths.
        /// </summary>
        /// <param name="str">
        /// The string to be split
        /// </param>
        /// <param name="strLens">
        /// An array of integers representing the lengths
        /// </param>
        public static string[] StringSplitToArray(string str, int[] strLens)
        {
            string[] strArray = new string[strLens.Length];

            // Return empty string list if the string is null
            //
            if(str == null)
            {
                return strArray;
            }

            int startIndex = 0;
            for (int i = 0; i < strLens.Length; ++i)
            {
                if(strLens[i] == NullStringIndicator)
                {
                    strArray[i] = null;
                    continue;
                }

                if(startIndex + strLens[i] > str.Length)
                {
                    strArray[i] = str.Substring(startIndex);
                    break;
                }

                strArray[i] = str.Substring(startIndex, strLens[i]);
                startIndex += strLens[i];
            }

            return strArray;
        }

        /// <summary>
        /// This method splits a UTF-8 byte buffer into an array of strings according to byte lengths.
        /// SQL Server sends UTF-8 encoded string data with byte lengths in the strLenOrNullMap.
        /// We decode each segment directly from the byte buffer to properly handle multi-byte
        /// UTF-8 characters (e.g., Chinese characters, Euro symbol), where character count != byte count.
        /// </summary>
        /// <param name="data">
        /// Pointer to the raw UTF-8 byte data
        /// </param>
        /// <param name="byteLens">
        /// An array of integers representing the byte lengths of each string segment.
        /// A value of NullStringIndicator (-1) indicates a null string.
        /// </param>
        /// <param name="totalBufferSize">
        /// Total size of the buffer in bytes for bounds checking.
        /// When provided (non-zero), the method validates that pointer arithmetic stays within
        /// valid memory and throws ArgumentException if a buffer overrun would occur.
        /// Callers should provide this value when available to guard against corrupted length data.
        /// </param>
        /// <returns>
        /// An array of decoded strings, with null entries for null indicators.
        /// </returns>
        /// <exception cref="ArgumentException">
        /// Thrown when totalBufferSize is provided and the cumulative byte offset would exceed it.
        /// </exception>
        public static unsafe string[] UTF8ByteSplitToArray(byte* data, int[] byteLens, int totalBufferSize = 0)
        {
            // Return empty array if byteLens is null or empty
            //
            if (byteLens == null || byteLens.Length == 0)
            {
                return Array.Empty<string>();
            }

            string[] strArray = new string[byteLens.Length];

            // Return empty string list if the data is null
            //
            if (data == null)
            {
                return strArray;
            }

            int byteOffset = 0;
            for (int i = 0; i < byteLens.Length; ++i)
            {
                if (byteLens[i] == NullStringIndicator)
                {
                    strArray[i] = null;
                    continue;
                }

                if (byteLens[i] == 0)
                {
                    strArray[i] = string.Empty;
                    continue;
                }

                // Validate bounds if buffer size is provided
                //
                if (totalBufferSize > 0 && byteOffset + byteLens[i] > totalBufferSize)
                {
                    throw new ArgumentException(
                        $"Buffer overrun detected: offset {byteOffset} + length {byteLens[i]} exceeds buffer size {totalBufferSize}");
                }

                // Decode the UTF-8 bytes for this segment.
                //
                strArray[i] = Encoding.UTF8.GetString(data + byteOffset, byteLens[i]);
                byteOffset += byteLens[i];
            }

            return strArray;
        }

        /// <summary>
        /// This method splits a UTF-16 byte buffer into an array of strings according to byte lengths.
        /// SQL Server sends UTF-16 (nvarchar/nchar) encoded string data with byte lengths in the strLenOrNullMap.
        /// We decode each segment directly from the byte buffer to properly handle multi-byte
        /// sequences where the entire buffer may contain multiple strings separated by their byte offsets.
        /// </summary>
        /// <param name="data">
        /// Pointer to the raw UTF-16 byte data (wchar_t buffer)
        /// </param>
        /// <param name="byteLens">
        /// An array of integers representing the byte lengths of each string segment.
        /// A value of NullStringIndicator (-1) indicates a null string.
        /// </param>
        /// <param name="totalBufferSize">
        /// Total size of the buffer in bytes for bounds checking.
        /// When provided (non-zero), the method validates that pointer arithmetic stays within
        /// valid memory and throws ArgumentException if a buffer overrun would occur.
        /// Callers should provide this value when available to guard against corrupted length data.
        /// </param>
        /// <returns>
        /// An array of decoded strings, with null entries for null indicators.
        /// </returns>
        /// <exception cref="ArgumentException">
        /// Thrown when totalBufferSize is provided and the cumulative byte offset would exceed it.
        /// </exception>
        public static unsafe string[] UTF16ByteSplitToArray(byte* data, int[] byteLens, int totalBufferSize = 0)
        {
            string[] strArray = new string[byteLens.Length];

            // Return empty string list if the data is null
            //
            if (data == null)
            {
                return strArray;
            }

            int byteOffset = 0;
            for (int i = 0; i < byteLens.Length; ++i)
            {
                if (byteLens[i] == NullStringIndicator)
                {
                    strArray[i] = null;
                    continue;
                }

                if (byteLens[i] == 0)
                {
                    strArray[i] = string.Empty;
                    continue;
                }

                // Validate bounds if buffer size is provided
                //
                if (totalBufferSize > 0 && byteOffset + byteLens[i] > totalBufferSize)
                {
                    throw new ArgumentException(
                        $"Buffer overrun detected: offset {byteOffset} + length {byteLens[i]} exceeds buffer size {totalBufferSize}");
                }

                // Decode the UTF-16 bytes for this segment.
                // In C#, sizeof(char) is always 2 bytes (UTF-16), regardless of platform.
                // Note: This differs from C++ where wchar_t is 2 bytes on Windows but 4 bytes on Linux.
                // Since this extension only supports Windows currently, we use C#'s char (2 bytes).
                //
                int charCount = byteLens[i] / sizeof(char);
                strArray[i] = new string((char*)(data + byteOffset), 0, charCount);
                byteOffset += byteLens[i];
            }

            return strArray;
        }
    }
}
