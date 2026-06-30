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
    /// Severity of an event emitted via <see cref="ExtensionEventLogger"/>.
    /// Lowest numeric value is the most severe, matching the Windows ETW
    /// TRACE_LEVEL_* convention used by the host.
    /// </summary>
    public enum ExtensionTraceLevel : ushort
    {
        /// <summary>Critical failure.</summary>
        Critical = 1,

        /// <summary>Error.</summary>
        Error = 2,

        /// <summary>Warning.</summary>
        Warning = 3,

        /// <summary>Informational message.</summary>
        Information = 4,

        /// <summary>Verbose diagnostic detail.</summary>
        Verbose = 5,
    }

    /// <summary>
    /// Emits traces from a .NET Core C# extension through the host's XEvent
    /// infrastructure (fires <c>SQLExtension.extension_trace_event</c> in the
    /// ExtHost satellite, which forwards to the SQL Server engine).
    /// </summary>
    /// <remarks>
    /// Availability is host-dependent: the host must implement Extension API
    /// version 3 or later and register the <c>LogXEvent</c> callback via
    /// <c>SetHostCallbacks</c>. When the host has not registered a callback,
    /// <see cref="IsAvailable"/> is <c>false</c> and <see cref="Log"/> is a
    /// silent no-op, so callers can invoke it unconditionally.
    ///
    /// Events are attributed to the session/task currently executing on this
    /// satellite. If no session is active (before <c>InitSession</c> or after
    /// <c>CleanupSession</c>) the event is emitted with an empty session id;
    /// the host drops empty-session events before they reach the engine, so
    /// they are not visible to end-user XEvent sessions.
    /// </remarks>
    public static class ExtensionEventLogger
    {
        /// <summary>
        /// True when the host has registered an XEvent logging callback and
        /// emitted events can reach the host. When false, <see cref="Log"/>
        /// does nothing.
        /// </summary>
        public static bool IsAvailable => Logging.HasLogXEventCallback;

        /// <summary>
        /// Emit a trace through the host's XEvent infrastructure, attributed to
        /// the session/task currently executing on this satellite. Safe to call
        /// even when no host callback is registered (no-op) and never throws.
        /// </summary>
        /// <param name="level">Severity of the event.</param>
        /// <param name="message">
        /// Message text. Callers are responsible for excluding secrets and other
        /// sensitive payloads; the host records this verbatim.
        /// </param>
        /// <param name="errorCode">
        /// Optional error code associated with the event. Defaults to 0.
        /// </param>
        /// <param name="extensionName">
        /// Optional name identifying the originating extension. When null or
        /// empty the host substitutes a default.
        /// </param>
        public static void Log(
            ExtensionTraceLevel level,
            string              message,
            int                 errorCode = 0,
            string              extensionName = null)
        {
            // Snapshot the current session once so concurrent cleanup between the
            // read and the use cannot turn the field accesses into a race.
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

        /// <summary>
        /// Map the public severity to the internal logging severity. The two
        /// enums share numeric values, but mapping explicitly keeps the public
        /// API decoupled from the internal type.
        /// </summary>
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
