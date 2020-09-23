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
// @File: Utilities.h
//
// Purpose:
//  Utility functions
//
//**************************************************************************************************

#pragma once

class Utilities
{
public:

	// Converts a SQLGUID to a string
	//
	static std::string ConvertGuidToString(const SQLGUID * guid);

	// Splits the given character string using the delimiter and
	// adds the tokens to the input vector.
	//
	static void Tokenize(char *input, const char* delimiter, std::vector<char*> *tokens);

	// Given a constant string input, generates a unique pointer
	// pointing to a char array containing the same contents as that of the input
	//
	static std::unique_ptr<char[]> GenerateUniquePtr(const std::string &input);

	// Gets the value of the given environment variable name.
	//
	static std::string GetEnvVariable(const std::string &envVarName, bool logError);

	// Sets the environment variable name to the specified value.
	//
	static int SetEnvVariable(const std::string &envVarName, const std::string &value);

	// From the given nanoSeconds value, generates a string
	// representing seconds after the decimal point.
	//
	static std::string GetSecondsAfterDecimalPointFromNanoSeconds(
		SQLUINTEGER nanoSeconds);

	// Finds the current time zone defined in R if any
	//
	static std::string GetTimeZoneInR();

	// Sets the time zone in R to the given value or unset it if the value is empty.
	//
	static void SetTimeZoneInR(std::string valueToSet);

	// Normalizes path strings by replacting \ with /
	//
	static std::string NormalizePathString(std::string pathString);
};
