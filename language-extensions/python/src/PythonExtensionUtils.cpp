//**************************************************************************************************
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
//**************************************************************************************************

#include "Logger.h"
#include "PythonExtensionUtils.h"

using namespace std;
namespace py = boost::python;

// Null values
//
const static float m_floatNull = NAN;
const static int m_intNull = 0;
const static bool m_boolNull = false;

// Map to store the ODBC C datatype to null value in Python.
//
const unordered_map<SQLSMALLINT, const void*> PythonExtensionUtils::m_DataTypeToNullMap =
{
	{SQL_C_BIT, static_cast<const void*>(&m_boolNull)},
	{SQL_C_SLONG, static_cast<const void*>(&m_intNull)},
	{SQL_C_FLOAT, static_cast<const void*>(&m_floatNull)},
	{SQL_C_DOUBLE, static_cast<const void*>(&m_floatNull)},
	{SQL_C_SBIGINT, static_cast<const void*>(&m_intNull)},
	{SQL_C_SSHORT, static_cast<const void*>(&m_intNull)},
	{SQL_C_UTINYINT, static_cast<const void*>(&m_intNull)}
};

// Parses the value of the active python exception
// Type, value, and traceback are in separate pointers
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
		py::handle<> handleTrace(pTraceback);

		// Load the traceback module and the format_tb function
		//
		py::object traceback(py::import("traceback"));
		py::object format_tb(traceback.attr("format_tb"));

		// Call format_tb to get a list of traceback strings
		//
		py::object traceList(format_tb(handleTrace));

		// Join the traceback strings into a single string
		//
		py::object tracePyStr(py::str("\n").join(traceList));

		// Extract the string, check the extraction, and fallback if necessary
		//
		string trace = ExtractString(tracePyStr.ptr());

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


// Extract the string from a boost::python object
//
string PythonExtensionUtils::ExtractString(PyObject *pObj)
{
	string ret;
	py::handle<> handle(pObj);
	py::str pyStr(handle);

	// Extract the string from the boost::python object
	//
	py::extract<string> extracted(pyStr);

	// If a valid string extraction is available, use it
	// otherwise return empty string
	//
	if (extracted.check())
	{
		ret = extracted();
	}

	return ret;
}
