//**************************************************************************************************
// ONNXRuntime-extension : A language extension implementing the SQL Server AIRuntime extension 
// for ONNX Runtime.
// Copyright (C) 2025 Microsoft Corporation.
//
// @File: util.cpp
//
// Purpose:
//  This file defines the helper methods used by the OnnxSession class.
//
//**************************************************************************************************

#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>
#include <iostream>
#include <windows.h>
#include "nlohmann/json.hpp"
#include "util.h"

using namespace std;

// Function to get the current timestamp in a specific format
// This function is used for logging purposes to provide a timestamp for each log entry.
std::string GetCurrentTimestamp() {
	auto now = std::chrono::system_clock::now();
	auto time_t_now = std::chrono::system_clock::to_time_t(now);
	std::tm tm_now;
#ifdef _WIN32
	localtime_s(&tm_now, &time_t_now);
#else
	localtime_r(&time_t_now, &tm_now);
#endif
	std::ostringstream oss;
	oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
	return oss.str();
}

// Function to convert log level enum to string
// This function is used to convert the log level enum to a string representation for logging.
std::string LogLevelToString(LogLevel level) {
	switch (level) {
		case LogLevel::INFO: return "INFO";
		case LogLevel::WARNING: return "WARNING";
		case LogLevel::CRITICAL_ERROR: return "ERROR";
		default: return "UNKNOWN";
	}
}

// Function to log messages with a timestamp and log level
// Usually this would show up in the SQL Server logs ExtensibilityData
// e.g. (`C:\Program Files\Microsoft SQL Server\MSSQL17.MSSQLSERVER\MSSQL\ExtensibilityData\AppContainer1\<sessionId>.txt`)
void LogMessage(const std::string& message, LogLevel level) {
	std::string timestamp = GetCurrentTimestamp();
	std::string levelStr = LogLevelToString(level);
	std::string logEntry = "[" + timestamp + "] [" + levelStr + "] " + message;

	// Print to console
	std::cout << logEntry << std::endl;
}

// Function to log messages with a timestamp and log level for wstring format
void LogMessage(const std::wstring& message, LogLevel level) {
	// Convert string to wstring for logging using String_to_Wstring
	std::string currentTime = GetCurrentTimestamp();
	std::wstring timestamp = String_to_Wstring(currentTime.data(), currentTime.size());
	std::wstring levelStr = String_to_Wstring(LogLevelToString(level).data(), LogLevelToString(level).size());
	std::wstring logEntry = L"[" + timestamp + L"] [" + levelStr + L"] " + message;

	// Print to console
	std::wcout << logEntry << std::endl;
}

// Function to convert a wide string to a UTF-8 encoded string
// This implementation uses Windows API to convert a wide string to a UTF-8 encoded string.
std::string Wstring_to_String(const wchar_t* data, size_t sizeInWchar) {
	if (sizeInWchar == 0 || data == nullptr) return {};
	int utf8_size = WideCharToMultiByte(CP_UTF8, 0, data, static_cast<int>(sizeInWchar), nullptr, 0, nullptr, nullptr);
	if (utf8_size == 0) return {};
	std::string result(utf8_size, '\0');
	WideCharToMultiByte(CP_UTF8, 0, data, static_cast<int>(sizeInWchar), result.data(), utf8_size, nullptr, nullptr);
	return result;
}

// Function to convert a UTF-8 encoded string to a wide string
// This implementation uses Windows API to convert a UTF-8 encoded string to a wide string.
std::wstring String_to_Wstring(const char* data, size_t sizeInChar) {
	if (sizeInChar == 0 || data == nullptr) return {};
	int size = MultiByteToWideChar(CP_UTF8, 0, data, static_cast<int>(sizeInChar), nullptr, 0);
	if (size == 0) return {};
	std::wstring result(size, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, data, static_cast<int>(sizeInChar), result.data(), size);
	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: OnnxRuntimeExtensionHelpers
//
// Description:
// Function to convert a floating-point array to a wide string
//
std::wstring
OnnxRuntimeExtensionHelpers::ConvertFloatsToWideString(const float *arr, int size)
{
	wostringstream woss; // Use wostringstream for wide string concatenation

	woss.precision(6); // Set precision to 6 decimal places (adjust as needed)
	woss << fixed;	   // Use fixed-point notation for consistent formatting

	for (int i = 0; i < size; ++i)
	{
		woss << arr[i]; // Append the float to the stream
		if (i < size - 1)
		{
			woss << L","; // Add a comma if it's not the last element
		}
	}

	return woss.str(); // Convert the stream to a wide string
}

//--------------------------------------------------------------------------------------------------
// Name: LoadBytesFromFile
//
// Description:
//	Utility function to load file into a string
//
string
OnnxRuntimeExtensionHelpers::LoadBytesFromFile(const wstring &path)
{
	ifstream file(path, ios::binary | ios::ate);
	if (!file)
	{
		throw runtime_error("Failed to open file: " + Wstring_to_String(path.data(), path.size()));
	}
	streamsize size = file.tellg();
	if (size <= 0) {
		throw runtime_error("File is empty or failed to determine file size: " + Wstring_to_String(path.data(), path.size()));
	}

	file.seekg(0, ios::beg);

	string buffer(size, '\0');
	if (!file.read(buffer.data(), size))
	{
		throw runtime_error("Failed to read file: " + Wstring_to_String(path.data(), path.size()));
	}

	return buffer;
}

//--------------------------------------------------------------------------------------------------
// Name: DoesFileExist
//
// Description:
//	Utility function to check if a file exists at the specified path.
//
bool
OnnxRuntimeExtensionHelpers::DoesFileExist(const std::wstring &filePath)
{
	return std::filesystem::exists(filePath);
}

//--------------------------------------------------------------------------------------------------
// Name: ExtractJsonKey
//
// Description:
//	Given an input string, check if its valid json string and extract the key
//
// Returns:
//  A wide string containing the value of the specified key if it exists and is a string
//	If the key does not exist or the input is not a valid JSON string, it returns an empty wide string.
//
std::wstring
OnnxRuntimeExtensionHelpers::ExtractJsonKey(std::wstring_view input, std::wstring_view key)
{
	try
	{
		// Handle case where input or key is empty
		if (input.empty() || key.empty())
		{
			return L"";
		}

		// convert wstring_view to string for JSON parsing
		std::string inputStr = Wstring_to_String(input.data(), input.size());
		std::string keyStr = Wstring_to_String(key.data(), key.size());

		nlohmann::json jsonObject = nlohmann::json::parse(inputStr);
		if (jsonObject.contains(keyStr) && jsonObject[keyStr].is_string())
		{
			// Convert the extracted string value back to wstring
			{
				std::string valueStr = jsonObject[keyStr].get<std::string>();
				return String_to_Wstring(valueStr.data(), valueStr.size());
			}
		}
	}
	catch (const nlohmann::json::parse_error &e)
	{
		LogMessage("JSON parsing error: " + string(e.what()), LogLevel::CRITICAL_ERROR);
	}
	catch (const exception &e)
	{
		LogMessage("Standard exception: " + string(e.what()), LogLevel::CRITICAL_ERROR);
		throw;
	}

	// Return empty string if key not found or parsing fails
	// This is intentionally returning empty string to indicate absence of the key.
	return L"";
}