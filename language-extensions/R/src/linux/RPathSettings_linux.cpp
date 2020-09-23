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
// @File: RPathSettings_linux.cpp
//
// Purpose:
//  Global class to keep language runtime path settings for linux.
//
//**************************************************************************************************

#include "Common.h"

using namespace std;

//--------------------------------------------------------------------------------------------------
// Name: RPathSettings::CheckAndSetRHome
//
// Description:
//  Checks if R_HOME is set, and sets it to be the language path if not already set.
//  Throws and exception if there is an error setting it.
//
void RPathSettings::CheckAndSetRHome()
{
	LOG("RPathSettings::CheckAndSetRHome");

	sm_RHomePath =
		Utilities::GetEnvVariable(
			"R_HOME", // envVarName
			// No need to log error on linux if R_HOME is not defined, since we can set it below.
			//
			false);   // logError

	if (sm_RHomePath == "")
	{
		int result = Utilities::SetEnvVariable("R_HOME", sm_languagePath);
		if (result != 0)
		{
			throw runtime_error("Error setting R_HOME");
		}
		else
		{
			sm_RHomePath = sm_languagePath;
			LOG("R_HOME set to be the extensionPath: " + sm_RHomePath);
		}
	}
	else
	{
		LOG("R_HOME is set as: " + sm_RHomePath);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: RPathSettings::CheckAndSetTZDir
//
// Description:
//  Its a no-op on linux since TZDIR is not useful on linux. The time zones are always looked up
//  from fixed path: /usr/share/zoneinfo
//
void RPathSettings::CheckAndSetTZDir()
{
}
