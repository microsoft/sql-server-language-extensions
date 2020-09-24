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

// @File: RTypeUtils.cpp
//
// Purpose:
//  Class providing functions to manipulate between R data types and SQL types.
//
//**************************************************************************************************

#include "Common.h"
#include <iostream>

#include "RTypeUtils.h"
#include "Unicode.h"

using namespace std;

//--------------------------------------------------------------------------------------------------
// Map to store the ODBC C datatype to NA value in R.
// R_NaInt, R_NaReal are R's special values indicative of null in C++.
// Even for logical type, NA value is R_NaInt.
//
const unordered_map<SQLSMALLINT, SQLPOINTER> RTypeUtils::sm_dataTypeToNAMap =
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
const unordered_map<string, SQLSMALLINT> RTypeUtils::sm_classInRToOdbcTypeMap =
{
	{"logical", SQL_C_BIT},
	{"integer", SQL_C_SLONG},
	{"numeric", SQL_C_DOUBLE},
	{"character", SQL_C_CHAR},
	{"raw", SQL_C_BINARY},
	{"Date", SQL_C_TYPE_DATE},
	{"POSIXct", SQL_C_TYPE_TIMESTAMP}
};

//--------------------------------------------------------------------------------------------------
// Name: RTypeUtils::CreateVector
//
// Description:
//  Templatized function to create an Rcpp vector encapsulating SEXP pointers to the equivalent R
//  objects for the given SQL type with given data. This is only for numeric,
//  integer or logical R types. rowsNumber indicates the number of elements to be added in the vector.
//  Iterates over data to set the value at each index of the vector.
//  strLen_or_Ind if non-null is an array, where each cell represents the size of the SQL data type
//  and null values are indicated by SQL_NULL_DATA.
//  If at any index strLen_or_Ind is SQL_NULL_DATA, we fill the equivalent R NA value in Rcpp vector.
//  If strLen_or_Ind is nullptr, there are no null values in the data.
//  If nullable is SQL_NO_NULLS, we ignore strLen_or_Ind.
//
template<class SQLType, class RVectorType, class NAType, SQLSMALLINT DataType>
RVectorType RTypeUtils::CreateVector(
	SQLULEN      rowsNumber,
	SQLPOINTER   data,
	SQLINTEGER   *strLen_or_Ind,
	SQLSMALLINT  nullable)
{
	LOG("RTypeUtils::CreateVector");

	// Note: Always preallocate the Rcpp vector with the size instead of using
	// push_back since Rcpp push_back involves copying to create a new vector in R environment.
	//
	RVectorType vectorInR(rowsNumber);
	bool isNullable = nullable == SQL_NULLABLE;

	for (SQLULEN j = 0; j < rowsNumber; ++j)
	{
		if (isNullable &&
			strLen_or_Ind != nullptr &&
			strLen_or_Ind[j] == SQL_NULL_DATA)
		{
			// It is NULL (NA) only in this case.
			//
			NAType valueForNA = *(static_cast<NAType*>(sm_dataTypeToNAMap.at(DataType)));
			vectorInR[j] = valueForNA;
		}
		else
		{
			// In all other scenarios, it is not NULL (NA).
			//
			
			SQLType value = static_cast<SQLType *>(data)[j];
			if constexpr (DataType != SQL_C_BIT)
			{
				vectorInR[j] = value;
			}
			else
			{
				vectorInR[j] = value != '0' && value != 0;
			}
		}
	}

	return vectorInR;
}

//--------------------------------------------------------------------------------------------------
// Name: RTypeUtils::CreateCharacterVector
//
// Description:
//  Creates a character Rcpp vector encapsulating SEXP pointers to the equivalent R character
//  objects with the given data. If the given data is wide character utf16, convert it to utf8
//  first before creating the Rcpp vector since R only accepts utf-8 encoding.
//  rowsNumber indicates the number of elements to be added in the vector.
//  Iterates over data to set the value at each index of the vector.
//  strLen_or_Ind if non-null is an array, where each cell represents the number of bytes occupied
//  by the corresponding string in the given data and null strings are indicated by SQL_NULL_DATA.
//  If at any index strLen_or_Ind is SQL_NULL_DATA, we fill in the NA_STRING value in Rcpp vector.
//  If strLen_or_Ind is nullptr, all the values in the data are null.
//
template<class CharType>
Rcpp::CharacterVector RTypeUtils::CreateCharacterVector(
	SQLULEN    rowsNumber,
	SQLPOINTER data,
	SQLINTEGER *strLen_or_Ind)
{
	LOG("RTypeUtils::CreateCharacterVector");

	CharType *baseCharData = static_cast<CharType *>(data);
	SQLULEN cumulativeLength = 0;

	// Note: Always preallocate the Rcpp vector with the size instead of using
	// push_back since Rcpp push_back involves copying to create a new vector in R environment.
	//
	Rcpp::CharacterVector charVector(rowsNumber);

	for (SQLULEN j = 0; j < rowsNumber; ++j)
	{
		if (strLen_or_Ind == nullptr || strLen_or_Ind[j] == SQL_NULL_DATA)
		{
			charVector[j] = NA_STRING;
		}
		else
		{
			CharType *str = baseCharData + cumulativeLength;
			SQLINTEGER strlen = strLen_or_Ind[j] / sizeof(CharType);
			string value;
			if constexpr (is_same_v<CharType, char16_t>)
			{
				estd::ToUtf8(str, strlen, value);
			}
			else
			{
				value = string(str, strlen);
			}

			charVector[j] = value.c_str();
			cumulativeLength += strlen;
		}
	}

	return charVector;
}

//--------------------------------------------------------------------------------------------------
// Name: RTypeUtils::CreateRawVector
//
// Description:
//  Creates a raw Rcpp vector in R encapsulating SEXP pointers to the equivalent R raw objects
//  with the given data. rowsNumber indicates the number of elements to be added in the data.
//  Each cell in a non-null strLen_or_Ind array indicates the length of each element.
//  Iterates over data and for each element, over each byte to push_back the value to the vector.
//  If at any index strLen_or_Ind is SQL_NULL_DATA, it pushes back an empty raw(0) element.
//  If strLen_or_Ind is nullptr, we push raw(0) for all the rows in the data.
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

	for (SQLULEN j = 0; j < rowsNumber; ++j)
	{
		if (strLen_or_Ind == nullptr || strLen_or_Ind[j] == SQL_NULL_DATA)
		{
			// For raw, 0 (the nul byte) represents NA.
			// https://www.rdocumentation.org/packages/base/versions/3.6.2/topics/raw
			//
			rawVector.push_back(0);
		}
		else
		{
			SQLCHAR *rawData = static_cast<SQLCHAR *>(baseRawData) + cumulativeRawDataLength;
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

//--------------------------------------------------------------------------------------------------
// Name: RTypeUtils::CreateDateTimeVector
//
// Description:
//  Templatized function to create an Rcpp vector encapsulating SEXP pointers to the equivalent R
//  objects for the given data types with given data. This is only for date, datetime(2)
//  SQL data types mapping to Rcpp::DateVector and DateTimeVector respectively.
//  rowsNumber indicates the number of elements to be added in the vector.
//  Iterates over data to set the value at each index of the vector.
//  strLen_or_Ind if non-null is an array, where each cell represents the size of the SQL data type
//  and null values are indicated by SQL_NULL_DATA.
//  If at any index strLen_or_Ind is SQL_NULL_DATA, we fill the equivalent R NA value in Rcpp vector.
//  If strLen_or_Ind is nullptr, there are no null values in the data.
//  If nullable is SQL_NO_NULLS, we ignore strLen_or_Ind.
//
template<class SQLType, class RVectorType, class DateTimeTypeInR>
RVectorType RTypeUtils::CreateDateTimeVector(
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind,
	SQLSMALLINT nullable)
{
	LOG("RTypeUtils::CreateDateTimeVector");

	// Note: Always preallocate the Rcpp vector with the size instead of using
	// push_back since Rcpp push_back involves copying to create a new vector in R environment.
	//
	RVectorType vectorInR(rowsNumber);

	// Get the time zone if any that R is set to and store it in a temporary variable.
	//
	string previousTimeZoneInR = "";

	try
	{
		previousTimeZoneInR = Utilities::GetTimeZoneInR();

		// Interpret the value sent by Exthost in UTC irrespective of what time zone R had been set to.
		//
		Utilities::SetTimeZoneInR("UTC");
		bool isNullable = nullable == SQL_NULLABLE;

		for (SQLULEN j = 0; j < rowsNumber; ++j)
		{
			if (isNullable &&
				strLen_or_Ind != nullptr &&
				strLen_or_Ind[j] == SQL_NULL_DATA)
			{
				// It is NULL (NA) only in this case.
				//
				vectorInR[j] = DateTimeTypeInR(R_NaReal);
			}
			else
			{
				// In all other scenarios, it is not NULL (NA).
				//

				SQLType value = static_cast<SQLType *>(data)[j];

				// Rcpp::Date accepts date in "YYYY-mm-dd" and
				// Rcpp::Datetime accepts datetime in "YYYY-mm-dd HH:MM:SS.MICROSEC"
				// string formats so generate the date time string.
				//
				string dateTimeInStringFormat =
					to_string(value.year) + "-" +
					to_string(value.month) + "-" +
					to_string(value.day);

				if constexpr (is_same_v<DateTimeTypeInR, Rcpp::Datetime>)
				{
					// "fraction" is stored in nanoseconds, we need to know the digits after the decimal point.
					//
					string secondsAfterDecimalPoint
						= Utilities::GetSecondsAfterDecimalPointFromNanoSeconds(value.fraction);

					dateTimeInStringFormat =
						dateTimeInStringFormat + " " +
						to_string(value.hour) + ":" +
						to_string(value.minute) + ":" +
						to_string(value.second) + "." +
						secondsAfterDecimalPoint;
				}

				vectorInR[j] = DateTimeTypeInR(dateTimeInStringFormat);
			}
		}
	}
	catch (...)
	{
		Utilities::SetTimeZoneInR(previousTimeZoneInR);

		throw runtime_error("Unable to set UTC time zone and create date(time) vector");
	}

	Utilities::SetTimeZoneInR(previousTimeZoneInR);

	return vectorInR;
}

//--------------------------------------------------------------------------------------------------
// Name: RTypeUtils::FillDataFromVector
//
// Description:
//  Given the vectorInR, copies its content into the given std::vector pointed to by data.
//  Copies the content only as far as the rowsNumber indicates.
//  The cells in the strLenOrInd array are set to the size of the SQL data type if the
//  corresponding rows in the vectorInR are not NA.
//  Otherwise if they are NA, sets nullable to true and the corresponding cells in
//  the array strLenOrInd to SQL_NULL_DATA.
//
template<class SQLType, class RVectorType, SQLSMALLINT DataType>
void RTypeUtils::FillDataFromRVector(
	SQLULEN         rowsNumber,
	RVectorType           vectorInR,
	vector<SQLType> *data,
	SQLINTEGER      *strLen_or_Ind,
	SQLSMALLINT     &nullable)
{
	LOG("RTypeUtils::FillDataFromRVector");

	data->resize(rowsNumber);
	for(SQLULEN index = 0 ; index < rowsNumber; ++index)
	{
		if (!RVectorType::is_na(vectorInR[index]))
		{
			if constexpr (DataType != SQL_C_BIT)
			{
				(*data)[index] = vectorInR[index];
			}
			else
			{
				(*data)[index] = vectorInR[index] ? 1 : 0;
			}

			strLen_or_Ind[index] = sizeof(SQLType);
		}
		else
		{
			strLen_or_Ind[index] = SQL_NULL_DATA;
			nullable = SQL_NULLABLE;
			(*data)[index] = vectorInR[index];
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: RTypeUtils::FillDataFromCharacterVector
//
// Description:
//  Given the vectorInR, copies its content into the given std::vector pointed to by data of SQLType.
//  Copies the content only as far as the rowsNumber indicates.
//  The cells in the strLenOrInd array are set to the number of bytes occupied by the
//  corresponding strings in the vector being filled if the strings are not NA.
//  Otherwise if they are NA strings, sets nullable and the corresponding cells in the array
//  strLenOrInd to SQL_NULL_DATA. Also, returns the maxLen of character string identified after all
//  the strings are scanned. maxLen or any of the string lengths cannot exceed the allowedLen.
//
template<class SQLType>
void RTypeUtils::FillDataFromCharacterVector(
	SQLULEN               rowsNumber,
	Rcpp::CharacterVector vectorInR,
	SQLULEN               allowedLen,
	vector<SQLType>       *data,
	SQLINTEGER            *strLenOrInd,
	SQLSMALLINT           &nullable,
	SQLULEN               &maxLen)
{
	LOG("RTypeUtils::FillDataFromCharacterVector");

	// Insert the character vectorInR into the charVector contiguously.
	//
	for(SQLULEN index = 0 ; index < rowsNumber; ++index)
	{
		if (!Rcpp::CharacterVector::is_na(vectorInR[index]))
		{
			// This calculates the total number of bytes occupied by the character in R
			// Since R utilizes utf-8 encoding, there could be characters that occupy
			// more than 1 byte, so this may not correspond to the string length i.e. the
			// number of characters.
			//
			SQLINTEGER bytesOccupied = strlen(vectorInR[index]);

			// When generating a string from utf-8 encoded R data, use the bytesOccupied
			// as the length of string.
			//
			string utf8String(vectorInR[index], bytesOccupied);

			SQLINTEGER lengthOfStringToInsert = 0;

			if constexpr (is_same_v<SQLType, SQLWCHAR>)
			{
				u16string utf16String;
				estd::ToUtf16(utf8String.c_str(), bytesOccupied, utf16String);
				lengthOfStringToInsert = utf16String.length();
				lengthOfStringToInsert =
					static_cast<SQLULEN>(lengthOfStringToInsert) <= allowedLen
					? lengthOfStringToInsert
					: static_cast<SQLINTEGER>(allowedLen);
				data->insert(data->end(), utf16String.begin(),
					utf16String.begin() + lengthOfStringToInsert);
			}
			else
			{
				lengthOfStringToInsert = bytesOccupied;
				lengthOfStringToInsert =
					static_cast<SQLULEN>(lengthOfStringToInsert) <= allowedLen
					? lengthOfStringToInsert
					: static_cast<SQLINTEGER>(allowedLen);
				data->insert(data->end(), utf8String.begin(),
					utf8String.begin() + lengthOfStringToInsert);
			}

			strLenOrInd[index] = lengthOfStringToInsert * sizeof(SQLType);
			if (maxLen < static_cast<SQLULEN>(lengthOfStringToInsert))
			{
				maxLen = lengthOfStringToInsert;
			}
		}
		else
		{
			nullable = SQL_NULLABLE;
			strLenOrInd[index] = SQL_NULL_DATA;
		}
	}
}

//-------------------------------------------------------------------------------------------------
// Name: RTypeUtils::FillDataFromRawVector
//
// Description:
//  Given the raw vectorInR, copy its content into the given std::vector pointed to by rawCharVector.
//  The filled vector contains only 1 member with all the raw bytes from the vectorInR copied in
//  that one cell. The smaller of allowedLen and size of vectorInR is the amount of bytes that
//  are to be copied. Set the value of the one-celled strLenOrInd array to that smaller value.
//
void RTypeUtils::FillDataFromRawVector(
	Rcpp::RawVector vectorInR,
	SQLULEN         allowedLen,
	vector<SQLCHAR> *data,
	SQLINTEGER      *strLenOrInd)
{
	LOG("RTypeUtils::FillDataFromRawVector");

	strLenOrInd[0] =
		static_cast<SQLULEN>(vectorInR.size()) <= allowedLen
			? vectorInR.size() : static_cast<SQLINTEGER>(allowedLen);
	data->resize(strLenOrInd[0]);
	for(SQLINTEGER index = 0 ; index < strLenOrInd[0]; ++index)
	{
		(*data)[index] = vectorInR[index];
	}
}

//-------------------------------------------------------------------------------------------------
// Name: RTypeUtils::FillDataFromDateTimeVector
//
// Description:
//  Given the vectorInR, copies its content into the given std::vector pointed to by data.
//  Copies the content only as far as the rowsNumber indicates.
//  The cells in the strLenOrInd array are set to the size of the SQL data type if the
//  corresponding rows in the vectorInR are not NA.
//  Otherwise if they are NA, sets nullable to true and the corresponding cells in
//  the array strLenOrInd to SQL_NULL_DATA.
//
template<class SQLType, class RVectorType, class DateTimeTypeInR>
void RTypeUtils::FillDataFromDateTimeVector(
	SQLULEN         rowsNumber,
	RVectorType     vectorInR,
	vector<SQLType> *data,
	SQLINTEGER      *strLenOrInd,
	SQLSMALLINT     &nullable)
{
	for(SQLULEN index = 0 ; index < rowsNumber; ++index)
	{
		DateTimeTypeInR valueInR = static_cast<DateTimeTypeInR>(vectorInR[index]);
		if (!valueInR.is_na())
		{
			// The valueInR is stored internally in UTC time zone in Rcpp
			// and is returned back to Exthost in UTC as well.
			//
			SQLType value;
			value.year = valueInR.getYear();
			value.month = valueInR.getMonth();
			value.day = valueInR.getDay();

			if constexpr (is_same_v<RVectorType, Rcpp::DatetimeVector>)
			{
				value.hour = valueInR.getHours();
				value.minute = valueInR.getMinutes();
				value.second = valueInR.getSeconds();

				// "fraction" in TIMESTAMP_STRUCT is stored in nanoseconds,
				// we convert from microseconds.
				//
				value.fraction = valueInR.getMicroSeconds() * 1000;
			}

			data->push_back(value);
			strLenOrInd[index] = sizeof(SQLType);
		}
		else
		{
			strLenOrInd[index] = SQL_NULL_DATA;
			nullable = SQL_NULLABLE;
		}
	}
}

// Do explicit template instantiations, so that object code is generated for these
// and the linker is able to find their definitions even after instantiations are in different
// translation units (i.e. CreateVector instantiation is in RParam.cpp)
//
template Rcpp::IntegerVector RTypeUtils::CreateVector<SQLINTEGER, Rcpp::IntegerVector, int, SQL_C_SLONG>(
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind,
	SQLSMALLINT nullable);

template Rcpp::NumericVector RTypeUtils::CreateVector<SQLREAL, Rcpp::NumericVector, double, SQL_C_FLOAT>(
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind,
	SQLSMALLINT nullable);

template Rcpp::NumericVector RTypeUtils::CreateVector<SQLDOUBLE, Rcpp::NumericVector, double, SQL_C_DOUBLE>(
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind,
	SQLSMALLINT nullable);

template Rcpp::NumericVector RTypeUtils::CreateVector<SQLBIGINT, Rcpp::NumericVector, double, SQL_C_SBIGINT>(
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind,
	SQLSMALLINT nullable);

template Rcpp::IntegerVector RTypeUtils::CreateVector<SQLSMALLINT, Rcpp::IntegerVector, int, SQL_C_SSHORT>(
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind,
	SQLSMALLINT nullable);

template Rcpp::IntegerVector RTypeUtils::CreateVector<SQLCHAR, Rcpp::IntegerVector, int, SQL_C_UTINYINT>(
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind,
	SQLSMALLINT nullable);

template Rcpp::LogicalVector RTypeUtils::CreateVector<SQLCHAR, Rcpp::LogicalVector, int, SQL_C_BIT>(
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind,
	SQLSMALLINT nullable);

template Rcpp::CharacterVector RTypeUtils::CreateCharacterVector<char>(
	SQLULEN    rowsNumber,
	SQLPOINTER data,
	SQLINTEGER *strLen_or_Ind);

template Rcpp::CharacterVector RTypeUtils::CreateCharacterVector<char16_t>(
	SQLULEN    rowsNumber,
	SQLPOINTER data,
	SQLINTEGER *strLen_or_Ind);

template Rcpp::DateVector RTypeUtils::CreateDateTimeVector
	<SQL_DATE_STRUCT, Rcpp::DateVector, Rcpp::Date>(
		SQLULEN     rowsNumber,
		SQLPOINTER  data,
		SQLINTEGER  *strLen_or_Ind,
		SQLSMALLINT nullable);

template Rcpp::DatetimeVector RTypeUtils::CreateDateTimeVector
	<SQL_TIMESTAMP_STRUCT, Rcpp::DatetimeVector, Rcpp::Datetime>(
		SQLULEN     rowsNumber,
		SQLPOINTER  data,
		SQLINTEGER  *strLen_or_Ind,
		SQLSMALLINT nullable);

template void RTypeUtils::FillDataFromRVector<SQLINTEGER, Rcpp::IntegerVector, SQL_C_SLONG>(
	SQLULEN             rowsNumber,
	Rcpp::IntegerVector vectorInR,
	vector<SQLINTEGER>  *data,
	SQLINTEGER          *strLen_or_Ind,
	SQLSMALLINT         &nullable);

template void RTypeUtils::FillDataFromRVector<SQLREAL, Rcpp::NumericVector, SQL_C_FLOAT>(
	SQLULEN             rowsNumber,
	Rcpp::NumericVector vectorInR,
	vector<SQLREAL>     *data,
	SQLINTEGER          *strLen_or_Ind,
	SQLSMALLINT         &nullable);

template void RTypeUtils::FillDataFromRVector<SQLDOUBLE, Rcpp::NumericVector, SQL_C_DOUBLE>(
	SQLULEN             rowsNumber,
	Rcpp::NumericVector vectorInR,
	vector<SQLDOUBLE>   *data,
	SQLINTEGER          *strLen_or_Ind,
	SQLSMALLINT         &nullable);

template void RTypeUtils::FillDataFromRVector<SQLBIGINT, Rcpp::NumericVector, SQL_C_SBIGINT>(
	SQLULEN             rowsNumber,
	Rcpp::NumericVector vectorInR,
	vector<SQLBIGINT>   *data,
	SQLINTEGER          *strLen_or_Ind,
	SQLSMALLINT         &nullable);

template void RTypeUtils::FillDataFromRVector<SQLSMALLINT, Rcpp::IntegerVector, SQL_C_SSHORT>(
	SQLULEN             rowsNumber,
	Rcpp::IntegerVector vectorInR,
	vector<SQLSMALLINT> *data,
	SQLINTEGER          *strLen_or_Ind,
	SQLSMALLINT         &nullable);

template void RTypeUtils::FillDataFromRVector<SQLCHAR, Rcpp::IntegerVector, SQL_C_UTINYINT>(
	SQLULEN             rowsNumber,
	Rcpp::IntegerVector vectorInR,
	vector<SQLCHAR>     *data,
	SQLINTEGER          *strLen_or_Ind,
	SQLSMALLINT         &nullable);

template void RTypeUtils::FillDataFromRVector<SQLCHAR, Rcpp::LogicalVector, SQL_C_BIT>(
	SQLULEN             rowsNumber,
	Rcpp::LogicalVector vectorInR,
	vector<SQLCHAR>     *data,
	SQLINTEGER          *strLen_or_Ind,
	SQLSMALLINT         &nullable);

template void RTypeUtils::FillDataFromCharacterVector<SQLCHAR>(
	SQLULEN               rowsNumber,
	Rcpp::CharacterVector vectorInR,
	SQLULEN               allowedLen,
	vector<SQLCHAR>       *data,
	SQLINTEGER            *strLenOrInd,
	SQLSMALLINT           &nullable,
	SQLULEN               &maxLen);

template void RTypeUtils::FillDataFromCharacterVector<SQLWCHAR>(
	SQLULEN               rowsNumber,
	Rcpp::CharacterVector vectorInR,
	SQLULEN               allowedLen,
	vector<SQLWCHAR>      *data,
	SQLINTEGER            *strLenOrInd,
	SQLSMALLINT           &nullable,
	SQLULEN               &maxLen);

template void RTypeUtils::FillDataFromDateTimeVector
	<SQL_DATE_STRUCT, Rcpp::DateVector, Rcpp::Date>(
		SQLULEN                 rowsNumber,
		Rcpp::DateVector        vectorInR,
		vector<SQL_DATE_STRUCT> *data,
		SQLINTEGER              *strLenOrInd,
		SQLSMALLINT             &nullable);

template void RTypeUtils::FillDataFromDateTimeVector
	<SQL_TIMESTAMP_STRUCT, Rcpp::DatetimeVector, Rcpp::Datetime>(
		SQLULEN                      rowsNumber,
		Rcpp::DatetimeVector         vectorInR,
		vector<SQL_TIMESTAMP_STRUCT> *data,
		SQLINTEGER                   *strLenOrInd,
		SQLSMALLINT                  &nullable);
