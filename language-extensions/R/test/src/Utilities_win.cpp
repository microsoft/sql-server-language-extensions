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

#include "windows.h"

#include <string>
#include <iostream>

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
	HMODULE hDLL = LoadLibraryA(libPath);
	if (hDLL == NULL)
	{
		DWORD lastError = GetLastError();
		cout << "Failed to load '" << libPath << "' with error" << lastError << "." << endl;
	}

	return hDLL;
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

	HMODULE hDLL = static_cast<HMODULE>(libHandle);
	if (hDLL != nullptr)
	{
		pFuncPtr = GetProcAddress(hDLL, fnName.c_str());
		if (pFuncPtr == NULL)
		{
			DWORD error = GetLastError();
			cout << "Error finding function '" << fnName << "' :"<< error << endl;
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
	void *libHandle = CrossPlatLoadLibrary(libPath.c_str());
	void *pFuncPtr = nullptr;
	if (libHandle != NULL)
	{
		pFuncPtr = CrossPlatGetFunctionFromLibHandle(libHandle, fnName);
		if (pFuncPtr == NULL)
		{
			FreeLibrary(static_cast<HMODULE>(libHandle));
		}
	}

	return pFuncPtr;
}
