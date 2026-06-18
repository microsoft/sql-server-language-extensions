//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: Common.h
//
// Purpose:
//  Common headers for .NET Core CSharpExtenion-test.
//
//*********************************************************************
#pragma once

#ifdef _WIN64
#include <windows.h>
#else
#include <dlfcn.h>
#include <limits.h>
#include <unistd.h>
#include <cstdlib>
typedef void* HINSTANCE;

// Win32-compat shims so the native test files build on Linux.
// The tests share a single loaded .so handle (sm_libHandle) and resolve
// exported entry points; on Linux these map onto the dl* / POSIX equivalents.
//
#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif

using HLOCAL = void*;

// Resolve an exported symbol from a loaded module (Linux equivalent of GetProcAddress).
//
static inline void* GetProcAddress(void* hModule, const char* lpProcName)
{
    return dlsym(hModule, lpProcName);
}

// Resolve the running executable path via /proc/self/exe (Linux equivalent of GetModuleFileName).
//
static inline unsigned long GetModuleFileName(void* /*hModule*/, char* lpFilename, unsigned long nSize)
{
    ssize_t len = readlink("/proc/self/exe", lpFilename, static_cast<size_t>(nSize) - 1);
    if (len <= 0)
    {
        lpFilename[0] = '\0';
        return 0;
    }

    lpFilename[len] = '\0';
    return static_cast<unsigned long>(len);
}

// On Linux, Marshal.AllocHGlobal maps to malloc(), so free() is the correct deallocator.
//
static inline HLOCAL LocalFree(HLOCAL hMem)
{
    free(hMem);
    return nullptr;
}
#endif

#include <iostream>
#include <cstring>
#ifdef _WIN32
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
#else
#include <filesystem>
#endif
#include <math.h>
#include <memory>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include "gtest/gtest.h"
#include "nativecsharpextension.h"
#include "CSharpExtensionApiTests.h"