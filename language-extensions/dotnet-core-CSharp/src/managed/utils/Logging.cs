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
using System.Diagnostics;
using System.IO;
using System.Text;

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
            catch (Exception ex)
            {
                // Ignore exceptions during error logging but log to debug output
                Debug.WriteLine($"Error logging failed: {ex}");
            }
        }

        /// <summary>
        /// Custom TextWriter that writes directly to the standard output/error streams
        /// using native system calls. This is required because the standard Console
        /// output redirection might not work as expected in the embedded environment
        /// where the .NET runtime is hosted by SQL Server.
        /// </summary>
        private class InteropTextWriter : TextWriter
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
                Write(new string(value, 1));
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
                try
                {
                    using (Stream stream = _fd == 1 ? Console.OpenStandardOutput() : Console.OpenStandardError())
                    {
                        stream.Write(buffer, 0, buffer.Length);
                        stream.Flush();
                    }
                }
                catch (Exception ex)
                {
                    // Fallback to standard .NET Console as last resort
                    try
                    {
                        Console.Error.WriteLine($"Writing to console stream failed with error: {ex.Message}");
                    }
                    catch { }
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
        }
    }
}
