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
// @File: RParamContainer.cpp
//
// Purpose:
//  RExtension input/output parameters wrappers, along with the container consolidating them.
//
//**************************************************************************************************

#include "Common.h"

#include "RParam.h"
#include "RParamContainer.h"

using namespace std;

//--------------------------------------------------------------------------------------------------
// Function map - maps a ODBC C data type to the appropriate param creator
//
const RParamContainer::CreateParamFnMap RParamContainer::sm_FnCreateParamMap =
{
	{static_cast<SQLSMALLINT>(SQL_C_SLONG),
		static_cast<fnCreateParam>(&RParamContainer::CreateParam
		<RParamTemplate<SQLINTEGER, Rcpp::IntegerVector, int, SQL_C_SLONG>>)},
	{static_cast<SQLSMALLINT>(SQL_C_SBIGINT),
		static_cast<fnCreateParam>(&RParamContainer::CreateParam
		<RParamTemplate<SQLBIGINT, Rcpp::NumericVector, double, SQL_C_SBIGINT>>)},
	{static_cast<SQLSMALLINT>(SQL_C_FLOAT),
		static_cast<fnCreateParam>(&RParamContainer::CreateParam
		<RParamTemplate<SQLREAL, Rcpp::NumericVector, double, SQL_C_FLOAT>>)},
	{static_cast<SQLSMALLINT>(SQL_C_DOUBLE),
		static_cast<fnCreateParam>(&RParamContainer::CreateParam
		<RParamTemplate<SQLDOUBLE, Rcpp::NumericVector, double, SQL_C_DOUBLE>>)},
	{static_cast<SQLSMALLINT>(SQL_C_SSHORT),
		static_cast<fnCreateParam>(&RParamContainer::CreateParam
		<RParamTemplate<SQLSMALLINT, Rcpp::IntegerVector, int, SQL_C_SSHORT>>)},
	{static_cast<SQLSMALLINT>(SQL_C_UTINYINT),
		static_cast<fnCreateParam>(&RParamContainer::CreateParam
		<RParamTemplate<SQLCHAR, Rcpp::IntegerVector, int, SQL_C_UTINYINT>>)},
	{static_cast<SQLSMALLINT>(SQL_C_BIT),
		static_cast<fnCreateParam>(&RParamContainer::CreateParam
		<RParamTemplate<SQLCHAR, Rcpp::LogicalVector, int, SQL_C_BIT>>)},
	{static_cast<SQLSMALLINT>(SQL_C_CHAR),
		static_cast<fnCreateParam>(&RParamContainer::CreateParam<RCharacterParam<char, SQLCHAR>>)},
	{static_cast<SQLSMALLINT>(SQL_C_WCHAR),
		static_cast<fnCreateParam>(&RParamContainer::CreateParam<RCharacterParam<char16_t, SQLWCHAR>>)},
	{static_cast<SQLSMALLINT>(SQL_C_BINARY),
		static_cast<fnCreateParam>(&RParamContainer::CreateParam<RRawParam>)},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_DATE),
		static_cast<fnCreateParam>(&RParamContainer::CreateParam
		<RDateTimeParam<SQL_DATE_STRUCT, Rcpp::DateVector, Rcpp::Date>>)},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_TIMESTAMP),
		static_cast<fnCreateParam>(&RParamContainer::CreateParam
		<RDateTimeParam<SQL_TIMESTAMP_STRUCT, Rcpp::DatetimeVector, Rcpp::Datetime>>)},
	{static_cast<SQLSMALLINT>(SQL_C_NUMERIC),
		static_cast<fnCreateParam>(&RParamContainer::CreateParam<RNumericParam>)},
};

//--------------------------------------------------------------------------------------------------
// Name: RParamContainer::Init
//
// Description:
//  Initialize this container with the number of parameters.
//
void RParamContainer::Init(SQLSMALLINT paramsNumber)
{
	LOG("RParamContainer::Init");

	m_params.resize(paramsNumber);
}

//--------------------------------------------------------------------------------------------------
// Name: RParamContainer::AddParamToEmbeddedR
//
// Description:
//  Creates an RParam object containing an RcppVector, assigns the parameter name to this
//  RcppVector so that the underlying R object can be accessed via this parameter name in the
//  embedded R environment.
//  Eventually, adds the RParam to m_params for future use.
//  Creation is done by calling the respective constructor based on the datatype.
//
void RParamContainer::AddParamToEmbeddedR(
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
	LOG("RParamContainer::AddParamToEmbeddedR");

	CreateParamFnMap::const_iterator it = sm_FnCreateParamMap.find(dataType);

	if (it == sm_FnCreateParamMap.end())
	{
		throw runtime_error("Unsupported parameter type encountered when creating param #"
			+ to_string(paramNumber));
	}

	(this->*it->second)(
		paramNumber,
		paramName,
		paramNameLength,
		dataType,
		paramSize,
		decimalDigits,
		paramValue,
		strLen_or_Ind,
		inputOutputType);
}

//--------------------------------------------------------------------------------------------------
// Name: RParamContainer::CreateParam
//
// Description:
//  Creates an RParam object, adds the parameter with paramValue for given dataType
//  to the Embedded R environment and stores it in m_params for future use.
//
template<class RParamType>
void RParamContainer::CreateParam(
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
	LOG("RParamContainer::CreateParam");
	unique_ptr<RParam> paramToBeAdded =
		make_unique<RParamType>(
			paramNumber,
			paramName,
			paramNameLength,
			dataType,
			paramSize,
			decimalDigits,
			paramValue,
			strLen_or_Ind,
			inputOutputType);

	RInside* embeddedREnvPtr = REnvironment::EmbeddedREnvironment();
	(*embeddedREnvPtr)[paramToBeAdded.get()->Name().c_str()] =
		static_cast<RParamType*>(
			paramToBeAdded.get())->RcppVector();

	Logger::LogRVariable(paramToBeAdded.get()->Name());
	m_params[paramNumber] = std::move(paramToBeAdded);
}

//--------------------------------------------------------------------------------------------------
// Name: RParamContainer::GetParamValueAndStrLenInd
//
// Description:
//  For the given paramNumber, calls RetriveValueAndStrLenOrInd() to retrieve the value from R and
//  returns it via paramValue. Return the strLenOrInd as well.
//
void RParamContainer::GetParamValueAndStrLenInd(
	SQLUSMALLINT paramNumber,
	SQLPOINTER   *paramValue,
	SQLINTEGER   *strLen_or_Ind)
{
	LOG("RParamContainer::GetParamValueAndStrLenInd");

	if (m_params[paramNumber] == nullptr)
	{
		throw runtime_error("InitParam not called for param #" + to_string(paramNumber));
	}

	RParam *param = m_params[paramNumber].get();

	if (param->InputOutputType() < SQL_PARAM_INPUT_OUTPUT)
	{
		throw runtime_error("Requested param #" + to_string(paramNumber) +
			" is not initialized as an output parameter");
	}

	// Retrieve the value from R
	//
	param->RetrieveValueAndStrLenInd();

	*paramValue = param->Value();
	*strLen_or_Ind = param->StrLenOrInd();
}
