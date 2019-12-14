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

// @File: RPathSettings.h
//
// Purpose:
//  Global class to keep language runtime settings.
//
//******************************************************************************************************
#pragma once

using namespace std;

//---------------------------------------------------------------------
// Description:
//	Global class storing the language runtime paths and parameters
//
class RPathSettings
{
public:
	// Initialize this global class
	//
	static void Init(
		const SQLCHAR *languageParams,
		const SQLCHAR *languagePath,
		const SQLCHAR *publicLibraryPath,
		const SQLCHAR *privateLibraryPath);

	// Get the private library path sent by SQL Server
	//
	static const string& GetPrivateLibraryPath() {return m_privateLibraryPath; }

	// Get the public library path sent by SQL Server
	//
	static const string& GetPublicLibraryPath() {return m_publicLibraryPath; }

	// Get the extension root folder
	//
	static const string& GetRootPath() {return m_languagePath; }

	// Get the language parameters sent by SQL Server
	//
	static const string& GetParams() {return m_languageParams; }

private:
	static string m_languagePath;
	static string m_languageParams;
	static string m_privateLibraryPath;
	static string m_publicLibraryPath;
};
