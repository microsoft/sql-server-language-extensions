//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpSetHostCallbacksTests.cpp
//
// Purpose:
//  Tests the optional SetHostCallbacks API exported by the .NET Core C#
//  extension. Verifies symbol resolution, null-pointer handling, and that
//  a non-null LogXEvent callback supplied via SQLEXTENSION_HOST_CALLBACKS
//  is invoked by managed code (callback registration + forwarding).
//
//*********************************************************************
#include "CSharpExtensionApiTests.h"

#include <cstring>
#include <string>
#include <vector>

using namespace std;

namespace ExtensionApiTest
{
    typedef SQLRETURN FN_setHostCallbacks(SQLEXTENSION_HOST_CALLBACKS *);

    namespace
    {
        // Captured invocation of the host LogXEvent callback.
        //
        struct CapturedLogEvent
        {
            string       extensionName;
            SQLUSMALLINT traceLevel;
            SQLINTEGER   errorCode;
            string       message;
        };

        // File-scope storage for events captured by TestLogXEventCallback.
        // Cleared at the start of each test that uses it.
        //
        static vector<CapturedLogEvent> g_capturedLogEvents;

        // Test stand-in for host's LogXEvent implementation. Records the
        // invocation so the test can assert on its contents.
        //
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
            CapturedLogEvent ev;
            if (extensionName != nullptr && extensionNameLength > 0)
            {
                ev.extensionName.assign(
                    reinterpret_cast<const char *>(extensionName),
                    static_cast<size_t>(extensionNameLength));
            }
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
    }

#define RESOLVE_SET_HOST_CALLBACKS() \
    reinterpret_cast<FN_setHostCallbacks *>( \
        GetProcAddress(sm_libHandle, "SetHostCallbacks"))

    //----------------------------------------------------------------------------------------------
    // Name: SetHostCallbacks_SymbolIsExported
    //
    // Description:
    //  Verifies the optional SetHostCallbacks entry point is exported from
    //  nativecsharpextension so host can discover it via GetProcAddress.
    //
    TEST_F(CSharpExtensionApiTests, SetHostCallbacks_SymbolIsExported)
    {
        FN_setHostCallbacks *fn = RESOLVE_SET_HOST_CALLBACKS();
        EXPECT_NE(fn, nullptr)
            << "SetHostCallbacks must be exported by the extension binary";
    }

    //----------------------------------------------------------------------------------------------
    // Name: SetHostCallbacks_NullPointerReturnsError
    //
    // Description:
    //  Passing a null SQLEXTENSION_HOST_CALLBACKS* must be rejected with
    //  SQL_ERROR rather than crashing the host.
    //
    TEST_F(CSharpExtensionApiTests, SetHostCallbacks_NullPointerReturnsError)
    {
        FN_setHostCallbacks *fn = RESOLVE_SET_HOST_CALLBACKS();
        ASSERT_NE(fn, nullptr);

        SQLRETURN rc = fn(nullptr);
        EXPECT_EQ(rc, SQL_ERROR);
    }

    //----------------------------------------------------------------------------------------------
    // Name: SetHostCallbacks_RegistersAndForwardsLogXEvent
    //
    // Description:
    //  Provides a non-null LogXEvent callback and verifies that managed
    //  SetHostCallbacks registers it and forwards an invocation back through
    //  it (the managed implementation emits an "extension loaded" XEvent
    //  immediately after registration), proving end-to-end callback wiring.
    //
    TEST_F(CSharpExtensionApiTests, SetHostCallbacks_RegistersAndForwardsLogXEvent)
    {
        FN_setHostCallbacks *fn = RESOLVE_SET_HOST_CALLBACKS();
        ASSERT_NE(fn, nullptr);

        g_capturedLogEvents.clear();

        SQLEXTENSION_HOST_CALLBACKS hostCallbacks{};
        hostCallbacks.Version    = SQLEXTENSION_HOST_CALLBACKS_VERSION_1;
        hostCallbacks.LogXEvent  = &TestLogXEventCallback;

        SQLRETURN rc = fn(&hostCallbacks);
        EXPECT_EQ(rc, SQL_SUCCESS);

        ASSERT_FALSE(g_capturedLogEvents.empty())
            << "Managed SetHostCallbacks should invoke the host LogXEvent "
               "callback at least once after registration";

        const CapturedLogEvent &ev = g_capturedLogEvents.front();
        EXPECT_EQ(ev.extensionName, string("CSharp"));
        EXPECT_EQ(ev.traceLevel, static_cast<SQLUSMALLINT>(Extension_Information));
        EXPECT_EQ(ev.errorCode, 0);
        EXPECT_NE(ev.message.find("CSharp extension loaded"), string::npos)
            << "Unexpected message: " << ev.message;
    }

    //----------------------------------------------------------------------------------------------
    // Name: SetHostCallbacks_NullLogXEventIsAccepted
    //
    // Description:
    //  A callbacks struct with a null LogXEvent slot is valid (host opted out
    //  of logging). SetHostCallbacks must succeed and must not invoke any
    //  callback.
    //
    TEST_F(CSharpExtensionApiTests, SetHostCallbacks_NullLogXEventIsAccepted)
    {
        FN_setHostCallbacks *fn = RESOLVE_SET_HOST_CALLBACKS();
        ASSERT_NE(fn, nullptr);

        g_capturedLogEvents.clear();

        SQLEXTENSION_HOST_CALLBACKS hostCallbacks{};
        hostCallbacks.Version   = SQLEXTENSION_HOST_CALLBACKS_VERSION_1;
        hostCallbacks.LogXEvent = nullptr;

        SQLRETURN rc = fn(&hostCallbacks);
        EXPECT_EQ(rc, SQL_SUCCESS);
        EXPECT_TRUE(g_capturedLogEvents.empty());
    }
}
