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

#include <vector>
#include <unordered_map>
#include <cctype>
#include <fstream>
#include <sstream>
#include <locale>
#include <iostream>
#if defined(_WIN32)
#include <windows.h>
#else
#include <iconv.h>
#include <errno.h>
#include <cstring>
#endif
#include <filesystem>
#include "nlohmann/json.hpp"
#include "util.h"

using namespace std;

// For Linux, the output buffer is sized using UTF8_EXPANSION_FACTOR to handle worst-case expansion from wchar_t to UTF-8.
// Maximum bytes per wchar_t when converting to UTF-8
constexpr size_t UTF8_EXPANSION_FACTOR = 4;

// For Linux, the output buffer is sized using WCHAR_EXPANSION_FACTOR to handle worst-case expansion from UTF-8 to wchar_t.
// Maximum wchar_t per UTF-8 byte (conservative)
constexpr size_t WCHAR_EXPANSION_FACTOR = 2;

// For Linux, the output buffer is sized using UTF8_FROM_UTF16_EXPANSION_FACTOR to handle worst-case expansion from UTF-16 to UTF-8.
// Maximum bytes per UTF-16 code unit when converting to UTF-8
constexpr size_t UTF8_FROM_UTF16_EXPANSION_FACTOR = 2;

// For Linux, the output buffer is sized using UTF16_FROM_UTF32_EXPANSION_FACTOR to handle worst-case expansion from UTF-32 to UTF-16.
// Maximum bytes per wchar_t for UTF-16 output
constexpr size_t UTF16_FROM_UTF32_EXPANSION_FACTOR = 2;

// Precision for floating-point to string conversion
// Number of decimal places for float to wstring conversion
constexpr int FLOAT_TO_WSTRING_PRECISION = 6;

// Function to get the current timestamp in a specific format
// This function is used for logging purposes to provide a timestamp for each log entry.
std::string GetCurrentTimestamp()
{
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
std::string LogLevelToString(LogLevel level)
{
	switch (level)
	{
	case LogLevel::INFO:
		return "INFO";
	case LogLevel::WARNING:
		return "WARNING";
	case LogLevel::CRITICAL_ERROR:
		return "ERROR";
	default:
		return "UNKNOWN";
	}
}

// Function to log messages with a timestamp and log level
// Usually this would show up in the SQL Server logs ExtensibilityData
// e.g. (`C:\Program Files\Microsoft SQL Server\MSSQL17.MSSQLSERVER\MSSQL\ExtensibilityData\AppContainer1\<sessionId>.txt`)
void LogMessage(const std::string &message, LogLevel level)
{
	std::string timestamp = GetCurrentTimestamp();
	std::string levelStr = LogLevelToString(level);
	std::string logEntry = "[" + timestamp + "] [" + levelStr + "] " + message;

	// Print to console
	std::cout << logEntry << std::endl;
}

// Function to log messages with a timestamp and log level for wstring format
void LogMessage(const std::wstring &message, LogLevel level)
{
	// Convert string to wstring for logging using String_to_Wstring
	std::string currentTime = GetCurrentTimestamp();
	std::wstring timestamp = String_to_Wstring(currentTime.data(), currentTime.size());
	std::wstring levelStr = String_to_Wstring(LogLevelToString(level).data(), LogLevelToString(level).size());
	std::wstring logEntry = L"[" + timestamp + L"] [" + levelStr + L"] " + message;

	// Print to console
	std::wcout << logEntry << std::endl;
}

// Allocates a new char buffer, copies the string, and ensures null-termination.
std::unique_ptr<char[]> ConvertAndAllocateUtf8Buffer(const std::string &src)
{
	auto newPtr = std::make_unique<char[]>(src.size() + 1);
#ifdef _WIN32
	memcpy_s(newPtr.get(), (src.size() + 1) * sizeof(char), src.data(), src.size());
#else
	memcpy(newPtr.get(), src.data(), src.size());
#endif
	newPtr[src.size()] = '\0';
	return newPtr;
}

// Converts a wide string to a UTF-8 encoded std::string.
// On Windows, uses the Windows API (WideCharToMultiByte). On Linux, uses iconv for conversion.
std::string Wstring_to_String(const wchar_t *data, size_t sizeInWchar)
{
	if (sizeInWchar == 0 || data == nullptr)
		return {};
#if defined(_WIN32)
	int utf8_size = WideCharToMultiByte(CP_UTF8, 0, data, static_cast<int>(sizeInWchar), nullptr, 0, nullptr, nullptr);
	if (utf8_size == 0)
		return {};
	std::string result(utf8_size, '\0');
	WideCharToMultiByte(CP_UTF8, 0, data, static_cast<int>(sizeInWchar), result.data(), utf8_size, nullptr, nullptr);
	return result;
#else
	iconv_t cd = iconv_open("UTF-8", "WCHAR_T");
	if (cd == (iconv_t)-1)
	{
		LogMessage("iconv_open failed in Wstring_to_String", LogLevel::CRITICAL_ERROR);
		return {};
	}
	
	try
	{
		size_t inbytes = sizeInWchar * sizeof(wchar_t);
		// UTF8_EXPANSION_FACTOR is used to ensure the output buffer is large enough for worst-case UTF-8 expansion
		size_t outbytes = inbytes * UTF8_EXPANSION_FACTOR; // UTF8_EXPANSION_FACTOR = 4
		size_t original_outbytes = outbytes;  // Preserve original value for size calculation
		char *inbuf = (char *)data;
		
		std::string out(outbytes, '\0');
		char *outbuf = &out[0];
		size_t result = iconv(cd, &inbuf, &inbytes, &outbuf, &outbytes);
		
		if (result == (size_t)-1)
		{
			iconv_close(cd);
			LogMessage("iconv conversion failed in Wstring_to_String", LogLevel::CRITICAL_ERROR);
			return {};
		}
		if (result > 0)
		{
			LogMessage("Warning: " + std::to_string(result) + " characters had irreversible conversions in Wstring_to_String", LogLevel::WARNING);
			// Continue processing - partial conversion may still be useful
		}
		out.resize(original_outbytes - outbytes);
		iconv_close(cd);
		return out;
	}
	catch (...)
	{
		iconv_close(cd);
		LogMessage("Exception occurred during Wstring_to_String conversion", LogLevel::CRITICAL_ERROR);
		throw;
	}
#endif
}

// Function to convert a UTF-8 encoded string to a wide string
// On Windows, uses MultiByteToWideChar. On Linux, uses iconv for conversion.
// For Linux, the output buffer is sized using WCHAR_EXPANSION_FACTOR to handle worst-case expansion from UTF-8 to wchar_t.
std::wstring String_to_Wstring(const char *data, size_t sizeInChar)
{
	if (sizeInChar == 0 || data == nullptr)
		return {};
#if defined(_WIN32)
	int size = MultiByteToWideChar(CP_UTF8, 0, data, static_cast<int>(sizeInChar), nullptr, 0);
	if (size == 0)
		return {};
	std::wstring result(size, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, data, static_cast<int>(sizeInChar), result.data(), size);
	return result;
#else
	iconv_t cd = iconv_open("WCHAR_T", "UTF-8");
	if (cd == (iconv_t)-1)
	{
		LogMessage("iconv_open failed in String_to_Wstring", LogLevel::CRITICAL_ERROR);
		return {};
	}
	
	try
	{
		size_t inbytes = sizeInChar;
		// WCHAR_EXPANSION_FACTOR is used to ensure the output buffer is large enough for worst-case expansion
		size_t outbytes = inbytes * sizeof(wchar_t) * WCHAR_EXPANSION_FACTOR; // WCHAR_EXPANSION_FACTOR = 2
		size_t original_outbytes = outbytes;  // Preserve original value for size calculation
		char *inbuf = (char *)data;
		
		std::wstring out(outbytes / sizeof(wchar_t), L'\0');
		char *outbuf = (char *)out.data();
		size_t result = iconv(cd, &inbuf, &inbytes, &outbuf, &outbytes);
		
		if (result == (size_t)-1)
		{
			iconv_close(cd);
			LogMessage("iconv conversion failed in String_to_Wstring", LogLevel::CRITICAL_ERROR);
			return {};
		}
		if (result > 0)
		{
			LogMessage("Warning: " + std::to_string(result) + " characters had irreversible conversions in String_to_Wstring", LogLevel::WARNING);
			// Continue processing - partial conversion may still be useful
		}
		
		size_t outchars = (original_outbytes - outbytes) / sizeof(wchar_t);
		out.resize(outchars);
		iconv_close(cd);
		return out;
	}
	catch (...)
	{
		iconv_close(cd);
		LogMessage("Exception occurred during String_to_Wstring conversion", LogLevel::CRITICAL_ERROR);
		throw;
	}
#endif
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

	woss.precision(FLOAT_TO_WSTRING_PRECISION); // Set precision using constant, FLOAT_TO_WSTRING_PRECISION = 6
	woss << fixed;								// Use fixed-point notation for consistent formatting

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
OnnxRuntimeExtensionHelpers::LoadBytesFromFile(const string &path)
{
#if defined(_WIN32)
	// Convert UTF-8 string path to wstring for Windows API
	std::wstring wpath = String_to_Wstring(path.data(), path.size());
	std::ifstream file(wpath, ios::binary | ios::ate);
#else
	std::ifstream file(path, ios::binary | ios::ate);
#endif
	if (!file)
	{
		throw runtime_error("Failed to open file: " + path);
	}
	streamsize size = file.tellg();
	if (size <= 0)
	{
		throw runtime_error("File is empty or failed to determine file size: " + path);
	}

	file.seekg(0, ios::beg);

	string buffer(size, '\0');
	if (!file.read(buffer.data(), size))
	{
		throw runtime_error("Failed to read file: " + path);
	}

	return buffer;
}

//--------------------------------------------------------------------------------------------------
// Name: DoesFileExist
//
// Description:
//  Utility function to check if a file exists at the specified UTF-8 path.
//
bool OnnxRuntimeExtensionHelpers::DoesFileExist(const std::string &filePath)
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
//  A string containing the value of the specified key if it exists and is a string
//	If the key does not exist or the input is not a valid JSON string, it returns an empty string.
//
std::string
OnnxRuntimeExtensionHelpers::ExtractJsonKey(std::string_view input, std::string_view key)
{
	try
	{
		// Handle case where input or key is empty
		if (input.empty() || key.empty())
		{
			return "";
		}

		nlohmann::json jsonObject = nlohmann::json::parse(input);
		if (jsonObject.contains(key) && jsonObject[key].is_string())
		{
			// Convert the extracted string value back to wstring
			{
				std::string valueStr = jsonObject[key].get<std::string>();
				return valueStr;
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
	return "";
}

// Utility to convert WCHAR* (UTF-16, platform-dependent) to UTF-8 std::string
// On Windows, WCHAR is wchar_t (UTF-16). On Linux, WCHAR is 2 bytes unsigned short (compatible with UTF-16LE).
std::string WCHAR_to_utf8_string(const WCHAR *src, size_t len)
{
	if (!src || len == 0)
		return {};
#if defined(_WIN32) || defined(_WIN64)
	// On Windows, WCHAR is wchar_t (UTF-16)
	return Wstring_to_String(reinterpret_cast<const wchar_t *>(src), len);
#else
	// On Linux, WCHAR is unsigned short (UTF-16) - using iconv for conversion
	iconv_t cd = iconv_open("UTF-8", "UTF-16LE");
	if (cd == (iconv_t)-1)
	{
		LogMessage("iconv_open failed in WCHAR_to_utf8_string", LogLevel::CRITICAL_ERROR);
		return {};
	}
	
	try
	{
		size_t inbytes = len * sizeof(uint16_t);
		// UTF8_FROM_UTF16_EXPANSION_FACTOR is used to ensure the output buffer is large enough for worst-case expansion
		size_t outbytes = inbytes * UTF8_FROM_UTF16_EXPANSION_FACTOR;  // UTF8_FROM_UTF16_EXPANSION_FACTOR = 2 
		size_t original_outbytes = outbytes;  // Preserve original value for size calculation
		char *inbuf = (char *)src;
		
		std::string out(outbytes, '\0');
		char *outbuf = &out[0];
		size_t result = iconv(cd, &inbuf, &inbytes, &outbuf, &outbytes);
		
		if (result == (size_t)-1)
		{
			iconv_close(cd);
			LogMessage("iconv conversion failed in WCHAR_to_utf8_string", LogLevel::CRITICAL_ERROR);
			return {};
		}
		if (result > 0)
		{
			LogMessage("Warning: " + std::to_string(result) + " characters had irreversible conversions in WCHAR_to_utf8_string", LogLevel::WARNING);
			// Continue processing - partial conversion may still be useful
		}
		out.resize(original_outbytes - outbytes);
		iconv_close(cd);
		return out;
	}
	catch (...)
	{
		iconv_close(cd);
		LogMessage("Exception occurred during WCHAR_to_utf8_string conversion", LogLevel::CRITICAL_ERROR);
		throw;
	}
#endif
}

// Utility to convert std::wstring (UTF-32 on Linux) to std::u16string (UTF-16)
// Uses iconv for robust conversion on non-Windows systems.
// On Windows, wstring is already UTF-16, so this conversion utility is used only for linux.
std::u16string ConvertUTF32ToUTF16(const std::wstring &utf32_str)
{
#if defined(_WIN32)
	// On Windows, wstring is already UTF-16
	return std::u16string(reinterpret_cast<const char16_t *>(utf32_str.data()), utf32_str.size());
#else
	if (utf32_str.empty())
		return {};
	iconv_t cd = iconv_open("UTF-16LE", "WCHAR_T");
	if (cd == (iconv_t)-1)
	{
		LogMessage("iconv_open failed in ConvertUTF32ToUTF16", LogLevel::CRITICAL_ERROR);
		return {};
	}
	
	try
	{
		size_t inbytes = utf32_str.size() * sizeof(wchar_t);
		size_t outbytes = inbytes * UTF16_FROM_UTF32_EXPANSION_FACTOR;
		size_t original_outbytes = outbytes;  // Preserve original value for size calculation
		char *inbuf = (char *)utf32_str.data();
		
		std::u16string out(outbytes / 2, 0);   // divide by 2 for UTF-16 since each character is 2 bytes
		char *outbuf = (char *)out.data();
		size_t result = iconv(cd, &inbuf, &inbytes, &outbuf, &outbytes);
		
		if (result == (size_t)-1)
		{
			iconv_close(cd);
			LogMessage("iconv conversion failed in ConvertUTF32ToUTF16", LogLevel::CRITICAL_ERROR);
			return {};
		}
		if (result > 0)
		{
			LogMessage("Warning: " + std::to_string(result) + " characters had irreversible conversions in ConvertUTF32ToUTF16", LogLevel::WARNING);
			// Continue processing - partial conversion may still be useful
		}
		
		size_t outchars = (original_outbytes - outbytes) / 2;
		out.resize(outchars);
		iconv_close(cd);
		return out;
	}
	catch (...)
	{
		iconv_close(cd);
		LogMessage("Exception occurred during ConvertUTF32ToUTF16", LogLevel::CRITICAL_ERROR);
		throw;
	}
#endif
}