//*************************************************************************************************
// RExtension : A language extension implementing the SQL Server
// external language communication protocol for R.
// Copyright (C) 2019 Microsoft Corporation.
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
// @File: Utilities_win.cpp
//
// Purpose:
//  Utility functions implemented for the windows platform.
//
//*************************************************************************************************

#include <stdlib.h>
#include <string.h>
#include "Common.h"

using namespace std;

//-------------------------------------------------------------------------------------------------
// Name: GetAndLogLastError
//
// Description:
//  Gets the last error and logs the message with the error in hex format. Windows only.
//
// Returns:
//  The last error obtained.
//
int GetAndLogLastError(const string msg)
{
	DWORD dwError = GetLastError();
	string errorHex;
	errorHex.resize(8); // hex is max size 8 bytes
	snprintf(&errorHex[0], errorHex.size() + 1, "%08lX", dwError);
	LOG_ERROR(msg + ": 0x" + errorHex);

	return dwError;
}

//-------------------------------------------------------------------------------------------------
// Name: Utilities::GetEnvVariable
//
// Description:
//  Gets the value of the given environment variable name.
//
// Returns:
//  String value of the environment variable requested.
//
string Utilities::GetEnvVariable(const string &envVarName)
{
	string envVarValue = "";

	DWORD result;

	// First call to get the length of the environment variable
	//
	result = GetEnvironmentVariableA(envVarName.c_str(), nullptr, 0);

	// If result is 0, there was an error.
	// Check GetLastError for the exact code.
	//
	if (result == 0)
	{
		GetAndLogLastError("Error while finding length of environment variable " + envVarName);
	}
	else
	{
		// Resize the return string to the length returned by GetEnvironmentVariableA,
		// minus null terminator because strings implicitly have null terminator
		//
		envVarValue.resize(result - 1);

		// Second call gets the actual environment variable
		//
		result = GetEnvironmentVariableA(envVarName.c_str(), &envVarValue[0], result);

		if (result == 0)
		{
			GetAndLogLastError("Error while getting the environment variable " + envVarName);
		}
	}

	return envVarValue;
}

//-------------------------------------------------------------------------------------------------
// Name: Utilities::SetEnvVariable
//
// Description:
//  Sets the environment variable name to the specified value
//
// Returns:
//  Returns 0 if it the result from SetEnvironmentVariableA indicates success, otherwise returns
//  the last error.
//
int Utilities::SetEnvVariable(const string &envVarName, const string &value)
{
	DWORD dwError = 0;

	// Set the environment variable; a non-zero result here represents success,
	// whereas a zero result from SetEnvironmentVariableA indicates failure.
	//
	BOOL result = SetEnvironmentVariableA(envVarName.c_str(), value.c_str());
	if (result == 0)
	{
		dwError = GetAndLogLastError("Error while setting the environment variable "
			+ envVarName + " to value " + value);
	}

	return result != 0 ? 0 : dwError;
}
