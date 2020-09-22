//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonColumn.cpp
//
// Purpose:
//  Encapsulate data column attributes
//
//*************************************************************************************************

#include "PythonColumn.h"

using namespace std;

//-------------------------------------------------------------------------------------------------
// Name: PythonColumn::PythonColumn
//
// Description:
//  Initializes the class that encapsulates one column in the Python DataFrame/Dictionary
//
PythonColumn::PythonColumn(
	const SQLCHAR *columnName,
	SQLSMALLINT   columnNameLength,
	SQLSMALLINT   dataType,
	SQLULEN       columnSize,
	SQLSMALLINT   decimalDigits,
	SQLSMALLINT   nullable) :
	m_dataType(dataType),
	m_size(columnSize),
	m_decimalDigits(decimalDigits),
	m_nullable(nullable)
{
	const char *name = static_cast<const char*>(static_cast<const void*>(columnName));

	// columnNameLength does not include the null terminator.
	//
#if defined(_DEBUG)
	if (static_cast<size_t>(columnNameLength) != strlen(name))
	{
		throw invalid_argument("Invalid column name length, it doesn't match string length.");
	}
#endif

	// Store the information for this column
	//
	m_name = string(name, columnNameLength);
}

