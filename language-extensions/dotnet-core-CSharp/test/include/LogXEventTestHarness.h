//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: LogXEventTestHarness.h
//
// Purpose:
//  Shared test harness for capturing the host LogXEvent callbacks forwarded
//  by the managed extension. Used by the SetHostCallbacks tests and by the
//  Execute session-tagging tests so both can assert on the events - and the
//  session identity - the extension emits.
//
//*********************************************************************
#pragma once

#include "Common.h"

#include <string>
#include <vector>

namespace ExtensionApiTest
{
    // A single captured invocation of the host LogXEvent callback, including the
    // session identity the event was tagged with.
    //
    struct CapturedLogEvent
    {
        std::string  extensionName;
        SQLGUID      sessionId;
        SQLUSMALLINT taskId;
        SQLUSMALLINT traceLevel;
        SQLINTEGER   errorCode;
        std::string  message;
    };

    // Storage for events captured by TestLogXEventCallback. Tests that rely on it
    // should clear it immediately before the action they want to observe.
    //
    extern std::vector<CapturedLogEvent> g_capturedLogEvents;

    // Fully-qualified name of the managed test executor that emits an event
    // through the SDK ExtensionEventLogger from inside Execute.
    //
    extern const std::string c_logInformationExecutor;

    // Marker text CSharpTestExecutorLogInformation logs. Must stay in sync with
    // CSharpTestExecutorConstants.LogEventMessage on the managed side.
    //
    extern const std::string c_logInformationMessage;

    // Registers TestLogXEventCallback with the extension so managed log events
    // are delivered into g_capturedLogEvents. Returns the SetHostCallbacks result.
    //
    SQLRETURN RegisterTestLogXEventCallback(HINSTANCE libHandle);

    // Test stand-in for the host's LogXEvent implementation. Records each
    // invocation - including the tagged session id and task id - into
    // g_capturedLogEvents so tests can assert on its contents.
    //
    extern "C" void TestLogXEventCallback(
        const SQLCHAR *extensionName,
        SQLULEN        extensionNameLength,
        SQLGUID        sessionId,
        SQLUSMALLINT   taskId,
        SQLUSMALLINT   traceLevel,
        SQLINTEGER     errorCode,
        const SQLCHAR *message,
        SQLULEN        messageLength);

    // Returns the first captured event whose message contains the executor's
    // marker text, or nullptr when the executor's event was not forwarded.
    //
    const CapturedLogEvent *FindLogInformationEvent();

    // Byte-wise equality for two SQLGUIDs (the 16-byte layout has no padding).
    //
    bool SqlGuidEquals(const SQLGUID &lhs, const SQLGUID &rhs);
}
