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
#include <iostream>
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
	cerr << GetCurrentTimestamp() << "Error: " << errorMsg << endl;
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
	cerr << GetCurrentTimestamp() << "Exception occurred: " << e.what() << endl;
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
	cerr << GetCurrentTimestamp() << "Exception occurred in Java: " << exceptionMsg << endl;
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
	cout << GetCurrentTimestamp() << std::this_thread::get_id() << msg << endl;
#endif
}
