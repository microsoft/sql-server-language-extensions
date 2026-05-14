//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: Logger.cpp
//
// Purpose:
//  Wrapper class around logging to standardize logging messages and errors.
//
//*********************************************************************
#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <mutex>
#include "Logger.h"

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#endif

using namespace std;

namespace
{
    // Diagnostic log file path. /tmp is writable in the sandbox; the file
    // gets persisted in the sandbox dir if /tmp is bind-mounted. We also
    // write to stderr so output is captured in launchpadd-stderr.
    constexpr const char *DIAG_LOG_PATH = "/tmp/csharpext-diag.log";

    std::mutex g_log_mutex;

    // Get a timestamp + pid prefix for log lines.
    std::string get_log_prefix()
    {
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count() % 1000;
        std::tm tm_buf;
#ifdef _WIN32
        gmtime_s(&tm_buf, &t);
#else
        gmtime_r(&t, &tm_buf);
#endif
        char buf[64];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
        std::ostringstream oss;
        oss << buf << "." << ms;
#ifndef _WIN32
        oss << " pid=" << getpid();
#endif
        oss << " CSharpExt: ";
        return oss.str();
    }

    // Write the message to stderr AND to the diagnostic file.
    // Best-effort: never throw, never block on file errors.
    void write_diag(const std::string &line)
    {
        std::lock_guard<std::mutex> lock(g_log_mutex);
        // stderr goes to launchpadd-stderr in the SQL container.
        std::cerr << line << std::endl;
        std::cerr.flush();
#ifndef _WIN32
        // Also append to a file under /tmp so we can find it even if
        // stderr is consumed/redirected by the satellite host.
        try
        {
            std::ofstream f(DIAG_LOG_PATH, std::ios::app);
            if (f.is_open())
            {
                f << line << std::endl;
            }
        }
        catch (...)
        {
            // ignore
        }
#endif
    }
}

#ifndef _WIN32
// Constructor function: runs the moment dlopen() loads this .so successfully.
// If we ever see this line in /tmp/csharpext-diag.log, we know exthost
// succeeded loading the extension. Absence of this line proves dlopen failed.
__attribute__((constructor))
static void csharp_extension_loaded()
{
    write_diag(get_log_prefix() + "[ctor] libnativecsharpextension.so LOADED via dlopen");
}

__attribute__((destructor))
static void csharp_extension_unloaded()
{
    write_diag(get_log_prefix() + "[dtor] libnativecsharpextension.so being unloaded");
}
#endif

//--------------------------------------------------------------------------------------------------
// Name: Logger::Log
//
// Description:
//  Logs a message to stdout.
//
void Logger::Log(const string &msg)
{
#ifdef DEBUG
    cout << msg <<endl;
#endif
}

//--------------------------------------------------------------------------------------------------
// Name: Logger::LogError
//
// Description:
//  Logs an error to stderr (and to /tmp/csharpext-diag.log on Linux for
//  postmortem retrieval — exthost may swallow stderr).
//
void Logger::LogError(const string &errorMsg)
{
#ifdef _WIN32
    cerr << "Error: " << errorMsg <<endl;
#else
    write_diag(get_log_prefix() + errorMsg);
#endif
}
