//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: Logger.h
//
// Purpose:
//  Logging functions for extension
//
//*************************************************************************************************

#pragma once
#include "Common.h"

#define LOG(msg) Logger::Log(msg)
#define LOG_ERROR(msg) Logger::LogError(msg)
#define LOG_EXCEPTION(e) Logger::LogException(e)

class Logger
{
public:
	// Log an error to stderr
	//
	static void LogError(const std::string &errorMsg);

	// Log a extension exception to stderr
	//
	static void LogException(const std::exception &e);

	// Log a message to stdout
	//
	static void Log(const std::string &msg);

private:
	// Get a string of the current timestamp in the same format
	// of SQL format
	//
	static const std::string GetCurrentTimestamp();

	// Buffer to hold the timestamp string.
	//
	static char sm_timestampBuffer[];
};
