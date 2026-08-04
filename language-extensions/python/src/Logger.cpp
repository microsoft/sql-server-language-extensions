//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: Logger.cpp
//
// Purpose:
//  Logging functions for extension
//
//*************************************************************************************************

#include "Logger.h"

#include <cstdio>

#define TIMESTAMP_LENGTH 35
using namespace std;

char Logger::sm_timestampBuffer[TIMESTAMP_LENGTH] = { 0 };

//-------------------------------------------------------------------------------------------------
// Name: LogError
//
// Description:
//  Log an error to stderr with format "TIMESTAMP Error: <message>".
//
// Note:
//  These write through stdio rather than std::cerr/std::cout on purpose. The extension is compiled
//  with GCC 13 but is loaded into hosts whose libstdc++ is older - RHEL 9 ships
//  libstdc++.so.6.0.29 and Ubuntu 22.04 ships 6.0.30 - and the stream insertion operators are
//  inlined from the GCC 13 headers into this library, so they execute against stream objects owned
//  by the older runtime. That combination segfaults inside libstdc++ on the very first log call,
//  which is why the Python satellite core-dumped on those two distros while passing on Ubuntu
//  24.04, RHEL 10 and AzureLinux 3. The symbol-version ABI gate cannot catch this: every symbol we
//  reference is within GLIBCXX_3.4.29, the problem is inlined code depending on newer internals.
//  stdio has no such coupling. EKM and the ONNX extension were fixed the same way.
//
void Logger::LogError(const string &errorMsg)
{
	// fwrite, not fprintf("%s"): errorMsg can carry an embedded NUL (it may be built from a
	// customer-supplied ZIP entry name via bp::extract<string>, which uses the Python length
	// rather than strlen). "%s" would stop at the NUL and silently drop the rest, whereas the
	// std::cerr insertion this replaced wrote all size() bytes.
	//
	const string timestamp = GetCurrentTimestamp();
	fwrite(timestamp.data(), 1, timestamp.size(), stderr);
	fputs("Error: ", stderr);
	fwrite(errorMsg.data(), 1, errorMsg.size(), stderr);
	fputc('\n', stderr);
	fflush(stderr);
}

//-------------------------------------------------------------------------------------------------
// Name: LogException
//
// Description:
//  Log a c++ exception to stderr with format "TIMESTAMP Exception
//  occurred: <message>".
//
void Logger::LogException(const exception &e)
{
	fprintf(stderr, "%sException occurred: %s\n", GetCurrentTimestamp().c_str(), e.what());
	fflush(stderr);
}

//-------------------------------------------------------------------------------------------------
// Name: Log
//
// Description:
//  Log a message to stdout with format "TIMESTAMP <message>".
//
void Logger::Log(const string &msg)
{
#if defined(_DEBUG)
	// See LogError: fwrite preserves embedded NULs that "%s" would truncate at.
	//
	const string timestamp = GetCurrentTimestamp();
	fwrite(timestamp.data(), 1, timestamp.size(), stdout);
	fwrite(msg.data(), 1, msg.size(), stdout);
	fputc('\n', stdout);
	fflush(stdout);
#endif
}

//-------------------------------------------------------------------------------------------------
// Name: GetCurrentTimestamp
//
// Description:
//  Gets the current system time and format it to the SQL log format
//  (Year-Month-Day Hour:Minute:Second.Millisecond).
//
const std::string Logger::GetCurrentTimestamp()
{
#if defined ( _MSC_VER )
	SYSTEMTIME sysTime;

	GetLocalTime(&sysTime);

	sprintf_s(sm_timestampBuffer, TIMESTAMP_LENGTH,
		"%04d-%02d-%02d %02d:%02d:%02d.%02d\t",
		sysTime.wYear,
		sysTime.wMonth,
		sysTime.wDay,
		sysTime.wHour,
		sysTime.wMinute,
		sysTime.wSecond,
		sysTime.wMilliseconds / 10);

	return sm_timestampBuffer;
#else
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];

	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S.000\t", &tstruct); // No millisecond support
	return buf;
#endif
}
