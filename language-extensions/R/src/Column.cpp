//******************************************************************************************************
// RExtension : A language extension implementing the SQL Server external language communication protocol.
// Copyright (C) 2019 Microsoft Corporation.

// This file is part of RExtension.

// RExtension is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// RExtension is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with RExtension.  If not, see <https://www.gnu.org/licenses/>.

// @File: Column.cpp
//
// Purpose:
//  Class encapsulating a dataset's column attributes
//
//******************************************************************************************************
#include "Common.h"
#include "Column.h"

Column::Column(
	std::string name,
	SQLSMALLINT dataType,
	SQLULEN size,
	SQLSMALLINT nullable,
	SQLSMALLINT decimalDigits)
{
	m_name = name;
	m_dataType = dataType;
	m_size = size;
	m_nullable = nullable;
	m_decimalDigits = decimalDigits;
}
