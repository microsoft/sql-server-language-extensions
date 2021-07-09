//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: InteropUtils.cs
//
// Purpose:
//  Type conversion management
//
//*********************************************************************
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class manages conversions of different types and objects
    /// </summary>
    public static unsafe class Interop
    {
        /// <summary>
        /// This method allocates a managed System.String and copies a specified
        /// number of bytes from an unmanaged null-terminated UTF8 string into it.
        /// </summary>
        /// <param name="str">
        /// The address of the first character of the unmanaged string.
        /// </param>
        /// <returns>
        /// A managed string that holds a copy of the unmanaged string if the value of the
        /// ptr parameter is not null; otherwise, this method returns null.
        /// </returns>
        public static string UTF8PtrToStr(char* str)
        {
            return Marshal.PtrToStringUTF8((IntPtr)str);
        }

        /// <summary>
        /// This method allocates a managed System.String and copies a specified number of bytes from
        /// an unmanaged null-terminated UTF8 string into it.
        /// </summary>
        /// <param name="str">
        /// The address of the first character of the unmanaged string.
        /// </param>
        /// <param name="length">
        /// The length of the unmanaged string.
        /// </param>
        /// <returns>
        /// A managed string that holds a copy of the unmanaged string if the value of the
        /// ptr parameter is not null; otherwise, this method returns null.
        /// </returns>
        public static string UTF8PtrToStr(char* str, ulong length)
        {
            return Marshal.PtrToStringUTF8((IntPtr)str, (int) length);
        }
    }
}
