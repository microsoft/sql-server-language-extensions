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
// @File: RColumn.h
//
// Purpose:
//  Class encapsulating a dataset's column attributes.
//
//**************************************************************************************************

#pragma once

//--------------------------------------------------------------------------------------------------
// Name: RColumn
//
// Description:
//  Class encapsulating a dataset's column attributes.
//
class RColumn
{
public:

	RColumn(
		const SQLCHAR *columnName,
		SQLSMALLINT   columnNameLength,
		SQLSMALLINT   dataType,
		SQLULEN       columnSize,
		SQLSMALLINT   decimalDigits,
		SQLSMALLINT   nullable);

	const std::string& Name() const
	{
		return m_name;
	}

	SQLSMALLINT DataType() const
	{
		return m_dataType;
	}

	SQLULEN Size() const
	{
		return m_size;
	}

	SQLSMALLINT Nullable() const
	{
		return m_nullable;
	}

	SQLSMALLINT DecimalDigits() const
	{
		return m_decimalDigits;
	}

private:

	// Name of the column.
	//
	std::string m_name;

	// Data Type of the column.
	//
	SQLSMALLINT m_dataType;

	// Size of the column.
	//
	SQLULEN m_size;

	// The column is nullable or not.
	//
	SQLSMALLINT m_nullable;

	// Decimal digits of the column.
	//
	SQLSMALLINT m_decimalDigits;
};
