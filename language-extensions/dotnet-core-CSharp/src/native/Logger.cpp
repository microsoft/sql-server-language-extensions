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
#include <stdio.h>
#include <windows.h>
#include "Logger.h"

using namespace std;

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
//  Logs an error to stderr
//
void Logger::LogError(const string &errorMsg)
{
    cerr << "Error: " << errorMsg <<endl;
}
