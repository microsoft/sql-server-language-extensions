//**************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonNamespace.cpp
//
// Purpose:
//  Global class to keep the global python namespace
//
//**************************************************************************************************

#include "PythonNamespace.h"
#include "PythonPathSettings.h"

using namespace std;
namespace py = boost::python;

//----------------------------------------------------------------------------
// Name: PythonNamespace::Init
//
// Description:
//	Initialize the class
//
void PythonNamespace::Init()
{
	m_mainModule = py::import("__main__");
	m_mainNamespace = m_mainModule.attr("__dict__");

	// Check that the module and namespace are populated, not None objects
	//
	if (m_mainModule == py::object() ||
		m_mainNamespace == py::object())
	{
		throw runtime_error("Main module or namespace was None");
	}

	// Setup the devnull device (which we do not have access to in Windows)
	// to redirect to a file. We create that file here for future use.
	// Also, import packages that we will need in later functions.
	//
	string setupScript =
		"import os\n"
		"import sys\n"
		"import platform\n"
		"from pandas import DataFrame\n"
		"import numpy as np\n"
		"_originalpath = list(sys.path)\n"
		"if platform.system() == 'Windows':\n"
		"    oldnulldevice = os.devnull\n"
		"    os.devnull = 'nulldeviceout.txt'\n"
		"    nulhandle = open(os.devnull, 'w+')\n"
		"    nulhandle.close()";

	py::exec(setupScript.c_str(), m_mainNamespace);

	string privateLibPath = PythonPathSettings::PrivateLibraryPath();
	string publicLibPath = PythonPathSettings::PublicLibraryPath();

	// Setup the path to include private and public external library paths
	// so that we can find the external packages that are installed.
	// We set the private/public paths in front of the sys.path so they are searched first.
	//
	m_originalPath = py::extract<py::list>(py::eval("sys.path", m_mainNamespace));
	py::list newPath(m_originalPath);
	newPath.insert(0, py::str(publicLibPath));
	newPath.insert(0, py::str(privateLibPath));

	PySys_SetObject("path", newPath.ptr());
}

//----------------------------------------------------------------------------
// Name: PythonNamespace::Cleanup
//
// Description:
//	Cleanup, reset the Python syspath
//
void PythonNamespace::Cleanup()
{
	PySys_SetObject("path", m_originalPath.ptr());
}

py::object PythonNamespace::m_mainNamespace;
py::object PythonNamespace::m_mainModule;
py::object PythonNamespace::m_originalPath;