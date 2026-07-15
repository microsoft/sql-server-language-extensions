//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: LogXEventTestHarness.cpp
//
// Purpose:
//  Shared storage and callback implementation for capturing the host
//  LogXEvent callbacks forwarded by the managed extension.
//
//*********************************************************************
#include "LogXEventTestHarness.h"

#include <cstring>
#include <utility>

namespace ExtensionApiTest
{
    std::vector<CapturedLogEvent> g_capturedLogEvents;

    extern const std::string c_logInformationExecutor = "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorLogInformation";

    extern const std::string c_logInformationMessage = "CSharpTestExecutorLogInformation emitted event";

    SQLRETURN RegisterTestLogXEventCallback(HINSTANCE libHandle)
    {
        typedef SQLRETURN FN_setHostCallbacks(SQLEXTENSION_HOST_CALLBACKS *);
        FN_setHostCallbacks *setHostCallbacks = reinterpret_cast<FN_setHostCallbacks *>(
            GetProcAddress(libHandle, "SetHostCallbacks"));
        if (setHostCallbacks == nullptr)
        {
            return SQL_ERROR;
        }

        SQLEXTENSION_HOST_CALLBACKS hostCallbacks{};
        hostCallbacks.Version   = SQLEXTENSION_HOST_CALLBACKS_VERSION_1;
        hostCallbacks.LogXEvent = &TestLogXEventCallback;
        return setHostCallbacks(&hostCallbacks);
    }

    extern "C" void TestLogXEventCallback(
        const SQLCHAR *extensionName,
        SQLULEN        extensionNameLength,
        SQLGUID        sessionId,
        SQLUSMALLINT   taskId,
        SQLUSMALLINT   traceLevel,
        SQLINTEGER     errorCode,
        const SQLCHAR *message,
        SQLULEN        messageLength)
    {
        CapturedLogEvent ev{};
        if (extensionName != nullptr && extensionNameLength > 0)
        {
            ev.extensionName.assign(
                reinterpret_cast<const char *>(extensionName),
                static_cast<size_t>(extensionNameLength));
        }
        ev.sessionId  = sessionId;
        ev.taskId     = taskId;
        ev.traceLevel = traceLevel;
        ev.errorCode  = errorCode;
        if (message != nullptr && messageLength > 0)
        {
            ev.message.assign(
                reinterpret_cast<const char *>(message),
                static_cast<size_t>(messageLength));
        }
        g_capturedLogEvents.push_back(std::move(ev));
    }

    const CapturedLogEvent *FindLogInformationEvent()
    {
        for (const CapturedLogEvent &ev : g_capturedLogEvents)
        {
            if (ev.message.find(c_logInformationMessage) != std::string::npos)
            {
                return &ev;
            }
        }
        return nullptr;
    }

    bool SqlGuidEquals(const SQLGUID &lhs, const SQLGUID &rhs)
    {
        return std::memcmp(&lhs, &rhs, sizeof(SQLGUID)) == 0;
    }
}
