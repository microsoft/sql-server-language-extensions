//**************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonTestUtilities.cpp
//
// Purpose:
//  Utility functions for Python Tests
//
//**************************************************************************************************

#include "PythonTestUtilities.h"

using namespace std;
namespace py = boost::python;

// Parses the value of the active python exception
// Type, value, and traceback are in separate pointers
//
string PythonTestUtilities::ParsePythonException()
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

// Extract the string from a PyObject, then DECREFS the PyObject.
// NOTE: This function STEALS the reference and destroys it.
//
string PythonTestUtilities::ExtractString(PyObject *pObj)
{
	py::handle<> handle(pObj);

	return ExtractString(py::object(handle));
}

// Extract the string from a boost::python object
//
string PythonTestUtilities::ExtractString(py::object handle)
{
	string ret;
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