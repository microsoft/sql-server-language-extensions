//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: Logger.cpp
// @Owner: brnieb
//
// Purpose:
//	 Implementation of logging for the java extension.
//
//*********************************************************************
#ifdef _WIN64
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <ctime>
#ifndef _WIN64
// These sal include headers must follow the standard c++ headers, or there
// will be compilation issues. This is because headers like iostream/algorithm use
// variables like __in which are the same as a SAL annotation causing redefinition issues.
//
#include <sal_def.h>
#include <xplat_sal.h>
#endif
#include "Logger.h"

using namespace std;

//---------------------------------------------------------------------
// Name: LogError
//
// Description:
//  Log an error to stderr with format "TIMESTAMP Error: <message>".
//
void Logger::LogError(_In_ const string &errorMsg)
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
void Logger::LogException(_In_ const exception &e)
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
void Logger::LogJavaException(_In_ const string &exceptionMsg)
{
	cerr << GetCurrentTimestamp() << "Exception occurred in Java: " << exceptionMsg << endl;
}

//---------------------------------------------------------------------
// Name: Log
//
// Description:
//  Log a message to stdout with format "TIMESTAMP <message>".
//
void Logger::Log(_In_ const string &msg)
{
#ifndef GOLDEN_BITS
	cout << GetCurrentTimestamp() << msg << endl;
#endif
}