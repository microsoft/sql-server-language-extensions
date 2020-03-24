//*************************************************************************************************
// RExtension : A language extension implementing the SQL Server
// external language communication protocol for R.
// Copyright (C) 2019 Microsoft Corporation.
//
// This file is part of RExtension.
//
// RExtension is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RExtension is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RExtension.  If not, see <https://www.gnu.org/licenses/>.
//
// @File: Logger.cpp
//
// Purpose:
// Wrapper class around logging to standardize logging messages
// and errors.
//
//*************************************************************************************************
#ifdef _WIN64
#include <windows.h>
#endif
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <string>

#include "Common.h"
#include "Logger.h"

#include <R_ext/Error.h>
#include <R_ext/Print.h>

#define TIMESTAMP_LENGTH 35
#define NANOSECONDS_IN_MILLISECOND 1000000

using namespace std;

char Logger::timestampBuffer[TIMESTAMP_LENGTH] = { 0 };

//---------------------------------------------------------------------
// Name: LogError
//
// Description:
//  Log an error to stderr with format "TIMESTAMP Error: <message>".
//
void Logger::LogError(const string &errorMsg)
{
	string errorMsgWithTimestamp = string(GetCurrentTimestamp()) + "Error: " + errorMsg + "\n";
	REprintf(errorMsgWithTimestamp.c_str());
}

//---------------------------------------------------------------------
// Name: LogException
//
// Description:
//  Log a c++ exception to stderr with format "TIMESTAMP Exception
//  occurred: <message>".
//
void Logger::LogException(const exception &e)
{
	string exceptionMsgWithTimestamp = string(GetCurrentTimestamp()) + "Exception occurred: " + e.what() + "\n";
	REprintf(exceptionMsgWithTimestamp.c_str());
}

//---------------------------------------------------------------------
// Name: LogRException
//
// Description:
//  Log an R exception to stderr with format "TIMESTAMP Exception
//  thrown in R: <message>".
//
void Logger::LogRException(const string &exceptionMsg)
{
	string exceptionMsgWithTimestamp = string(GetCurrentTimestamp()) + "Exception occurred in R: " + exceptionMsg + "\n";
	REprintf(exceptionMsgWithTimestamp.c_str());
}

//---------------------------------------------------------------------
// Name: Log
//
// Description:
//  Log a message to stdout with format "TIMESTAMP <message>".
//
void Logger::Log(const string &msg)
{
#if defined(_DEBUG) || defined(_VERBOSE)
	string msgWithTimestamp = string(GetCurrentTimestamp()) + msg + "\n";
	Rprintf(msgWithTimestamp.c_str());
#endif
}

//---------------------------------------------------------------------
// Name: LogRVariable
//
// Description:
//  Print out the value of the given variable name using R's print function.
//  Only available in debug mode, not in release.
//  This function has a precondition that R's environment is already initialized,
//  otherwise it is a no-op.
//
void Logger::LogRVariable(const string &name)
{
#if defined(_DEBUG) || defined(_VERBOSE)
	if (g_embeddedRPtr != nullptr)
	{
		string printVariable = "print(" + name + ");";
		(*g_embeddedRPtr).parseEval(printVariable);
	}
#endif
}

//---------------------------------------------------------------------
// Name: GetCurrentTimestamp
//
// Description:
//  Gets the current system time and format it to the SQL log format
//  (Year-Month-Day Hour:Minute:Second.Millisecond).
//
const char* Logger::GetCurrentTimestamp()
{
#ifdef _WIN64
	SYSTEMTIME sysTime;

	GetLocalTime(&sysTime);

	sprintf_s(timestampBuffer, TIMESTAMP_LENGTH,
		"%04d-%02d-%02d %02d:%02d:%02d.%02d\t",
		sysTime.wYear,
		sysTime.wMonth,
		sysTime.wDay,
		sysTime.wHour,
		sysTime.wMinute,
		sysTime.wSecond,
		sysTime.wMilliseconds / 10);

	return timestampBuffer;
#else
	timespec ts;
	char buffer[TIMESTAMP_LENGTH] = { 0 };

	clock_gettime(CLOCK_REALTIME, &ts);

	// Convert the time to Year-Month-Day Hour:Minute:Second
	//
	strftime(buffer, sizeof(buffer), "%F %T", gmtime(&ts.tv_sec));

	// Append the milliseconds and the tab to the timestamp
	//
	long ms = (ts.tv_nsec / NANOSECONDS_IN_MILLISECOND);

	// SQL outputs milliseconds in decimal with precision of 2, divide
	// by 10 to remove the last digit.
	//
	ms /= 10;

	// Append the milliseconds and the tab to the timestamp
	//
	sprintf(timestampBuffer, "%s.%02li\t", buffer, ms);

	return timestampBuffer;
#endif
}
