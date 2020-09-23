//**************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server
// external language communication protocol.
// Copyright (C) 2020 Microsoft Corporation.
//
// This file is part of RExtension-test.
//
// RExtension-test is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RExtension-test is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RExtension-test.  If not, see <https://www.gnu.org/licenses/>.
//
// @File: Utilities.cpp
//
// Purpose:
//  Utility functions
//
//**************************************************************************************************

#include <ctime>
#include <chrono>

#include "Common.h"

using namespace std;

//--------------------------------------------------------------------------------------------------
// Name: Utilities::GetMaxLength
//
// Description:
//  Gets max length from given arrayOfLengths with size = rowsNumber.
//
SQLINTEGER Utilities::GetMaxLength(
	SQLINTEGER *arrayOfLengths,
	SQLULEN    rowsNumber)
{
	SQLINTEGER maxLen = 0;
	for (SQLULEN index = 0; index < rowsNumber; ++index)
	{
		if (arrayOfLengths[index] != SQL_NULL_DATA && maxLen < arrayOfLengths[index])
		{
			maxLen = arrayOfLengths[index];
		}
	}

	return maxLen;
}

//--------------------------------------------------------------------------------------------------
// Name: Utilities::GetWStringLength
//
// Description:
//  Gets the length of a wchar_t *.
//  wcslen does not work in Linux with -fshort-wchar, so we use this function instead.
//
SQLULEN Utilities::GetWStringLength(const wchar_t *wstr)
{
	SQLULEN distance = -1;

	// If nullptr, return
	//
	if (wstr)
	{
		// Get distance from end of string to beginning
		//
		const wchar_t *newstr = wstr;
		while (*newstr)
		{
			++newstr;
		}

		distance = newstr - wstr;
	}

	return distance;
}

//--------------------------------------------------------------------------------------------------
// Name: Utilities::GetNow
//
// Description:
//  Returns the current system clock time in terms of time_t which represents number of seconds
//  since epoch January 1, 1970 00:00:00.
//
time_t Utilities::GetNow()
{
	chrono::system_clock::time_point now = chrono::system_clock::now();
	time_t time = chrono::system_clock::to_time_t(now);

	return time;
}

//--------------------------------------------------------------------------------------------------
// Name: Utilities::GetTodaysDate
//
// Description:
//  Gets the current UTC date in the form of a SQL_DATE_STRUCT
//
template<SQLSMALLINT DateType>
SQL_DATE_STRUCT Utilities::GetTodaysDate()
{
	time_t time = GetNow();
	tm timeAsComponents;
	memset(&timeAsComponents, 0, sizeof(tm));

	if constexpr (DateType == UTC_DATE)
	{
		timeAsComponents = *gmtime(&time);
	}
	else
	{
		timeAsComponents= *localtime(&time);
	}

	// For converting into SQL_DATE_STRUCT, add 1900 to the tm_year since the latter stores year
	// relative to 1900 whereas the former denotes absolute value of year.
	// Also add 1 to tm_mon since the latter is months since January
	// whereas the former has a 1-based month value.
	//
	SQL_DATE_STRUCT todaysDate =
		{ static_cast<SQLSMALLINT>(1900 + timeAsComponents.tm_year),
		  static_cast<SQLUSMALLINT>(1 + timeAsComponents.tm_mon),
		  static_cast<SQLUSMALLINT>(timeAsComponents.tm_mday) };

	return todaysDate;
}

template SQL_DATE_STRUCT Utilities::GetTodaysDate<LOCAL_DATE>();
template SQL_DATE_STRUCT Utilities::GetTodaysDate<UTC_DATE>();

//--------------------------------------------------------------------------------------------------
// Name: Utilities::ToUtc
//
// Description:
//  Converts the given SQLDateTimeType into an equivalent UTC SQLDateTimeType
//
template<class SQLDateTimeType>
SQLDateTimeType Utilities::ToUtc(SQLDateTimeType givenDateTime)
{
	tm timeAsComponents;
	memset(&timeAsComponents, 0, sizeof(tm));

	// Setting Daylight Saving Time to -1 instructs mktime to find
	// the value of Daylight Saving Time for us.
	// A positive value indicates its on whereas 0 indicates its off.
	//
	timeAsComponents.tm_isdst = -1;

	// tm.tm_year is expressed as a year relative to 1900;
	// So we need to subtract 1900 from the SQLDateTimeType year value.
	//
	timeAsComponents.tm_year = givenDateTime.year - 1900;

	// tm.tm_mon has a 0-based index range from 0 to 11 to indicate months since January
	// so subtract 1 from the SQLDateTimeType month value.
	//
	timeAsComponents.tm_mon = givenDateTime.month - 1;
	timeAsComponents.tm_mday = givenDateTime.day;

	if constexpr (is_same_v<SQLDateTimeType, SQL_TIMESTAMP_STRUCT>)
	{
		timeAsComponents.tm_hour = givenDateTime.hour;
		timeAsComponents.tm_min = givenDateTime.minute;
		timeAsComponents.tm_sec = givenDateTime.second;
	}

	time_t time = mktime(&timeAsComponents);
	tm utcTimeAsComponents = *gmtime(&time);

	SQLDateTimeType utcDateTime;

	// For converting into SQLDateTimeType, add 1900 to the tm_year since the latter stores year
	// relative to 1900 whereas the former denotes absolute value of year.
	//
	utcDateTime.year = static_cast<SQLSMALLINT>(1900 + utcTimeAsComponents.tm_year);

	// For converting into SQLDateTimeType, add 1 to tm_mon since the latter is months since January
	// whereas the former has a 1-based month value.
	//
	utcDateTime.month = static_cast<SQLUSMALLINT>(1 + utcTimeAsComponents.tm_mon);
	utcDateTime.day = static_cast<SQLUSMALLINT>(utcTimeAsComponents.tm_mday);

	cout << "UTC Time is: " << utcDateTime.year << "-" << utcDateTime.month << "-"
		<< utcDateTime.day;

	if constexpr (is_same_v<SQLDateTimeType, SQL_TIMESTAMP_STRUCT>)
	{
		utcDateTime.hour = static_cast<SQLUSMALLINT>(utcTimeAsComponents.tm_hour);
		utcDateTime.minute = static_cast<SQLUSMALLINT>(utcTimeAsComponents.tm_min);
		utcDateTime.second = static_cast<SQLUSMALLINT>(utcTimeAsComponents.tm_sec);

		// Use the fraction value as is.
		//
		utcDateTime.fraction = givenDateTime.fraction;

		cout << " " << utcDateTime.hour << ":" << utcDateTime.minute << ":"
			<< utcDateTime.second;
	}

	cout << endl;

	return utcDateTime;
}

template SQL_DATE_STRUCT Utilities::ToUtc
	<SQL_DATE_STRUCT>(SQL_DATE_STRUCT givenDate);
template SQL_TIMESTAMP_STRUCT Utilities::ToUtc
	<SQL_TIMESTAMP_STRUCT>(SQL_TIMESTAMP_STRUCT givenTime);

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
