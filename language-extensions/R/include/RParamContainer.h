//*************************************************************************************************
// RExtension : A language extension implementing the SQL Server
// external language communication protocol for R.
// Copyright (C) 2019 Microsoft Corporation.
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
// A container that stores the input and output parameters passed to the R script.
//
//*************************************************************************************************
#pragma once

using namespace std;

//-------------------------------------------------------------------------------------------------
// Name: RParamContainer
//
// Description:
// Container for RExtension parameters.
//
class RParamContainer
{
public:
	// Initialize the container with the number of parameters.
	//
	void Init(SQLSMALLINT paramsNumber);

	// Creates an RParam object, adds the parameter with paramValue for given dataType
	// to the embedded R environment and stores it in m_params for future use.
	//
	void AddParamToEmbeddedR(
		SQLUSMALLINT paramNumber,
		std::string name,
		SQLSMALLINT dataType,
		SQLULEN     paramSize,
		SQLSMALLINT decimalDigits,
		SQLPOINTER  paramValue,
		SQLINTEGER  strLen_or_Ind,
		SQLSMALLINT inputOutputType);

	// In debug or verbose mode, prints the parameter value as retrieved from the embedded R environment.
	//
	void PrintParamValue(SQLUSMALLINT id);

	// Returns the number of parameters in the container.
	// Note: The container is initialized with the desired number of parameters,
	// so the size returned here does not change even after adding parameters to
	// embedded R.
	//
	SQLSMALLINT GetSize()
	{
		return m_params.size();
	}

private:

	vector<unique_ptr<RParam>> m_params;
};
