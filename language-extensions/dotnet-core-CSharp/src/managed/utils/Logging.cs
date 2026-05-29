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
        /// File descriptor for standard output.
        /// </summary>
        private const int StdOut = 1;

        /// <summary>
        /// File descriptor for standard error.
        /// </summary>
        private const int StdErr = 2;

        /// <summary>
        /// Host-provided LogXEvent callback, set via SetHostCallbacks.
        /// Marked volatile so concurrent readers in LogXEvent always
        /// observe the latest write from SetLogXEventCallback.
        /// </summary>
        private static volatile CSharpExtension.LogXEventCallbackDelegate _logXEventCallback;

        /// <summary>
        /// Static constructor to initialize the custom text writers for stdout and stderr.
        /// This ensures that any Console.WriteLine or Console.Error.WriteLine calls
        /// are routed through our InteropTextWriter, which bypasses the .NET Console
        /// abstraction and writes directly to the native OS handles.
        /// </summary>
        static Logging()
        {
            Console.SetOut(new InteropTextWriter(StdOut));
            Console.SetError(new InteropTextWriter(StdErr));
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
        /// Stores the host-provided LogXEvent callback for later use.
        /// Called from CSharpExtension.SetHostCallbacks.
        /// </summary>
        /// <param name="callback">The delegate wrapping the host's LogXEvent function pointer.</param>
        public static void SetLogXEventCallback(CSharpExtension.LogXEventCallbackDelegate callback)
        {
            _logXEventCallback = callback;
        }

        /// <summary>
        /// Returns true if the host has provided a LogXEvent callback.
        /// </summary>
        public static bool HasLogXEventCallback => _logXEventCallback != null;

        /// <summary>
        /// Trace levels for events logged via the host's LogXEvent callback.
        /// Lowest numeric value is the most severe, matching Windows ETW TRACE_LEVEL_* convention.
        /// </summary>
        public enum TraceLevel : ushort
        {
            Critical    = 1,
            Error       = 2,
            Warning     = 3,
            Information = 4,
            Verbose     = 5,
        }

        /// <summary>
        /// Default name of the Extension to be used for XEvent logging when
        /// the caller does not supply one.
        /// </summary>
        private const string DefaultExtensionName = "CSharp";

        /// <summary>
        /// Logs a message through the host's XEvent infrastructure.
        /// If no host callback is registered, this is a no-op.
        /// </summary>
        /// <param name="extensionName">Extension name.</param>
        /// <param name="sessionId">Session GUID.</param>
        /// <param name="taskId">Task identifier.</param>
        /// <param name="traceLevel">Trace severity.</param>
        /// <param name="errorCode">Error code for non-informational logs.</param>
        /// <param name="message">The message to log.</param>
        public static unsafe void LogXEvent(
            string     extensionName,
            Guid       sessionId,
            ushort     taskId,
            TraceLevel traceLevel,
            int        errorCode,
            string     message)
        {
            // Snapshot the callback once so a concurrent cleanup between the null-check and the invocation
            // cannot turn this into a NullReferenceException.
            //
            CSharpExtension.LogXEventCallbackDelegate callback = _logXEventCallback;
            if (callback == null)
            {
                return;
            }

            // Ensure message is not null to avoid issues during UTF-8 encoding.
            string safeMessage = message ?? string.Empty;

            // Convert the message to a UTF-8 byte array for native interop.
            byte[] utf8MessageBytes = Encoding.UTF8.GetBytes(safeMessage);

            // Capture the real byte length.
            ulong messageLen = (ulong)utf8MessageBytes.Length;

            // As `fixed` on a zero-length array yields a null pointer,
            // validate that the byte array is not empty.
            //
            if (utf8MessageBytes.Length == 0)
            {
                utf8MessageBytes = new byte[] { 0 };
            }

            // Use the caller-supplied Extension name when non-empty,
            // otherwise fall back to the default value.
            //
            string safeExtensionName = string.IsNullOrEmpty(extensionName)
                ? DefaultExtensionName
                : extensionName;
            byte[] utf8ExtNameBytes = Encoding.UTF8.GetBytes(safeExtensionName);
            ulong  extNameLen       = (ulong)utf8ExtNameBytes.Length;

            // Call the host's LogXEvent callback with the prepared parameters.
            try
            {
                fixed (byte* pExtName = utf8ExtNameBytes)
                fixed (byte* pMessage = utf8MessageBytes)
                {
                    callback(
                        pExtName,
                        extNameLen,
                        sessionId,
                        taskId,
                        (ushort)traceLevel,
                        errorCode,
                        pMessage,
                        messageLen);
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"LogXEvent host callback threw: {ex}");
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
                    using (Stream stream = _fd == StdOut ? Console.OpenStandardOutput() : Console.OpenStandardError())
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
