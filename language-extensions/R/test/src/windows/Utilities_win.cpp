//**************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server
// external language communication protocol.
// Copyright (C) 2020 Microsoft Corporation.
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
// @File: Utilities_win.cpp
//
// Purpose:
//  Utility functions
//
//**************************************************************************************************

#include "Common.h"

using namespace std;

//--------------------------------------------------------------------------------------------------
// Name: Utilities::CrossPlatLoadLibrary
//
// Description:
//  Gets the handle for the given library path after loading it dynamically
//
// Returns:
//  On success, returns handle to the library. Otherwise, throws a runtime_error exception.
//
void* Utilities::CrossPlatLoadLibrary(const char *libPath)
{
	HMODULE hDLL = LoadLibraryA(libPath);
	if (hDLL == NULL)
	{
		DWORD error = GetLastError();
		throw runtime_error("CrossPlatLoadLibrary failed loading '" + string(libPath) +
			"' with error: " + to_string(error));
	}

	return hDLL;
}

//--------------------------------------------------------------------------------------------------
// Name: Utilities::CrossPlatGetFunctionFromLibHandle
//
// Description:
//  Get the function pointer for the given function name from the given library handle
//
// Returns:
//  On success, returns handle to the function. Otherwise, throws a runtime_error exception.
//
void* Utilities::CrossPlatGetFunctionFromLibHandle(void *libHandle, const std::string &fnName)
{
	// this will be the pointer to the function to be returned
	//
	void *pFuncPtr = nullptr;

	HMODULE hDLL = static_cast<HMODULE>(libHandle);
	if (hDLL != nullptr)
	{
		pFuncPtr = reinterpret_cast<void *>(GetProcAddress(hDLL, fnName.c_str()));
		if (pFuncPtr == NULL)
		{
			DWORD error = GetLastError();
			CrossPlatCloseLibrary(libHandle);
			throw runtime_error("CrossPlatGetFunctionFromLibHandle failed finding function '"
				+ fnName + "' with error:" + to_string(error));
		}
	}
	else
	{
		cout << "Received null library handle" << endl;
	}

	return pFuncPtr;
}

//--------------------------------------------------------------------------------------------------
// Name: Utilities::CrossPlatCloseLibrary
//
// Description:
//  Given a valid library handle, close it.
//
// Returns:
//  On success, returns nothing. Otherwise, throws a runtime_error exception.
//
void Utilities::CrossPlatCloseLibrary(void *libHandle)
{
	if (libHandle != nullptr)
	{
		BOOL returnVal = FreeLibrary(static_cast<HMODULE>(libHandle));
		if( returnVal == 0 )
		{
			DWORD error = GetLastError();
			throw runtime_error("CrossPlatCloseLibrary failed with error: " + to_string(error));
		}
	}
}
