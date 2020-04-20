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
// @File: RTypeUtils.h
//
// Purpose:
//  Class providing functions to manipulate between R data types and SQL types.
//
//*************************************************************************************************

#pragma once

class RTypeUtils
{
public:

	// Templatized function to create a vector of an equivalent R type for the given SQL type
	// with the given data. This is only for numeric or integer R types.
	//
	template<class SQLType, class RType, class NAType>
	static RType CreateVector(
		SQLULEN      rowsNumber,
		SQLPOINTER   data,
		SQLINTEGER   *strLen_or_Ind,
		const NAType valueForNA);

	// Create a logical vector in R corresponding to the given data.
	//
	static Rcpp::LogicalVector CreateLogicalVector(
		SQLULEN    rowsNumber,
		SQLPOINTER data,
		SQLINTEGER *strLen_or_Ind);

	// Create a character vector in R with the given data.
	//
	static Rcpp::CharacterVector CreateCharacterVector(
		SQLULEN    rowsNumber,
		SQLPOINTER data,
		SQLINTEGER *strLen_or_Ind);

	// Create a raw vector in R corresponding to the given binary data.
	//
	static Rcpp::RawVector CreateRawVector(
		SQLULEN    rowsNumber,
		SQLPOINTER data,
		SQLINTEGER *strLen_or_Ind);

	// Map to store the R class to ODBC C type mapping
	//
	static const std::unordered_map<std::string, SQLSMALLINT> m_classInRToOdbcTypeMap;

	// Type map typedefs
	//
	typedef std::unordered_map<std::string, SQLSMALLINT> RToOdbcTypeMap;
};
