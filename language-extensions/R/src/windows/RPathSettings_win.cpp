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
// @File: RPathSettings_win.cpp
//
// Purpose:
//  Global class to keep language runtime path settings for windows.
//
//*************************************************************************************************

#include "Common.h"

#include <exception>

#include "RPathSettings.h"

using namespace std;

//-------------------------------------------------------------------------------------------------
// Name: RPathSettings::CheckAndSetRHome
//
// Description:
//  Checks if R_HOME is set and throws an exception if its not set.
//
void RPathSettings::CheckAndSetRHome()
{
	LOG("RPathSettings::CheckAndSetRHome");

	sm_RHomePath = Utilities::GetEnvVariable("R_HOME");
	if (sm_RHomePath == "")
	{
		throw runtime_error("On Windows, R_HOME needs to be defined in CREATE EXTERNAL LANGUAGE.");
	}
	else
	{
		LOG("R_HOME is set as: " + sm_RHomePath);
	}
}

//-------------------------------------------------------------------------------------------------
// Name: RPathSettings::SetTZDir
//
// Description:
//  Check if environment variable TZDIR is set.
//  If it is not set, set sm_RHomePath first and then set TZDIR to R_HOME\share\zoneinfo path
//  so that time zone information is available when calling OlsonNames() in R.
//
void RPathSettings::CheckAndSetTZDir()
{
	LOG("RPathSettings::CheckAndSetTZDir");

	string tzDir = Utilities::GetEnvVariable("TZDIR");

	if (tzDir == "")
	{
		if (sm_RHomePath == "")
		{
			CheckAndSetRHome();
		}

		string valueForTZDir = sm_RHomePath + "\\share\\zoneinfo";
		Utilities::SetEnvVariable("TZDIR", valueForTZDir);
	}
	else
	{
		LOG("TZDIR is set as: " + tzDir);
	}
}
