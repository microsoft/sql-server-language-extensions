//**************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonColumn.h
//
// Purpose:
//	 Encapsulate dataset column attributes
//
//**************************************************************************************************

#pragma once
#include "Common.h"

// Encapsulate column information
//
class PythonColumn
{
public:
	PythonColumn(
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

	SQLSMALLINT DataType()
	{
		return m_dataType;
	}

protected:

	// Name of the column.
	//
	std::string m_name;

	// Data Type of the column.
	//
	SQLSMALLINT m_dataType;

	// Size of the column.
	//
	SQLULEN m_size;

	// Decimal digits of the column.
	//
	SQLSMALLINT m_decimalDigits;

	// The column is nullable or not.
	//
	SQLSMALLINT m_nullable;
};
