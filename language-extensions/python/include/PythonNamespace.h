//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonNamespace.h
//
// Purpose:
//  Global class to keep the global python namespace
//
//*************************************************************************************************

#pragma once
#include "Common.h"

//-------------------------------------------------------------------------------------------------
// Description:
//  Global class storing the python namespace
//
class PythonNamespace
{
public:
	// Initialize this global class
	//
	static void Init();

	// Cleanup this global class
	//
	static void Cleanup();

	// Get the main python module
	//
	static boost::python::object &MainModule() { return sm_mainModule; }

	// Get the main python namespace
	//
	static boost::python::object& MainNamespace() { return sm_mainNamespace; }

	// Get the original path list
	//
	static boost::python::object& OriginalPath() { return sm_originalPath; }

private:
	static boost::python::object sm_mainModule; // The boost python module, contains the namespace.

	// The underlying boost::python namespace, which contains all the python variables.
	// We execute any python scripts on this namespace.
	//
	static boost::python::object sm_mainNamespace;
	
	static boost::python::object sm_originalPath;  // The original system python path
};
