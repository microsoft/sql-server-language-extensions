//*************************************************************************************************
// RExtension : A language extension implementing the SQL Server
// external language communication protocol for R.
// Copyright (C) 2019 Microsoft Corporation.
//
// This file is part of RExtension.
//
// RExtension is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RExtension is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RExtension.  If not, see <https://www.gnu.org/licenses/>.
//
// @File: RPathSettings.cpp
//
// Purpose:
//  Global class to keep language runtime settings.
//
//*************************************************************************************************
#include "Common.h"

#include <exception>

#include "RPathSettings.h"

using namespace std;

string RPathSettings::m_languagePath;
string RPathSettings::m_languageParams;
string RPathSettings::m_privateLibraryPath;
string RPathSettings::m_publicLibraryPath;
string RPathSettings::m_RHomePath;

//-------------------------------------------------------------------------------------------------
// Name: RPathSettings::Init
//
// Description:
//  Initialize the class
//
void
RPathSettings::Init(
	const SQLCHAR *languageParams,
	const SQLCHAR *languagePath,
	const SQLCHAR *publicLibraryPath,
	const SQLCHAR *privateLibraryPath)
{
	LOG("RPathSettings::Init");

	// nullptrs are mapped to empty strings - has the same effect when
	// the paths are used and avoids an additional flag.
	//
	m_languageParams =
		(languageParams == nullptr) ? "" : static_cast<const char*>(
			static_cast<const void*>(languageParams));

	m_languagePath =
		(languagePath == nullptr) ? "" : static_cast<const char*>(
			static_cast<const void*>(languagePath));

	m_publicLibraryPath =
		(publicLibraryPath == nullptr) ? "" : static_cast<const char*>(
			static_cast<const void*>(publicLibraryPath));

	m_privateLibraryPath =
		(privateLibraryPath == nullptr) ? "" : static_cast<const char*>(
			static_cast<const void*>(privateLibraryPath));
}

//-------------------------------------------------------------------------------------------------
// Name: RPathSettings::CheckAndSetRHome
//
// Description:
//  Checks if R_HOME is set, and sets it to be the language path if not already set.
//  Throws and exception if there is an error setting it.
//
void
RPathSettings::CheckAndSetRHome()
{
	LOG("RPathSettings::CheckAndSetRHome");

	m_RHomePath = Utilities::GetEnvVariable("R_HOME");
	if (m_RHomePath == "")
	{
#ifdef _WIN64
		throw runtime_error("On Windows, R_HOME needs to be defined in CREATE EXTERNAL LANGUAGE.");
#else
		int result = Utilities::SetEnvVariable("R_HOME", m_languagePath);
		if (result != 0)
		{
			throw runtime_error("Error setting R_HOME");
		}
		else
		{
			m_RHomePath = m_languagePath;
			LOG("R_HOME set to be the extensionPath: " + m_RHomePath);
		}
#endif
	}
	else
	{
		LOG("R_HOME is set as: " + m_RHomePath);
	}
}
