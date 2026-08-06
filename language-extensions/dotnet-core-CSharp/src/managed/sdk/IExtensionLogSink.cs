//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: IExtensionLogSink.cs
//
// Purpose:
//  Abstraction that decouples the ExtensionEventLogger facade from the
//  Extension's logging implementation. The facade depends only on this
//  sink. The Extension supplies a concrete implementation and registers
//  it during load.
//
//*********************************************************************
namespace Microsoft.SqlServer.CSharpExtension.SDK
{
    /// <summary>
    /// Destination that <see cref="ExtensionEventLogger"/> forwards log events to.
    /// The SDK facade holds a reference to one of these and never reaches into the
    /// Extension's logging types directly. The Extension provides the implementation
    /// and registers it via <see cref="ExtensionEventLogger.Sink"/> when it loads.
    /// </summary>
    internal interface IExtensionLogSink
    {
        /// <summary>
        /// True when the sink is ready to deliver events (e.g. the host has registered a
        /// callback). Callers can check this to skip building expensive messages when
        /// logging is off.
        /// </summary>
        bool IsEnabled { get; }

        /// <summary>
        /// Delivers a single log event to the underlying logging implementation.
        /// </summary>
        /// <param name="level">Event severity.</param>
        /// <param name="errorCode">Error code for non-informational events; 0 when unused.</param>
        /// <param name="message">Message to log. A null message is treated as empty.</param>
        /// <param name="extensionName">
        /// Name recorded as the event's originating extension; null or empty selects the
        /// Extension's default name.
        /// </param>
        void Log(ExtensionTraceLevel level, int errorCode, string message, string extensionName);
    }
}
