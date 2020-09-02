//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonParamContainer.h
//
// Purpose:
//  A container that stores the input and output parameters passed to the python script.
//
//*************************************************************************************************

#pragma once
#include "Common.h"

#include <unordered_map>

//-------------------------------------------------------------------------------------------------
// Name: PythonParamContainer
//
// Description:
// Container for PythonExtension parameters.
//
class PythonParamContainer
{
public:
	// Initialize the container with the number of parameters.
	//
	void Init(SQLSMALLINT paramsNumber);

	// Creates an PythonParam object, adds the parameter with paramValue for given dataType
	// to the boost::python namespace and stores it in m_params for future use.
	//
	void AddParamToNamespace(
		boost::python::object nameSpace,
		SQLUSMALLINT          paramNumber,
		const SQLCHAR         *paramName,
		SQLSMALLINT           paramNameLength,
		SQLSMALLINT           dataType,
		SQLULEN               paramSize,
		SQLSMALLINT           decimalDigits,
		SQLPOINTER            paramValue,
		SQLINTEGER            strLen_or_Ind,
		SQLSMALLINT           inputOutputType);

	// Returns the number of parameters in the container.
	// Note: The container is initialized with the desired number of parameters,
	// so the size returned here does not change even after adding parameters to
	// the boost::python namespace.
	//
	SQLUSMALLINT GetSize() const
	{
		return static_cast<SQLUSMALLINT>(m_params.size());
	}

	// For the given paramNumber, retrieve and return paramValue and strLen_or_Ind.
	//
	void GetParamValueAndStrLenInd(
		boost::python::object mainNamespace,
		SQLUSMALLINT          paramNumber,
		SQLPOINTER            *paramValue,
		SQLINTEGER            *strLen_or_Ind);

private:

	// Template to create a parameter and add it to the python namespace
	//
	template<class ParamType>
	void CreateParam(
		boost::python::object nameSpace,
		SQLUSMALLINT          paramNumber,
		const SQLCHAR         *paramName,
		SQLSMALLINT           paramNameLength,
		SQLSMALLINT           dataType,
		SQLULEN               paramSize,
		SQLSMALLINT           decimalDigits,
		SQLPOINTER            paramValue,
		SQLINTEGER            strLen_or_Ind,
		SQLSMALLINT           inputOutputType);

	// Create parameter function pointer definition
	//
	using fnCreateParam = void (PythonParamContainer::*)(
		boost::python::object nameSpace,
		SQLUSMALLINT          paramNumber,
		const SQLCHAR         *paramName,
		SQLSMALLINT           paramNameLength,
		SQLSMALLINT           dataType,
		SQLULEN               paramSize,
		SQLSMALLINT           decimalDigits,
		SQLPOINTER            paramValue,
		SQLINTEGER            strLen_or_Ind,
		SQLSMALLINT           inputOutputType);

	// Vector to store created parameters
	//
	std::vector<std::unique_ptr<PythonParam>> m_params;

	// Function map to add parameters to the namespace and its typedef
	//
	static std::unordered_map<SQLSMALLINT, fnCreateParam> sm_FnCreateParamMap;
	typedef std::unordered_map<SQLSMALLINT, fnCreateParam> CreateParamFnMap;
};
