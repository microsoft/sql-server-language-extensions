//*********************************************************************
//                Copyright (C) Microsoft Corporation.
//
// @File: ExtensionApiTest.cpp
//
// Purpose:
//  Test the Python extension using the Extension API
//
//*********************************************************************
#ifdef _WIN64
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>

#include "gtest/gtest.h"
#include "sqlexternallanguage.h"

namespace
{
	using namespace std;

	class ExtensionApiTest : public ::testing::Test
	{
	protected:

		// Code here will be called immediately after the constructor (right
		// before each test).
		//
		void SetUp() override
		{
			setupVariables();
		}

		// Name: setupVariables
		//
		// Description:
		// Set up default, valid variables for use in tests
		//
		void setupVariables()
		{
			sessionId = new SQLGUID();
			taskId = 0;
			numTasks = 1;
			inputSchemaColumnsNumber = 1;
			parametersNumber = 1;

			string scriptString = "print('Hello')";
			script = reinterpret_cast<SQLCHAR *>(const_cast<char *>(scriptString.c_str()));
			scriptLength = scriptString.length() + 1; // null terminator

			string inputDataNameString = "InputDataSet";
			inputDataName = reinterpret_cast<SQLCHAR *>(const_cast<char *>(inputDataNameString.c_str()));
			inputDataNameLength = inputDataNameString.length() + 1; // null terminator

			string outputDataNameString = "OutputDataSet";
			outputDataName = reinterpret_cast<SQLCHAR *>(const_cast<char *>(outputDataNameString.c_str()));
			outputDataNameLength = outputDataNameString.length() + 1; // null terminator
		}

		// Name: initializeSession
		//
		// Description:
		// Initialize a valid, default session for later tests
		//
		void initializeSession()
		{
			SQLRETURN result = SQL_SUCCESS;

			result = Init(nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0);
			EXPECT_EQ(result, SQL_SUCCESS);

			result = InitSession(
				*sessionId,
				taskId,
				numTasks,
				script,
				scriptLength,
				inputSchemaColumnsNumber,
				parametersNumber,
				inputDataName,
				inputDataNameLength,
				outputDataName,
				outputDataNameLength
			);
			EXPECT_EQ(result, SQL_SUCCESS);
		}

		// Objects declared here can be used by all tests in the test suite
		//
		SQLGUID			*sessionId;
		SQLUSMALLINT	taskId;
		SQLUSMALLINT	numTasks;
		SQLUSMALLINT	inputSchemaColumnsNumber;
		SQLUSMALLINT	parametersNumber;

		SQLCHAR			*script;
		SQLSMALLINT		scriptLength;
		SQLCHAR			*inputDataName;
		SQLSMALLINT		inputDataNameLength;
		SQLCHAR			*outputDataName;
		SQLSMALLINT		outputDataNameLength;
	};

	// Test Init() API with all nullptrs.
	// This tests the extension initialization, using nullptrs because
	// param and library paths are optional.
	// We may need to make this fail later when we implement and
	// require external params/libraries
	//
	TEST_F(ExtensionApiTest, InitWithNullsTest)
	{
		SQLRETURN result = Init(nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0);
		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Test Init() API with actual values.
	//
	TEST_F(ExtensionApiTest, InitTest)
	{
		SQLCHAR *ExtensionParams = nullptr;
		SQLULEN ExtensionParamsLength = 0;
		string ExtensionPath = "C:/Path/To/ExternalLanguages/1/65554";
		SQLULEN ExtensionPathLength = ExtensionPath.length() + 1; // null terminator
		string PublicLibraryPath = "C:/Path/To/ExternalLanguages/1/65554/1";
		SQLULEN PublicLibraryPathLength = PublicLibraryPath.length() + 1; // null terminator
		SQLCHAR *PrivateLibraryPath = nullptr;
		SQLULEN PrivateLibraryPathLength = 0;

		SQLRETURN result = Init(ExtensionParams,
			ExtensionParamsLength,
			reinterpret_cast<SQLCHAR *>(const_cast<char *>(ExtensionPath.c_str())),
			ExtensionPathLength,
			reinterpret_cast<SQLCHAR *>(const_cast<char *>(PublicLibraryPath.c_str())),
			PublicLibraryPathLength,
			PrivateLibraryPath,
			PrivateLibraryPathLength
		);
		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Test InitSession() API with valid values
	//
	TEST_F(ExtensionApiTest, InitSessionTest)
	{
		Init(nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0);
		SQLRETURN result = InitSession(
			*sessionId,
			taskId,
			numTasks,
			script,
			scriptLength,
			inputSchemaColumnsNumber,
			parametersNumber,
			inputDataName,
			inputDataNameLength,
			outputDataName,
			outputDataNameLength
		);
		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Negative test
	// Test InitColumn() API with null column name
	//
	TEST_F(ExtensionApiTest, InitInvalidColumnTest)
	{
		initializeSession();

		SQLRETURN result = InitColumn(
			*sessionId,
			taskId,
			0,
			nullptr, // NULL name
			0,
			SQL_C_SLONG,
			sizeof(int),
			0,
			1,
			-1,
			-1
		);
		EXPECT_EQ(result, SQL_ERROR);
	}

	// Negative test
	// Test InitColumn() API with bad column numbers (null or too big)
	//
	TEST_F(ExtensionApiTest, InitInvalidColumnNumberTest)
	{
		initializeSession();

		SQLCHAR * columnName = reinterpret_cast<SQLCHAR *>("Hello");
		SQLSMALLINT columnNameLength = 6;
		SQLRETURN result = InitColumn(
			*sessionId,
			taskId,
			-1, // negative column number
			columnName,
			columnNameLength,
			SQL_C_SLONG,
			sizeof(int),
			0,
			1,
			-1,
			-1
		);
		EXPECT_EQ(result, SQL_ERROR);

		result = InitColumn(
			*sessionId,
			taskId,
			inputSchemaColumnsNumber + 1, // column number greater than inputSchemaColumnsNumber
			columnName,
			columnNameLength,
			SQL_C_SLONG,
			sizeof(int),
			0,
			1,
			-1,
			-1
		);
		EXPECT_EQ(result, SQL_ERROR);
	}

	// Test InitColumn() API with valid values
	//
	TEST_F(ExtensionApiTest, InitColumnTest)
	{
		initializeSession();

		SQLCHAR * columnName = reinterpret_cast<SQLCHAR *>("Hello");
		SQLSMALLINT columnNameLength = 6;
		SQLRETURN result = InitColumn(
			*sessionId,
			taskId,
			0,
			columnName,
			columnNameLength,
			SQL_C_SLONG,
			sizeof(int),
			0,
			1,
			-1,
			-1
		);
		EXPECT_EQ(result, SQL_SUCCESS);
	}
}
