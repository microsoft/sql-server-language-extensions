//*************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server
// external language communication protocol.
// Copyright (C) 2019 Microsoft Corporation.
//
// This file is part of RExtension-test.
//
// RExtension-test is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RExtension-test is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RExtension-test.  If not, see <https://www.gnu.org/licenses/>.
//
// @File: Utilities.cpp
//
// Purpose:
//  Utility functions
//
//*************************************************************************************************

#include <dlfcn.h>
#include <iostream>
#include <string>

#include "Utilities.h"

using namespace std;

//----------------------------------------------------------------------------
// Name: Utilities::CrossPlatLoadLibrary
//
// Description:
//	Gets the handle for the given library path after loading it dynamically
//
// Returns:
//	On success, returns handle to the library. Otherwise, nullptr.
//
void* Utilities::CrossPlatLoadLibrary(const char *libPath)
{
	void *libHandle = nullptr;

	libHandle = dlopen(libPath, RTLD_LAZY);
	if (libHandle == nullptr)
	{
		char* error = dlerror();
		cout << "Failed to load the lib path '" << libPath
			<< "' with error : " << error << endl;
	}

	return libHandle;
}

//----------------------------------------------------------------------------
// Name: Utilities::CrossPlatGetFunctionFromLibHandle
//
// Description:
//	Get the function pointer for the given function name from the given library handle
//
// Returns:
//	On success, returns handle to the function. Otherwise, nullptr.
//
void* Utilities::CrossPlatGetFunctionFromLibHandle(void *libHandle, const std::string &fnName)
{
	// this will be the pointer to the function to be returned
	//
	void *pFuncPtr = nullptr;

	if (libHandle != nullptr)
	{
		pFuncPtr = static_cast<void*>(dlsym(libHandle, fnName.c_str()));
		if (pFuncPtr == nullptr)
		{
			char* error = dlerror();
			cout << "Error finding function '" << fnName << "' :" << error << endl;
		}
	}
	else
	{
		cout << "Received null library handle" << endl;
	}

	return pFuncPtr;
}

//----------------------------------------------------------------------------
// Name: Utilities::CrossPlatGetFunctionFromLibPath
//
// Description:
//	Get the function pointer for the given function name from the given library path
//
// Returns:
//	On success, returns handle to the function. Otherwise, nullptr.
//
void* Utilities::CrossPlatGetFunctionFromLibPath(const std::string &libPath, const std::string &fnName)
{
	void* libHandle = CrossPlatLoadLibrary(libPath.c_str());
	void* pFuncPtr = nullptr;
	if (libHandle != nullptr)
	{
		pFuncPtr = CrossPlatGetFunctionFromLibHandle(libHandle, fnName);
		if (pFuncPtr == nullptr)
		{
			dlclose(libHandle);
		}
	}

	return pFuncPtr;
}
