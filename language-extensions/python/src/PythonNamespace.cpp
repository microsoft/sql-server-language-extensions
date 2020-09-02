//*************************************************************************************************
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
//*************************************************************************************************

#include "PythonNamespace.h"
#include "PythonPathSettings.h"

using namespace std;
namespace bp = boost::python;

//-------------------------------------------------------------------------------------------------
// Name: PythonNamespace::Init
//
// Description:
//  Initialize the class
//
void PythonNamespace::Init()
{
	sm_mainModule = bp::import("__main__");
	sm_mainNamespace = sm_mainModule.attr("__dict__");

	// Check that the module and namespace are populated, not None objects
	//
	if (sm_mainModule == bp::object() ||
		sm_mainNamespace == bp::object())
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

	bp::exec(setupScript.c_str(), sm_mainNamespace);

	string privateLibPath = PythonPathSettings::PrivateLibraryPath();
	string publicLibPath = PythonPathSettings::PublicLibraryPath();

	// Setup the path to include private and public external library paths
	// so that we can find the external packages that are installed.
	// We set the private/public paths in front of the sys.path so they are searched first.
	//
	sm_originalPath = bp::extract<bp::list>(bp::eval("sys.path", sm_mainNamespace));
	bp::list newPath(sm_originalPath);
	newPath.insert(0, bp::str(publicLibPath));
	newPath.insert(0, bp::str(privateLibPath));

	PySys_SetObject("path", newPath.ptr());
}

//-------------------------------------------------------------------------------------------------
// Name: PythonNamespace::Cleanup
//
// Description:
//  Cleanup, reset the Python syspath
//
void PythonNamespace::Cleanup()
{
	PySys_SetObject("path", sm_originalPath.ptr());
}

bp::object PythonNamespace::sm_mainNamespace;
bp::object PythonNamespace::sm_mainModule;
bp::object PythonNamespace::sm_originalPath;
