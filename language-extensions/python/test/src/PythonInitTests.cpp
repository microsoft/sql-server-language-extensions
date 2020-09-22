//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonInitTests.cpp
//
// Purpose:
//  Test the Python extension initialization and
//  session initialization using the Extension API
//
//*************************************************************************************************
#include "PythonExtensionApiTests.h"

using namespace std;

namespace ExtensionApiTest
{
	// Name: TestInitWithNulls
	//
	// Description:
	//  Test Init() API with all nullptrs.
	//  This tests the extension initialization, using nullptrs because
	//  param and library paths are optional.
	//
	TEST_F(PythonExtensionApiTests, TestInitWithNulls)
	{
		SQLRETURN result = Init(
			nullptr, // Extension Params
			0,       // Extension Params Length
			nullptr, // Extension Path
			0,       // Extension Path Length
			nullptr, // Public Library Path
			0,       // Public Library Path Length
			nullptr, // Private Library Path
			0        // Private Library Path Length
		);

		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Name: TestInitWithValues
	//
	// Description:
	//  Test Init() API with actual values.
	//
	TEST_F(PythonExtensionApiTests, TestInitWithValues)
	{
		SQLCHAR *extensionParams = nullptr;
		SQLULEN extensionParamsLength = 0;
		string extensionPath = "C:/Path/To/ExternalLanguages/1/65554";
		SQLULEN extensionPathLength = extensionPath.length();
		string publicLibraryPath = "C:/Path/To/ExternalLanguages/1/65554/1";
		SQLULEN publicLibraryPathLength = publicLibraryPath.length();
		SQLCHAR *privateLibraryPath = nullptr;
		SQLULEN privateLibraryPathLength = 0;

		SQLCHAR *unsignedExtensionPath = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(extensionPath.c_str())));
		SQLCHAR *unsignedPublicLibraryPath = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(publicLibraryPath.c_str())));

		SQLRETURN result = Init(extensionParams,
			extensionParamsLength,
			unsignedExtensionPath,
			extensionPathLength,
			unsignedPublicLibraryPath,
			publicLibraryPathLength,
			privateLibraryPath,
			privateLibraryPathLength
		);

		EXPECT_EQ(result, SQL_SUCCESS);
	}
}
