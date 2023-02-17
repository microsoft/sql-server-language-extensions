//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonExtensionUtils_win.cpp
//
// Purpose:
//  Windows specific utility functions for Python Extension
//
//*************************************************************************************************

#ifdef _WIN64
	#include <filesystem>
#else
	#include <experimental/filesystem>
#endif

#include "Logger.h"
#include "PythonExtensionUtils.h"

namespace fs = std::filesystem;

const CHAR *GuidFormat = "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X";

//-------------------------------------------------------------------------------------------------
// Name: PythonExtensionUtils::GetEnvVariable
//
// Description:
//  Get the value of an environment variable
//
// Returns:
//  String value of the environment variable requested
//
std::string PythonExtensionUtils::GetEnvVariable(const std::string &envVarName)
{
	std::string envVarValue;

	DWORD result;

	// First call to get the length of the environment variable
	//
	result = GetEnvironmentVariableA(envVarName.c_str(), nullptr, 0);

	// If result is 0, there was an error.
	// Check GetLastError for the exact code.
	//
	if (result == 0)
	{
		DWORD dwError = GetLastError();
		if (dwError == ERROR_ENVVAR_NOT_FOUND)
		{
			throw std::runtime_error("Error: could not find environment variable " + envVarName);
		}
		else
		{
			std::string errorHex;
			errorHex.resize(8); // hex is max size 8 bytes
			sprintf_s(&errorHex[0], errorHex.size() + 1, "%08X", dwError);
			throw std::runtime_error("Error while loading " + envVarName + ": 0x" + errorHex);
		}
	}

	// Resize the return string to the length returned by GetEnvironmentVariableA,
	// minus null terminator because strings implicitly have null terminator
	//
	envVarValue.resize(result - 1);

	// Second call gets the actual environment variable
	//
	result = GetEnvironmentVariableA(envVarName.c_str(), &envVarValue[0], result);

	if (result == 0)
	{
		DWORD dwError = GetLastError();
		std::string errorHex;
		errorHex.resize(8); // hex is max size 8 bytes
		sprintf_s(&errorHex[0], errorHex.size() + 1, "%08X", dwError);
		throw std::runtime_error("Error while loading " + envVarName + ": 0x" + errorHex);
	}

	return envVarValue;
}

//-------------------------------------------------------------------------------------------------
// Name: PythonExtensionUtils::ConvertGuidToString
//
// Description:
//  Converts a SQLGUID to a string
//
// Returns:
//  string of the guid
//
std::string PythonExtensionUtils::ConvertGuidToString(const SQLGUID *guid)
{
	// 32 hex chars + 4 hyphens + null terminator, so 37 characters.
	//
	char guidString[37];

	sprintf_s(guidString, sizeof(guidString) / sizeof(guidString[0]),
		GuidFormat,
		guid->Data1, guid->Data2, guid->Data3,
		guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
		guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);

	std::string s(guidString);
	return s;
}

//-------------------------------------------------------------------------------------------------
// Name: PythonExtensionUtils::FreeDLL
//
// Description:
//  Close an open dll handle
//
void PythonExtensionUtils::FreeDLL(void *pDll)
{
	if (static_cast<HMODULE>(pDll) != nullptr)
	{
		FreeLibrary(static_cast<HMODULE>(pDll));
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonExtensionUtils::GetEnvVariable
//
// Description:
//  Get the path to the python executable
//
// Returns:
//  String value of the python executable path
//
std::string PythonExtensionUtils::GetPathToPython()
{
	std::string pythonhome = GetEnvVariable("PYTHONHOME");
	fs::path pathToPython = fs::absolute(pythonhome) / "python.exe";

	std::string pathString = NormalizePathString(pathToPython.string());

	return pathString;
}
