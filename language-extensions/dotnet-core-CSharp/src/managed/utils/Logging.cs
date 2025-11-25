//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: Logging.cs
//
// Purpose:
//  Implementation of logging for the .NET Core C# extension.
//
//*********************************************************************
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class implements logging for the .NET Core C# extension.
    /// </summary>
    class Logging
    {
        private delegate int WriteDelegate(int fd, [MarshalAs(UnmanagedType.LPArray)] byte[] buffer, int count);
        private static readonly WriteDelegate _write;

        static Logging()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                IntPtr libHandle = IntPtr.Zero;
#if DEBUG
                NativeLibrary.TryLoad("ucrtbased.dll", out libHandle);
#endif
                if (libHandle == IntPtr.Zero)
                {
                    if (!NativeLibrary.TryLoad("ucrtbase.dll", out libHandle))
                    {
                        NativeLibrary.TryLoad("msvcrt.dll", out libHandle);
                    }
                }

                if (libHandle != IntPtr.Zero)
                {
                    if (NativeLibrary.TryGetExport(libHandle, "_write", out IntPtr writeAddr))
                    {
                        _write = Marshal.GetDelegateForFunctionPointer<WriteDelegate>(writeAddr);
                    }
                }
            }
        }

        private static void WriteToFd(int fd, string message)
        {
            if (_write == null) return;
            try
            {
                byte[] buffer = Encoding.UTF8.GetBytes(message + Environment.NewLine);
                _write(fd, buffer, buffer.Length);
            }
            catch
            {
                // Ignore native write errors
            }
        }

        /// <summary>
        /// This method logs a message to stdout.
        /// </summary>
        /// <param name="message">
        /// Trace message
        /// </param>
        public static void Trace(string message)
        {
            WriteToFd(1, message);
        }

        /// <summary>
        /// This method logs an error to stderr.
        /// </summary>
        /// <param name="message">
        /// Error message
        /// </param>
        public static void Error(string message)
        {
            WriteToFd(2, message);
        }
    }
}
