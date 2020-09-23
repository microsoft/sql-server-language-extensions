//**************************************************************************************************
// RExtension : A language extension implementing the SQL Server
// external language communication protocol for R.
// Copyright (C) 2020 Microsoft Corporation.
//
// This file is part of RExtension.
//
// RExtension is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RExtension is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RExtension.  If not, see <https://www.gnu.org/licenses/>.
//
// @File: RParamContainer.h
//
// Purpose:
//  A container that stores the input and output parameters passed to the R script.
//
//**************************************************************************************************

#pragma once

//--------------------------------------------------------------------------------------------------
// Name: RParamContainer
//
// Description:
//  Container for RExtension parameters.
//
class RParamContainer
{
public:

	// Initializes the container with the number of parameters.
	//
	void Init(SQLSMALLINT paramsNumber);

	// Looks up the CreateParam function for the given dataType and calls it
	// to add the parameter to Embedded R environment.
	//
	void AddParamToEmbeddedR(
		SQLUSMALLINT  paramNumber,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   dataType,
		SQLULEN       paramSize,
		SQLSMALLINT   decimalDigits,
		SQLPOINTER    paramValue,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   inputOutputType);

	// For the given paramNumber, retrieves and returns paramValue and strLen_or_Ind.
	//
	void GetParamValueAndStrLenInd(
		SQLUSMALLINT paramNumber,
		SQLPOINTER   *paramValue,
		SQLINTEGER   *strLen_or_Ind);

	// Returns the number of parameters in the container.
	// Note: The container is initialized with the desired number of parameters,
	// so the size returned here does not change even after adding parameters to
	// embedded R.
	//
	SQLUSMALLINT GetSize()
	{
		return static_cast<SQLUSMALLINT>(m_params.size());
	}

private:

	// Creates an RParam object, adds the parameter with paramValue for given dataType
	// to the Embedded R environment and stores it in m_params for future use.
	//
	template<class ParamType>
	void CreateParam(
		SQLUSMALLINT  paramNumber,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   dataType,
		SQLULEN       paramSize,
		SQLSMALLINT   decimalDigits,
		SQLPOINTER    paramValue,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   inputOutputType);

	// CreateParam function pointer definition.
	//
	using fnCreateParam = void (RParamContainer::*)(
		SQLUSMALLINT  paramNumber,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   dataType,
		SQLULEN       paramSize,
		SQLSMALLINT   decimalDigits,
		SQLPOINTER    paramValue,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   inputOutputType);

	std::vector<std::unique_ptr<RParam>> m_params;

	// Function map for adding parameter.
	//
	static const std::unordered_map<SQLSMALLINT, fnCreateParam> sm_FnCreateParamMap;

	// Function map typedef.
	//
	typedef std::unordered_map<SQLSMALLINT, fnCreateParam> CreateParamFnMap;
};
