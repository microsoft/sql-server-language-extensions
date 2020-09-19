//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonExtensionUtils.h
//
// Purpose:
//  Platform specific utility functions for Python Extension
//
//*************************************************************************************************

#pragma once
#include "Common.h"
#include <unordered_map>

class PythonExtensionUtils
{
public:

	// Parses the value of the active python exception
	// Type, value, and traceback are in separate pointers
	//
	static std::string ParsePythonException();

	// Extract the string from a boost::python object or PyObject*
	//
	static std::string ExtractString(PyObject *pObj);
	static std::string ExtractString(boost::python::object handle);

	// Get the value of an environment variable
	//
	static std::string GetEnvVariable(const std::string &envVarName);

	// Normalize path strings from \ to /
	//
	static std::string NormalizePathString(std::string pathString);

	// Check if bitValue is True or not
	//
	static bool IsBitTrue(SQLCHAR bitValue);

	// Converts a SQLGUID to a string
	//
	static std::string ConvertGuidToString(const SQLGUID *guid);

	// Close an open dll handle
	//
	static void FreeDLL(void *pDll);

	// Get the path to the python executable
	//
	static std::string GetPathToPython();

	// Map to store the ODBC C type to null value mapping
	//
	static const std::unordered_map<SQLSMALLINT, const void *> sm_DataTypeToNullMap;
};
