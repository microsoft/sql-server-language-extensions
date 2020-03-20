//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JavaLibraryUtils_linux.cpp
//
// Purpose:
//  Linux specific utility functions to handle library management
//  for the Java extension.
//
//*********************************************************************
#include "JavaLibraryUtils.h"
#include "JavaExtensionUtils.h"
#include "Logger.h"
#include <sys/types.h>
#include <dirent.h>

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
	DIR *directory = nullptr;
	struct dirent *entry = nullptr;

	directory = opendir(basePath.c_str());
	if (directory != nullptr)
	{
		entry = readdir(directory);
		while (entry != nullptr)
		{
			string fileName = string(entry->d_name);

			if (fileName.compare("..") != 0 && fileName.compare(".") != 0)
			{
				// Add the separator if needed.
				//
				if (!output.empty())
				{
					output += JavaExtensionUtils::GetClassPathSeparator();
				}

				output += basePath + "/" + fileName;
			}

			// Get next entry
			//
			entry = readdir(directory);
		}

		closedir(directory);
	}
	else
	{
		string msg = "Error opening library directory " + basePath;
		LOG_ERROR(msg);
	}
}