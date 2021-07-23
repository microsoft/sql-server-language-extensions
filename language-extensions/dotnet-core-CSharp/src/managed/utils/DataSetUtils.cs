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
    }
}
