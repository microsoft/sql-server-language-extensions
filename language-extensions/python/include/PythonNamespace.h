//**************************************************************************************************
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
//**************************************************************************************************

#pragma once
#include "Common.h"

//---------------------------------------------------------------------
// Description:
//	Global class storing the python namespace
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

	// Get the main python namespace
	//
	static boost::python::object& MainNamespace() { return m_mainNamespace; }

	// Get the main python module
	//
	static boost::python::object& MainModule() { return m_mainModule; }

	// Get the original path list
	//
	static boost::python::object& OriginalPath() { return m_originalPath; }

private:
	static boost::python::object m_mainNamespace;
	static boost::python::object m_mainModule;
	static boost::python::object m_originalPath;
};
