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
// @File: RTypeUtils.h
//
// Purpose:
//  Class providing functions to manipulate between R data types and SQL types.
//
//**************************************************************************************************

#pragma once

class RTypeUtils
{
public:

	// Templatized function to create a vector of an equivalent R type for the given SQL type
	// with the given data. This is only for numeric, integer or logical R types.
	//
	template<class SQLType, class RVectorType, class NAType, SQLSMALLINT DataType>
	static RVectorType CreateVector(
		SQLULEN     rowsNumber,
		SQLPOINTER  data,
		SQLINTEGER  *strLen_or_Ind,
		SQLSMALLINT nullable);

	// Creates a character vector in R with the data of type CharType.
	// CharType could be the default char with utf8 encoding,
	// or char16_t with utf16 encoding.
	//
	template<class CharType>
	static Rcpp::CharacterVector CreateCharacterVector(
		SQLULEN    rowsNumber,
		SQLPOINTER data,
		SQLINTEGER *strLen_or_Ind);

	// Creates a raw vector in R corresponding to the given binary data.
	//
	static Rcpp::RawVector CreateRawVector(
		SQLULEN    rowsNumber,
		SQLPOINTER data,
		SQLINTEGER *strLen_or_Ind);

	// Creates a date/datetime vector in R corresponding to the given data.
	//
	template<class SQLType, class RVectorType, class DateTimeTypeInR>
	static RVectorType CreateDateTimeVector(
		SQLULEN     rowsNumber,
		SQLPOINTER  data,
		SQLINTEGER  *strLen_or_Ind,
		SQLSMALLINT nullable);

	// Given the vectorInR, copies its content into the given std::vector pointed to by data.
	//
	template<class SQLType, class RVectorType, SQLSMALLINT DataType>
	static void FillDataFromRVector(
		SQLULEN              rowsNumber,
		RVectorType                vectorInR,
		std::vector<SQLType> *data,
		SQLINTEGER           *strLen_or_Ind,
		SQLSMALLINT          &nullable);

	// Given the vectorInR, copies its content into the given std::vector pointed to by charVector.
	//
	template<class SQLType>
	static void FillDataFromCharacterVector(
		SQLULEN               rowsNumber,
		Rcpp::CharacterVector vectorInR,
		SQLULEN               allowedLen,
		std::vector<SQLType>  *data,
		SQLINTEGER            *strLenOrInd,
		SQLSMALLINT           &nullable,
		SQLULEN               &maxLen);

	// Given the raw vectorInR, copies its content into the given std::vector
	// pointed to by rawCharVector.
	//
	static void FillDataFromRawVector(
		Rcpp::RawVector      vectorInR,
		SQLULEN              allowedLen,
		std::vector<SQLCHAR> *data,
		SQLINTEGER           *strLenOrInd);

	// Given the vectorInR, copies its content into the given std::vector pointed to by data.
	//
	template<class SQLType, class RVectorType, class DateTimeTypeInR>
	static void FillDataFromDateTimeVector(
		SQLULEN              rowsNumber,
		RVectorType                vectorInR,
		std::vector<SQLType> *data,
		SQLINTEGER           *strLenOrInd,
		SQLSMALLINT          &nullable);

	// Maps to store the ODBC C type to NA value mapping
	//
	static const std::unordered_map<SQLSMALLINT, SQLPOINTER> sm_dataTypeToNAMap;

	// Maps to store the R class to ODBC C type mapping
	//
	static const std::unordered_map<std::string, SQLSMALLINT> sm_classInRToOdbcTypeMap;

	// Maps typedef.
	//
	typedef std::unordered_map<std::string, SQLSMALLINT> RToOdbcTypeMap;
};
