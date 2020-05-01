//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonParam.h
//
// Purpose:
//  Classes storing information about an PythonExtension input/output parameter.
//
//**************************************************************************************************

#pragma once
#include "Common.h"

//-------------------------------------------------------------------------------------------------
// Name: PythonParam
//
// Description:
// Base class storing information about the PythonExtension input/output parameter.
//
class PythonParam
{
public:
	// Get m_name
	//
	const std::string& Name() const
	{
		return m_name;
	}

	// Get m_pyObject
	//
	const boost::python::object& PythonValue() const
	{
		return m_pyObject;
	}

protected:
	// Protected constructor to initialize the members.
	// Do not allow other classes to create objects of this class - it is used as a base class.
	//
	PythonParam(
		SQLUSMALLINT  id,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   type,
		SQLULEN       paramSize,
		SQLSMALLINT   decimalDigits,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   inputOutputType);

	// Verifies if the input paramSize is equal to the size of the template type T.
	//
	template<class T>
	void CheckParamSize();

	// The boost::python object with the value to be placed in the nameSpace
	//
	boost::python::object m_pyObject;

	// Id of the parameter.
	//
	SQLUSMALLINT m_id;

	// Name of the parameter.
	//
	std::string m_name;

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
	//   value       m_strLenOrInd          m_size
	//   NULL        SQL_NULL_DATA          sizeof<type>
	//   Non-NULL    0                      sizeof<type>
	//
	// For char(n), binary(n) types,
	//   value       m_strLenOrInd          m_size
	//   NULL        SQL_NULL_DATA          n
	//   Non-NULL    n                      n
	//
	// For varchar(n), varbinary(n) types,
	//   value       m_strLenOrInd          m_size
	//   NULL        SQL_NULL_DATA          n
	//   Non-NULL    actualNumberOfChars    n
	//
	SQLINTEGER m_strLenOrInd;

	// Parameter type (Input/Output)
	//
	SQLSMALLINT m_inputOutputType;
};

//-------------------------------------------------------------------------------------------------
// Name: PythonParamTemplate
//
// Description:
// Template class representing numeric, integer parameters
//
template<class SQLType>
class PythonParamTemplate : public PythonParam
{

public:
	// Constructor to initialize the members
	//
	PythonParamTemplate(
		SQLUSMALLINT  id,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   type,
		SQLULEN       paramSize,
		SQLSMALLINT   decimalDigits,
		SQLPOINTER    paramValue,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   inputOutputType);
};

//-------------------------------------------------------------------------------------------------
// Name: PythonBooleanParam
//
// Description:
// Class representing a boolean parameter.
// Corresponds to ODBC C type SQL_C_BIT.
//
class PythonBooleanParam : public PythonParam
{
public:

	// Constructor to initialize the members
	//
	PythonBooleanParam(
		SQLUSMALLINT  id,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   type,
		SQLULEN       paramSize,
		SQLSMALLINT   decimalDigits,
		SQLPOINTER    paramValue,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   inputOutputType);
};

//-------------------------------------------------------------------------------------------------
// Name: PythonStringParam
//
// Description:
// Class representing a string parameter.
// Corresponds to ODBC C type SQL_C_CHAR.
//
class PythonStringParam : public PythonParam
{
public:

	// Constructor to initialize the members
	//
	PythonStringParam(
		SQLUSMALLINT  id,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   type,
		SQLULEN       paramSize,
		SQLSMALLINT   decimalDigits,
		SQLPOINTER    paramValue,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   inputOutputType);
};

//-------------------------------------------------------------------------------------------------
// Name: PythonRawParam
//
// Description:
// Class representing a raw parameter.
// Corresponds to ODBC C type SQL_C_BINARY.
//
class PythonRawParam : public PythonParam
{
public:

	// Constructor to initialize the members
	//
	PythonRawParam(
		SQLUSMALLINT  id,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   type,
		SQLULEN       paramSize,
		SQLSMALLINT   decimalDigits,
		SQLPOINTER    paramValue,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   inputOutputType);
};
