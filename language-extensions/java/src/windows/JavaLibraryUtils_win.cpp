//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JavaLibraryUtils_win.cpp
//
// Purpose:
//  Windows specific utility functions to handle library management
//  for the Java extension.
//
//*********************************************************************
#include "JavaLibraryUtils.h"
#include "JavaExtensionUtils.h"

using namespace std;

//----------------------------------------------------------------------------
// Name: JavaLibraryUtils::AppendFileNames
//
// Description:
//	Finds all files under the base directory and appends them to the output.
//
// Notes:
//	This function will silently fail if an error occurs, this is expected
//	because the query should attempt to continue.
//
// Returns:
//  none
//
void JavaLibraryUtils::FindAppendFileNames(const string &basePath, string &output)
{
	// String to indicate we should find all files under this directory
	//
	string wildCard = basePath + "\\*";

	HRESULT hr = S_OK;
	WIN32_FIND_DATAA ffa;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	// Find the first file in the directory.
	//
	if (SUCCEEDED(hr))
	{
		hFind = FindFirstFileA(wildCard.c_str(), &ffa);
		if (INVALID_HANDLE_VALUE == hFind)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
	}

	// Loop through each file in the folder which represents a library, and delete it.
	//
	if (SUCCEEDED(hr))
	{
		do
		{
			// Skip over the entry for the current directory
			//
			if (ffa.cFileName[0] == '.')
			{
				continue;
			}

			// Add the separator if needed.
			//
			if (!output.empty())
			{
				output += JavaExtensionUtils::GetClassPathSeparator();
			}

			output += basePath + "\\" + string(ffa.cFileName);
		} while (FindNextFileA(hFind, &ffa) != 0);

		FindClose(hFind);
	}
}
