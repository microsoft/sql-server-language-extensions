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
		std::string name,
		SQLSMALLINT dataType,
		SQLULEN     size,
		SQLSMALLINT nullable,
		SQLSMALLINT decimalDigits);

	const std::string& Name() const
	{
		return m_name;
	}

	SQLSMALLINT DataType()
	{
		return m_dataType;
	}

	SQLULEN Size()
	{
		return m_size;
	}

	SQLSMALLINT Nullable()
	{
		return m_nullable;
	}

	SQLSMALLINT DecimalDigits()
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
