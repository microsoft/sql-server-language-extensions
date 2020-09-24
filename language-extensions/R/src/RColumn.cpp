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
// @File: RColumn.cpp
//
// Purpose:
//  Class encapsulating a DataSet's column attributes
//
//**************************************************************************************************

#include "Common.h"
#include "RColumn.h"

using namespace std;

//--------------------------------------------------------------------------------------------------
// Name: RColumn::RColumn
//
// Description:
//  The constructor assigning the given column information to members.
//
RColumn::RColumn(
	const SQLCHAR *columnName,
	SQLSMALLINT   columnNameLength,
	SQLSMALLINT   dataType,
	SQLULEN       columnSize,
	SQLSMALLINT   decimalDigits,
	SQLSMALLINT   nullable)
{
	const char *name = static_cast<const char*>(
			static_cast<const void*>(columnName));

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
	m_dataType = dataType;
	m_size = columnSize;
	m_decimalDigits = decimalDigits;
	m_nullable = nullable;
}
