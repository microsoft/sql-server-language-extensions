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
// @File: Column.h
//
// Purpose:
//  Class encapsulating a dataset's column attributes
//
//*************************************************************************************************
#pragma once

// Encapsulate column information
//
class Column
{
public:
	Column(
		std::string name,
		SQLSMALLINT dataType,
		SQLULEN size,
		SQLSMALLINT nullable,
		SQLSMALLINT decimalDigits);

	const std::string& Name() const
	{
		return m_name;
	}

	const SQLSMALLINT& DataType() const
	{
		return m_dataType;
	}

	const SQLULEN& Size() const
	{
		return m_size;
	}

	const SQLSMALLINT& Nullable() const
	{
		return m_nullable;
	}

	const SQLSMALLINT& DecimalDigits() const
	{
		return m_decimalDigits;
	}

private:
	std::string m_name;
	SQLSMALLINT m_dataType;
	SQLULEN m_size;
	SQLSMALLINT m_nullable;
	SQLSMALLINT m_decimalDigits;
};
