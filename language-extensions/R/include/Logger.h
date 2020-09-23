//**************************************************************************************************
// RExtension : A language extension implementing the SQL Server
// external language communication protocol for R.
// Copyright (C) 2020 Microsoft Corporation.
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
// @File: Logger.h
//
// Purpose:
// Wrapper class around logging to standardize logging messages and errors.
//
//**************************************************************************************************

#pragma once

#define LOG(msg) Logger::Log(msg)
#define LOG_ERROR(msg) Logger::LogError(msg)
#define LOG_EXCEPTION(e) Logger::LogException(e)

class Logger
{
public:

	// Logs an error to stderr
	//
	static void LogError(const std::string &errorMsg);

	// Logs an extension exception to stderr
	//
	static void LogException(const std::exception &e);

	// Logs a message to stdout
	//
	static void Log(const std::string &errorMsg);

	// Logs an R variable using R's print function
	//
	static void LogRVariable(const std::string &name);

private:

	// Logs the message to stderr
	//
	static void LogToStdErr(const std::string &errorMsgWithTimestamp);

	// Gets a string of the current timestamp in the same format
	// of SQL format
	//
	static const char* GetCurrentTimestamp();

	// Buffer to hold the timestamp string.
	//
	static char sm_timestampBuffer[];
};
