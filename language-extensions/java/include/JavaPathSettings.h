//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JavaPathSettings.h
//
// Purpose:
//  Global class to keep language runtime settings.
//
//*********************************************************************
#pragma once

#include "Common.h"

//---------------------------------------------------------------------
// Description:
//	Global class storing the language runtime paths and parameters
//
class JavaPathSettings
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
	static const std::string& GetPrivateLibraryPath() { return m_privateLibraryPath; }

	// Get the public library path sent by SQL Server
	//
	static const std::string& GetPublicLibraryPath() { return m_publicLibraryPath; }

	// Get the extension root folder
	//
	static const std::string& GetRootPath() { return m_languagePath; }

	// Get the language parameters sent by SQL Server
	//
	static const std::string& GetParams() { return m_languageParams; }

private:
	static std::string m_languagePath;
	static std::string m_languageParams;
	static std::string m_privateLibraryPath;
	static std::string m_publicLibraryPath;
};
