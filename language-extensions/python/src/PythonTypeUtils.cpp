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
namespace py = boost::python;

//--------------------------------------------------------------------------------------------------
// Name: AddParamToNamespace
//
// Description:
// Adds a parameter to a python namespace (dictionary that represents the environment)
// so that the variables are accessible in python
//
void PythonTypeUtils::AddParamToNamespace(
	py::object  nameSpace,
	std::string name,
	SQLSMALLINT dataType,
	SQLULEN     paramSize,
	SQLSMALLINT decimalDigits,
	SQLPOINTER  paramValue,
	SQLINTEGER  strLen_or_Ind)
{
	LOG("PythonTypeUtils::AddParamToNamespace");
	if (paramValue == nullptr)
	{
		// Initialize with None if the parameter is null
		//
		nameSpace[name] = py::object();
	}
	else
	{
		switch (dataType)
		{
		case SQL_C_SLONG:
		{
			nameSpace[name] = *static_cast<SQLINTEGER*>(paramValue);
			break;
		}
		case SQL_C_BIT:
		{
			nameSpace[name] = *static_cast<SQLCHAR*>(paramValue) != '0' ? true : false;
			break;
		}
		case SQL_C_FLOAT:
		{
			nameSpace[name] = *static_cast<SQLREAL*>(paramValue);
			break;
		}
		case SQL_C_DOUBLE:
		{
			nameSpace[name] = *static_cast<SQLDOUBLE*>(paramValue);
			break;
		}
		case SQL_C_SBIGINT:
		{
			nameSpace[name] = *static_cast<SQLBIGINT*>(paramValue);
			break;
		}
		case SQL_C_UTINYINT:
		{
			nameSpace[name] = *static_cast<SQLCHAR*>(paramValue);
			break;
		}
		case SQL_C_SSHORT:
		{
			nameSpace[name] = *static_cast<SQLSMALLINT*>(paramValue);
			break;
		}
		case SQL_C_CHAR:
		{
			SQLINTEGER strlen = strLen_or_Ind / sizeof(CHAR);
			string value(static_cast<CHAR*>(paramValue), strlen);
			nameSpace[name] = value;
			break;
		}
		case SQL_C_BINARY:
		{
			SQLINTEGER strlen = strLen_or_Ind / sizeof(SQLCHAR);

			// Create a Python memoryview object from binary
			//
			py::object memViewObj(py::handle<>(
				PyBytes_FromObject(PyMemoryView_FromMemory(
					static_cast<CHAR*>(paramValue), strlen, PyBUF_READ
				))
			));

			nameSpace[name] = memViewObj;
			break;
		}
		case SQL_C_WCHAR:
		case SQL_C_GUID:
		case SQL_C_TYPE_DATE:
		case SQL_C_NUMERIC:
		case SQL_C_TYPE_TIMESTAMP:
		default:
			throw invalid_argument("Unsupported input parameter type");
		}
	}
}
