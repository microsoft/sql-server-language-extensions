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
//*************************************************************************************************

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
	const boost::python::object& PythonObject() const
	{
		return m_pyObject;
	}

	// Get m_strLenOrInd
	//
	SQLINTEGER StrLenOrInd() const
	{
		return m_strLenOrInd;
	}

	// Get m_inputOutputType
	//
	SQLSMALLINT InputOutputType() const
	{
		return m_inputOutputType;
	}

	// Get m_value
	//
	virtual SQLPOINTER Value() const = 0;

	// Retrieve data from python namespace, fill it in m_value
	// and set m_strLenOrInd accordingly.
	//
	virtual void RetrieveValueAndStrLenInd(boost::python::object mainNamespace) = 0;

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
	//
	// For fixed non-char non-binary types,
	//     value   |  m_strLenorInd                                |  m_size
	//----------------------------------------------------------------------------
	//     NULL    |  SQL_NULL_DATA                                |  sizeof<type>
	//     Non-NULL|  0                                            |  sizeof<type>
	//
	// For char(n), binary(n) types,
	//     value   |  m_strLenOrInd                                |  m_size
	//----------------------------------------------------------------------------
	//     NULL    |  SQL_NULL_DATA                                |  n
	//     Non-NULL|  n                                            |  n
	//
	// For varchar(n), varbinary(n) types,
	//     value   |  m_strLenOrInd                                |  m_size
	//----------------------------------------------------------------------------
	//     NULL    |  SQL_NULL_DATA                                |  n
	//     Non-NULL|  actualNumberOfBytes(same as length)          |  n
	//
	// For nchar(n) type,
	//     value   |  m_strLenOrInd                                |  m_size
	//----------------------------------------------------------------------------
	//     NULL    |  SQL_NULL_DATA                                |  n
	//     Non-NULL|  n*sizeof(char16_t))                          |  n
	//
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

	// Retrieve data from python namespace, fill it in m_value
	// and set m_strLenOrInd accordingly
	//
	void RetrieveValueAndStrLenInd(boost::python::object mainNamespace) override;

	// Get the data underlying m_value vector
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
	// Vector holding the value of the parameter as retrieved from python namespace,
	// holding the contents before sending them back to ExtHost
	// Only useful for output parameter types.
	//
	std::vector<SQLType> m_value;
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

	// Retrieve data from python namespace, fill it in m_value
	// and set m_strLenOrInd accordingly
	//
	void RetrieveValueAndStrLenInd(boost::python::object mainNamespace) override;

	// Get the data underlying m_value vector
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
	// Character vector holding the contents before sending them back to ExtHost.
	// Useful for output parameter types.
	//
	std::vector<SQLCHAR> m_value;
};

//-------------------------------------------------------------------------------------------------
// Name: PythonStringParam
//
// Description:
// Class representing a string parameter.
// Corresponds to ODBC C type SQL_C_CHAR and SQL_C_WCHAR.
//
template<class CharType>
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

	// Retrieve data from python namespace, fill it in m_value
	// and set m_strLenOrInd accordingly
	//
	void RetrieveValueAndStrLenInd(boost::python::object mainNamespace) override;

	// Get the data underlying m_value vector
	//
	SQLPOINTER Value() const override
	{
		if (m_value.size() > 0)
		{
			return static_cast<SQLPOINTER>(
				const_cast<CharType*>(m_value.data()));
		}
		else
		{
			return nullptr;
		}
	}

private:
	// Character vector holding the contents before sending them back to ExtHost.
	// Useful for output parameter types.
	//
	std::vector<CharType> m_value;
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

	// Retrieve data from python namespace, fill it in m_value
	// and set m_strLenOrInd accordingly
	//
	void RetrieveValueAndStrLenInd(boost::python::object mainNamespace) override;

	// Get the data underlying m_value vector
	//
	SQLPOINTER Value() const override
	{
		if (m_value.size() > 0)
		{
			return static_cast<SQLPOINTER>(
				const_cast<SQLCHAR *>(m_value.data()));
		}
		else
		{
			return nullptr;
		}
	}

private:
	// Character vector holding the contents before sending them back to ExtHost.
	// Useful for output parameter types.
	//
	std::vector<SQLCHAR> m_value;
};

//-------------------------------------------------------------------------------------------------
// Name: PythonDateTimeParam
//
// Description:
//  Class representing a Date/DateTime parameter.
//  Corresponds to ODBC C type SQL_C_TYPE_DATE and SQL_C_TYPE_TIMESTAMP.
//
template<SQLSMALLINT SQLType>
class PythonDateTimeParam : public PythonParam
{
public:

	// Constructor to initialize the members
	//
	PythonDateTimeParam(
		SQLUSMALLINT  id,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   type,
		SQLULEN       paramSize,
		SQLSMALLINT   decimalDigits,
		SQLPOINTER    paramValue,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   inputOutputType);

	// Retrieve data from python namespace, fill it in m_value
	// and set m_strLenOrInd accordingly
	//
	void RetrieveValueAndStrLenInd(boost::python::object mainNamespace) override;

	// Get the data underlying m_value vector
	//
	SQLPOINTER Value() const override
	{
		if (m_value.size() > 0)
		{
			return static_cast<SQLPOINTER>(
				const_cast<SQL_TIMESTAMP_STRUCT *>(m_value.data()));
		}
		else
		{
			return nullptr;
		}
	}

private:
	// Timestamp vector holding the contents before sending them back to ExtHost.
	// Useful for output parameter types.
	//
	std::vector<SQL_TIMESTAMP_STRUCT> m_value;
};
