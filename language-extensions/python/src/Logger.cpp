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

#define TIMESTAMP_LENGTH 35
using namespace std;

char Logger::sm_timestampBuffer[TIMESTAMP_LENGTH] = { 0 };

//-------------------------------------------------------------------------------------------------
// Name: LogError
//
// Description:
//  Log an error to stderr with format "TIMESTAMP Error: <message>".
//
void Logger::LogError(const string &errorMsg)
{
	cerr << GetCurrentTimestamp() << "Error: " << errorMsg << endl;
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
	cerr << GetCurrentTimestamp() << "Exception occurred: " << e.what() << endl;
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
	cout << GetCurrentTimestamp() << msg << endl;
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
