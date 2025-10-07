//**************************************************************************************************
// ONNXRuntime-extension : A language extension implementing the SQL Server AIRuntime extension 
// for ONNX Runtime.
// Copyright (C) 2025 Microsoft Corporation.
//
// @File: util.h
//
// Purpose:
//  This file contains utility functions for generating embeddings using ONNX Runtime.
//
//**************************************************************************************************

#pragma once

#include "sqlexternallanguage.h"

#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>

enum class LogLevel {
	INFO,
	WARNING,
	CRITICAL_ERROR
};

// Function to get the current timestamp in a specific format
std::string GetCurrentTimestamp();

// Function to convert log level enum to string
std::string LogLevelToString(LogLevel level);

// Function to log messages with a timestamp and log level
void LogMessage(const std::string& message, LogLevel level);

void LogMessage(const std::wstring& message, LogLevel level);

// Logs a vector of numeric values (int, float, etc.) as a comma-separated list in square brackets.
template <typename T>
void LogVerboseVector(const std::wstring& label, const std::vector<T>& vec, LogLevel level) {
	std::wstringstream ss;
	ss << label << L"[";
	for (size_t i = 0; i < vec.size(); ++i) {
		ss << vec[i];
		if (i + 1 < vec.size()) ss << L", ";
	}
	ss << L"]";
	LogMessage(ss.str(), level);
}

// Allocates a new char buffer, copies the string, and ensures null-termination.
std::unique_ptr<char[]> ConvertAndAllocateUtf8Buffer(const std::string& src);

// Function to convert a wide string to a UTF-8 encoded string
std::string Wstring_to_String(const wchar_t* data, size_t sizeInWchar);

// Function to convert a UTF-8 encoded string to a wide string
std::wstring String_to_Wstring(const char* data, size_t sizeInChar);

// Utility to convert WCHAR* (UTF-16, platform-dependent) to UTF-8 std::string
std::string WCHAR_to_utf8_string(const WCHAR* src, size_t len);

// Utility to convert std::wstring (UTF-32 on Linux) to std::u16string (UTF-16)
std::u16string ConvertUTF32ToUTF16(const std::wstring& utf32_str);

//--------------------------------------------------------------------------------------------------
// Name: OnnxRuntimeExtensionHelpers
//
// Description:
//  A utility class providing helper functions for ONNX Runtime extensions.
//  This class contains static methods for various utility operations such as
//  converting arrays of floats to wide strings, loading bytes from files, and
//  checking file existence.
//
class OnnxRuntimeExtensionHelpers
{
public:
	// Converts an array of floats to a wide string representation.
	//
	// @param arr Pointer to the array of floats to be converted.
	// @param size The number of elements in the array.
	// @return A wide string representation of the float array.
	static std::wstring ConvertFloatsToWideString(const float *arr, int size);

	// Loads the contents of a file as a string of bytes.
	//
	// @param path The file path as std::string.
	// @return A string containing the bytes read from the file.
	static std::string LoadBytesFromFile(const std::string &path);

	// Checks if a file exists at the specified path.
	//
	// @param filePath The file path as std::string.
	// @return True if the file exists, false otherwise.
	static bool DoesFileExist(const std::string &filePath);

	// Extracts the value of a specific key from a JSON string.
	//
	// @param input The input string to check, validation will be done to ensure it is a valid JSON string.
	// @param key The key to extract from the JSON string.
	static std::string ExtractJsonKey(std::string_view input, std::string_view key);
};
