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

#pragma once

# define LOCAL_DATE 1
# define UTC_DATE 2

class Utilities
{
public:

	// Gets the handle for the given library path after loading it dynamically
	//
	static void* CrossPlatLoadLibrary(
		const char *libPath);

	// Gets the function pointer for the given function name from the given library handle
	//
	static void* CrossPlatGetFunctionFromLibHandle(
		void *libHandle,
		const std::string &fnName);

	// Given a valid library handle, closes it.
	//
	static void CrossPlatCloseLibrary(
		void *libHandle);

	// Gets max length from given arrayOfLengths with size = rowsNumber.
	//
	static SQLINTEGER GetMaxLength(
		SQLINTEGER *arrayOfLengths,
		SQLULEN    rowsNumber);

	// Gets the length of a wchar_t *.
	//
	static SQLULEN GetWStringLength(const wchar_t *wstr);

	// Returns the current system clock time in terms of time_t which represents number of seconds
	// since epoch January 1, 1970 00:00:00.
	//
	static time_t GetNow();

	// Gets the current date based on its type (UTC or local) in the form of a SQL_DATE_STRUCT.
	//
	template<SQLSMALLINT DateType>
	static SQL_DATE_STRUCT GetTodaysDate();

	//  Convert the given SQLDateTimeType into an equivalent UTC SQLDateTimeType
	//
	template<class SQLDateTimeType>
	static SQLDateTimeType ToUtc(SQLDateTimeType givenTimeStamp);

	// Normalizes path string.
	//
	static std::string NormalizePathString(std::string pathString);
};
