//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: ExtensionEventLogger.cs
//
// Purpose:
//  Public, session-aware logging facade for .NET Core C# extensions. Lets an
//  extension emit a trace through the host's Extended Events (XEvent)
//  infrastructure so end users can observe extension activity from an XEvent
//  session, without the extension needing the session/task identifiers or the
//  internal host-callback plumbing.
//
//*********************************************************************
using System;

namespace Microsoft.SqlServer.CSharpExtension.SDK
{
    /// <summary>
    /// Severity of an event emitted via <see cref="ExtensionEventLogger"/>. Lowest
    /// value is most severe, matching the host's ETW TRACE_LEVEL_* convention.
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
    /// Emits traces from a C# extension through the host's XEvent infrastructure
    /// (fires <c>SQLExtension.extension_trace_event</c>, forwarded to the engine).
    /// </summary>
    /// <remarks>
    /// Safe to call unconditionally: <see cref="Log"/> is a silent no-op when the
    /// host has registered no callback (see <see cref="IsAvailable"/>). Events are
    /// attributed to the session executing on this satellite; when no session is
    /// active the host drops the event, so it is not visible to end users.
    /// </remarks>
    public static class ExtensionEventLogger
    {
        /// <summary>
        /// True when the host has registered a callback and emitted events can
        /// reach it. When false, <see cref="Log"/> does nothing.
        /// </summary>
        public static bool IsAvailable => Logging.HasLogXEventCallback;

        /// <summary>
        /// Emit a trace through the host's XEvent infrastructure, attributed to the
        /// current session/task. Safe to call with no host callback (no-op); never throws.
        /// </summary>
        /// <param name="level">Severity of the event.</param>
        /// <param name="message">Message text; callers must exclude secrets — recorded verbatim.</param>
        /// <param name="errorCode">Optional error code. Defaults to 0.</param>
        /// <param name="extensionName">Optional originating extension name; host substitutes a default when empty.</param>
        public static void Log(
            ExtensionTraceLevel level,
            string              message,
            int                 errorCode = 0,
            string              extensionName = null)
        {
            // Snapshot once so a concurrent cleanup can't race the field reads.
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

        // Map to the internal severity by name to keep the public enum decoupled.
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
