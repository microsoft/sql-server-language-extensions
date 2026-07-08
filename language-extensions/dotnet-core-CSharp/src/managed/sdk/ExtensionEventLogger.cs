//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: ExtensionEventLogger.cs
//
// Purpose:
//  Public SDK logging facade that lets user scripts and the external
//  libraries they load in-process emit events through the host's
//  XEvent callback.
//
//*********************************************************************
namespace Microsoft.SqlServer.CSharpExtension.SDK
{
    /// <summary>
    /// Severity of an event logged through <see cref="ExtensionEventLogger"/>.
    /// Mirrors the Windows ETW TRACE_LEVEL_* convention where the lowest numeric
    /// value is the most severe.
    /// </summary>
    public enum ExtensionTraceLevel : ushort
    {
        /// <summary>Fatal condition. The operation cannot continue.</summary>
        Critical    = 1,

        /// <summary>A recoverable error occurred.</summary>
        Error       = 2,

        /// <summary>An unexpected but non-fatal condition.</summary>
        Warning     = 3,

        /// <summary>Informational progress or status message.</summary>
        Information = 4,

        /// <summary>Detailed diagnostic message for troubleshooting.</summary>
        Verbose     = 5,
    }

    /// <summary>
    /// Logging facade available to user executors and to any external library they load
    /// in-process. Messages are forwarded to the host through the XEvent callback
    /// the host registers when the extension is loaded, and are tagged with the session that
    /// is currently executing so they become visible in XEvent sessions.
    /// </summary>
    public static class ExtensionEventLogger
    {
        /// <summary>
        /// True when the host has registered an XEvent callback and log calls will be
        /// delivered. Check this to skip building expensive messages when logging is off.
        /// </summary>
        public static bool IsEnabled => Logging.HasLogXEventCallback;

        /// <summary>Logs a critical-severity event.</summary>
        /// <param name="message">Message to log.</param>
        /// <param name="errorCode">Optional error code associated with the event.</param>
        public static void LogCritical(string message, int errorCode = 0) =>
            Log(ExtensionTraceLevel.Critical, message, errorCode);

        /// <summary>Logs an error-severity event.</summary>
        /// <param name="message">Message to log.</param>
        /// <param name="errorCode">Optional error code associated with the event.</param>
        public static void LogError(string message, int errorCode = 0) =>
            Log(ExtensionTraceLevel.Error, message, errorCode);

        /// <summary>Logs a warning-severity event.</summary>
        /// <param name="message">Message to log.</param>
        public static void LogWarning(string message) =>
            Log(ExtensionTraceLevel.Warning, message);

        /// <summary>Logs an informational event.</summary>
        /// <param name="message">Message to log.</param>
        public static void LogInformation(string message) =>
            Log(ExtensionTraceLevel.Information, message);

        /// <summary>Logs a verbose/diagnostic event.</summary>
        /// <param name="message">Message to log.</param>
        public static void LogVerbose(string message) =>
            Log(ExtensionTraceLevel.Verbose, message);

        /// <summary>
        /// Logs an event at the specified severity.
        /// </summary>
        /// <param name="level">Event severity.</param>
        /// <param name="message">Message to log. A null message is treated as empty.</param>
        /// <param name="errorCode">Optional error code for non-informational events.</param>
        public static void Log(ExtensionTraceLevel level, string message, int errorCode = 0)
        {
            Logging.LogXEventFromCurrentSession(level, errorCode, message);
        }
    }
}
