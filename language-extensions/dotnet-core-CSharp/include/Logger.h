//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: Logger.h
//
// Purpose:
//  Wrapper class around logging to standardize logging messages and errors.
//
//*********************************************************************
#include <string>
#include <iostream>

using namespace std;
#define LOG(msg) Logger::Log(msg)
#define LOG_ERROR(msg) Logger::LogError(msg)

class Logger
{
public:
    // Logs a message to stdout.
    //
    static void Log(const string &msg);

    // Logs an error to stderr
    //
    static void LogError(const string &errorMsg);
};
