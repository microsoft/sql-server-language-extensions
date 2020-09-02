//*************************************************************************************************
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
//*************************************************************************************************


#include "Logger.h"
#include "PythonParam.h"
#include "PythonParamContainer.h"

using namespace std;
namespace bp = boost::python;

// Function map - maps a SQL data type to the appropriate param creator
//
unordered_map<SQLSMALLINT, PythonParamContainer::fnCreateParam>
PythonParamContainer::sm_FnCreateParamMap =
{
	{static_cast<SQLSMALLINT>(SQL_C_BIT),
	 static_cast<fnCreateParam>(&PythonParamContainer::CreateParam<PythonBooleanParam>)},
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
	 static_cast<fnCreateParam>(&PythonParamContainer::CreateParam<PythonStringParam<char>>)},
	{static_cast<SQLSMALLINT>(SQL_C_WCHAR),
	 static_cast<fnCreateParam>(&PythonParamContainer::CreateParam<PythonStringParam<wchar_t>>)},
	{static_cast<SQLSMALLINT>(SQL_C_BINARY),
	 static_cast<fnCreateParam>(&PythonParamContainer::CreateParam<PythonRawParam>)},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_DATE), static_cast<fnCreateParam>(
		 &PythonParamContainer::CreateParam<PythonDateTimeParam<SQL_C_TYPE_DATE>>)},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_TIMESTAMP), static_cast<fnCreateParam>(
		 &PythonParamContainer::CreateParam<PythonDateTimeParam<SQL_C_TYPE_TIMESTAMP>>)},
};

//-------------------------------------------------------------------------------------------------
// Name: Init
//
// Description:
//  Initialize this container with the number of parameters.
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
//  Creates a PythonParam object and adds it to the boost:python namespace.
//  Eventually, adds the PythonParam to m_params for future use.
//  Creation is done by finding the appropriate function from the function map.
//
void PythonParamContainer::AddParamToNamespace(
	bp::object    nameSpace,
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

	CreateParamFnMap::const_iterator it = sm_FnCreateParamMap.find(dataType);

	if (it == sm_FnCreateParamMap.end())
	{
		throw runtime_error("Unsupported parameter type " + to_string(dataType) +
			" encountered when creating param #" + to_string(paramNumber));
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
//  Template to create a parameter and add it to the python namespace
//
template<class ParamType>
void PythonParamContainer::CreateParam(
	bp::object    nameSpace,
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
	nameSpace[name] = paramToBeAdded.get()->PythonObject();
	m_params[paramNumber] = std::move(paramToBeAdded);
}

//-------------------------------------------------------------------------------------------------
// Name: PythonParamContainer::GetParamValueAndStrLenInd
//
// Description:
//  For the given paramNumber, call RetriveValueAndStrLenOrInd() to retrieve the value from python
//  and return it via paramValue. Return the strLenOrInd as well.
//  Note the value returned is allocated on the heap and
//  will be cleaned up when param is destructed.
//
void PythonParamContainer::GetParamValueAndStrLenInd(
	bp::object   mainNamespace,
	SQLUSMALLINT paramNumber,
	SQLPOINTER   *paramValue,
	SQLINTEGER   *strLen_or_Ind)
{
	LOG("PythonParamContainer::GetParamValueAndStrLenInd");

	if (m_params[paramNumber] == nullptr)
	{
		throw runtime_error("InitParam not called for paramNumber " + to_string(paramNumber));
	}

	PythonParam *param = m_params[paramNumber].get();

	if (param->InputOutputType() <= SQL_PARAM_INPUT)
	{
		throw runtime_error("Requested param #" + to_string(paramNumber) +
			" is not initialized as an output parameter");
	}

	// Retrieve the value from Python namespace
	//
	param->RetrieveValueAndStrLenInd(mainNamespace);

	*paramValue = param->Value();
	*strLen_or_Ind = param->StrLenOrInd();
}
