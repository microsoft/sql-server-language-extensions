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
#include <sqlext.h>

#include "Logger.h"
#include "RTypeUtils.h"

#include "Rcpp.h"

using namespace std;

//-------------------------------------------------------------------------------------------------
// Name: CreateVector
//
// Description:
// Templatized function to create an Rcpp vector encapsulating SEXP pointers to the equivalent R
// objects for the given SQL type with given data. This is only for numeric or integer R types.
// rowsNumber indicates the number of elements to be added in the vector.
// Iterate over data to push_back the value to the vector.
// If at any index strLen_or_Ind is SQL_NULL_DATA, it fills in the equivalent R NA value instead.
// Creating the R object and Encapsulating an SEXP pointer (pointing to the R object) in an Rcpp object.
//
template<class SQLType, class RType, class NAType>
RType RTypeUtils::CreateVector(
	SQLULEN      rowsNumber,
	SQLPOINTER   data,
	SQLINTEGER   *strLen_or_Ind,
	const NAType valueForNA)
{
	LOG("RTypeUtils::CreateVector");

	RType vectorInR = RType::create();
	for (SQLULEN j = 0; j < rowsNumber; j++)
	{
		if (strLen_or_Ind != nullptr && strLen_or_Ind[j] == SQL_NULL_DATA)
		{
			vectorInR.push_back(valueForNA);
		}
		else
		{
			SQLType value = reinterpret_cast<SQLType *>(data)[j];
			vectorInR.push_back(value);
		}
	}

	return vectorInR;
}

//-------------------------------------------------------------------------------------------------
// Name: CreateLogicalVector
//
// Description:
// Create an Rcpp logical vector encapsulating SEXP pointers to the equivalent R boolean objects
// with the given data. rowsNumber indicates the number of elements to be added in the vector.
// Iterate over data to push_back the value to the vector. False if value is '0', else true.
// If at any index strLen_or_Ind is SQL_NULL_DATA, it fills in the NA_LOGICAL value.
//
Rcpp::LogicalVector RTypeUtils::CreateLogicalVector(
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER *strLen_or_Ind)
{
	LOG("RTypeUtils::CreateLogicalVector");

	Rcpp::LogicalVector logicalVector = Rcpp::LogicalVector::create();
	for (SQLULEN j = 0; j < rowsNumber; j++)
	{
		if (strLen_or_Ind != nullptr && strLen_or_Ind[j] == SQL_NULL_DATA)
		{
			logicalVector.push_back(NA_LOGICAL);
		}
		else
		{
			SQLCHAR value = reinterpret_cast<SQLCHAR *>(data)[j];
			if (value != '0')
			{
				logicalVector.push_back(true);
			}
			else
			{
				logicalVector.push_back(false);
			}
		}
	}

	return logicalVector;
}

//-------------------------------------------------------------------------------------------------
// Name: CreateCharacterVector
//
// Description:
// Create a character Rcpp vector encapsulating SEXP pointers to the equivalent R character
// objects with the given data. rowsNumber indicates the number of elements
// to be added in the vector. Iterate over data to push_back the value to the vector.
// If at any index strLen_or_Ind is SQL_NULL_DATA, it fills in the NA_STRING value.
//
Rcpp::CharacterVector RTypeUtils::CreateCharacterVector(
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER *strLen_or_Ind)
{
	LOG("RTypeUtils::CreateCharacterVector");

	char *baseCharData = reinterpret_cast<char *>(data);
	SQLULEN cumulativeLength = 0;
	Rcpp::CharacterVector charVector = Rcpp::CharacterVector::create();

	for (SQLULEN j = 0; j < rowsNumber; j++)
	{
		if ((strLen_or_Ind == nullptr) ||
			(strLen_or_Ind != nullptr && strLen_or_Ind[j] == SQL_NULL_DATA))
		{
			charVector.push_back(NA_STRING);
		}
		else
		{
			char *str = baseCharData + cumulativeLength;
			SQLINTEGER strlen = strLen_or_Ind[j] / sizeof(char);
			string value(str, strlen);
			charVector.push_back(value.c_str());
			cumulativeLength += strlen;
		}
	}

	return charVector;
}

//-------------------------------------------------------------------------------------------------
// Name: CreateRawVector
//
// Description:
// Create a raw Rcpp vector in R encapsulating SEXP pointers to the equivalent R raw objects
// with the given data. rowsNumber indicates the number of elements to be added in the vector.
// strLen_or_Ind indicates the length of each element.
// Iterate over data and for each element, over each byte to push_back the value to the vector.
// If at any index strLen_or_Ind is SQL_NULL_DATA, it fills in the empty raw(0) element.
//
Rcpp::RawVector RTypeUtils::CreateRawVector(
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER *strLen_or_Ind)
{
	LOG("RTypeUtils::CreateRawVector");

	Rcpp::RawVector rawVector = Rcpp::RawVector::create();
	Rcpp::RawVector valueForNA = Rcpp::RawVector::create();
	SQLCHAR* baseRawData = reinterpret_cast<SQLCHAR *>(data);
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
			SQLCHAR* rawData = reinterpret_cast<SQLCHAR *>(baseRawData) + cumulativeRawDataLength;
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

// Do explicit template instantiations, so that object code is generated for these
// and the linker is able to find their definitions even after instantiations are in different
// translation units (i.e. CreateVector instantiation is in RParam.cpp)
//
template Rcpp::IntegerVector RTypeUtils::CreateVector<SQLINTEGER, Rcpp::IntegerVector, int>(
	SQLULEN    rowsNumber,
	SQLPOINTER data,
	SQLINTEGER *strLen_or_Ind,
	const int  valueForNA);

template Rcpp::NumericVector RTypeUtils::CreateVector<SQLREAL, Rcpp::NumericVector, double>(
	SQLULEN      rowsNumber,
	SQLPOINTER   data,
	SQLINTEGER   *strLen_or_Ind,
	const double valueForNA);

template Rcpp::NumericVector RTypeUtils::CreateVector<SQLDOUBLE, Rcpp::NumericVector, double>(
	SQLULEN      rowsNumber,
	SQLPOINTER   data,
	SQLINTEGER   *strLen_or_Ind,
	const double valueForNA);

template Rcpp::NumericVector RTypeUtils::CreateVector<SQLBIGINT, Rcpp::NumericVector, double>(
	SQLULEN      rowsNumber,
	SQLPOINTER   data,
	SQLINTEGER   *strLen_or_Ind,
	const double valueForNA);

template Rcpp::IntegerVector RTypeUtils::CreateVector<SQLSMALLINT, Rcpp::IntegerVector, int>(
	SQLULEN    rowsNumber,
	SQLPOINTER data,
	SQLINTEGER *strLen_or_Ind,
	const int  valueForNA);

template Rcpp::IntegerVector RTypeUtils::CreateVector<SQLCHAR, Rcpp::IntegerVector, int>(
	SQLULEN    rowsNumber,
	SQLPOINTER data,
	SQLINTEGER *strLen_or_Ind,
	const int  valueForNA);
