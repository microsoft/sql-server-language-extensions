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

// @File: RTypeUtils.cpp
//
// Purpose:
//  Class providing functions to manipulate between R data types and SQL types.
//
//*************************************************************************************************

#include "Common.h"
#include <iostream>

#include "Logger.h"
#include "RTypeUtils.h"

#include "Rcpp.h"

using namespace std;

//-------------------------------------------------------------------------------------------------
// Map to store the ODBC C datatype to NA value in R.
// R_NaInt, R_NaReal are R's special values indicative of null in C++.
//
const unordered_map<SQLSMALLINT, SQLPOINTER> RTypeUtils::m_dataTypeToNAMap =
{
	{SQL_C_BIT, static_cast<SQLPOINTER>(&R_NaInt)},
	{SQL_C_SLONG, static_cast<SQLPOINTER>(&R_NaInt)},
	{SQL_C_FLOAT, static_cast<SQLPOINTER>(&R_NaReal)},
	{SQL_C_DOUBLE, static_cast<SQLPOINTER>(&R_NaReal)},
	{SQL_C_SBIGINT, static_cast<SQLPOINTER>(&R_NaReal)},
	{SQL_C_SSHORT, static_cast<SQLPOINTER>(&R_NaInt)},
	{SQL_C_UTINYINT, static_cast<SQLPOINTER>(&R_NaInt)}
};

// Map to store the R class to ODBC C dataType mapping
//
const unordered_map<string, SQLSMALLINT> RTypeUtils::m_classInRToOdbcTypeMap =
{
	{"logical", SQL_C_BIT},
	{"integer", SQL_C_SLONG},
	{"numeric", SQL_C_DOUBLE},
	{"character", SQL_C_CHAR},
	{"raw", SQL_C_BINARY}
};

//-------------------------------------------------------------------------------------------------
// Name: CreateVector
//
// Description:
//  Templatized function to create an Rcpp vector encapsulating SEXP pointers to the equivalent R
//  objects for the given SQL type with given data. This is only for numeric or integer R types.
//  rowsNumber indicates the number of elements to be added in the vector.
//  Iterate over data to push_back the value to the vector.
//  If at any index strLen_or_Ind is SQL_NULL_DATA, it fills in the equivalent R NA value instead.
//  Creating the R object and Encapsulating an SEXP pointer (pointing to the R object) in an Rcpp object.
//
template<class SQLType, class RType, class NAType, SQLSMALLINT DataType>
RType RTypeUtils::CreateVector(
	SQLULEN      rowsNumber,
	SQLPOINTER   data,
	SQLINTEGER   *strLen_or_Ind)
{
	LOG("RTypeUtils::CreateVector");

	RType vectorInR(rowsNumber);
	for (SQLULEN j = 0; j < rowsNumber; j++)
	{
		if (strLen_or_Ind != nullptr && strLen_or_Ind[j] == SQL_NULL_DATA)
		{
			NAType valueForNA = *(static_cast<NAType*>(m_dataTypeToNAMap.at(DataType)));
			vectorInR[j] = valueForNA;
		}
		else
		{
			SQLType value = static_cast<SQLType *>(data)[j];
			vectorInR[j] = value;
		}
	}

	return vectorInR;
}

//-------------------------------------------------------------------------------------------------
// Name: CreateLogicalVector
//
// Description:
//  Create an Rcpp logical vector encapsulating SEXP pointers to the equivalent R boolean objects
//  with the given data. rowsNumber indicates the number of elements to be added in the vector.
//  Iterate over data to push_back the value to the vector. False if value is '0', else true.
//  If at any index strLen_or_Ind is SQL_NULL_DATA, it fills in the NA_LOGICAL value.
//
Rcpp::LogicalVector RTypeUtils::CreateLogicalVector(
	SQLULEN    rowsNumber,
	SQLPOINTER data,
	SQLINTEGER *strLen_or_Ind)
{
	LOG("RTypeUtils::CreateLogicalVector");

	Rcpp::LogicalVector logicalVector(rowsNumber);
	for (SQLULEN j = 0; j < rowsNumber; j++)
	{
		if (strLen_or_Ind != nullptr && strLen_or_Ind[j] == SQL_NULL_DATA)
		{
			logicalVector[j] = NA_LOGICAL;
		}
		else
		{
			SQLCHAR value = static_cast<SQLCHAR *>(data)[j];
			if (value != '0')
			{
				logicalVector[j] = true;
			}
			else
			{
				logicalVector[j] = false;
			}
		}
	}

	return logicalVector;
}

//-------------------------------------------------------------------------------------------------
// Name: CreateCharacterVector
//
// Description:
//  Create a character Rcpp vector encapsulating SEXP pointers to the equivalent R character
//  objects with the given data. rowsNumber indicates the number of elements
//  to be added in the vector. Iterate over data to push_back the value to the vector.
//  If at any index strLen_or_Ind is SQL_NULL_DATA, it fills in the NA_STRING value.
//
Rcpp::CharacterVector RTypeUtils::CreateCharacterVector(
	SQLULEN    rowsNumber,
	SQLPOINTER data,
	SQLINTEGER *strLen_or_Ind)
{
	LOG("RTypeUtils::CreateCharacterVector");

	char *baseCharData = static_cast<char *>(data);
	SQLULEN cumulativeLength = 0;
	Rcpp::CharacterVector charVector(rowsNumber);

	for (SQLULEN j = 0; j < rowsNumber; j++)
	{
		if ((strLen_or_Ind == nullptr) ||
			(strLen_or_Ind != nullptr && strLen_or_Ind[j] == SQL_NULL_DATA))
		{
			charVector[j] = NA_STRING;
		}
		else
		{
			char *str = baseCharData + cumulativeLength;
			SQLINTEGER strlen = strLen_or_Ind[j] / sizeof(char);
			string value(str, strlen);
			charVector[j] = value.c_str();
			cumulativeLength += strlen;
		}
	}

	return charVector;
}

//-------------------------------------------------------------------------------------------------
// Name: CreateRawVector
//
// Description:
//  Create a raw Rcpp vector in R encapsulating SEXP pointers to the equivalent R raw objects
//  with the given data. rowsNumber indicates the number of elements to be added in the vector.
//  strLen_or_Ind indicates the length of each element.
//  Iterate over data and for each element, over each byte to push_back the value to the vector.
//  If at any index strLen_or_Ind is SQL_NULL_DATA, it fills in the empty raw(0) element.
//
Rcpp::RawVector RTypeUtils::CreateRawVector(
	SQLULEN    rowsNumber,
	SQLPOINTER data,
	SQLINTEGER *strLen_or_Ind)
{
	LOG("RTypeUtils::CreateRawVector");

	Rcpp::RawVector rawVector = Rcpp::RawVector::create();
	SQLCHAR* baseRawData = static_cast<SQLCHAR *>(data);
	int cumulativeRawDataLength = 0;

	for (SQLULEN j = 0; j < rowsNumber; j++)
	{
		if ((strLen_or_Ind == nullptr) ||
			(strLen_or_Ind != nullptr && strLen_or_Ind[j] == SQL_NULL_DATA))
		{
			// For raw, 0 (the nul byte) represents NA.
			// https://www.rdocumentation.org/packages/base/versions/3.6.2/topics/raw
			//
			rawVector.push_back(0);
		}
		else
		{
			SQLCHAR* rawData = static_cast<SQLCHAR *>(baseRawData) + cumulativeRawDataLength;
			for(int index = 0; index < strLen_or_Ind[j]; ++index)
			{
				SQLCHAR value = *(rawData + index);
				rawVector.push_back(value);
			}

			cumulativeRawDataLength += strLen_or_Ind[j];
		}
	}

	return rawVector;
}

//-------------------------------------------------------------------------------------------------
// Do explicit template instantiations, so that object code is generated for these
// and the linker is able to find their definitions even after instantiations are in different
// translation units (i.e. CreateVector instantiation is in RParam.cpp)
//
template Rcpp::IntegerVector RTypeUtils::CreateVector<SQLINTEGER, Rcpp::IntegerVector, int, SQL_C_SLONG>(
	SQLULEN    rowsNumber,
	SQLPOINTER data,
	SQLINTEGER *strLen_or_Ind);

template Rcpp::NumericVector RTypeUtils::CreateVector<SQLREAL, Rcpp::NumericVector, double, SQL_C_FLOAT>(
	SQLULEN      rowsNumber,
	SQLPOINTER   data,
	SQLINTEGER   *strLen_or_Ind);

template Rcpp::NumericVector RTypeUtils::CreateVector<SQLDOUBLE, Rcpp::NumericVector, double, SQL_C_DOUBLE>(
	SQLULEN      rowsNumber,
	SQLPOINTER   data,
	SQLINTEGER   *strLen_or_Ind);

template Rcpp::NumericVector RTypeUtils::CreateVector<SQLBIGINT, Rcpp::NumericVector, double, SQL_C_SBIGINT>(
	SQLULEN      rowsNumber,
	SQLPOINTER   data,
	SQLINTEGER   *strLen_or_Ind);

template Rcpp::IntegerVector RTypeUtils::CreateVector<SQLSMALLINT, Rcpp::IntegerVector, int, SQL_C_SSHORT>(
	SQLULEN    rowsNumber,
	SQLPOINTER data,
	SQLINTEGER *strLen_or_Ind);

template Rcpp::IntegerVector RTypeUtils::CreateVector<SQLCHAR, Rcpp::IntegerVector, int, SQL_C_UTINYINT>(
	SQLULEN    rowsNumber,
	SQLPOINTER data,
	SQLINTEGER *strLen_or_Ind);
