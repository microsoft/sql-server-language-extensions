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
#include <climits>
#include <cmath>
#include <iostream>

#include "RTypeUtils.h"
#include "Unicode.h"

using namespace std;

//--------------------------------------------------------------------------------------------------
static constexpr SQLUSMALLINT x_MaxScale = 38;
static constexpr SQLUSMALLINT x_HexaDecimalBase = 16;
const double x_PowersOf10[x_MaxScale + 1] =
	{ 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12,
	  1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22, 1e23, 1e24,
	  1e25, 1e26, 1e27, 1e28, 1e29, 1e30, 1e31, 1e32, 1e33, 1e34, 1e35, 1e36,
	  1e37, 1e38 };

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

	for (SQLULEN index = 0; index < rowsNumber; ++index)
	{
		if (isNullable &&
			strLen_or_Ind != nullptr &&
			strLen_or_Ind[index] == SQL_NULL_DATA)
		{
			// It is NULL (NA) only in this case.
			//
			NAType valueForNA = *(static_cast<NAType*>(sm_dataTypeToNAMap.at(DataType)));
			vectorInR[index] = valueForNA;
		}
		else
		{
			// In all other scenarios, it is not NULL (NA).
			//
			SQLType value = static_cast<SQLType *>(data)[index];
			if constexpr (DataType != SQL_C_BIT)
			{
				vectorInR[index] = value;
			}
			else
			{
				vectorInR[index] = value != '0' && value != 0;
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

	for (SQLULEN index = 0; index < rowsNumber; ++index)
	{
		if (strLen_or_Ind == nullptr || strLen_or_Ind[index] == SQL_NULL_DATA)
		{
			charVector[index] = NA_STRING;
		}
		else
		{
			CharType *str = baseCharData + cumulativeLength;
			SQLINTEGER strlen = strLen_or_Ind[index] / sizeof(CharType);
			string value;

			// If there are bad characters w.r.t CharType
			// throws an error
			//
			if constexpr (is_same_v<CharType, char16_t>)
			{
				estd::ToUtf8(str, strlen, value, true /*throwOnError*/);
			}
			else
			{	
				// Check if string is a valid UTF8
				//
				bool isValidUTF8 = estd::IsValidUTF8(const_cast<char*>(str), strlen);

				if(!isValidUTF8)
				{
					throw std::invalid_argument("There is a bad UTF-8 character");
				}

				value = string(str, strlen);
			}

			charVector[index] = value.c_str();
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
//  If strLen_or_Ind is nullptr, returns an empty RawVector,
//  Otherwise, iterates over data and for each element, if strLen_or_Ind at any index is not 
//  SQL_NULL_DATA, go over each of its byte to push_back the value to the vector.
//  If strLen_or_Ind at any index is SQL_NULL_DATA, does not push_back anything.
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
	
	if (strLen_or_Ind != nullptr)
	{
		for (SQLULEN index = 0; index < rowsNumber; ++index)
		{
			if (strLen_or_Ind[index] != SQL_NULL_DATA)
			{
				SQLCHAR *rawData = static_cast<SQLCHAR *>(baseRawData) + cumulativeRawDataLength;
				for(int rawByteIndex = 0; rawByteIndex < strLen_or_Ind[index]; ++rawByteIndex)
				{
					SQLCHAR value = *(rawData + rawByteIndex);
					rawVector.push_back(value);
				}

				cumulativeRawDataLength += strLen_or_Ind[index];
			}
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

		for (SQLULEN index = 0; index < rowsNumber; ++index)
		{
			if (isNullable &&
				strLen_or_Ind != nullptr &&
				strLen_or_Ind[index] == SQL_NULL_DATA)
			{
				// It is NULL (NA) only in this case.
				//
				vectorInR[index] = DateTimeTypeInR(R_NaReal);
			}
			else
			{
				// In all other scenarios, it is not NULL (NA).
				//

				SQLType value = static_cast<SQLType *>(data)[index];

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

				vectorInR[index] = DateTimeTypeInR(dateTimeInStringFormat);
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
// Name: RTypeUtils::CreateNumericVector
//
// Description:
//  Creates a Numeric Rcpp vector encapsulating SEXP pointers to the equivalent R
//  objects for the numeric data passed in a SQL_NUMERIC_STRUCT.
//  rowsNumber indicates the number of elements to be added in the vector.
//  Iterates over data to set the value at each index of the vector.
//  strLen_or_Ind if non-null is an array, where each cell represents the size of the SQL data type
//  and null values are indicated by SQL_NULL_DATA.
//  If at any index strLen_or_Ind is SQL_NULL_DATA, we fill the equivalent R NA value in Rcpp vector.
//  If strLen_or_Ind is nullptr, there are no null values in the data.
//  If nullable is SQL_NO_NULLS, we ignore strLen_or_Ind.
//
Rcpp::NumericVector RTypeUtils::CreateNumericVector(
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind,
	SQLSMALLINT decimalDigits,
	SQLSMALLINT nullable)
{
	LOG("RTypeUtils::CreateNumericVector");

	// Note: Always preallocate the Rcpp vector with the size instead of using
	// push_back since Rcpp push_back involves copying to create a new vector in R environment.
	//
	Rcpp::NumericVector vectorInR(rowsNumber);

	bool isNullable = nullable == SQL_NULLABLE;

	for (SQLULEN index = 0; index < rowsNumber; ++index)
	{
		if (isNullable &&
			strLen_or_Ind != nullptr &&
			strLen_or_Ind[index] == SQL_NULL_DATA)
		{
			// It is NULL (NA) only in this case.
			//
			vectorInR[index] = R_NaReal;
		}
		else
		{
			// In all other scenarios, it is not NULL (NA).
			//
			const SQL_NUMERIC_STRUCT value = static_cast<SQL_NUMERIC_STRUCT*>(data)[index];

			// The val array in the numeric struct contains the little endian byte representation
			// of the (decimal number * 10 ^ decimalDigits).
			// e.g. for type numeric(8, 3) i.e. 8 precision and 3 as the scale i.e. decimalDigits,
			// a value of 3.4 will have the val array = 3.4 * 10 ^ 3 = 3400
			//
			double scaledValue = ConvertBytesToDouble(value.val);
			double numeric = scaledValue / x_PowersOf10[decimalDigits];
			if (value.sign != 1)
			{
				numeric *= (-1.0);
			}

			vectorInR[index] = numeric;
		}
	}

	return vectorInR;
}

//--------------------------------------------------------------------------------------------------
// Name: RTypeUtils::ConvertBytesToDouble
//
// Description:
//  Given a little endian byte array of max length SQL_MAX_NUMERIC_LEN,
//  converts it into a double value of base 10.
//  It views each byte as two hexadecimal digits. Starting with the lowest powerOf16 = 1,
//  it incrementally multiplies the digits (both least significant and most significant)
//  with the correct powerOf16, and cumulatively keeps adding the product to the result.
//
// Returns:
//  A double value of base 10 equivalent to the given little endian byte array.
//
// Remarks:
//  We do not convert the given byte array into a long integer
//  since the range occupied by long is limited whereas a double value can accommodate the max
//  value of represented by the array.
//
double RTypeUtils::ConvertBytesToDouble(const SQLCHAR *leBytesArray)
{
	double result = 0.0;
	double powerOf16 = 1.0;
	int currentByteValue = 0;
	int lsd = 0, msd = 0;

	for(int byte = 0; byte < SQL_MAX_NUMERIC_LEN; ++byte)
	{
		currentByteValue = static_cast<int>(leBytesArray[byte]);
		lsd = currentByteValue % x_HexaDecimalBase; // Obtain LSD
		msd = currentByteValue / x_HexaDecimalBase; // Obtain MSD

		result += static_cast<double>(powerOf16 * lsd);
		powerOf16 = powerOf16 * x_HexaDecimalBase;

		result += static_cast<double>(powerOf16 * msd);
		powerOf16 = powerOf16 * x_HexaDecimalBase;
	}

	return result;
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
	RVectorType     vectorInR,
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
// Name: RTypeUtils::InsertStringBasedOnLengthAndRowsNumber
//
// Description:
//  Given the unicode string, inserts it into the data vector based on the
//  rowsNumber(total number of rows) in the data
//  and the length of the string adjusted as per allowedLen.
//  We trim the string upto the allowedLen if the original length goes beyond it.
//  For multiple rows with interspersed empty strings in the data, it doesn't add 0 length strings
//  between two non-empty strings. But for a single row, it adds a 0 length string explicitly.
//
// Returns:
//  The length of the string that is inserted.
//
// Remarks:
//  In the case when an empty string is inserted in the vector, the return value is 0 since the
//  length of the string inserted is 0 but the vector's size is still 1.
//
template<class SQLType, class UnicodeString>
SQLINTEGER RTypeUtils::InsertStringBasedOnLengthAndRowsNumber(
	const SQLULEN   allowedLen,
	const SQLULEN   rowsNumber,
	vector<SQLType> *data,
	UnicodeString   &unicodeString)
{
	SQLINTEGER lengthOfStringToInsert = unicodeString.length();

	// Truncate size of return data to only be the size allowed,
	// unless allowed length is equal to or greater than USHRT_MAX.
	// In this case do not truncate as we have a max sized variable,
	// ie: varchar(max) 
	//
	lengthOfStringToInsert =
		static_cast<SQLULEN>(lengthOfStringToInsert) <= allowedLen || allowedLen >= USHRT_MAX
		? lengthOfStringToInsert
		: static_cast<SQLINTEGER>(allowedLen);

	if (rowsNumber > 1 || lengthOfStringToInsert > 0)
	{
		// Do this when there are multiple rows in the data OR
		// in case of single row, only when it is not an empty string.
		//
		data->insert(data->end(), unicodeString.begin(),
			unicodeString.begin() + lengthOfStringToInsert);
	}
	else
	{
		// When its a single row and is an empty string,
		// add a single element of value '\0' to the vector.
		//
		data->push_back(unicodeString[0]);
	}

	return lengthOfStringToInsert;
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

			SQLINTEGER lengthOfStringInserted = 0;

			if constexpr (is_same_v<SQLType, SQLWCHAR>)
			{
				u16string utf16String;
				estd::ToUtf16(utf8String.c_str(), bytesOccupied, utf16String);
				lengthOfStringInserted =
					InsertStringBasedOnLengthAndRowsNumber<SQLWCHAR, u16string>(
						allowedLen,
						rowsNumber,
						data,
						utf16String);
			}
			else
			{
				lengthOfStringInserted =
					InsertStringBasedOnLengthAndRowsNumber<SQLCHAR, string>(
						allowedLen,
						rowsNumber,
						data,
						utf8String);
			}

			strLenOrInd[index] = lengthOfStringInserted * sizeof(SQLType);
			if (maxLen < static_cast<SQLULEN>(lengthOfStringInserted))
			{
				maxLen = lengthOfStringInserted;
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

	// Truncate size of return data to only be the size allowed,
	// unless allowed length is equal to or greater than USHRT_MAX.
	// In thise case do not truncate as we have a max sized variable,
	// ie: varbinary(max) 
	//
	strLenOrInd[0] =
		static_cast<SQLULEN>(vectorInR.size()) <= allowedLen || allowedLen >= USHRT_MAX
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
//  Otherwise if they are NA, sets nullable to true, pushes back an empty zeroed struct
//  into data, and sets the corresponding cells in the array strLenOrInd to SQL_NULL_DATA.
//
template<class SQLType, class RVectorType, class DateTimeTypeInR>
void RTypeUtils::FillDataFromDateTimeVector(
	SQLULEN         rowsNumber,
	RVectorType     vectorInR,
	vector<SQLType> *data,
	SQLINTEGER      *strLenOrInd,
	SQLSMALLINT     &nullable)
{
	LOG("RTypeUtils::FillDataFromDateTimeVector");

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
			// Add zeroed value for the null placeholder.
			// The ODBC protocol used for exchange of data between Exthost and the extension expects
			// empty value for null if the type is fixed and date/datetime/datetime2 are all
			// fixed types.
			//
			SQLType value;
			memset(&value, 0, sizeof(SQLType));
			data->push_back(value);

			strLenOrInd[index] = SQL_NULL_DATA;
			nullable = SQL_NULLABLE;
		}
	}
}

//-------------------------------------------------------------------------------------------------
// Name: RTypeUtils::FillDataFromNumericVector
//
// Description:
//  Given the vectorInR, copies its content into the given std::vector pointed to by data.
//  Copies the content only as far as the rowsNumber indicates.
//  The cells in the strLenOrInd array are set to the size of the SQL data type if the
//  corresponding rows in the vectorInR are not NA.
//  Otherwise if they are NA, sets nullable to true and the corresponding cells in
//  the array strLenOrInd to SQL_NULL_DATA.
//
void RTypeUtils::FillDataFromNumericVector(
	SQLULEN                    rowsNumber,
	Rcpp::NumericVector        vectorInR,
	vector<SQL_NUMERIC_STRUCT> *data,
	SQLINTEGER                 *strLenOrInd,
	SQLSMALLINT                &nullable,
	SQLSMALLINT                decimalDigits,
	SQLCHAR                    precision)
{
	LOG("RTypeUtils::FillDataFromNumericVector");

	for(SQLULEN index = 0 ; index < rowsNumber; ++index)
	{
		double valueInR = vectorInR[index];
		if (!Rcpp::NumericVector::is_na(valueInR))
		{
			SQL_NUMERIC_STRUCT value = { 0, 0, 0, 0 };
			if (valueInR >= 0)
			{
				value.sign = 1;
			}
			else
			{
				valueInR *= (-1.0);
			}

			value.precision = precision;
			value.scale = decimalDigits;

			// The scaled value is computed as (the given decimal number * 10 ^ decimalDigits).
			// e.g. for type numeric(8, 3) i.e. 8 precision and 3 as the scale i.e. decimalDigits,
			// a value of 3.4 will have the scaled value = 3.4 * 10 ^ 3 = 3400
			// If the value in R has more decimal digits than supported by scale,
			// we will have left over decimal digits, so we round it to make sure scaledValue is
			// has no decimal portion before converting to bytes.
			//
			double scaledValue = round(valueInR * x_PowersOf10[decimalDigits]);

			// Convert the scaledValue into the val array in the numeric struct
			// as an equivalent little endian byte representation
			// e.g. for 3400, val[0]=0x48 and val[1]=0x0d.
			//
			if (scaledValue < static_cast<double>(ULLONG_MAX))
			{
				// If the scaledValue is < ULLONG_MAX, we optimize this by simply assigning
				// the uint64_t value as is since all processors - x86, x64 store in little endian
				// representation internally.
				//
				SQLCHAR *basePointer = &value.val[0];
				*((uint64_t*)basePointer) = static_cast<uint64_t>(scaledValue);
			}
			else
			{
				ConvertDoubleToBytes(scaledValue, value.val);
			}

			data->push_back(value);
			strLenOrInd[index] = sizeof(SQL_NUMERIC_STRUCT);
		}
		else
		{
			strLenOrInd[index] = SQL_NULL_DATA;
			nullable = SQL_NULLABLE;
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: RTypeUtils::ConvertDoubleToBytes
//
// Description:
//  Given a double value of base 10, converts it into hexadecimal value which is stored into the
//  given byte array of max length SQL_MAX_NUMERIC_LEN in little endian byte order.
//  It creates a byte equal to the value of two hexadecimal digits - msd and lsd.
//  Starting with the given value as the dividend, it stores the remainder as hexadecimal digits
//  and uses the quotient as the subsequent dividend.
//
void RTypeUtils::ConvertDoubleToBytes(
	double  value,
	SQLCHAR *leBytesArray)
{
	double quotient = 0.0;
	double dividend = value;
	int lsd = 0, msd = 0;

	for(int byte = 0; byte < SQL_MAX_NUMERIC_LEN && dividend != 0.0 ; ++byte)
	{
		quotient = floor(dividend / x_HexaDecimalBase);
		lsd = static_cast<int>(dividend - quotient * x_HexaDecimalBase);
		dividend = quotient;

		quotient = floor(dividend / x_HexaDecimalBase);
		msd = static_cast<int>(dividend - quotient * x_HexaDecimalBase);
		dividend = quotient;

		int currentByteValue = static_cast<int>(msd * x_HexaDecimalBase) + lsd;
		leBytesArray[byte] = static_cast<SQLCHAR>(currentByteValue);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: RTypeUtils::CopySQLTypeVector
//
// Description:
// Given a SQLType vector, copy it into a unique pointer to SQLType array
// and return the unique pointer.
//
template<class SQLType>
unique_ptr<SQLType[]> RTypeUtils::CopySQLTypeVector(const vector<SQLType> &src)
{
	// The size of vector data in bytes:
	//
	int vectorSizeInBytes = sizeof(SQLType) * src.size();

	unique_ptr<SQLType[]> dataPtr(new SQLType[src.size()]);
	memcpy(dataPtr.get(), src.data(), vectorSizeInBytes);
	return dataPtr;
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

template unique_ptr<SQLCHAR[]> RTypeUtils::CopySQLTypeVector
	<SQLCHAR>(const vector<SQLCHAR> &src);

template unique_ptr<SQLDOUBLE[]> RTypeUtils::CopySQLTypeVector
	<SQLDOUBLE>(const vector<SQLDOUBLE> &src);

template unique_ptr<SQLINTEGER[]> RTypeUtils::CopySQLTypeVector
	<SQLINTEGER>(const vector<SQLINTEGER> &src);

template unique_ptr<SQL_DATE_STRUCT[]> RTypeUtils::CopySQLTypeVector
	<SQL_DATE_STRUCT>(const vector<SQL_DATE_STRUCT> &src);

template unique_ptr<SQL_TIMESTAMP_STRUCT[]> RTypeUtils::CopySQLTypeVector
	<SQL_TIMESTAMP_STRUCT>(const vector<SQL_TIMESTAMP_STRUCT> &src);
