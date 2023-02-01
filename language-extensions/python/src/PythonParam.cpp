//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonParam.cpp
//
// Purpose:
//  Class storing information about the PythonExtension input/output parameter.
//
//*************************************************************************************************

#include "Logger.h"
#include "PythonExtensionUtils.h"
#include "PythonParam.h"

// datetime.h includes macros that define the PyDateTime APIs
//
#include <datetime.h>

using namespace std;
namespace bp = boost::python;

//-------------------------------------------------------------------------------------------------
// Name: PythonParam
//
// Description:
//  Constructor.
//
PythonParam::PythonParam(
	SQLUSMALLINT  id,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   type,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType) :
	m_id(id),
	m_type(type),
	m_size(paramSize),
	m_decimalDigits(decimalDigits),
	m_strLenOrInd(strLen_or_Ind),
	m_inputOutputType(inputOutputType)
{

	// Remove "@" from the front of the name
	//
	const char *name = static_cast<const char*>(static_cast<const void*>(paramName + 1));

	// paramNameLength includes @, we remove it
	//
#if defined(_DEBUG)
	if (static_cast<size_t>(paramNameLength - 1) != strlen(name))
	{
		throw invalid_argument("Invalid parameter name length, it doesn't match string length.");
	}
#endif

	// Store the information for this column
	//
	m_name = string(name, paramNameLength - 1);
};

//-------------------------------------------------------------------------------------------------
// Name: CheckParamSize
//
// Description:
//  Verifies if m_Size is equal to the size of the template type T.
//  Returns nothing if the check succeeds, throws an exception otherwise.
//
template<class T>
void PythonParam::CheckParamSize()
{
	size_t dataTypeSize = sizeof(T);
	if (dataTypeSize != m_size)
	{
		string error("The parameter size(" + to_string(m_size) +
			") does not match the size of the supported datatype(" +
			to_string(dataTypeSize) + ").");
		LOG_ERROR(error);
		throw invalid_argument(error);
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonParamTemplate
//
// Description:
//  Constructor.
//  Calls the base constructor then populates m_pyObject with a boost::python object that contains
//  the parameter value, in a way that python can use, or bp::object which is None.
//
template<class SQLType>
PythonParamTemplate<SQLType>::PythonParamTemplate(
	SQLUSMALLINT  id,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   type,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType)
	: PythonParam(id,
		paramName,
		paramNameLength,
		type,
		paramSize,
		decimalDigits,
		strLen_or_Ind,
		inputOutputType)
{
	CheckParamSize<SQLType>();

	if (strLen_or_Ind != SQL_NULL_DATA)
	{
		m_pyObject = bp::object(*static_cast<SQLType*>(paramValue));
	}
	else
	{
		// Use None object for NULLs
		//
		m_pyObject = bp::object();
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonParamTemplate::RetrieveValueAndStrLenInd
//
// Description:
//  Retrieves the value from the namespace and populates m_value and m_strLenOrInd.
//  Template for int/float types.
//
template<class SQLType>
void PythonParamTemplate<SQLType>::RetrieveValueAndStrLenInd(bp::object mainNamespace)
{
	bp::dict dictNamespace = bp::extract<bp::dict>(mainNamespace);

	m_strLenOrInd = SQL_NULL_DATA;

	if (dictNamespace.has_key(m_name))
	{
		bp::object tempObj = mainNamespace[m_name];

		if(!tempObj.is_none())
		{
			bp::extract<SQLType> extractedObj(tempObj);
			if(extractedObj.check())
			{
				m_value.push_back(SQLType(extractedObj));
				m_strLenOrInd = sizeof(SQLType);
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonBooleanParam
//
// Description:
//  Constructor.
//  Calls the base constructor then populates m_pyObject with a boost::python object that contains
//  the parameter value, in a way that python can use, or bp::object which is None.
//
PythonBooleanParam::PythonBooleanParam(
	SQLUSMALLINT  id,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   type,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType)
	: PythonParam(id,
				paramName,
				paramNameLength,
				type,
				paramSize,
				decimalDigits,
				strLen_or_Ind,
				inputOutputType)
{
	if (strLen_or_Ind != SQL_NULL_DATA)
	{
		SQLCHAR val = *static_cast<SQLCHAR *>(paramValue);
		bool value = PythonExtensionUtils::IsBitTrue(val);
		m_pyObject = bp::object(value);
	}
	else
	{
		// Use None object for NULLs
		//
		m_pyObject = bp::object();
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonBooleanParam::RetrieveValueAndStrLenInd
//
// Description:
//  Retrieves the value from the namespace and populates m_value and m_strLenOrInd
//
void PythonBooleanParam::RetrieveValueAndStrLenInd(bp::object mainNamespace)
{
	bp::dict dictNamespace = bp::extract<bp::dict>(mainNamespace);
	if (dictNamespace.has_key(m_name))
	{
		bp::object tempObj = mainNamespace[m_name];

		m_strLenOrInd = SQL_NULL_DATA;

		if (!tempObj.is_none())
		{
			bp::extract<bool> extractedObj(tempObj);
			if (extractedObj.check())
			{
				m_value.push_back(static_cast<SQLCHAR>(bool(extractedObj)));
				m_strLenOrInd = sizeof(bool);
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonStringParam
//
// Description:
//  Constructor.
//  Calls the base constructor then populates m_pyObject with a boost::python object that contains
//  the parameter value, in a way that python can use, or bp::object which is None.
//  We use StrLen_or_Ind to calculate how long the string is before creating the python object.
//
template<class CharType>
PythonStringParam<CharType>::PythonStringParam(
	SQLUSMALLINT  id,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   type,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType)
	: PythonParam(id,
		paramName,
		paramNameLength,
		type,
		paramSize,
		decimalDigits,
		strLen_or_Ind,
		inputOutputType)
{
	if (strLen_or_Ind != SQL_NULL_DATA)
	{
		PyObject *pyObj = nullptr;
		SQLINTEGER strlen = strLen_or_Ind;

		char *str = reinterpret_cast<char *>(paramValue);

		// Create a string PyObject from the str and strLen.
		// This DOES copy the underlying string into a new buffer and null terminates it.
		// Then, convert to a boost object so that boost handles ref counting.
		//
		if constexpr (is_same_v<CharType, char>)
		{
			pyObj = PyUnicode_DecodeUTF8(
				str,      // char * version of string
				strlen,   // len of string in bytes
				nullptr); // special error handling options, we don't need any
		}
		else
		{
			int byteOrder = -1; // -1: little endian
			pyObj = PyUnicode_DecodeUTF16(
				str,         // char * version of string
				strlen,      // len of string in bytes
				nullptr,     // special error handling options, we don't need any
				&byteOrder); // byte order to parse UTF-16. SQL Server uses little-endian.
		}

		m_pyObject = bp::object(bp::handle<>(pyObj));
	}
	else
	{
		// Use None object for NULLs
		//
		m_pyObject = bp::object();
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonStringParam::RetrieveValueAndStrLenInd
//
// Description:
//  Retrieves the value from the namespace and populates m_value and m_strLenOrInd
//
template<class CharType>
void PythonStringParam<CharType>::RetrieveValueAndStrLenInd(bp::object mainNamespace)
{
	bp::dict dictNamespace = bp::extract<bp::dict>(mainNamespace);
	if (dictNamespace.has_key(m_name))
	{
		bp::object tempObj = mainNamespace[m_name];

		m_strLenOrInd = SQL_NULL_DATA;

		if (!tempObj.is_none())
		{
			if constexpr (is_same_v<CharType,char>)
			{
				// Check to make sure the extracted data exists and is of the correct type
				//
				bp::extract<string> extractedObj(tempObj);

				if (extractedObj.check())
				{
					// Extract and copy the string characters into the vector.
					//
					string value = extractedObj;
					if (!value.empty())
					{
						m_value = vector<CharType>(value.begin(), value.end());
					}
					else
					{
						m_value.push_back('\0');
						m_strLenOrInd = 0;
					}
				}
			}
			else
			{
				// Get length of the unicode object in pyObj
				//
				int size = PyUnicode_GET_LENGTH(tempObj.ptr());

				if (size > 0)
				{
					// Get a byte representation of the string as UTF16.
					// PyUnicode_AsUTF16String adds a BOM to the front of every string.
					//
					char *utf16str = PyBytes_AsString(PyUnicode_AsUTF16String(tempObj.ptr()));

					// Reinterpret the bytes as wchar_t *, which we will return.
					//
					CharType *wData = reinterpret_cast<CharType *>(utf16str);

					// Ignore 2 byte BOM at front of wData that was added by PyUnicode_AsUTF16String
					//
					m_value = vector<CharType>(wData + 1, wData + 1 + size);
				}
				else
				{
					m_value.push_back(L'\0');
					m_strLenOrInd = 0;
				}
			}

			// Truncate the return data to only be the size specified when creating. Only
			// truncate if m_size is less than USHRT_MAX because otherwise we have a
			// max sized variable, and we do not want to truncate max sized variables,
			// ie: varchar(max)
			//
			if (m_value.size() > m_size && m_size < USHRT_MAX)
			{
				m_value.resize(m_size);
				m_value.shrink_to_fit();
			}

			if (m_strLenOrInd == SQL_NULL_DATA)
			{
				m_strLenOrInd = m_value.size() * sizeof(CharType);
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonRawParam
//
// Description:
//  Constructor.
//  Calls the base constructor then populates m_pyObject with a boost::python object that contains
//  the bytes object of the data, in a way that python can use, or bp::object which is None.
//
PythonRawParam::PythonRawParam(
	SQLUSMALLINT  id,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   type,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType)
	: PythonParam(id,
		paramName,
		paramNameLength,
		type,
		paramSize,
		decimalDigits,
		strLen_or_Ind,
		inputOutputType)
{
	if (strLen_or_Ind != SQL_NULL_DATA)
	{
		SQLINTEGER strlen = strLen_or_Ind / sizeof(SQLCHAR);

		// Create a Python bytes object from binary
		//
		m_pyObject = bp::object(bp::handle<>(
			PyBytes_FromObject(PyMemoryView_FromMemory(
				static_cast<char *>(paramValue), strlen, PyBUF_READ
			))
			));
	}
	else
	{
		// Use None object for NULLs
		//
		m_pyObject = bp::object();
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonRawParam::RetrieveValueAndStrLenInd
//
// Description:
//  Retrieves the value from the namespace and populates m_value and m_strLenOrInd
//
void PythonRawParam::RetrieveValueAndStrLenInd(bp::object mainNamespace)
{
	bp::dict dictNamespace = bp::extract<bp::dict>(mainNamespace);

	if (dictNamespace.has_key(m_name))
	{
		bp::object tempObj = mainNamespace[m_name];
		
		m_strLenOrInd = SQL_NULL_DATA;

		if (!tempObj.is_none())
		{
			// The uninitialized iterator is equivalent to the end of the iterable
			//
			bp::stl_input_iterator<SQLCHAR> begin(tempObj), end;

			// Copy the py_buffer into a local buffer.
			//
			m_value = vector<SQLCHAR>(begin, end);

			if (m_value.empty()) 
			{
				m_value.push_back(L'\0');
				m_strLenOrInd = 0;
			}

			// Truncate the return data to only be the size specified when creating. Only
			// truncate if m_size is less than USHRT_MAX because otherwise we have a
			// max sized variable, and we do not want to truncate max sized variables,
			// ie: varbinary(max)
			//
			if (m_value.size() > m_size && m_size < USHRT_MAX)
			{
				m_value.resize(m_size);
				m_value.shrink_to_fit();
			}

			if (m_strLenOrInd == SQL_NULL_DATA)
			{
				m_strLenOrInd = m_value.size();
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonDateTimeParam
//
// Description:
//  Constructor.
//  Calls the base constructor then populates m_pyObject with a boost::python object that contains
//  the date/datetime object of the data, in a way that python can use, or bp::object which is None.
//
template<SQLSMALLINT DataType>
PythonDateTimeParam<DataType>::PythonDateTimeParam(
	SQLUSMALLINT  id,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   type,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType)
	: PythonParam(id,
		paramName,
		paramNameLength,
		type,
		paramSize,
		decimalDigits,
		strLen_or_Ind,
		inputOutputType)
{
	if (strLen_or_Ind != SQL_NULL_DATA)
	{
		// Use the PyDateTime_IMPORT macro to get the Python Date/Time APIs
		//
		PyDateTime_IMPORT;
		PyObject *dtObject = Py_None;

		// SQL_C_TYPE_DATE for Date objects in SQL, SQL_C_TYPE_TIMESTAMP for Datetime
		//
		if (DataType == SQL_C_TYPE_DATE)
		{
			SQL_DATE_STRUCT dateParam = *(static_cast<SQL_DATE_STRUCT *>(paramValue));

			// Create a Python Date object
			//
			dtObject = PyDate_FromDate(dateParam.year, dateParam.month, dateParam.day);
		}
		else if (DataType == SQL_C_TYPE_TIMESTAMP)
		{
			SQL_TIMESTAMP_STRUCT timeStampParam = *(static_cast<SQL_TIMESTAMP_STRUCT *>(paramValue));
			
			// "fraction" is stored in nanoseconds, we need microseconds.
			//
			SQLUINTEGER usec = timeStampParam.fraction / 1000;

			// Create a Python DateTime object
			//
			dtObject = PyDateTime_FromDateAndTime(
				timeStampParam.year, timeStampParam.month, timeStampParam.day,
				timeStampParam.hour, timeStampParam.minute, timeStampParam.second, usec);
		}

		m_pyObject = bp::object(bp::handle<>(dtObject));
	}
	else
	{
		// Use None object for NULLs
		//
		m_pyObject = bp::object();
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonDateTimeParam::RetrieveValueAndStrLenInd
//
// Description:
//  Retrieves the value from the namespace and populates m_value and m_strLenOrInd
//
template<SQLSMALLINT DataType>
void PythonDateTimeParam<DataType>::RetrieveValueAndStrLenInd(bp::object mainNamespace)
{
	bp::dict dictNamespace = bp::extract<bp::dict>(mainNamespace);
	if (dictNamespace.has_key(m_name))
	{
		bp::object tempObj = mainNamespace[m_name];

		bp::exec("from numpy import isnat, datetime64", mainNamespace);
		bool isNaT = false;

		if (DataType == SQL_C_TYPE_TIMESTAMP)
		{
			string checkNaTScript = "isnat(datetime64(" + m_name + "))";
			isNaT = bp::extract<bool>(bp::eval(checkNaTScript.c_str(), mainNamespace));
		}

		m_strLenOrInd = SQL_NULL_DATA;

		// Make sure the boost object is not pointing at Python None.
		// Also check the object type for NaT (Not a Time), a special timestamp type,
		// because that should be NULL in SQL as well.
		//
		if (!tempObj.is_none() && !isNaT)
		{
			PyDateTime_IMPORT;
			PyObject *dateObject = tempObj.ptr();

			SQLSMALLINT year = PyDateTime_GET_YEAR(dateObject);
			SQLUSMALLINT month = PyDateTime_GET_MONTH(dateObject);
			SQLUSMALLINT day = PyDateTime_GET_DAY(dateObject);
			SQLUSMALLINT hour = PyDateTime_DATE_GET_HOUR(dateObject);
			SQLUSMALLINT minute = PyDateTime_DATE_GET_MINUTE(dateObject);
			SQLUSMALLINT second = PyDateTime_DATE_GET_SECOND(dateObject);
			SQLUINTEGER usec = PyDateTime_DATE_GET_MICROSECOND(dateObject);

			// "fraction" in TIMESTAMP_STRUCT is stored in nanoseconds, we convert from microseconds.
			//
			SQL_TIMESTAMP_STRUCT datetime = { year, month, day, hour, minute, second, usec * 1000 };

			m_value.push_back(datetime);
			m_strLenOrInd = sizeof(SQL_TIMESTAMP_STRUCT);
		}
	}
}

//------------------------------------------------------------------------------------------------
// Do explicit template instantiations, so that object code is generated for these
// and the linker is able to find their definitions even after instantiations are in different
// translation units (i.e. PythonParamTemplate instantiation is in PythonParamContainer.cpp)
//
template PythonParamTemplate<SQLINTEGER>::PythonParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR *,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template PythonParamTemplate<SQLREAL>::PythonParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR *,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template PythonParamTemplate<SQLDOUBLE>::PythonParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR *,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template PythonParamTemplate<SQLBIGINT>::PythonParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR *,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template PythonParamTemplate<SQLSMALLINT>::PythonParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR *,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template PythonParamTemplate<SQLCHAR>::PythonParamTemplate(
	SQLUSMALLINT,
	const SQLCHAR *,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template PythonStringParam<char>::PythonStringParam(
	SQLUSMALLINT,
	const SQLCHAR *,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template PythonStringParam<wchar_t>::PythonStringParam(
	SQLUSMALLINT,
	const SQLCHAR *,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template PythonDateTimeParam<SQL_C_TYPE_DATE>::PythonDateTimeParam(
	SQLUSMALLINT,
	const SQLCHAR *,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);

template PythonDateTimeParam<SQL_C_TYPE_TIMESTAMP>::PythonDateTimeParam(
	SQLUSMALLINT,
	const SQLCHAR *,
	SQLSMALLINT,
	SQLSMALLINT,
	SQLULEN,
	SQLSMALLINT,
	SQLPOINTER,
	SQLINTEGER,
	SQLSMALLINT);
