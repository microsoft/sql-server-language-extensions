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
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class is used for Interop between managed and unmanaged (native) interaction.
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
            if (str == null) return null;
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
            if (str == null) return null;
            if (length > int.MaxValue) throw new ArgumentOutOfRangeException(nameof(length), "Length exceeds supported range.");
            return Marshal.PtrToStringUTF8((IntPtr)str, (int) length);
        }

        /// <summary>
        /// This method allocates a managed System.String and copies characters from
        /// an unmanaged null-terminated UTF16 (Unicode) string into it.
        /// The string must be null-terminated as this overload reads until the null terminator.
        /// </summary>
        /// <param name="str">
        /// The address of the first character of the unmanaged null-terminated Unicode string.
        /// </param>
        /// <returns>
        /// A managed string that holds a copy of the unmanaged string if the value of the
        /// ptr parameter is not null; otherwise, this method returns null.
        /// </returns>
        public static string UTF16PtrToStr(char* str)
        {
            if (str == null) return null;
            return Marshal.PtrToStringUni((IntPtr)str);
        }

        /// <summary>
        /// This method allocates a managed System.String and copies a specified number of characters from
        /// an unmanaged UTF16 (Unicode) string into it.
        /// This overload does not require null termination as it reads exactly the specified number of characters.
        /// </summary>
        /// <param name="str">
        /// The address of the first character of the unmanaged Unicode string.
        /// </param>
        /// <param name="length">
        /// The number of Unicode characters to copy.
        /// </param>
        /// <returns>
        /// A managed string that holds a copy of the unmanaged string if the value of the
        /// ptr parameter is not null; otherwise, this method returns null.
        /// </returns>
        public static string UTF16PtrToStr(char* str, int length)
        {
            if (str == null) return null;
            if (length < 0) throw new ArgumentOutOfRangeException(nameof(length), "Length must be non-negative.");
            return Marshal.PtrToStringUni((IntPtr)str, length);
        }

        /// <summary>
        /// This method copies the contents of one block of memory to a managed int array.
        /// </summary>
        /// <param name="source">
        /// Source of the block of memory
        /// </param>
        /// <param name="destination">
        /// The array that stores the values from the block of memory
        /// </param>
        /// <param name="startIndex">
        /// The start index of the block of memory
        /// </param>
        /// <param name="length">
        /// The length of the block of memory
        /// </param>
        public static void Copy(int* source, int[] destination, int startIndex, int length)
        {
            if(source == null)
            {
                return;
            }

            Marshal.Copy((IntPtr)source, destination, startIndex, length);
        }
    }
}
