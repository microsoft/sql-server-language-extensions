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
// @File: RPathSettings.h
//
// Purpose:
//  Global class to keep language runtime path settings.
//
//**************************************************************************************************

#pragma once

//--------------------------------------------------------------------------------------------------
// Name: RPathSettings
//
// Description:
//  Global class storing the language runtime paths and parameters.
//
class RPathSettings
{
public:

	// Initializes this global class
	//
	static void Init(
		const SQLCHAR *languageParams,
		const SQLCHAR *languagePath,
		const SQLCHAR *publicLibraryPath,
		const SQLCHAR *privateLibraryPath);

	// Gets the private library path sent by SQL Server
	//
	static const std::string& PrivateLibraryPath() {return sm_privateLibraryPath; }

	// Gets the public library path sent by SQL Server
	//
	static const std::string& PublicLibraryPath() {return sm_publicLibraryPath; }

	// Gets the extension root folder
	//
	static const std::string& RootPath() {return sm_languagePath; }

	// Gets the language parameters sent by SQL Server
	//
	static const std::string& Params() {return sm_languageParams; }

	// Checks if R_HOME is set, and sets it to be the language path if not already set.
	//
	static void CheckAndSetRHome();

	// Sets the environment variable TZDIR.
	//
	static void CheckAndSetTZDir();

private:

	// Directory where R language extension library is extracted onto.
	//
	static std::string sm_languagePath;

	// Parameters passed to R language extension
	//
	static std::string sm_languageParams;

	// Private library path
	//
	static std::string sm_privateLibraryPath;

	// Public library path
	//
	static std::string sm_publicLibraryPath;

	// R_HOME path
	//
	static std::string sm_RHomePath;
};
