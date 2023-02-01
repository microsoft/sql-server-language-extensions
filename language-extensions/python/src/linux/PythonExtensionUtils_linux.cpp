//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonExtensionUtils_linux.cpp
//
// Purpose:
//  Linux specific utility functions for Python Extension
//
//*************************************************************************************************

#include <dlfcn.h>
#include <experimental/filesystem>

#include "Logger.h"
#include "PythonExtensionUtils.h"

namespace fs = std::experimental::filesystem;

const CHAR *GuidFormat = "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X";

//--------------------------------------------------------------------------------------------------
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
	char* envVarValue;
	envVarValue = getenv(envVarName.c_str());

	if (envVarValue == NULL)
	{
		throw std::runtime_error("Error while loading " + envVarName);
	}

	return std::string(envVarValue);
}

//--------------------------------------------------------------------------------------------------
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

	snprintf(guidString, sizeof(guidString) / sizeof(guidString[0]),
		GuidFormat,
		static_cast<unsigned long>(guid->Data1), guid->Data2, guid->Data3,
		guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
		guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);

	std::string s(guidString);

	return s;
}

//--------------------------------------------------------------------------------------------------
// Name: PythonExtensionUtils::FreeDLL
//
// Description:
//  Close an open dll handle
//
void PythonExtensionUtils::FreeDLL(void *pDll)
{
	if (pDll != nullptr)
	{
		dlclose(pDll);
	}
}

//--------------------------------------------------------------------------------------------------
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
	fs::path pathToPython = fs::path("python3.10");
	return pathToPython.string();
}
