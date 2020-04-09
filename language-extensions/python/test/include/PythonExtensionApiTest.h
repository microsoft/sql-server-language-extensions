//*********************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonExtensionApiTest.h
//
// Purpose:
//  Define common methods and variables needed to test the Extension API
//
//*********************************************************************
#pragma once
#include "Common.h"

namespace ExtensionApiTest
{
	// All the tests in the PythonextensionApiTest suite run one after the other
	//
	class PythonExtensionApiTest : public ::testing::Test
	{
	protected:

		// Code here will be called immediately after the constructor (right
		// before each test).
		//
		void SetUp() override;

		// Code here will be called immediately after each test (right
		// before the destructor).
		//
		void TearDown() override;

		// Initialize a valid, default session for later tests
		//
		void InitializeSession();

		// Set up default, valid variables for use in tests
		//
		void SetupVariables();

		// Call Cleanup on the PythonExtension.
		// Testing if Cleanup is implemented correctly.
		//
		void DoCleanup();

		// Template to test all input parameter data types
		//
		template<class SQLType, SQLSMALLINT dataType>
		void TestParameter(
			SQLType paramValue,
			bool isNull = false);

		void TestCharParameter(
			const char * paramValue,
			SQLULEN paramSize,
			bool isFixedType);

		void TestBinaryParameter(
			const SQLCHAR paramValue[],
			SQLINTEGER strLenOrInd,
			SQLULEN paramSize,
			bool isFixedType);

		// Objects declared here can be used by all tests in the test suite
		//
		SQLGUID *m_sessionId;
		SQLUSMALLINT m_taskId;
		SQLUSMALLINT m_numTasks;
		SQLUSMALLINT m_parametersNumber;

		std::string m_paramName;
		SQLSMALLINT m_paramNameLength;

		SQLCHAR *m_script;
		std::string m_scriptString;
		SQLSMALLINT m_scriptLength;

		SQLUSMALLINT m_inputSchemaColumnsNumber;
		SQLCHAR *m_inputDataName;
		std::string m_inputDataNameString;
		SQLSMALLINT m_inputDataNameLength;

		SQLCHAR *m_outputDataName;
		std::string m_outputDataNameString;
		SQLSMALLINT m_outputDataNameLength;

		// The boost python module; python will run in this object
		//
		boost::python::object m_mainModule;

		// The boost python namespace; dictionary containing all python variables
		//
		boost::python::object m_mainNamespace;
	};
}