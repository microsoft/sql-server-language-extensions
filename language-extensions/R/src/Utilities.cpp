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
// @File: Utilities.cpp
//
// Purpose:
//  Utility functions implemented for both the platforms.
//
//**************************************************************************************************

#include "Common.h"

using namespace std;

// Represents the max number of digits after the decimal point in the string representation
// of nanoseconds converted to seconds.
//
#define MAXDIGITS_IN_NANOSECONDS 9

const char* GuidFormat="%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X";

//--------------------------------------------------------------------------------------------------
// Name: Utilities::GenerateUniquePtr
//
// Description:
//  Given a constant string input, generates a unique pointer
//  pointing to a char array containing the same contents as that of the input
//
unique_ptr<char[]> Utilities::GenerateUniquePtr(const string &inputStr)
{
	int inputStrLen = inputStr.length();

	// Reserve additional space for null terminating character.
	//
	unique_ptr<char[]> outPtr = make_unique<char[]>(inputStrLen + 1);
	memcpy(outPtr.get(), inputStr.c_str(), inputStrLen);
	*(outPtr.get() + inputStrLen) = '\0';
	return outPtr;
}

//--------------------------------------------------------------------------------------------------
// Name: Utilities::ConvertGuidToString
//
// Description:
//  Converts a SQLGUID to a string
//
// Returns:
//  string of the guid
//
string Utilities::ConvertGuidToString(const SQLGUID *guid)
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
// Name: Utilities::Tokenize
//
// Description:
//  Splits the given character string using the delimiter and
//  adds the tokens to the input vector.
// Remarks:
//  If the input is invalid and unable to be parsed,
//  token will be nullptr and we add nothing to the vector.
//
// Returns:
//  nothing.
//
void Utilities::Tokenize(
	char           *input,
	const char     *delimiter,
	vector<char *> *tokens)
{
	char *internalState = nullptr;
	char *token = strtok_r(input, delimiter, &internalState);

	// Use strtok_r iteratively to do the tokenization.
	//
	while (token != nullptr)
	{
		(*tokens).push_back(token);
		token = strtok_r(nullptr, delimiter, &internalState);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: Utilities::GetSecondsAfterDecimalPointFromNanoSeconds
//
// Description:
//  From the given nanoSeconds value, generates a string representing seconds
//  after the decimal point.
//
// Returns:
//  string representing seconds after the decimal point.
//
string Utilities::GetSecondsAfterDecimalPointFromNanoSeconds(
	SQLUINTEGER nanoSeconds)
{
	string secondsAfterDecimalPoint = to_string(nanoSeconds);
	size_t digitsInNanoSeconds = strlen(secondsAfterDecimalPoint.c_str());

	// 1 second = 10^9 nanoseconds = 10^6 microseconds;
	// 0.100'000'000 second = 10^8 nanoseconds = 10^5 microseconds.
	// We need to prefix with zeros the string representation of the seconds after decimal point
	// if digits in nanoseconds are less than 9 to represent the correct seconds value.
	//
	if (digitsInNanoSeconds < MAXDIGITS_IN_NANOSECONDS)
	{
		size_t numberOfZeros = MAXDIGITS_IN_NANOSECONDS - digitsInNanoSeconds;
		secondsAfterDecimalPoint = string(numberOfZeros, '0') + secondsAfterDecimalPoint;
	}

	return secondsAfterDecimalPoint;
}

//--------------------------------------------------------------------------------------------------
// Name: Utilities::GetTimeZoneInR
//
// Description:
//  Finds the current time zone defined in R if any
//
// Returns:
//  The time zone environment variable's value as a string if it is set or an empty string if not.
//
string Utilities::GetTimeZoneInR()
{
	// This script finds the value of the environment variable TZ that defines the time zone if set
	//
	string scriptToFindTimeZoneInR = "Sys.getenv('TZ')[1]";
	RInside* embeddedREnvPtr = REnvironment::EmbeddedREnvironment();

	// Evaluate the script and store the result in a string
	//
	Rcpp::CharacterVector timeZoneInRVector(static_cast<SEXP>(
		embeddedREnvPtr->parseEval(scriptToFindTimeZoneInR)));

	string timeZoneInR = "";

	if (timeZoneInRVector.size() > 0)
	{
		timeZoneInR = timeZoneInRVector[0];
	}

	return timeZoneInR;
}

//--------------------------------------------------------------------------------------------------
// Name: Utilities::SetTimeZoneInR
//
// Description:
//  Sets the time zone in R to the given value or unset it if the value is empty.
//
// Returns:
//  Nothing.
//
void Utilities::SetTimeZoneInR(string valueToSet)
{
	if (valueToSet.length() > 0)
	{
		// Reset the stored time zone using the temporary variable above.
		//
		ExecuteScript("Sys.setenv(TZ = '" + valueToSet + "')");
	}
	else
	{
		// If time zone was not set, unset it again.
		// On Windows, it default is UTC while on Linux it is the local time zone.
		//
		ExecuteScript("Sys.unsetenv('TZ')");
	}
}

//--------------------------------------------------------------------------------------------------
// Name: Utilities::NormalizePathString
//
// Description:
//  Normalizes path strings by replacting \ with / and the trailing / with a null terminator.
//
// Returns:
//  The normalized path string
//
string Utilities::NormalizePathString(string pathString)
{
	replace(pathString.begin(), pathString.end(), '\\', '/');

	// Replace trailing / with \0.
	//
	if (pathString.length() > 0 && *(pathString.end() - 1) == '/')
	{
		*(pathString.end() - 1) = '\0';
	}

	return pathString;
}
