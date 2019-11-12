//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: Logger.cpp
//
// Purpose:
//  Implementation of Linux platform dependent logging for the java
//  extension.
//
//*********************************************************************
#include "Logger.h"
#include <ctime>
#include <cstdio>

#define TIMESTAMP_LENGTH 35
#define NANOSECONDS_IN_MILLISECOND 1000000

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
	timespec ts;
	char buffer[TIMESTAMP_LENGTH] = {0};

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
}

