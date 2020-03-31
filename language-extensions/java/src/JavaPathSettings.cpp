//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JavaPathSettings.cpp
//
// Purpose:
//  Global class to keep language runtime settings.
//
//*********************************************************************
#include "JavaPathSettings.h"

//----------------------------------------------------------------------------
// Name: ExtensionConfig::Init
//
// Description:
//	Initialize the class
//
void
JavaPathSettings::Init(
	const SQLCHAR *languageParams,
	const SQLCHAR *languagePath,
	const SQLCHAR *publicLibraryPath,
	const SQLCHAR *privateLibraryPath)
{
	// nullptrs are mapped to empty strings - has the same effect when
	// the paths are used and avoids an additional flag.
	//
	m_languageParams =
		(languageParams == nullptr) ? "" : reinterpret_cast<const char*>(languageParams);

	m_languagePath =
		(languagePath == nullptr) ? "" : reinterpret_cast<const char*>(languagePath);

	m_publicLibraryPath =
		(publicLibraryPath == nullptr) ? "" : reinterpret_cast<const char*>(publicLibraryPath);

	m_privateLibraryPath =
		(privateLibraryPath == nullptr) ? "" : reinterpret_cast<const char*>(privateLibraryPath);
}

std::string JavaPathSettings::m_languagePath;
std::string JavaPathSettings::m_languageParams;
std::string JavaPathSettings::m_privateLibraryPath;
std::string JavaPathSettings::m_publicLibraryPath;
