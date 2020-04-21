//**************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonParam.cpp
//
// Purpose:
//  Class storing information about the PythonExtension input/output parameter.
//
//**************************************************************************************************


#include "Logger.h"
#include "PythonParam.h"

using namespace std;
namespace py = boost::python;

//-------------------------------------------------------------------------------------------------
// Name: PythonParam
//
// Description:
// Constructor.
//
PythonParam::PythonParam(
	SQLUSMALLINT  id,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   type,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType) :
	m_id(id),
	m_type(type),
	m_size(paramSize),
	m_decimalDigits(decimalDigits),
	m_strLenOrInd(strLen_or_Ind),
	m_inputOutputType(inputOutputType)
{

	// Remove "@" from the front of the name
	//
	const char *name = static_cast<const char*>(static_cast<const void*>(paramName + 1));

	// paramNameLength includes @, we remove it
	//
#if defined(_DEBUG)
	if (static_cast<size_t>(paramNameLength - 1) != strlen(name))
	{
		throw invalid_argument("Invalid parameter name length, it doesn't match string length.");
	}
#endif

	// Store the information for this column
	//
	m_name = string(name, paramNameLength - 1);
};

//-------------------------------------------------------------------------------------------------
// Name: CheckParamSize
//
// Description:
// Verifies if m_Size is equal to the size of the template type T.
// Returns nothing if the check succeeds, throws an exception otherwise.
//
template<class T>
void PythonParam::CheckParamSize()
{
	size_t dataTypeSize = sizeof(T);
	if (dataTypeSize != m_size)
	{
		string error("The parameter size(" + to_string(m_size) +
			") does not match the size of the supported datatype(" +
			to_string(dataTypeSize) + ").");
		LOG_ERROR(error);
		throw invalid_argument(error);
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonParamTemplate
//
// Description:
//  Constructor.
//  Calls the base constructor then populates m_pyObject with a boost::python object that contains
//  the parameter value, in a way that python can use, or py::object which is None.
//
template<class SQLType>
PythonParamTemplate<SQLType>::PythonParamTemplate(
	SQLUSMALLINT  id,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   type,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType)
	: PythonParam(id,
				paramName,
				paramNameLength,
				type,
				paramSize,
				decimalDigits,
				strLen_or_Ind,
				inputOutputType)
{
	CheckParamSize<SQLType>();

	if (strLen_or_Ind != SQL_NULL_DATA)
	{
		m_pyObject = py::object(*static_cast<SQLType*>(paramValue));
	}
	else
	{
		m_pyObject = py::object();
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonLogicalParam
//
// Description:
//  Constructor.
//  Calls the base constructor then populates m_pyObject with a boost::python object that contains
//  the parameter value, in a way that python can use, or py::object which is None.
//
PythonLogicalParam::PythonLogicalParam(
	SQLUSMALLINT  id,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   type,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType)
	: PythonParam(id,
				paramName,
				paramNameLength,
				type,
				paramSize,
				decimalDigits,
				strLen_or_Ind,
				inputOutputType)
{
	if (strLen_or_Ind != SQL_NULL_DATA)
	{
		bool value = *static_cast<SQLCHAR*>(paramValue) != '0' ? true : false;
		m_pyObject = py::object(value);
	}
	else
	{
		m_pyObject = py::object();
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonStringParam
//
// Description:
//  Constructor.
//  Calls the base constructor then populates m_pyObject with a boost::python object that contains
//  the parameter value, in a way that python can use, or py::object which is None.
//  We use StrLen_or_Ind to calculate how long the string is before creating the python object.
//
PythonStringParam::PythonStringParam(
	SQLUSMALLINT  id,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   type,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType)
	: PythonParam(id,
				paramName,
				paramNameLength,
				type,
				paramSize,
				decimalDigits,
				strLen_or_Ind,
				inputOutputType)
{
	if (strLen_or_Ind != SQL_NULL_DATA)
	{
		SQLINTEGER strlen = strLen_or_Ind / sizeof(char);
		string value(static_cast<char*>(paramValue), strlen);
		m_pyObject = py::object(value);
	}
	else
	{
		m_pyObject = py::object();
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonRawParam
//
// Description:
// Constructor.
//
PythonRawParam::PythonRawParam(
	SQLUSMALLINT  id,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   type,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType)
	: PythonParam(id,
				paramName,
				paramNameLength,
				type,
				paramSize,
				decimalDigits,
				strLen_or_Ind,
				inputOutputType)
{
	if (strLen_or_Ind != SQL_NULL_DATA)
	{
		SQLINTEGER strlen = strLen_or_Ind / sizeof(SQLCHAR);

		// Create a Python bytes object from binary
		//
		m_pyObject = py::object(py::handle<>(
			PyBytes_FromObject(PyMemoryView_FromMemory(
				static_cast<char*>(paramValue), strlen, PyBUF_READ
			))
			));
	}
	else
	{
		m_pyObject = py::object();
	}
}

//--------------------------------------------------------------------------------------------------
// Do explicit template instantiations, so that object code is generated for these
// and the linker is able to find their definitions even after instantiations are in different
// translation units (i.e. PythonParamTemplate instantiation is in PythonParamContainer.cpp)
//
template PythonParamTemplate<SQLINTEGER>::PythonParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR *,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template PythonParamTemplate<SQLREAL>::PythonParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR *,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template PythonParamTemplate<SQLDOUBLE>::PythonParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR *,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template PythonParamTemplate<SQLBIGINT>::PythonParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR *,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template PythonParamTemplate<SQLSMALLINT>::PythonParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR *,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template PythonParamTemplate<SQLCHAR>::PythonParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR *,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);
