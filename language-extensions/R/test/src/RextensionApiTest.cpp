//*************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server
// external language communication protocol.
// Copyright (C) 2019 Microsoft Corporation.
//
// This file is part of RExtension-test.
//
// RExtension-test is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RExtension-test is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RExtension-test.  If not, see <https://www.gnu.org/licenses/>.
//
// @File: RextensionApiTest.cpp
//
// Purpose:
//  Tests the Rextension's implementation of the external language apis
//
//*************************************************************************************************
#ifdef _WIN64
#include <windows.h>
#endif
#include <memory>
#include <string.h>
#include <stdio.h>
#include <sqltypes.h>
#include <sqlext.h>
#include "gtest/gtest.h"

#include "Utilities.h"

using namespace std;

#ifdef _WIN64
	const string RextensionLibName = "libRextension.dll";
#else
	const string RextensionLibName = "libRextension.so.1.0";
#endif // _WIN64

extern int g_argc;
extern const char **g_argv;

namespace
{
	typedef SQLRETURN FN_init(
		SQLCHAR *,
		SQLULEN,
		SQLCHAR *,
		SQLULEN,
		SQLCHAR *,
		SQLULEN,
		SQLCHAR *,
		SQLULEN);

	typedef SQLRETURN FN_cleanup();

	class RextensionApiTest : public ::testing::Test
	{
	protected:

		// Code here will be called immediately after the constructor (right
		// before each test).
		//
		void SetUp() override
		{
			char *RHome = getenv("R_HOME");

			// If RHome is not defined it could be passed as the first argument to the test executable
			// In terms of gtest, the first argument is the name of the test executable
			// and RHome then becomes the second argument.
			//
			if (RHome == nullptr && g_argc >= 2)
			{
				RHome = const_cast<char *>(g_argv[1]);
				putenv(RHome);
			}
		}

		// Code here will be called immediately after each test (right
		// before the destructor).
		//
		void TearDown() override
		{

		}

	};

	// Testing if RExtension is successfully loaded dynamically and
	// that Init and Cleanup are implemented correctly.
	//
	TEST_F(RextensionApiTest, InitCleanup)
	{
		FN_init* init = reinterpret_cast<FN_init*>(
			Utilities::CrossPlatGetFunctionFromLibPath(
				RextensionLibName,
				"Init"));

		int returnVal = SQL_ERROR;
		const string cmdLine = "dummyInputScript --no-save";
		int paramsLength = strlen(cmdLine.c_str());
		unique_ptr<SQLCHAR> extensionParams(new SQLCHAR[paramsLength]);
		memcpy(extensionParams.get(), cmdLine.c_str(), paramsLength);
		if (init != nullptr)
		{
			returnVal = (*init)(
				extensionParams.get(),
				paramsLength,
				nullptr,
				0,
				nullptr,
				0,
				nullptr,
				0);
		}
		EXPECT_EQ(returnVal, SQL_SUCCESS);

		FN_cleanup* cleanup = reinterpret_cast<FN_cleanup*>(
			Utilities::CrossPlatGetFunctionFromLibPath(
				RextensionLibName,
				"Cleanup"));
		returnVal = SQL_ERROR;
		if (cleanup != nullptr)
		{
			returnVal = (*cleanup)();
		}

		EXPECT_EQ(returnVal, SQL_SUCCESS);
	}
}

