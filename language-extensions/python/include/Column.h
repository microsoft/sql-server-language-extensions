//*********************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: Column.h
//
// Purpose:
//	 Encapsulate dataset column attributes
//
//*********************************************************************

#pragma once
#include "Common.h"

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
