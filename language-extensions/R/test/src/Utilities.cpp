//**************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server
// external language communication protocol.
// Copyright (C) 2019 Microsoft Corporation.
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
// Name: GetMaxLength
//
// Description:
// Get max length from given arrayOfLengths with size = rowsNumber.
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
// Name: GetWStringLength
//
// Description:
//  Get the length of a wchar_t *.
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
// Name: GetDate
//
// Description:
//  Get the current UTC date in the form of a SQL_DATE_STRUCT
//
template<SQLSMALLINT DateType>
SQL_DATE_STRUCT Utilities::GetDate()
{
	chrono::system_clock::time_point now = chrono::system_clock::now();
	time_t time = chrono::system_clock::to_time_t(now);
	tm timeAsComponents;

	if constexpr (DateType == UTC_DATE)
	{
		timeAsComponents = *gmtime(&time);
	}
	else
	{
		timeAsComponents= *localtime(&time);
	}

	SQL_DATE_STRUCT utcDate =
		{ static_cast<SQLSMALLINT>(1900 + timeAsComponents.tm_year),
		  static_cast<SQLUSMALLINT>(1 + timeAsComponents.tm_mon),
		  static_cast<SQLUSMALLINT>(timeAsComponents.tm_mday) };

	return utcDate;
}

template SQL_DATE_STRUCT Utilities::GetDate<LOCAL_DATE>();
template SQL_DATE_STRUCT Utilities::GetDate<UTC_DATE>();
