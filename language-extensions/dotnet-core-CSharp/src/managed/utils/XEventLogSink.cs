//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: XEventLogSink.cs
//
// Purpose:
//  Extension-side implementation of IExtensionLogSink that forwards events
//  from the SDK's ExtensionEventLogger facade to the host's XEvent
//  infrastructure. This is the only place that couples the SDK logging
//  facade to the Extension's Logging implementation.
//
//*********************************************************************
using Microsoft.SqlServer.CSharpExtension.SDK;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// Bridges <see cref="ExtensionEventLogger"/> to <see cref="Logging"/>. Registered on the
    /// facade during <see cref="CSharpExtension.SetHostCallbacks"/> so that events emitted
    /// through the SDK facade are routed to the host's XEvent callback, tagged with the session
    /// that is currently executing.
    /// </summary>
    internal sealed class XEventLogSink : IExtensionLogSink
    {
        /// <inheritdoc/>
        public bool IsEnabled => Logging.HasLogXEventCallback;

        /// <inheritdoc/>
        public void Log(ExtensionTraceLevel level, int errorCode, string message, string extensionName) =>
            Logging.LogXEventFromCurrentSession(level, errorCode, message, extensionName);
    }
}
