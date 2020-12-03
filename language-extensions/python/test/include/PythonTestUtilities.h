//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonTestUtilities.h
//
// Purpose:
//  Utility functions for the tests
//
//*************************************************************************************************

#pragma once
#include "Common.h"

class PythonTestUtilities
{
public:

	// Parses the value of the active python exception
	// Type, value, and traceback are in separate pointers
	//
	static std::string ParsePythonException();

	// Extract the string from a boost::python object
	//
	static std::string ExtractString(PyObject * pObj);
	static std::string ExtractString(boost::python::object handle);
	static void Tokenize(char *input, const char *delimiter, std::vector<char *> *tokens);
};
