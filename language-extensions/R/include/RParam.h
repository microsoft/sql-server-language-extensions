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
// @File: RParam.h
//
// Purpose:
//  Classes storing information about an RExtension input/output parameter.
//
//**************************************************************************************************

#pragma once

//--------------------------------------------------------------------------------------------------
// Name: RParam
//
// Description:
//  Base class storing information about the RExtension input/output parameter.
//
class RParam
{
public:

	// Retrieves data from m_RcppVector, fill it in m_value
	// and set m_strLenOrInd accordingly.
	//
	virtual void RetrieveValueAndStrLenInd() = 0;

	// Gets m_value
	//
	virtual SQLPOINTER Value() const = 0;

	// Gets m_name
	//
	const std::string& Name() const
	{
		return m_name;
	}

	// Gets m_strLenOrInd
	//
	SQLINTEGER StrLenOrInd() const
	{
		return m_strLenOrInd;
	}

	// Gets m_inputOutputType
	//
	SQLSMALLINT InputOutputType() const
	{
		return m_inputOutputType;
	}

protected:

	// Protected constructor to initialize the members.
	// Do not allow other classes to create objects of this class - it is used as a base class.
	//
	RParam(
		SQLUSMALLINT  paramNumber,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   dataType,
		SQLULEN       paramSize,
		SQLSMALLINT   decimalDigits,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   inputOutputType);

	// Verifies if the input paramSize is equal to the size of the template type T.
	//
	template<class T>
	void CheckParamSize();

	// Id of the parameter.
	//
	SQLUSMALLINT m_id;

	// Name of the parameter.
	//
	std::string m_name;

	// Data type of the parameter.
	//
	SQLSMALLINT m_dataType;

	// Size of the parameter.
	//
	SQLULEN m_size;

	// Decimal digits of the parameter.
	//
	SQLSMALLINT m_decimalDigits;

	// Indicates string length or SQL_NULL_DATA for null.
	// Note about expected m_strLenOrInd, m_size according to type:
	// For fixed non-char non-binary types,
	//     value   |  m_strLenorInd                                |  m_size
	//----------------------------------------------------------------------------
	//     NULL    |  SQL_NULL_DATA                                |  sizeof<type>
	//     Non-NULL|  0                                            |  sizeof<type>
	// For char(n), binary(n) types,
	//     value   |  m_strLenOrInd                                |  m_size
	//----------------------------------------------------------------------------
	//     NULL    |  SQL_NULL_DATA                                |  n
	//     Non-NULL|  n                                            |  n
	// For varchar(n), varbinary(n) types,
	//     value   |  m_strLenOrInd                                |  m_size
	//----------------------------------------------------------------------------
	//     NULL    |  SQL_NULL_DATA                                |  n
	//     Non-NULL|  actualNumberOfBytes(same as length)          |  n
	// For nchar(n) type,
	//     value   |  m_strLenOrInd                                |  m_size
	//----------------------------------------------------------------------------
	//     NULL    |  SQL_NULL_DATA                                |  n
	//     Non-NULL|  n*sizeof(char16_t))                          |  n
	// For nvarchar(n) type,
	//     value   |  m_strLenOrInd                                |  m_size
	//----------------------------------------------------------------------------
	//     NULL    |  SQL_NULL_DATA                                |  n
	//     Non-NULL|  actualNumberOfBytes(length*sizeof(char16_t)) |  n
	//
	SQLINTEGER m_strLenOrInd;

	// Parameter type (Input/Output)
	//
	SQLSMALLINT m_inputOutputType;
};

//--------------------------------------------------------------------------------------------------
// Name: RParamTemplate
//
// Description:
//  Template class representing numeric, integer, logical parameters by storing the RcppVector of
//  given RVectorType for the corresponding SQLType and having NA value of NAType.
//
template<class SQLType, class RVectorType, class NAType, SQLSMALLINT DataType>
class RParamTemplate : public RParam
{

public:
	// Constructor to initialize the members
	//
	RParamTemplate(
		SQLUSMALLINT  paramNumber,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   dataType,
		SQLULEN       paramSize,
		SQLSMALLINT   decimalDigits,
		SQLPOINTER    paramValue,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   inputOutputType);

	// Retrieves data from m_RcppVector, fill it in m_value
	// and set m_strLenOrInd accordingly
	//
	void RetrieveValueAndStrLenInd() override;

	// Gets m_RcppVector
	//
	RVectorType& RcppVector()
	{
		return m_RcppVector;
	}

	// Gets the data underlying m_value vector
	//
	SQLPOINTER Value() const override
	{
		if (m_value.size() > 0)
		{
			return static_cast<SQLPOINTER>(
				const_cast<SQLType*>(m_value.data()));
		}
		else
		{
			return nullptr;
		}
	}

private:

	// Templatized function to set the RcppVector by creating an equivalent R type
	// for the given SQL type with given value.
	//
	void SetRcppVector(SQLPOINTER paramValue);

	// Vector holding the value of the parameter as retrieved from embedded R environment,
	// holding the contents before sending them back to ExtHost
	// Only useful for output parameter types.
	//
	std::vector<SQLType> m_value;

	// The Rcpp::Vector encapsulating the SEXP pointer
	// pointing to the R object with the param value.
	//
	RVectorType m_RcppVector;
};

//--------------------------------------------------------------------------------------------------
// Name: RCharacterParam
//
// Description:
//  Class representing a character parameter by storing Rcpp::CharacterVector.
//  Corresponds to ODBC C type SQL_C_CHAR and SQL_C_WCHAR.
//
template<class CharType, class SQLType>
class RCharacterParam : public RParam
{
public :

	// Constructor to initialize the members
	//
	RCharacterParam(
		SQLUSMALLINT  paramNumber,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   dataType,
		SQLULEN       paramSize,
		SQLSMALLINT   decimalDigits,
		SQLPOINTER    paramValue,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   inputOutputType);

	// Retrieves data from m_RcppVector, fill it in m_value
	// and set m_strLenOrInd accordingly
	//
	void RetrieveValueAndStrLenInd() override;

	// Gets m_RcppVector
	//
	Rcpp::CharacterVector& RcppVector()
	{
		return m_RcppVector;
	}

	// Gets the data underlying m_value vector
	//
	SQLPOINTER Value() const override
	{
		if (m_value.size() > 0)
		{
			return static_cast<SQLPOINTER>(
				const_cast<SQLType*>(m_value.data()));
		}
		else
		{
			return nullptr;
		}
	}

private:

	// Sets the RcppVector by creating a character object in R with given value.
	//
	void SetRcppVector(SQLPOINTER paramValue);

	// Character vector holding the contents before sending them back to ExtHost.
	// Useful for output parameter types.
	//
	std::vector<SQLType> m_value;

	// The Rcpp::CharacterVector encapsulating the SEXP pointer
	// pointing to the R object with the param value.
	//
	Rcpp::CharacterVector m_RcppVector;
};

//--------------------------------------------------------------------------------------------------
// Name: RRawParam
//
// Description:
//  Class representing a raw parameter by storing Rcpp::RawVector.
//
class RRawParam : public RParam
{
public:

	// Constructor to initialize the members
	//
	RRawParam(
		SQLUSMALLINT  paramNumber,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   dataType,
		SQLULEN       paramSize,
		SQLSMALLINT   decimalDigits,
		SQLPOINTER    paramValue,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   inputOutputType);

	// Retrieves data from m_RcppVector, fill it in m_value
	// and set m_strLenOrInd accordingly
	//
	void RetrieveValueAndStrLenInd() override;

	// Gets m_RcppVector
	//
	Rcpp::RawVector& RcppVector()
	{
		return m_RcppVector;
	}

	// Gets the data underlying m_value vector
	//
	SQLPOINTER Value() const override
	{
		if (m_value.size() > 0)
		{
			return static_cast<SQLPOINTER>(
				const_cast<SQLCHAR*>(m_value.data()));
		}
		else
		{
			return nullptr;
		}
	}

private:

	// Sets the RcppVector by creating a raw object in R with given value.
	//
	void SetRcppVector(SQLPOINTER paramValue);

	// The Rcpp::RawVector encapsulating the SEXP pointer
	// pointing to the R object with the param value.
	//
	Rcpp::RawVector m_RcppVector;

	// Character vector holding the raw contents before sending them back to ExtHost.
	// Useful for output parameter types.
	//
	std::vector<SQLCHAR> m_value;
};

//--------------------------------------------------------------------------------------------------
// Name: RDateTimeParam
//
// Description:
//  Class representing a Date/DateTime parameter.
//  Corresponds to ODBC C type SQL_C_TYPE_DATE and SQL_C_TYPE_TIMESTAMP.
//  RVectorType is the parameter of type Rcpp vector where each of its elements are of the scalar type
//  DateTimeTypeInR.
//
template<class SQLType, class RVectorType, class DateTimeTypeInR>
class RDateTimeParam : public RParam
{
public:

	// Constructor to initialize the members
	//
	RDateTimeParam(
		SQLUSMALLINT  paramNumber,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   dataType,
		SQLULEN       paramSize,
		SQLSMALLINT   decimalDigits,
		SQLPOINTER    paramValue,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   inputOutputType);

	// Retrieves data from m_RcppVector, fill it in m_value
	// and set m_strLenOrInd accordingly
	//
	void RetrieveValueAndStrLenInd() override;

	// Gets m_RcppVector
	//
	RVectorType& RcppVector()
	{
		return m_RcppVector;
	}

	// Gets the data underlying m_value vector
	//
	SQLPOINTER Value() const override
	{
		if (m_value.size() > 0)
		{
			return static_cast<SQLPOINTER>(
				const_cast<SQLType *>(m_value.data()));
		}
		else
		{
			return nullptr;
		}
	}

private:

	// Templatized function to set the RcppVector by creating an equivalent R type
	// for the given SQL type with given value.
	//
	void SetRcppVector(SQLPOINTER paramValue);

	// Vector holding the value of the parameter as retrieved from embedded R environment,
	// holding the contents before sending them back to ExtHost
	// Only useful for output parameter types.
	//
	std::vector<SQLType> m_value;

	// The Rcpp::Vector encapsulating the SEXP pointer
	// pointing to the R object with the param value.
	//
	RVectorType m_RcppVector = RVectorType(1);
};

//--------------------------------------------------------------------------------------------------
// Name: RNumericParam
//
// Description:
//  Class representing a numeric(p,s) or decimal(p,s) parameter stored in a Rcpp::NumericVector.
//
//  Note: Although double or real SQL parameter types also map to the R 'numeric' class
//  and are also stored in an Rcpp::NumericVector, this class is not used for those types.
//  Rather an instantiation of RParamTemplate is used for them.
//
class RNumericParam : public RParam
{
public:

	// Constructor to initialize the members
	//
	RNumericParam(
		SQLUSMALLINT  paramNumber,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   dataType,
		SQLULEN       paramSize,
		SQLSMALLINT   decimalDigits,
		SQLPOINTER    paramValue,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   inputOutputType);

	// Retrieves data from m_RcppVector, fill it in m_value
	// and set m_strLenOrInd accordingly
	//
	void RetrieveValueAndStrLenInd() override;

	// Gets m_RcppVector
	//
	Rcpp::NumericVector& RcppVector()
	{
		return m_RcppVector;
	}

	// Gets the data underlying m_value vector
	//
	SQLPOINTER Value() const override
	{
		if (m_value.size() > 0)
		{
			return static_cast<SQLPOINTER>(
				const_cast<SQL_NUMERIC_STRUCT*>(m_value.data()));
		}
		else
		{
			return nullptr;
		}
	}

private:

	// Sets the RcppVector by creating a numeric vector in R with given value.
	//
	void SetRcppVector(SQLPOINTER paramValue);

	// The Rcpp::NumericVector encapsulating the SEXP pointer
	// pointing to the R object with the param value.
	//
	Rcpp::NumericVector m_RcppVector;

	// Vector holding the value of the parameter as retrieved from embedded R environment,
	// holding the contents before sending them back to ExtHost
	// Only useful for output parameter types.
	//
	std::vector<SQL_NUMERIC_STRUCT> m_value;

	// Store the precision so it can be used for sending back the output parameter.
	//
	SQLCHAR m_precision = 0;
};
