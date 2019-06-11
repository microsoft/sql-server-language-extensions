//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: Logger.h
// @Owner: brnieb
//
// Purpose:
// Wrapper class around logging to standardize logging messages
// and errors.
//
//*********************************************************************
#pragma once

#ifdef DEBUG
#define LOG(msg) Logger::Log(msg)
#else
#define LOG(msg)
#endif
#define LOG_ERROR(msg) Logger::LogError(msg)
#define LOG_EXCEPTION(e) Logger::LogException(e)
#define LOG_JAVA_EXCEPTION(msg) Logger::LogJavaException(msg)

class Logger
{
public:
	// Log an error to stderr
	//
	static void LogError(_In_ const std::string &errorMsg);

	// Log a extension exception to stderr
	//
	static void LogException(_In_ const std::exception &e);

	// Log a java exception to stderr
	//
	static void LogJavaException(_In_ const std::string &errorMsg);

	// Log a message to stdout
	//
	static void Log(_In_ const std::string &errorMsg);

private:
	// Get a string of the current timestamp in the same format
	// of SQL format
	//
	static const char* GetCurrentTimestamp();

	// Buffer to hold the timestamp string.
	//
	static char timestampBuffer[];
};