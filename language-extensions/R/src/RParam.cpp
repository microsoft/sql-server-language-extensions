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
// @File: RParam.cpp
//
// Purpose:
// Class storing information about the RExtension input/output parameter.
//
//*************************************************************************************************

#include "Common.h"

#include <sqlext.h>
#include "string.h"
#include "Logger.h"
#include "RParam.h"
#include "RTypeUtils.h"

#include "Rcpp.h"

using namespace std;

//-------------------------------------------------------------------------------------------------
// Name: RParam
//
// Description:
// Constructor.
//
RParam::RParam(
	SQLUSMALLINT  paramNumber,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   dataType,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType) :
	m_id(paramNumber),
	m_dataType(dataType),
	m_size(paramSize),
	m_decimalDigits(decimalDigits),
	m_strLenOrInd(strLen_or_Ind),
	m_inputOutputType(inputOutputType)
{
	const char* name = static_cast<const char*>(
			static_cast<const void*>(paramName));

	LOG("RParam::RParam Initializing parameter " + string(name));

#if defined(_DEBUG)
	if (static_cast<size_t>(paramNameLength) != strlen(name))
	{
		throw invalid_argument("Invalid parameter name length, it doesn't match string length.");
	}
#endif

	// +1 points to the next character after @ in front of the parameter name.
	// paramNameLength includes @ so do a -1 to exclude it.
	//
	m_name = string(name + 1, paramNameLength - 1);
};

//-------------------------------------------------------------------------------------------------
// Name: CheckParamSize
//
// Description:
// Verifies if m_Size is equal to the size of the template type T.
// Returns nothing if the check succeeds, throws an exception otherwise.
//
template<class T>
void RParam::CheckParamSize()
{
	LOG("RParam::CheckParamSize");

	size_t dataTypeSize = sizeof(T);
	if (dataTypeSize != m_size)
	{
		string error("The parameter size(" + to_string(m_size) +
			") doesn't match the size of the supported data type(" +
			to_string(dataTypeSize) + ").");
		LOG_ERROR(error);
		throw invalid_argument(error);
	}
}

//-------------------------------------------------------------------------------------------------
// Name: RParamTemplate
//
// Description:
// Constructor.
//
template<class SQLType, class RType, class NAType>
RParamTemplate<SQLType, RType, NAType>::RParamTemplate(
	SQLUSMALLINT  paramNumber,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   dataType,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType,
	const NAType  valueForNA)
	: RParam(paramNumber,
		paramName,
		paramNameLength,
		dataType,
		paramSize,
		decimalDigits,
		strLen_or_Ind,
		inputOutputType)
{
	SetRcppVector(paramValue, valueForNA);
}

//-------------------------------------------------------------------------------------------------
// Name: SetRcppVector
//
// Description:
// Templatized function to set RcppVector with given paramValue.
// This is only for numeric or integer R types.
// It is a wrapper to CreateVector with rowsNumber = 1.
// Before creation, it verifies whether m_size is equal to size of the SQL type.
// NA is R's special value indicative of null in C++.
// For each R type, there is a special NA value with a corresponding NA type in C++.
//
template<class SQLType, class RType, class NAType>
void RParamTemplate<SQLType, RType, NAType>::SetRcppVector(
	SQLPOINTER   paramValue,
	const NAType valueForNA)
{
	LOG("RParamTemplate::SetRcppVector");

	CheckParamSize<SQLType>();
	SQLINTEGER strLenOrInd = StrLenOrInd();
	if (strLenOrInd == SQL_NULL_DATA)
	{
		SQLINTEGER strLen_or_Ind[1] = {SQL_NULL_DATA};
		m_RcppVector = RTypeUtils::CreateVector<SQLType, RType, NAType>(
			1, // rowsNumber
			paramValue,
			strLen_or_Ind,
			valueForNA);
	}
	else
	{
		m_RcppVector = RTypeUtils::CreateVector<SQLType, RType, NAType>(
			1, // rowsNumber
			paramValue,
			nullptr, // strLen_or_Ind
			valueForNA);
	}
}

//-------------------------------------------------------------------------------------------------
// Name: RLogicalParam
//
// Description:
// Constructor.
//
RLogicalParam::RLogicalParam(
	SQLUSMALLINT  paramNumber,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   dataType,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType)
	: RParam(paramNumber,
		paramName,
		paramNameLength,
		dataType,
		paramSize,
		decimalDigits,
		strLen_or_Ind,
		inputOutputType)
{
	SetRcppVector(paramValue);
}

//-------------------------------------------------------------------------------------------------
// Name: SetRcppVector
//
// Description:
// Set the RcppVector for RLogicalParam with the given paramValue.
// This is a wrapper to CreateLogicalVector with rowsNumber = 1.
// Before creation, it verifies whether m_size is equal to size of SQLCHAR.
// For null parameters, a size one vector with member value = NA_LOGICAL is created.
//
void RLogicalParam::SetRcppVector(SQLPOINTER paramValue)
{
	LOG("RLogicalParam::SetRcppVector");

	CheckParamSize<SQLCHAR>();
	SQLINTEGER strLenOrInd = StrLenOrInd();
	if (strLenOrInd == SQL_NULL_DATA)
	{
		SQLINTEGER strLen_or_Ind[1] = {SQL_NULL_DATA};
		m_RcppVector = RTypeUtils::CreateLogicalVector(
			1, // rowsNumber
			paramValue,
			strLen_or_Ind);
	}
	else
	{
		m_RcppVector = RTypeUtils::CreateLogicalVector(
			1, // rowsNumber
			paramValue,
			nullptr); // strLenOrInd
	}
}

//-------------------------------------------------------------------------------------------------
// Name: RCharacterParam
//
// Description:
// Constructor.
//
RCharacterParam::RCharacterParam(
	SQLUSMALLINT  paramNumber,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   dataType,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType)
	: RParam(paramNumber,
		paramName,
		paramNameLength,
		dataType,
		paramSize,
		decimalDigits,
		strLen_or_Ind,
		inputOutputType)
{
	SetRcppVector(paramValue);
}

//-------------------------------------------------------------------------------------------------
// Name: SetRcppVector
//
// Description:
// Set the RcppVector for RCharacterParam with the given paramValue.
// This is a wrapper to CreateCharacterVector with rowsNumber = 1.
// For null parameters, a size one vector with member value = NA_STRING is created.
//
void RCharacterParam::SetRcppVector(SQLPOINTER paramValue)
{
	LOG("RCharacterParam::SetRcppVector");

	SQLINTEGER strLenOrInd = StrLenOrInd();
	if (strLenOrInd == SQL_NULL_DATA)
	{
		m_RcppVector = RTypeUtils::CreateCharacterVector(
			1,       // rowsNumber
			nullptr, // data
			nullptr);// strLen_or_Ind
	}
	else
	{
		SQLINTEGER strLen_or_IndArray[1] = { strLenOrInd };
		m_RcppVector = RTypeUtils::CreateCharacterVector(
			1, //rowsNumber
			paramValue,
			strLen_or_IndArray);
	}
}

//-------------------------------------------------------------------------------------------------
// Name: RRawParam
//
// Description:
// Constructor.
//
RRawParam::RRawParam(
	SQLUSMALLINT  paramNumber,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   dataType,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType)
	: RParam(paramNumber,
		paramName,
		paramNameLength,
		dataType,
		paramSize,
		decimalDigits,
		strLen_or_Ind,
		inputOutputType)
{
	SetRcppVector(paramValue);
}

//-------------------------------------------------------------------------------------------------
// Name: SetRcppVector
//
// Description:
// Set the RcppVector for RRawParam with the given paramValue.
// This is a wrapper to CreateRawVector with rowsNumber = 1.
// For null parameters, a size one vector with member value = raw(0) is created.
//
void RRawParam::SetRcppVector(SQLPOINTER paramValue)
{
	LOG("RRawParam::SetRcppVector");

	SQLINTEGER strLen_or_IndArray[1] = {0};
	SQLINTEGER strLenOrInd = StrLenOrInd();
	strLen_or_IndArray[0] = { strLenOrInd };
	m_RcppVector = RTypeUtils::CreateRawVector(
		1,        // rowsNumber
		paramValue,
		strLen_or_IndArray);
}

//--------------------------------------------------------------------------------------------------
// Do explicit template instantiations, so that object code is generated for these
// and the linker is able to find their definitions even after instantiations are in different
// translation units (i.e. RParamTemplate instantiation is in RParamContainer.cpp)
//
template RParamTemplate<SQLINTEGER, Rcpp::IntegerVector, int>::RParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT,
	int);

template RParamTemplate<SQLREAL, Rcpp::NumericVector, double>::RParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT,
	double);

template RParamTemplate<SQLDOUBLE, Rcpp::NumericVector, double>::RParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT,
	double);

template RParamTemplate<SQLBIGINT, Rcpp::NumericVector, double>::RParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT,
	double);

template RParamTemplate<SQLSMALLINT, Rcpp::IntegerVector, int>::RParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT,
	int);

template RParamTemplate<SQLCHAR, Rcpp::IntegerVector, int>::RParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT,
	int);
