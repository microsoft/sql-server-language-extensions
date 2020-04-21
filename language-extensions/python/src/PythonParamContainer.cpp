//**************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonParamContainer.cpp
//
// Purpose:
//  PythonExtension input/output parameters wrappers,
//  along with the container consolidating them.
//
//**************************************************************************************************


#include "Logger.h"
#include "PythonParam.h"
#include "PythonParamContainer.h"

using namespace std;
namespace py = boost::python;

// Function map - maps a SQL data type to the appropriate param creator
//
unordered_map<SQLSMALLINT, PythonParamContainer::fnCreateParam> PythonParamContainer::m_fnCreateParamMap =
{
	{static_cast<SQLSMALLINT>(SQL_C_BIT),
	 static_cast<fnCreateParam>(&PythonParamContainer::CreateParam<PythonLogicalParam>)},
	{static_cast<SQLSMALLINT>(SQL_C_SLONG),
	 static_cast<fnCreateParam>(&PythonParamContainer::CreateParam<PythonParamTemplate<SQLINTEGER>>)},
	{static_cast<SQLSMALLINT>(SQL_C_DOUBLE),
	 static_cast<fnCreateParam>(&PythonParamContainer::CreateParam<PythonParamTemplate<SQLDOUBLE>>)},
	{static_cast<SQLSMALLINT>(SQL_C_FLOAT),
	 static_cast<fnCreateParam>(&PythonParamContainer::CreateParam<PythonParamTemplate<SQLREAL>>)},
	{static_cast<SQLSMALLINT>(SQL_C_SSHORT),
	 static_cast<fnCreateParam>(&PythonParamContainer::CreateParam<PythonParamTemplate<SQLSMALLINT>>)},
	{static_cast<SQLSMALLINT>(SQL_C_UTINYINT),
	 static_cast<fnCreateParam>(&PythonParamContainer::CreateParam<PythonParamTemplate<SQLCHAR>>)},
	{static_cast<SQLSMALLINT>(SQL_C_SBIGINT),
	 static_cast<fnCreateParam>(&PythonParamContainer::CreateParam<PythonParamTemplate<SQLBIGINT>>)},
	{static_cast<SQLSMALLINT>(SQL_C_CHAR),
	 static_cast<fnCreateParam>(&PythonParamContainer::CreateParam<PythonStringParam>)},
	{static_cast<SQLSMALLINT>(SQL_C_BINARY),
	 static_cast<fnCreateParam>(&PythonParamContainer::CreateParam<PythonRawParam>)},
};

//-------------------------------------------------------------------------------------------------
// Name: Init
//
// Description:
// Initialize this container with the number of parameters.
//
void PythonParamContainer::Init(SQLSMALLINT paramsNumber)
{
	LOG("PythonParamContainer::Init");

	m_params.resize(paramsNumber);
}

//-------------------------------------------------------------------------------------------------
// Name: AddParamToNamespace
//
// Description:
// Creates a PythonParam object and adds it to the boost:python namespace.
// Eventually, adds the PythonParam to m_params for future use.
// Creation is done by finding the appropriate function from the function map.
//
void PythonParamContainer::AddParamToNamespace(
	py::object    nameSpace,
	SQLUSMALLINT  paramNumber,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   dataType,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType)
{
	LOG("PythonParamContainer::AddParamToNamespace");

	CreateParamFnMap::const_iterator it = m_fnCreateParamMap.find(dataType);

	if (it == m_fnCreateParamMap.end())
	{
		throw runtime_error("Unsupported parameter type encountered when creating param #" + to_string(paramNumber));
	}

	(this->*it->second)(
		nameSpace,
		paramNumber,
		paramName,
		paramNameLength,
		dataType,
		paramSize,
		decimalDigits,
		paramValue,
		strLen_or_Ind,
		inputOutputType
		);
}

//-------------------------------------------------------------------------------------------------
// Name: CreateParam
//
// Description:
// Template to create a parameter and add it to the python namespace
//
template<class ParamType>
void PythonParamContainer::CreateParam(
	py::object    nameSpace,
	SQLUSMALLINT  paramNumber,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   dataType,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType)
{
	// Remove "@" from the front of the name and the length
	//

	unique_ptr<PythonParam> paramToBeAdded = make_unique<ParamType>(
		paramNumber,
		paramName,
		paramNameLength,
		dataType,
		paramSize,
		decimalDigits,
		paramValue,
		strLen_or_Ind,
		inputOutputType
		);

	string name = paramToBeAdded.get()->Name();
	nameSpace[name] = paramToBeAdded.get()->PythonValue();
	m_params[paramNumber] = std::move(paramToBeAdded);
}
