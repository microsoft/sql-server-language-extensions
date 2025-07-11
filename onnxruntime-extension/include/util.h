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

#include <string>

#include <iostream>
#include <fstream>
#include <string>
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

// Function to log messages with a timestamp and log level for wstring format
void LogMessage(const std::wstring& message, LogLevel level);

// Function to convert a wide string to a UTF-8 encoded string
std::string Wstring_to_String(const wchar_t* data, size_t sizeInWchar);

// Function to convert a UTF-8 encoded string to a wide string
std::wstring String_to_Wstring(const char* data, size_t sizeInChar);

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
	// @param path The file path as a wide string.
	// @return A string containing the bytes read from the file.
	static std::string LoadBytesFromFile(const std::wstring &path);

	// Checks if a file exists at the specified path.
	//
	// @param filePath The file path as a wide string.
	// @return True if the file exists, false otherwise.
	static bool DoesFileExist(const std::wstring &filePath);

	// Extracts the value of a specific key from a JSON string.
	//
	// @param input The input string to check, validation will be done to ensure it is a valid JSON string.
	// @param key The key to extract from the JSON string.
	static std::wstring ExtractJsonKey(std::wstring_view input, std::wstring_view key);
};
