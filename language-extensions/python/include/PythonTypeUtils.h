//*********************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonTypeUtils.h
//
// Purpose:
//  Python functions that deal with type conversions and additions
//
//*********************************************************************

#pragma once
#include "Common.h"

class PythonTypeUtils
{
public:

	static void AddParamToNamespace(
		boost::python::object nameSpace,
		std::string           name,
		SQLSMALLINT           dataType,
		SQLULEN               paramSize,
		SQLSMALLINT           decimalDigits,
		SQLPOINTER            paramValue,
		SQLINTEGER            strLen_or_Ind);
};
