//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: ExtensionEventLogger.cs
//
// Purpose:
//  Provides session-aware XEvent logging for .NET Core C# extensions.
//
//*********************************************************************
using System;

namespace Microsoft.SqlServer.CSharpExtension.SDK
{
    /// <summary>
    /// Severity of an extension trace event.
    /// </summary>
    public enum ExtensionTraceLevel : ushort
    {
        Critical = 1,
        Error = 2,
        Warning = 3,
        Information = 4,
        Verbose = 5,
    }

    /// <summary>
    /// Emits session-attributed traces through the host's XEvent infrastructure.
    /// </summary>
    /// <remarks>
    /// <see cref="Log"/> does nothing when no host callback is registered. Events
    /// emitted without an active session are not visible to end users.
    /// </remarks>
    public static class ExtensionEventLogger
    {
        /// <summary>
        /// Gets whether the host accepts extension trace events.
        /// </summary>
        public static bool IsAvailable => Logging.HasLogXEventCallback;

        /// <summary>
        /// Emits a trace attributed to the current session and task.
        /// </summary>
        /// <param name="level">Severity of the event.</param>
        /// <param name="message">Message recorded verbatim; callers must exclude secrets.</param>
        /// <param name="errorCode">Optional error code.</param>
        /// <param name="extensionName">Optional originating extension name.</param>
        public static void Log(
            ExtensionTraceLevel level,
            string              message,
            int                 errorCode = 0,
            string              extensionName = null)
        {
            CSharpSession session = CSharpExtension.CurrentSession;

            Guid   sessionId = session?.SessionId ?? Guid.Empty;
            ushort taskId    = session?.TaskId ?? 0;

            Logging.LogXEvent(
                extensionName,
                sessionId,
                taskId,
                ToLoggingTraceLevel(level),
                errorCode,
                message);
        }

        private static Logging.TraceLevel ToLoggingTraceLevel(ExtensionTraceLevel level)
        {
            switch (level)
            {
                case ExtensionTraceLevel.Critical:    return Logging.TraceLevel.Critical;
                case ExtensionTraceLevel.Error:       return Logging.TraceLevel.Error;
                case ExtensionTraceLevel.Warning:     return Logging.TraceLevel.Warning;
                case ExtensionTraceLevel.Verbose:     return Logging.TraceLevel.Verbose;
                case ExtensionTraceLevel.Information:  return Logging.TraceLevel.Information;
                default:                              return Logging.TraceLevel.Information;
            }
        }
    }
}
