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
                if(strLens[i] == -1)
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
        /// Unlike StringSplitToArray, this properly handles multi-byte UTF-8 characters by
        /// decoding each segment independently based on byte lengths.
        /// </summary>
        /// <param name="data">
        /// Pointer to the raw UTF-8 byte data
        /// </param>
        /// <param name="byteLens">
        /// An array of integers representing the byte lengths of each string segment
        /// </param>
        public static unsafe string[] UTF8ByteSplitToArray(byte* data, int[] byteLens)
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
                if (byteLens[i] == -1)
                {
                    strArray[i] = null;
                    continue;
                }

                if (byteLens[i] == 0)
                {
                    strArray[i] = string.Empty;
                    continue;
                }

                // Decode the UTF-8 bytes for this segment.
                //
                strArray[i] = Encoding.UTF8.GetString(data + byteOffset, byteLens[i]);
                byteOffset += byteLens[i];
            }

            return strArray;
        }
    }
}
