//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonPathSettings.cpp
//
// Purpose:
//  Global class to keep language runtime settings
//
//*************************************************************************************************

#include "PythonPathSettings.h"
#include "PythonExtensionUtils.h"

//-------------------------------------------------------------------------------------------------
// Name: PythonPathSettings::Init
//
// Description:
//  Initialize the class
//
void
PythonPathSettings::Init(
	const SQLCHAR *languageParams,
	const SQLCHAR *languagePath,
	const SQLCHAR *publicLibraryPath,
	const SQLCHAR *privateLibraryPath)
{
	// nullptrs are mapped to empty strings - has the same effect when
	// the paths are used and avoids an additional flag.
	//
	sm_languageParams =
		(languageParams == nullptr) ? "" : reinterpret_cast<const char *>(languageParams);

	sm_languagePath =
		(languagePath == nullptr) ? "" : reinterpret_cast<const char *>(languagePath);

	sm_publicLibraryPath =
		(publicLibraryPath == nullptr) ? "" : reinterpret_cast<const char *>(publicLibraryPath);

	sm_privateLibraryPath =
		(privateLibraryPath == nullptr) ? "" : reinterpret_cast<const char *>(privateLibraryPath);

	// Remove single slashes and replace with forward slashes in the paths
	//
	sm_languagePath = PythonExtensionUtils::NormalizePathString(sm_languagePath);
	sm_privateLibraryPath = PythonExtensionUtils::NormalizePathString(sm_privateLibraryPath);
	sm_publicLibraryPath = PythonExtensionUtils::NormalizePathString(sm_publicLibraryPath);
}

std::string PythonPathSettings::sm_languageParams;
std::string PythonPathSettings::sm_languagePath;
std::string PythonPathSettings::sm_privateLibraryPath;
std::string PythonPathSettings::sm_publicLibraryPath;
