//******************************************************************************************************
// RExtension : A language extension implementing the SQL Server external language communication protocol.
// Copyright (C) 2019 Microsoft Corporation.

// This file is part of RExtension.

// RExtension is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// RExtension is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with RExtension.  If not, see <https://www.gnu.org/licenses/>.

// @File: RPathSettings.cpp
//
// Purpose:
//  Global class to keep language runtime settings.
//
//******************************************************************************************************
#include "Common.h"

#include <exception>
#include <sqlext.h>

#include "RPathSettings.h"

using namespace std;

//----------------------------------------------------------------------------
// Name: RPathSettings::Init
//
// Description:
//	Initialize the class
//
void
RPathSettings::Init(
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

string RPathSettings::m_languagePath;
string RPathSettings::m_languageParams;
string RPathSettings::m_privateLibraryPath;
string RPathSettings::m_publicLibraryPath;