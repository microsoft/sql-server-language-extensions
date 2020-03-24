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
// @File: RParamContainer.cpp
//
// Purpose:
// RExtension input/output parameters wrappers, along with the container consolidating them.
//
//*************************************************************************************************

#include "Common.h"
#include <sqlext.h>
#include <string>

#include "Logger.h"
#include "RParam.h"
#include "RParamContainer.h"

#include "Rcpp.h"

using namespace std;

//-------------------------------------------------------------------------------------------------
// Name: Init
//
// Description:
// Initialize this container with the number of parameters.
//
void RParamContainer::Init(SQLSMALLINT paramsNumber)
{
	LOG("RParamContainer::Init");

	m_params.resize(paramsNumber);
}

//-------------------------------------------------------------------------------------------------
// Name: AddParamToEmbeddedR
//
// Description:
// Creates an RParam object containing an RcppVector, assigns the parameter name to this RcppVector
// so that the underlying R object can be accessed via this parameter name in the embedded R environment.
// Eventually, adds the RParam to m_params for future use.
// Creation is done by calling the respective constructor based on the datatype.
//
void RParamContainer::AddParamToEmbeddedR(
	SQLUSMALLINT paramNumber,
	std::string  name,
	SQLSMALLINT  dataType,
	SQLULEN      paramSize,
	SQLSMALLINT  decimalDigits,
	SQLPOINTER   paramValue,
	SQLINTEGER   strLen_or_Ind,
	SQLSMALLINT  inputOutputType)
{
	LOG("RParamContainer::AddParamToEmbeddedR");

	unique_ptr<RParam> paramToBeAdded = nullptr;
	const char* paramName = name.c_str();

	switch (dataType)
	{
		case SQL_C_SLONG:
		{
			paramToBeAdded =
				make_unique<RParamTemplate<SQLINTEGER, Rcpp::IntegerVector, int>>(
					paramNumber,
					name,
					dataType,
					paramSize,
					decimalDigits,
					paramValue,
					strLen_or_Ind,
					inputOutputType,
					NA_INTEGER);
			(*g_embeddedRPtr)[paramName] =
				reinterpret_cast<RParamTemplate<SQLINTEGER, Rcpp::IntegerVector, int>*>(
					paramToBeAdded.get())->GetRcppVector();
			break;
		}
		case SQL_C_BIT:
		{
			paramToBeAdded =
				make_unique<RLogicalParam>(
					paramNumber,
					name,
					dataType,
					paramSize,
					decimalDigits,
					paramValue,
					strLen_or_Ind,
					inputOutputType);
			(*g_embeddedRPtr)[paramName] =
				reinterpret_cast<RLogicalParam*>(paramToBeAdded.get())->GetRcppVector();
			break;
		}
		case SQL_C_FLOAT:
		{
			paramToBeAdded =
				make_unique<RParamTemplate<SQLREAL, Rcpp::NumericVector, double>>(
					paramNumber,
					name,
					dataType,
					paramSize,
					decimalDigits,
					paramValue,
					strLen_or_Ind,
					inputOutputType,
					NA_REAL);
			(*g_embeddedRPtr)[paramName] =
				reinterpret_cast<RParamTemplate<SQLREAL, Rcpp::NumericVector, double>*>(
					paramToBeAdded.get())->GetRcppVector();
			break;
		}
		case SQL_C_DOUBLE:
		{
			paramToBeAdded =
				make_unique<RParamTemplate<SQLDOUBLE, Rcpp::NumericVector, double>>(
					paramNumber,
					name,
					dataType,
					paramSize,
					decimalDigits,
					paramValue,
					strLen_or_Ind,
					inputOutputType,
					NA_REAL);
			(*g_embeddedRPtr)[paramName] =
				reinterpret_cast<RParamTemplate<SQLDOUBLE, Rcpp::NumericVector, double>*>(
					paramToBeAdded.get())->GetRcppVector();
			break;
		}
		case SQL_C_SBIGINT:
		{
			paramToBeAdded =
				make_unique<RParamTemplate<SQLBIGINT, Rcpp::NumericVector, double>>(
					paramNumber,
					name,
					dataType,
					paramSize,
					decimalDigits,
					paramValue,
					strLen_or_Ind,
					inputOutputType,
					NA_REAL);
			(*g_embeddedRPtr)[paramName] =
				reinterpret_cast<RParamTemplate<SQLBIGINT, Rcpp::NumericVector, double>*>(
					paramToBeAdded.get())->GetRcppVector();
			break;
		}
		case SQL_C_UTINYINT:
		{
			paramToBeAdded =
				make_unique<RParamTemplate<SQLCHAR, Rcpp::IntegerVector, int>>(
					paramNumber,
					name,
					dataType,
					paramSize,
					decimalDigits,
					paramValue,
					strLen_or_Ind,
					inputOutputType,
					NA_INTEGER);
			(*g_embeddedRPtr)[paramName] =
				reinterpret_cast<RParamTemplate<SQLCHAR, Rcpp::IntegerVector, int>*>(
					paramToBeAdded.get())->GetRcppVector();
			break;
		}
		case SQL_C_SSHORT:
		{
			paramToBeAdded =
				make_unique<RParamTemplate<SQLSMALLINT, Rcpp::IntegerVector, int>>(
					paramNumber,
					name,
					dataType,
					paramSize,
					decimalDigits,
					paramValue,
					strLen_or_Ind,
					inputOutputType,
					NA_INTEGER);
			(*g_embeddedRPtr)[paramName] =
				reinterpret_cast<RParamTemplate<SQLSMALLINT, Rcpp::IntegerVector, int>*>(
					paramToBeAdded.get())->GetRcppVector();
			break;
		}
		case SQL_C_CHAR:
		{
			paramToBeAdded =
				make_unique<RCharacterParam>(
					paramNumber,
					name,
					dataType,
					paramSize,
					decimalDigits,
					paramValue,
					strLen_or_Ind,
					inputOutputType);
			(*g_embeddedRPtr)[paramName] =
				reinterpret_cast<RCharacterParam*>(paramToBeAdded.get())->GetRcppVector();
			break;
		}
		case SQL_C_BINARY:
		{
			paramToBeAdded =
				make_unique<RRawParam>(
					paramNumber,
					name,
					dataType,
					paramSize,
					decimalDigits,
					paramValue,
					strLen_or_Ind,
					inputOutputType);
			(*g_embeddedRPtr)[paramName] =
				reinterpret_cast<RRawParam*>(paramToBeAdded.get())->GetRcppVector();
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

	Logger::LogRVariable(name);
	m_params[paramNumber] = std::move(paramToBeAdded);
}
