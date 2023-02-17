//*************************************************************************************************
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
//*************************************************************************************************

#include "PythonTestUtilities.h"

using namespace std;
namespace bp = boost::python;

// Name: PythonTestUtilities::ParsePythonException
//
// Description:
//  Parses the value of the active python exception.
//  Type, value, and traceback are in separate pointers.
//
// Returns:
//  String version of the python error
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

// Name: PythonExtensionUtils::ExtractString
//
// Description:
//  Extract the string from a PyObject, then DECREFS the PyObject.
//  NOTE: This function STEALS the reference and destroys it.
//
// Returns:
//  String inside the PyObject
//
string PythonTestUtilities::ExtractString(PyObject *pObj)
{
	bp::handle<> handle(pObj);

	return ExtractString(bp::object(handle));
}

// Name: PythonExtensionUtils::ExtractString
//
// Description:
//  Extract the string from a boost::python object
//
// Returns:
//  String inside the boost::python object
//
string PythonTestUtilities::ExtractString(bp::object handle)
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

//--------------------------------------------------------------------------------------------------
// Name: Utilities::Tokenize
//
// Description:
//  Splits the given character string using the delimiter and
//  adds the tokens to the input vector.
// Remarks:
//  If the input is invalid and unable to be parsed,
//  token will be nullptr and we add nothing to the vector
//
// Returns:
//  nothing.
//
void PythonTestUtilities::Tokenize(
	char           *input,
	const char     *delimiter,
	vector<char *> *tokens)
{
	char *token = strtok(input, delimiter);

	// Use strtok iteratively to do the tokenization.
	//
	while (token != nullptr)
	{
		(*tokens).push_back(token);
		token = strtok(nullptr, delimiter);
	}
}