//*********************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonTypeUtils.cpp
//
// Purpose:
//  Python functions that deal with type conversions and additions
//
//*********************************************************************

#include "Logger.h"
#include "PythonTypeUtils.h"

using namespace std;

//--------------------------------------------------------------------------------------------------
// Name: AddParamToNamespace
//
// Description:
// Adds a parameter to a python namespace (dictionary that represents the environment)
// so that the variables are accessible in python
//
void PythonTypeUtils::AddParamToNamespace(
	boost::python::object nameSpace,
	std::string name,
	SQLSMALLINT	  DataType,
	SQLULEN		  ArgSize,
	SQLSMALLINT	  DecimalDigits,
	SQLPOINTER	  ArgValue,
	SQLINTEGER	  StrLen_or_Ind)
{
	if (ArgValue == nullptr)
	{
		// Initialize with None if the parameter is null
		//
		nameSpace[name] = boost::python::object();
	}
	else
	{
		switch (DataType)
		{
		case SQL_C_SLONG:
		{
			nameSpace[name] = *reinterpret_cast<SLONG*>(ArgValue);
			break;
		}
		case SQL_C_BIT:
		{
			nameSpace[name] = *reinterpret_cast<SQLCHAR*>(ArgValue);
			break;
		}
		case SQL_C_FLOAT:
		{
			float num = *reinterpret_cast<SQLREAL*>(ArgValue);
			nameSpace[name] = num;
			break;
		}
		case SQL_C_DOUBLE:
		{
			double num = *reinterpret_cast<SQLDOUBLE*>(ArgValue);
			nameSpace[name] = num;
			break;
		}
		case SQL_C_SBIGINT:
		{
			nameSpace[name] = *reinterpret_cast<SQLBIGINT*>(ArgValue);
			break;
		}
		case SQL_C_UTINYINT:
		{
			nameSpace[name] = *reinterpret_cast<SQLCHAR*>(ArgValue);
			break;
		}
		case SQL_C_SSHORT:
		{
			nameSpace[name] = *reinterpret_cast<SQLSMALLINT*>(ArgValue);
			break;
		}
		case SQL_C_CHAR:
		{
			int strlen = StrLen_or_Ind / sizeof(CHAR);
			string value(reinterpret_cast<CHAR*>(ArgValue), strlen);
			nameSpace[name] = value;
			break;
		}
		case SQL_C_WCHAR:
		{
			int strlen = StrLen_or_Ind / sizeof(WCHAR);
			wstring value(reinterpret_cast<WCHAR*>(ArgValue), strlen);
			nameSpace[name] = value;
			break;
		}
		case SQL_C_BINARY:
		{
			nameSpace[name] = *reinterpret_cast<SQLCHAR*>(ArgValue);
			break;
		}
		case SQL_C_GUID:
		case SQL_C_TYPE_DATE:
		case SQL_C_NUMERIC:
		case SQL_C_TYPE_TIMESTAMP:
		default:
			throw invalid_argument("Unsupported input parameter type");
		}
	}
}
