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
using System.Text;
using System.Runtime.InteropServices;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class implements logging for the .NET Core C# extension.
    /// </summary>
    class Logging
    {
        /// <summary>
        /// Static constructor to initialize the custom text writers for stdout and stderr.
        /// This ensures that any Console.WriteLine or Console.Error.WriteLine calls
        /// are routed through our InteropTextWriter, which bypasses the .NET Console
        /// abstraction and writes directly to the native OS handles.
        /// </summary>
        static Logging()
        {
            Console.SetOut(new InteropTextWriter(1));
            Console.SetError(new InteropTextWriter(2));
        }

        /// <summary>
        /// This method logs a message to stdout.
        /// </summary>
        /// <param name="message">
        /// Trace message
        /// </param>
        public static void Trace(string message)
        {
        #if DEBUG
            Console.WriteLine(message);
        #endif
        }

        /// <summary>
        /// This method logs an error to stderr.
        /// </summary>
        /// <param name="message">
        /// Error message
        /// </param>
        public static void Error(string message)
        {
            try
            {
                Console.Error.WriteLine(message);
            }
            catch (Exception)
            {
                // Ignore exceptions during error logging
            }
        }

        /// <summary>
        /// Custom TextWriter that writes directly to the standard output/error streams
        /// using native system calls. This is required because the standard Console
        /// output redirection might not work as expected in the embedded environment
        /// where the .NET runtime is hosted by SQL Server.
        /// </summary>
        private class InteropTextWriter : System.IO.TextWriter
        {
            private readonly int _fd;

            /// <summary>
            /// Initializes a new instance of the InteropTextWriter class.
            /// </summary>
            /// <param name="fd">The file descriptor (1 for stdout, 2 for stderr).</param>
            public InteropTextWriter(int fd)
            {
                _fd = fd;
            }

            /// <summary>
            /// Gets the character encoding in which the output is written.
            /// </summary>
            public override Encoding Encoding => Encoding.UTF8;

            /// <summary>
            /// Writes a character to the text stream.
            /// </summary>
            /// <param name="value">The character to write.</param>
            public override void Write(char value)
            {
                Write(value.ToString());
            }

            /// <summary>
            /// Writes a string to the text stream.
            /// This method overrides the default implementation to write directly to
            /// the native file descriptors/handles, ensuring that data is flushed
            /// immediately and correctly to the parent process (SQL Server).
            /// </summary>
            /// <param name="value">The string to write.</param>
            public override void Write(string value)
            {
                if (string.IsNullOrEmpty(value)) return;

                byte[] buffer = Encoding.UTF8.GetBytes(value);
                if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                {
                    // STD_OUTPUT_HANDLE = -11
                    // STD_ERROR_HANDLE = -12
                    IntPtr handle = GetStdHandle(_fd == 1 ? -11 : -12);
                    if (handle != IntPtr.Zero && handle != (IntPtr)(-1))
                    {
                        uint bytesWritten;
                        WriteFile(handle, buffer, (uint)buffer.Length, out bytesWritten, IntPtr.Zero);
                    }
                }
                else
                {
                    // On Linux, write directly to the file descriptor (1 for stdout, 2 for stderr)
                    write(_fd, buffer, (IntPtr)buffer.Length);
                }
            }

            /// <summary>
            /// Writes a string followed by a line terminator to the text stream.
            /// </summary>
            /// <param name="value">The string to write.</param>
            public override void WriteLine(string value)
            {
                Write(value + Environment.NewLine);
            }

            /// <summary>
            /// Retrieves a handle to the specified standard device (standard input, standard output, or standard error).
            /// This is used to bypass the .NET Console abstraction and write directly to the OS handle,
            /// ensuring output is captured correctly by the SQL Server host process.
            /// </summary>
            [DllImport("kernel32.dll", SetLastError = true)]
            private static extern IntPtr GetStdHandle(int nStdHandle);

            /// <summary>
            /// Writes data to the specified file or input/output (I/O) device.
            /// Used to write directly to the standard output/error handles on Windows,
            /// bypassing potential redirection issues in the managed runtime.
            /// </summary>
            [DllImport("kernel32.dll", SetLastError = true)]
            private static extern bool WriteFile(IntPtr hFile, byte[] lpBuffer, uint nNumberOfBytesToWrite, out uint lpNumberOfBytesWritten, IntPtr lpOverlapped);

            /// <summary>
            /// Writes to a file descriptor.
            /// Used on Linux to write directly to stdout (1) or stderr (2),
            /// ensuring output reaches the SQL Server host process reliably.
            /// </summary>
            [DllImport("libc", SetLastError = true)]
            private static extern IntPtr write(int fd, byte[] buf, IntPtr count);
        }
    }
}
