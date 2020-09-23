//**************************************************************************************************
// RExtension : A language extension implementing the SQL Server
// external language communication protocol for R.
// Copyright (C) 2020 Microsoft Corporation.
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
//  Global class to keep language runtime path settings for both platforms.
//
//**************************************************************************************************

#include "Common.h"

using namespace std;

string RPathSettings::sm_languagePath;
string RPathSettings::sm_languageParams;
string RPathSettings::sm_privateLibraryPath;
string RPathSettings::sm_publicLibraryPath;
string RPathSettings::sm_RHomePath;

//--------------------------------------------------------------------------------------------------
// Name: RPathSettings::Init
//
// Description:
//  Initialize the class
//
void RPathSettings::Init(
	const SQLCHAR *languageParams,
	const SQLCHAR *languagePath,
	const SQLCHAR *publicLibraryPath,
	const SQLCHAR *privateLibraryPath)
{
	LOG("RPathSettings::Init");

	// nullptrs are mapped to empty strings - has the same effect when
	// the paths are used and avoids an additional flag.
	//
	sm_languageParams =
		(languageParams == nullptr) ? "" : static_cast<const char*>(
			static_cast<const void*>(languageParams));

	sm_languagePath =
		(languagePath == nullptr) ? "" : static_cast<const char*>(
			static_cast<const void*>(languagePath));

	sm_publicLibraryPath =
		(publicLibraryPath == nullptr) ? "" : static_cast<const char*>(
			static_cast<const void*>(publicLibraryPath));

	sm_privateLibraryPath =
		(privateLibraryPath == nullptr) ? "" : static_cast<const char*>(
			static_cast<const void*>(privateLibraryPath));
}
