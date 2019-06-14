//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: JavaPathSettings.cpp
// @Owner: ivpop
//
// Purpose:
//	 Global class to keep language runtime settings.
//
//*********************************************************************
#ifdef _WIN64
#include <windows.h>
#endif
#include <exception>
#include <string>
#ifndef _WIN64
#include <sal_def.h>
#include <xplat_sal.h>
#endif
#include <sqltypes.h>
#include <sqlext.h>
#include "JavaPathSettings.h"

using namespace std;

//----------------------------------------------------------------------------
// Name: ExtensionConfig::Init
//
// Description:
//	Initialize the class
//
void
JavaPathSettings::Init(
	_In_ const SQLCHAR *languageParams,
	_In_ const SQLCHAR *languagePath,
	_In_ const SQLCHAR *publicLibraryPath,
	_In_ const SQLCHAR *privateLibraryPath)
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

string JavaPathSettings::m_languagePath;
string JavaPathSettings::m_languageParams;
string JavaPathSettings::m_privateLibraryPath;
string JavaPathSettings::m_publicLibraryPath;