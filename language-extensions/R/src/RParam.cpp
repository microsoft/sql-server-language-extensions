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
// @File: RParam.cpp
//
// Purpose:
//  Class storing information about the RExtension input/output parameter.
//
//**************************************************************************************************

#include "Common.h"

#include "RParam.h"
#include "RTypeUtils.h"

using namespace std;

//--------------------------------------------------------------------------------------------------
// Name: RParam::RParam
//
// Description:
//  Constructor.
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
}

//--------------------------------------------------------------------------------------------------
// Name: RParam::CheckParamSize
//
// Description:
//  Verifies if m_Size is equal to the size of the template type T.
//  Returns nothing if the check succeeds, throws an exception otherwise.
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

//--------------------------------------------------------------------------------------------------
// Name: RParamTemplate::RParamTemplate
//
// Description:
//  Constructor.
//
template<class SQLType, class RVectorType, class NAType, SQLSMALLINT DataType>
RParamTemplate<SQLType, RVectorType, NAType, DataType>::RParamTemplate(
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

//--------------------------------------------------------------------------------------------------
// Name: RParamTemplate::SetRcppVector
//
// Description:
//  Templatized function to set RcppVector with given paramValue.
//  This is only for numeric or integer R types.
//  It is a wrapper to CreateVector with rowsNumber = 1.
//  Before creation, it verifies whether m_size is equal to size of the SQL type.
//  NA is R's special value indicative of null in C++.
//  For each R type, there is a special NA value with a corresponding NA type in C++.
//
template<class SQLType, class RVectorType, class NAType, SQLSMALLINT DataType>
void RParamTemplate<SQLType, RVectorType, NAType, DataType>::SetRcppVector(
	SQLPOINTER   paramValue)
{
	LOG("RParamTemplate::SetRcppVector");

	CheckParamSize<SQLType>();
	SQLINTEGER strLenOrInd = StrLenOrInd();
	if (strLenOrInd == SQL_NULL_DATA)
	{
		SQLINTEGER strLen_or_Ind[1] = { SQL_NULL_DATA };
		m_RcppVector =
			RTypeUtils::CreateVector<SQLType, RVectorType, NAType, DataType>(
				1, // rowsNumber
				paramValue,
				strLen_or_Ind,
				SQL_NULLABLE);
	}
	else
	{
		m_RcppVector = RTypeUtils::CreateVector<SQLType, RVectorType, NAType, DataType>(
			1,          // rowsNumber
			paramValue,
			nullptr,    // strLen_or_Ind
			SQL_NO_NULLS);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: RParamTemplate::RetrieveValueAndStrLenInd
//
// Description:
//  Retrieves data from m_RcppVector, fill it in m_value and set m_strLenOrInd accordingly.
//
template<class SQLType, class RVectorType, class NAType, SQLSMALLINT DataType>
void RParamTemplate<SQLType, RVectorType, NAType, DataType>::RetrieveValueAndStrLenInd()
{
	LOG("RParamTemplate::RetrieveValueAndStrLenInd");

	Rcpp::Environment globalEnv = Rcpp::Environment::global_env();

	if (globalEnv.exists(m_name.c_str()))
	{
		Logger::LogRVariable(m_name);
		RInside* embeddedREnvPtr = REnvironment::EmbeddedREnvironment();

		m_RcppVector = (*embeddedREnvPtr)[m_name.c_str()];

		if (m_RcppVector.size() == 0)
		{
			m_strLenOrInd = SQL_NULL_DATA;
		}
		else
		{
			SQLSMALLINT nullable = SQL_NO_NULLS;

			// m_RcppVector could possibly have size > 1,
			// but fill only the first value as the parameter value.
			// hence pass in rowsNumber = 1.
			//
			RTypeUtils::FillDataFromRVector<SQLType, RVectorType, DataType>(
				1,  // rowsNumber
				m_RcppVector,
				&m_value,
				&m_strLenOrInd,
				nullable);
		}
	}
	else
	{
		m_strLenOrInd = SQL_NULL_DATA;
	}
}

//--------------------------------------------------------------------------------------------------
// Name: RCharacterParam::RCharacterParam
//
// Description:
//  Constructor.
//
template<class CharType, class SQLType>
RCharacterParam<CharType, SQLType>::RCharacterParam(
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

//--------------------------------------------------------------------------------------------------
// Name: RCharacterParam::SetRcppVector
//
// Description:
//  Sets the RcppVector for RCharacterParam with the given paramValue.
//  This is a wrapper to CreateCharacterVector with rowsNumber = 1.
//  For null parameters, a size one vector with member value = NA_STRING is created.
//
template<class CharType, class SQLType>
void RCharacterParam<CharType, SQLType>::SetRcppVector(SQLPOINTER paramValue)
{
	LOG("RCharacterParam::SetRcppVector");

	SQLINTEGER strLenOrInd = StrLenOrInd();
	if (strLenOrInd == SQL_NULL_DATA)
	{
		m_RcppVector = RTypeUtils::CreateCharacterVector<CharType>(
			1,        // rowsNumber
			nullptr,  // data
			nullptr); // strLen_or_Ind
	}
	else
	{
		SQLINTEGER strLen_or_IndArray[1] = { strLenOrInd };
		m_RcppVector = RTypeUtils::CreateCharacterVector<CharType>(
			1, //rowsNumber
			paramValue,
			strLen_or_IndArray);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: RCharacterParam::RetrieveValueAndStrLenInd
//
// Description:
//  Retrieves data from m_RcppVector, fill it in m_value and set m_strLenOrInd accordingly.
//
template<class CharType, class SQLType>
void RCharacterParam<CharType, SQLType>::RetrieveValueAndStrLenInd()
{
	LOG("RCharacterParam::RetrieveValueAndStrLenInd");

	Rcpp::Environment globalEnv = Rcpp::Environment::global_env();

	if (globalEnv.exists(m_name.c_str()))
	{
		Logger::LogRVariable(m_name);
		RInside* embeddedREnvPtr = REnvironment::EmbeddedREnvironment();

		m_RcppVector = (*embeddedREnvPtr)[m_name.c_str()];

		if (m_RcppVector.size() == 0)
		{
			m_strLenOrInd = SQL_NULL_DATA;
		}
		else
		{
			SQLSMALLINT nullable = SQL_NO_NULLS;
			SQLULEN maxLen = 0;

			// m_RcppVector could possibly have size > 1,
			// but fill only the first value as the parameter value.
			// hence pass in rowsNumber = 1.
			//
			RTypeUtils::FillDataFromCharacterVector<SQLType>(
				1,      // rowsNumber
				m_RcppVector,
				m_size, // allowedLen
				&m_value,
				&m_strLenOrInd,
				nullable,
				maxLen);
		}
	}
	else
	{
		m_strLenOrInd = SQL_NULL_DATA;
	}
}

//--------------------------------------------------------------------------------------------------
// Name: RRawParam::RRawParam
//
// Description:
//  Constructor.
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

//--------------------------------------------------------------------------------------------------
// Name: RRawParam::SetRcppVector
//
// Description:
//  Sets the RcppVector for RRawParam with the given paramValue.
//  This is a wrapper to CreateRawVector with rowsNumber = 1.
//  For null parameters, a size one vector with member value = raw(0) is created.
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
// Name: RRawParam::RetrieveValueAndStrLenInd
//
// Description:
//  Retrieves data from m_RcppVector, fill it in m_value and set m_strLenOrInd accordingly.
//
void RRawParam::RetrieveValueAndStrLenInd()
{
	LOG("RRawParam::RetrieveValueAndStrLenInd");

	Rcpp::Environment globalEnv = Rcpp::Environment::global_env();

	if (globalEnv.exists(m_name.c_str()))
	{
		Logger::LogRVariable(m_name);
		RInside* embeddedREnvPtr = REnvironment::EmbeddedREnvironment();

		m_RcppVector = (*embeddedREnvPtr)[m_name.c_str()];

		if (m_RcppVector.size() == 0)
		{
			m_strLenOrInd = SQL_NULL_DATA;
		}
		else
		{
			RTypeUtils::FillDataFromRawVector(
				m_RcppVector,
				m_size,
				&m_value,
				&m_strLenOrInd);
		}
	}
	else
	{
		m_strLenOrInd = SQL_NULL_DATA;
	}
}

//--------------------------------------------------------------------------------------------------
// Name: RDateTimeParam::RDateTimeParam
//
// Description:
//  Constructor.
//  Calls the base constructor then calls the function to Set the Rcpp vector with the paramValue.
//
template<class SQLType, class RVectorType, class DateTimeTypeInR>
RDateTimeParam<SQLType, RVectorType, DateTimeTypeInR>::RDateTimeParam(
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

//--------------------------------------------------------------------------------------------------
// Name: RDateTimeParam::SetRcppVector
//
// Description:
//  Sets the RcppVector for RDateTimeParam with the given paramValue.
//  This is a wrapper to CreateDateTimeVector with rowsNumber = 1.
//  For null parameters, a size one vector with member value = NA is created.
//
template<class SQLType, class RVectorType, class DateTimeTypeInR>
void RDateTimeParam<SQLType, RVectorType, DateTimeTypeInR>::SetRcppVector(SQLPOINTER paramValue)
{
	LOG("RDateTimeParam::SetRcppVector");

	SQLINTEGER strLenOrInd = StrLenOrInd();
	if (strLenOrInd == SQL_NULL_DATA)
	{
		SQLINTEGER strLen_or_Ind[1] = { SQL_NULL_DATA };
		m_RcppVector = RTypeUtils::CreateDateTimeVector<SQLType, RVectorType, DateTimeTypeInR>(
			1, // rowsNumber
			paramValue,
			strLen_or_Ind,
			SQL_NULLABLE);
	}
	else
	{
		SQLINTEGER strLen_or_IndArray[1] = { strLenOrInd };
		m_RcppVector = RTypeUtils::CreateDateTimeVector<SQLType, RVectorType, DateTimeTypeInR>(
			1, // rowsNumber
			paramValue,
			strLen_or_IndArray,
			SQL_NO_NULLS);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: RDateTimeParam::RetrieveValueAndStrLenInd
//
// Description:
//  Retrieves data from m_RcppVector, fill it in m_value and set m_strLenOrInd accordingly.
//
template<class SQLType, class RVectorType, class DateTimeTypeInR>
void RDateTimeParam<SQLType, RVectorType, DateTimeTypeInR>::RetrieveValueAndStrLenInd()
{
	LOG("RDateTimeParam::RetrieveValueAndStrLenInd");

	Rcpp::Environment globalEnv = Rcpp::Environment::global_env();

	if (globalEnv.exists(m_name.c_str()))
	{
		Logger::LogRVariable(m_name);
		RInside* embeddedREnvPtr = REnvironment::EmbeddedREnvironment();

		m_RcppVector = (*embeddedREnvPtr)[m_name.c_str()];

		if (m_RcppVector.size() > 0)
		{
			SQLSMALLINT nullable = SQL_NO_NULLS;

			// m_RcppVector could possibly have size > 1,
			// but fill only the first value as the parameter value.
			// hence pass in rowsNumber = 1.
			//
			RTypeUtils::FillDataFromDateTimeVector<SQLType, RVectorType, DateTimeTypeInR>(
				1,  // rowsNumber
				m_RcppVector,
				&m_value,
				&m_strLenOrInd,
				nullable);
		}
		else
		{
			// This case means the parameter is defined in R as a vector but with no elements in it.
			//
			m_strLenOrInd = SQL_NULL_DATA;
		}
	}
	else
	{
		// This means the parameter is not defined at all in the R environment.
		//
		m_strLenOrInd = SQL_NULL_DATA;
	}
}

//--------------------------------------------------------------------------------------------------
// Name: RNumericParam::RNumericParam
//
// Description:
//  Constructor.
//
RNumericParam::RNumericParam(
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

	// paramSize is the precision value ranging from 1 to 38.
	//
	m_precision = paramSize;
}

//--------------------------------------------------------------------------------------------------
// Name: RNumericParam::SetRcppVector
//
// Description:
//  Sets the RcppVector for RNumericParam with the given paramValue.
//  This is a wrapper to CreateNumericVector with rowsNumber = 1.
//  For null parameters, a size one vector with member value = NA is created.
//
void RNumericParam::SetRcppVector(SQLPOINTER paramValue)
{
	LOG("RNumericParam::SetRcppVector");

	SQLINTEGER strLenOrInd = StrLenOrInd();
	if (strLenOrInd == SQL_NULL_DATA)
	{
		SQLINTEGER strLen_or_IndArray[1] = { SQL_NULL_DATA };
		m_RcppVector = RTypeUtils::CreateNumericVector(
			1, // rowsNumber
			paramValue,
			strLen_or_IndArray,
			m_decimalDigits,
			SQL_NULLABLE);
	}
	else
	{
		SQLINTEGER strLen_or_IndArray[1] = { strLenOrInd };
		m_RcppVector = RTypeUtils::CreateNumericVector(
			1,        // rowsNumber
			paramValue,
			strLen_or_IndArray,
			m_decimalDigits,
			SQL_NO_NULLS);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: RNumericParam::RetrieveValueAndStrLenInd
//
// Description:
//  Retrieves data from m_RcppVector, fills it in m_value and sets m_strLenOrInd accordingly.
//
void RNumericParam::RetrieveValueAndStrLenInd()
{
	LOG("RNumericParam::RetrieveValueAndStrLenInd");

	Rcpp::Environment globalEnv = Rcpp::Environment::global_env();

	if (globalEnv.exists(m_name.c_str()))
	{
		Logger::LogRVariable(m_name);
		RInside* embeddedREnvPtr = REnvironment::EmbeddedREnvironment();

		m_RcppVector = (*embeddedREnvPtr)[m_name.c_str()];

		if (m_RcppVector.size() == 0)
		{
			m_strLenOrInd = SQL_NULL_DATA;
		}
		else
		{
			SQLSMALLINT nullable = SQL_NO_NULLS;
			RTypeUtils::FillDataFromNumericVector(
				1, // rowsNumber
				m_RcppVector,
				&m_value,
				&m_strLenOrInd,
				nullable,
				m_decimalDigits,
				m_precision);
		}
	}
	else
	{
		m_strLenOrInd = SQL_NULL_DATA;
	}
}

//--------------------------------------------------------------------------------------------------
// Do explicit template instantiations, so that object code is generated for these
// and the linker is able to find their definitions even after instantiations are in different
// translation units (i.e. RParamTemplate instantiation is in RParamContainer.cpp)
//
template RParamTemplate<SQLINTEGER, Rcpp::IntegerVector, int, SQL_C_SLONG>::RParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template RParamTemplate<SQLREAL, Rcpp::NumericVector, double, SQL_C_FLOAT>::RParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template RParamTemplate<SQLDOUBLE, Rcpp::NumericVector, double, SQL_C_DOUBLE>::RParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template RParamTemplate<SQLBIGINT, Rcpp::NumericVector, double, SQL_C_SBIGINT>::RParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template RParamTemplate<SQLSMALLINT, Rcpp::IntegerVector, int, SQL_C_SSHORT>::RParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template RParamTemplate<SQLCHAR, Rcpp::IntegerVector, int, SQL_C_UTINYINT>::RParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template RParamTemplate<SQLCHAR, Rcpp::LogicalVector, int, SQL_C_BIT>::RParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template RCharacterParam<char, SQLCHAR>::RCharacterParam(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template RCharacterParam<char16_t, SQLWCHAR>::RCharacterParam(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template RDateTimeParam<SQL_DATE_STRUCT, Rcpp::DateVector, Rcpp::Date>::RDateTimeParam(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template RDateTimeParam<SQL_TIMESTAMP_STRUCT, Rcpp::DatetimeVector, Rcpp::Datetime>::RDateTimeParam(
	SQLUSMALLINT,
	const SQLCHAR*,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);
