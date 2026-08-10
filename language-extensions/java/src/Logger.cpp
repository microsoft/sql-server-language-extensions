//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: Logger.cpp
//
// Purpose:
//  Implementation of logging for the java extension.
//
//*********************************************************************
#include "Logger.h"
#include <cstdio>
#include <ctime>

using namespace std;

//---------------------------------------------------------------------
// Name: LogError
//
// Description:
//  Log an error to stderr with format "TIMESTAMP Error: <message>".
//
void Logger::LogError(const string &errorMsg)
{
	fputs(GetCurrentTimestamp(), stderr);
	fputs("Error: ", stderr);
	fwrite(errorMsg.data(), 1, errorMsg.size(), stderr);
	fputc('\n', stderr);
	fflush(stderr);
}

//---------------------------------------------------------------------
// Name: LogJavaException
//
// Description:
//  Log a c++ exception to stderr with format "TIMESTAMP Exception
//  occurred: <message>".
//
void Logger::LogException(const exception &e)
{
	fputs(GetCurrentTimestamp(), stderr);
	fputs("Exception occurred: ", stderr);
	fputs(e.what(), stderr);
	fputc('\n', stderr);
	fflush(stderr);
}

//---------------------------------------------------------------------
// Name: LogJavaException
//
// Description:
//  Log a java exception to stderr with format "TIMESTAMP Exception
//  thrown in Java: <message>".
//
void Logger::LogJavaException(const string &exceptionMsg)
{
	fputs(GetCurrentTimestamp(), stderr);
	fputs("Exception occurred in Java: ", stderr);
	fwrite(exceptionMsg.data(), 1, exceptionMsg.size(), stderr);
	fputc('\n', stderr);
	fflush(stderr);
}

//---------------------------------------------------------------------
// Name: Log
//
// Description:
//  Log a message to stdout with format "TIMESTAMP <message>".
//
void Logger::Log(const string &msg)
{
#ifdef DEBUG
	fputs(GetCurrentTimestamp(), stdout);
	fwrite(msg.data(), 1, msg.size(), stdout);
	fputc('\n', stdout);
	fflush(stdout);
#endif
}
