//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: Logger.cpp
//
// Purpose:
//  Implementation of platform dependent logging for the java
//  extension.
//
//*********************************************************************
#include "Logger.h"
#include <iostream>
#include <ctime>

#define TIMESTAMP_LENGTH 35

using namespace std;

char Logger::timestampBuffer[TIMESTAMP_LENGTH] = {0};

//---------------------------------------------------------------------
// Name: GetCurrentTimestamp
//
// Description:
//  Gets the current system time and format it to the SQL log format
//  (Year-Month-Day Hour:Minute:Second.Millisecond).
//
const char* Logger::GetCurrentTimestamp()
{
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
}

