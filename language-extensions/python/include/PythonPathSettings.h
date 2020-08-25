//**************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonPathSettings.h
//
// Purpose:
//  Global class to keep language runtime settings
//
//**************************************************************************************************

#pragma once
#include "Common.h"

//---------------------------------------------------------------------
// Description:
//	Global class storing the language runtime paths and parameters
//
class PythonPathSettings
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
	static const std::string& PrivateLibraryPath() { return m_privateLibraryPath; }

	// Get the public library path sent by SQL Server
	//
	static const std::string& PublicLibraryPath() { return m_publicLibraryPath; }

	// Get the extension root folder
	//
	static const std::string& RootPath() { return m_languagePath; }

	// Get the language parameters sent by SQL Server
	//
	static const std::string& Params() { return m_languageParams; }

private:
	static std::string m_languagePath;
	static std::string m_languageParams;
	static std::string m_privateLibraryPath;
	static std::string m_publicLibraryPath;
};
