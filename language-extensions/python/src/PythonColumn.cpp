//**************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonColumn.cpp
//
// Purpose:
//	 Encapsulate data column attributes
//
//**************************************************************************************************

#include "PythonColumn.h"

PythonColumn::PythonColumn(
	std::string name,
	SQLSMALLINT dataType,
	SQLULEN     size,
	SQLSMALLINT nullable,
	SQLSMALLINT decimalDigits)
{
	m_name = name;
	m_dataType = dataType;
	m_size = size;
	m_nullable = nullable;
	m_decimalDigits = decimalDigits;
}
