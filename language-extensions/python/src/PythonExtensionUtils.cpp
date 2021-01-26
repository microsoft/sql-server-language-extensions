//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonExtensionUtils.cpp
//
// Purpose:
//  Non-platform specific utility functions for PythonExtension
//
//*************************************************************************************************

#include "Logger.h"
#include "PythonExtensionUtils.h"
#include "PythonPathSettings.h"

using namespace std;
namespace bp = boost::python;

// Null values
//
const static float sm_floatNull = NAN;
const static int sm_intNull = 0;
const static bool sm_boolNull = false;

// Map to store the ODBC C datatype to null value in Python.
//
const unordered_map<SQLSMALLINT, const void*> PythonExtensionUtils::sm_DataTypeToNullMap =
{
	{SQL_C_BIT, static_cast<const void*>(&sm_boolNull)},
	{SQL_C_SLONG, static_cast<const void*>(&sm_intNull)},
	{SQL_C_FLOAT, static_cast<const void*>(&sm_floatNull)},
	{SQL_C_DOUBLE, static_cast<const void*>(&sm_floatNull)},
	{SQL_C_SBIGINT, static_cast<const void*>(&sm_intNull)},
	{SQL_C_SSHORT, static_cast<const void*>(&sm_intNull)},
	{SQL_C_UTINYINT, static_cast<const void*>(&sm_intNull)}
};

//-------------------------------------------------------------------------------------------------
// Name: PythonExtensionUtils::ParsePythonException
//
// Description:
//  Parses the value of the active python exception.
//  Type, value, and traceback are in separate pointers.
//
// Returns:
//  String version of the python error
//
string PythonExtensionUtils::ParsePythonException()
{
	PyObject *pType = NULL;
	PyObject *pValue = NULL;
	PyObject *pTraceback = NULL;

	// Fetch the exception info from the Python C API
	//
	PyErr_Fetch(&pType, &pValue, &pTraceback);

	// Fallback error
	//
	string ret("Unfetchable Python error");

	// If the fetch got a type pointer, parse the type into the exception string
	//
	if (pType != NULL)
	{
		string type = ExtractString(pType);

		// If a valid string extraction is available, use it
		// otherwise use fallback string
		//
		if (type.empty())
		{
			ret = "Unknown exception type";
		}
		else
		{
			ret = type;
		}
	}

	// Do the same for the exception value (the stringification of the exception)
	//
	if (pValue != NULL)
	{
		string value = ExtractString(pValue);

		if (value.empty())
		{
			ret += string(": Unparseable Python error: ");
		}
		else
		{
			ret += ": " + value;
		}
	}

	// Parse lines from the traceback using the Python traceback module
	//
	if (pTraceback != NULL)
	{
		bp::handle<> handleTrace(pTraceback);

		// Load the traceback module and the format_tb function
		//
		bp::object traceback(bp::import("traceback"));
		bp::object format_tb(traceback.attr("format_tb"));

		// Call format_tb to get a list of traceback strings
		//
		bp::object traceList(format_tb(handleTrace));

		// Join the traceback strings into a single string
		//
		bp::object tracePyStr(bp::str("\n").join(traceList));

		// Extract the string, check the extraction, and fallback if necessary
		//
		string trace = ExtractString(tracePyStr);

		if (trace.empty())
		{
			ret += string(": Unparseable Python traceback");
		}
		else
		{
			ret += ": " + trace;
		}
	}

	return ret;
}

//-------------------------------------------------------------------------------------------------
// Name: PythonExtensionUtils::ExtractString
//
// Description:
//  Extract the string from a PyObject, then DECREFS the PyObject.
//  NOTE: This function STEALS the reference and destroys it.
//
// Returns:
//  String inside the PyObject
//
string PythonExtensionUtils::ExtractString(PyObject *pObj)
{
	bp::handle<> handle(pObj);

	return PythonExtensionUtils::ExtractString(bp::object(handle));
}

//-------------------------------------------------------------------------------------------------
// Name: PythonExtensionUtils::ExtractString
//
// Description:
//  Extract the string from a boost::python object
//
// Returns:
//  String inside the boost::python object
//
string PythonExtensionUtils::ExtractString(bp::object handle)
{
	string ret;
	bp::str pyStr(handle);

	// Extract the string from the boost::python object
	//
	bp::extract<string> extracted(pyStr);

	// If a valid string extraction is available, use it
	// otherwise return empty string
	//
	if (extracted.check())
	{
		ret = extracted();
	}

	return ret;
}

//-------------------------------------------------------------------------------------------------
// Name: PythonExtensionUtils::NormalizePathString
//
// Description:
//  Normalize path strings by replacting \ with /
//
// Returns:
//  The normalized path string
//
string PythonExtensionUtils::NormalizePathString(string pathString)
{
	replace(pathString.begin(), pathString.end(), '\\', '/');
	return pathString;
}

//-------------------------------------------------------------------------------------------------
// Name: PythonExtensionUtils::IsBitTrue
//
// Description:
//  Check if a SQLCHAR bit is True (not 0 or '0').
//
// Returns:
//  What the boolean value of the bit is
//
bool PythonExtensionUtils::IsBitTrue(SQLCHAR bitValue)
{
	return bitValue != '0' && bitValue != 0;
}