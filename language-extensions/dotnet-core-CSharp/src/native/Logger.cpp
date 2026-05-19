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
#include <fcntl.h>
#include <cstdio>
#endif

using namespace std;

namespace
{
    // Diagnostic log file paths - try multiple locations so at least one
    // gets captured by the test infrastructure.
    //
    // CRITICAL: SQL Linux extension runs INSIDE a sandboxed namespace where:
    // - /var/opt/mssql/log is NOT mounted (invisible to satellite uid)
    // - /var/opt/mssql-extensibility/log is NOT bind-mounted (invisible)
    // - The only writable host-visible paths are:
    //     /tmp                                                   (tmpfs in ns)
    //     /home/mssql_satellite/externallanguagessandboxpath     (bind-mounted, ro for libs)
    //     /home/mssql_satellite/externallanguagessandboxtemppath (bind-mounted, may be rw)
    //     /home/mssql_satellite                                  (the sandbox HOME)
    //
    // For PVS test capture (looks at host /var/opt/mssql/log/), we use a
    // trick: write to a directory THAT IS bind-mounted, where the host
    // checkinstallextensibility.sh has pre-created a symlink targeting
    // /var/opt/mssql/log/csharpext-diag.log.
    //
    // The simplest reliable path is /tmp (tmpfs inside the satellite ns -
    // doesn't survive the session but proves the .so was loaded).
    // We also try /home/mssql_satellite which is the satellite uid's home.
    static const char* const DIAG_LOG_PATHS[] = {
        "/tmp/csharpext-diag.log",
        "/home/mssql_satellite/csharpext-diag.log",
        "/var/opt/mssql/log/csharpext-diag.log",
        "/var/opt/mssql-extensibility/log/csharpext-diag.log",
        nullptr
    };

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

    // Write the message to stderr AND to the diagnostic file(s).
    // Best-effort: never throw, never block on file errors.
    void write_diag(const std::string &line)
    {
        std::lock_guard<std::mutex> lock(g_log_mutex);
        // stderr goes to launchpadd-stderr in the SQL container.
        std::cerr << line << std::endl;
        std::cerr.flush();
#ifndef _WIN32
        // Try writing to ALL diagnostic paths so at least one is captured by
        // PVS test attachments. Each is best-effort - silently ignore failures.
        for (const char* const* p = DIAG_LOG_PATHS; *p != nullptr; ++p)
        {
            try
            {
                std::ofstream f(*p, std::ios::app);
                if (f.is_open())
                {
                    f << line << std::endl;
                }
            }
            catch (...)
            {
                // ignore - try next path
            }
        }
#endif
    }
}

#ifndef _WIN32
// Highest-priority constructor (101 - lowest user-allowed number = runs first).
// Bypasses get_log_prefix() (which uses C++ iostreams, chrono, etc) to avoid
// any dependency on global initialization order. Uses pure POSIX syscalls.
// This is the FIRST evidence we have that the .so was actually loaded by exthost.
__attribute__((constructor(101)))
static void csharp_extension_loaded_early()
{
    const char *paths[] = {
        "/tmp/csharpext-diag.log",
        "/home/mssql_satellite/csharpext-diag.log",
        "/var/opt/mssql/log/csharpext-diag.log",
        "/var/opt/mssql-extensibility/log/csharpext-diag.log",
        nullptr
    };
    char buf[512];
    int n = snprintf(buf, sizeof(buf),
        "[ctor-early] libnativecsharpextension.so LOADED pid=%d uid=%d euid=%d gid=%d\n",
        (int)getpid(), (int)getuid(), (int)geteuid(), (int)getgid());
    if (n < 0) return;
    // stderr - direct write, bypassing any iostreams init
    if (write(STDERR_FILENO, buf, (size_t)n) < 0) { /* ignore */ }
    // Then files (best-effort)
    for (const char* const* p = paths; *p != nullptr; ++p)
    {
        int fd = open(*p, O_WRONLY | O_APPEND | O_CREAT, 0666);
        if (fd >= 0)
        {
            if (write(fd, buf, (size_t)n) < 0) { /* ignore */ }
            close(fd);
        }
    }
}

// Higher-level constructor: runs after C++ globals are initialized.
// Uses Logger.LogError (which uses iostreams) so test outputs are consistent.
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
