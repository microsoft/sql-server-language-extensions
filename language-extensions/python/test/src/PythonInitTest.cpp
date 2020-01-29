//*********************************************************************
//                Copyright (C) Microsoft Corporation.
//
// @File: PythonInitTest.cpp
//
// Purpose:
//  Test the Python extension initialization and
//  session initialization using the Extension API
//
//*********************************************************************
#include "PythonExtensionApiTest.h"

using namespace std;

namespace ExtensionApiTest
{
	// Test Init() API with all nullptrs.
	// This tests the extension initialization, using nullptrs because
	// param and library paths are optional.
	// We may need to make this fail later when we implement and
	// require external params/libraries
	//
	TEST_F(PythonExtensionApiTest, InitWithNullsTest)
	{
		SQLRETURN result = Init(
			nullptr, // Extension Params
			0,		 // Extension Params Length
			nullptr, // Extension Path
			0,		 // Extension Path Length
			nullptr, // Public Library Path
			0,		 // Public Library Path Length
			nullptr, // Private Library Path
			0		 // Private Library Path Length
		);
		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Test Init() API with actual values.
	//
	TEST_F(PythonExtensionApiTest, InitTest)
	{
		SQLCHAR *extensionParams = nullptr;
		SQLULEN extensionParamsLength = 0;
		string extensionPath = "C:/Path/To/ExternalLanguages/1/65554";
		SQLULEN extensionPathLength = extensionPath.length() + 1; // null terminator
		string publicLibraryPath = "C:/Path/To/ExternalLanguages/1/65554/1";
		SQLULEN publicLibraryPathLength = publicLibraryPath.length() + 1; // null terminator
		SQLCHAR *privateLibraryPath = nullptr;
		SQLULEN privateLibraryPathLength = 0;

		SQLRETURN result = Init(extensionParams,
			extensionParamsLength,
			reinterpret_cast<SQLCHAR *>(const_cast<char *>(extensionPath.c_str())),
			extensionPathLength,
			reinterpret_cast<SQLCHAR *>(const_cast<char *>(publicLibraryPath.c_str())),
			publicLibraryPathLength,
			privateLibraryPath,
			privateLibraryPathLength
		);
		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Test InitSession() API with valid values
	//
	TEST_F(PythonExtensionApiTest, InitSessionTest)
	{
		Init(nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0);
		SQLRETURN result = InitSession(
			*m_sessionId,
			m_taskId,
			m_numTasks,
			m_script,
			m_scriptLength,
			m_inputSchemaColumnsNumber,
			m_parametersNumber,
			m_inputDataName,
			m_inputDataNameLength,
			m_outputDataName,
			m_outputDataNameLength
		);
		EXPECT_EQ(result, SQL_SUCCESS);
	}
}
