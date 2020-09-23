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
// @File: Utilities_linux.cpp
//
// Purpose:
//  Utility functions implemented for the linux platform.
//
//**************************************************************************************************

#include "Common.h"

using namespace std;

//--------------------------------------------------------------------------------------------------
// Name: Utilities::GetEnvVariable
//
// Description:
//  Gets the value of the given environment variable name and if logError is true, logs an error
//  message if the variable is not found.
//
// Returns:
//  String value of the environment variable requested.
//
string Utilities::GetEnvVariable(const string &envVarName, bool logError)
{
	string envVarValueString;
	char * envVarValue = getenv(envVarName.c_str());
	if (envVarValue != nullptr)
	{
		envVarValueString = string(const_cast<const char*>(envVarValue),
			strlen(envVarValue));
	}
	else if (logError)
	{
		LOG_ERROR("Did not find the environment variable " + envVarName);
	}

	return envVarValueString;
}

//--------------------------------------------------------------------------------------------------
// Name: Utilities::SetEnvVariable
//
// Description:
//  Sets the environment variable name to the specified value
//
// Returns:
//  Returns the result of the putenv operation. 0 indicates success, nonzero indicates error.
//
int Utilities::SetEnvVariable(const string &envVarName, const string &value)
{
	errno = 0;
	string keyValue = string(envVarName + "=" + value);
	int result = putenv(const_cast<char*>(keyValue.c_str()));
	if (result != 0)
	{
		LOG_ERROR("Failed to set the value " + value +
			" to the environment variable " + envVarName +
			" with system error #" + to_string(errno) + ".");
	}

	return result;
}
