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
// @File: RParam.h
//
// Purpose:
// Classes storing information about an RExtension input/output parameter.
//
//*************************************************************************************************
#pragma once

using namespace std;

//-------------------------------------------------------------------------------------------------
// Name: RParam
//
// Description:
// Base class storing information about the RExtension input/output parameter.
//
class RParam
{
	friend class RParamContainer;

public:

	// Constructor to initialize the members
	//
	RParam(
		SQLUSMALLINT id,
		string       name,
		SQLSMALLINT  type,
		SQLULEN      size,
		SQLSMALLINT  decimalDigits,
		SQLINTEGER   strLen_or_Ind,
		SQLSMALLINT  inputOutputType);

	// Verifies if the input paramSize is equal to the size of the template type T.
	//
	template<class T>
	void CheckParamSize();

	// Get m_name
	//
	string GetName() const
	{
		return m_name;
	}

	// Get m_strLenOrInd
	//
	SQLINTEGER GetStrLenOrInd()
	{
		return m_strLenOrInd;
	}

private:

	// Id of the parameter.
	//
	SQLUSMALLINT m_id;

	// Name of the parameter.
	//
	string m_name;

	// Data type of the parameter.
	//
	SQLSMALLINT m_type;

	// Size of the parameter.
	//
	SQLULEN m_size;

	// Decimal digits of the parameter.
	//
	SQLSMALLINT m_decimalDigits;

	// Indicates string length or SQL_NULL_DATA for null.
	// Note about expected m_strLenOrInd, m_size according to type:
	// For fixed non-char non-binary types,
	//   value    m_strLenorInd    m_size
	//   NULL     SQL_NULL_DATA   sizeof<type>
	//   Non-NULL   0              sizeof<type>
	// For char(n), binary(n) types,
	//   value    m_strLenOrInd    m_size
	//   NULL      SQL_NULL_DATA    n
	//   Non-NULL   n               n
	// For varchar(n), varbinary(n) types,
	//   value    m_strLenOrInd     m_size
	//   NULL     SQL_NULL_DATA     n
	//   Non-NULL  actualNumberOfChars n
	//
	SQLINTEGER m_strLenOrInd;

	// Parameter type (Input/Output)
	//
	SQLSMALLINT m_inputOutputType;
};

//-------------------------------------------------------------------------------------------------
// Name: RParamTemplate
//
// Description:
// Template class representing numeric, integer parameters by storing the RcppVector of
// given RType for the corresponding SQLType and having NA value of NAType.
//
template<class SQLType, class RType, class NAType>
class RParamTemplate : public RParam
{

public:
	// Constructor to initialize the members
	//
	RParamTemplate(
		SQLUSMALLINT id,
		string       name,
		SQLSMALLINT  type,
		SQLULEN      size,
		SQLSMALLINT  decimalDigits,
		SQLPOINTER   paramValue,
		SQLINTEGER   strLen_or_Ind,
		SQLSMALLINT  inputOutputType,
		const NAType valueForNA);

	// Get m_RcppVector
	//
	RType& GetRcppVector()
	{
		return m_RcppVector;
	}

	// Templatized function to set the RcppVector by creating an equivalent R type
	// for the given SQL type with given value. This is only for numeric or integer R types.
	// NA is R's special value indicative of null in C++.
	// For each R type, there is a special NA value with a corresponding NA type in C++.
	//
	void SetRcppVector(SQLPOINTER paramValue, const NAType valueForNA);

	// The Rcpp::Vector encapsulating the SEXP pointer
	// pointing to the R object with the param value.
	//
	RType m_RcppVector;
};

//-------------------------------------------------------------------------------------------------
// Name: RLogicalParam
//
// Description:
// Class representing a logical parameter by storing Rcpp::LogicalVector
// corresponding to ODBC C type SQL_C_BIT and SQL type SQLCHAR.
//
class RLogicalParam : public RParam
{
public :

	// Constructor to initialize the members
	//
	RLogicalParam(
		SQLUSMALLINT id,
		string       name,
		SQLSMALLINT  type,
		SQLULEN      size,
		SQLSMALLINT  decimalDigits,
		SQLPOINTER   paramValue,
		SQLINTEGER   strLen_or_Ind,
		SQLSMALLINT  inputOutputType);

	// Get m_RcppVector
	//
	Rcpp::LogicalVector& GetRcppVector()
	{
		return m_RcppVector;
	}

	// Set the RcppVector by creating a logical object in R with given value.
	//
	void SetRcppVector(SQLPOINTER paramValue);

private:

	// The Rcpp::LogicalVector encapsulating the SEXP pointer
	// pointing to the R object with the param value.
	//
	Rcpp::LogicalVector m_RcppVector;
};

//-------------------------------------------------------------------------------------------------
// Name: RCharacterParam
//
// Description:
// Class representing a character parameter by storing Rcpp::CharacterVector.
//
class RCharacterParam : public RParam
{
public :

	// Constructor to initialize the members
	//
	RCharacterParam(
		SQLUSMALLINT id,
		string       name,
		SQLSMALLINT  type,
		SQLULEN      size,
		SQLSMALLINT  decimalDigits,
		SQLPOINTER   paramValue,
		SQLINTEGER   strLen_or_Ind,
		SQLSMALLINT  inputOutputType);

	// Get m_RcppVector
	//
	Rcpp::CharacterVector& GetRcppVector()
	{
		return m_RcppVector;
	}

	// Set the RcppVector by creating a character object in R with given value.
	//
	void SetRcppVector(SQLPOINTER paramValue);

private:

	// The Rcpp::CharacterVector encapsulating the SEXP pointer
	// pointing to the R object with the param value.
	//
	Rcpp::CharacterVector m_RcppVector;
};

//-------------------------------------------------------------------------------------------------
// Name: RRawParam
//
// Description:
// Class representing a raw parameter by storing Rcpp::RawVector.
//
class RRawParam : public RParam
{
public:

	// Constructor to initialize the members
	//
	RRawParam(
		SQLUSMALLINT id,
		string       name,
		SQLSMALLINT  type,
		SQLULEN      size,
		SQLSMALLINT  decimalDigits,
		SQLPOINTER   paramValue,
		SQLINTEGER   strLen_or_Ind,
		SQLSMALLINT  inputOutputType);

	// Get m_RcppVector
	//
	Rcpp::RawVector& GetRcppVector()
	{
		return m_RcppVector;
	}


	// Set the RcppVector by creating a raw object in R with given value.
	//
	void SetRcppVector(SQLPOINTER paramValue);

private:

	// The Rcpp::RawVector encapsulating the SEXP pointer
	// pointing to the R object with the param value.
	//
	Rcpp::RawVector m_RcppVector;
};
