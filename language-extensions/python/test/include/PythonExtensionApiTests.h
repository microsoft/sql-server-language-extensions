//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonExtensionApiTests.h
//
// Purpose:
//  Define common methods and variables needed to test the Extension API
//
//*************************************************************************************************
#pragma once
#include "Common.h"

namespace ExtensionApiTest
{
	// All the tests in the PythonextensionApiTest suite run one after the other
	//
	class PythonExtensionApiTests : public ::testing::Test
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
		void InitializeSession(SQLUSMALLINT inputSchemaColumnsNumber = 0);

		// Initialize a column
		//
		void InitializeColumn(
			SQLSMALLINT columnNumber,
			std::string columnNameString,
			SQLSMALLINT dataType,
			SQLULEN     columnSize);

		// Set up default, valid variables for use in tests
		//
		void SetupVariables();

		// Call Cleanup on the PythonExtension.
		// Testing if Cleanup is implemented correctly.
		//
		void DoCleanup();

		// Get the python exception out of the boost::python namespace
		// Use only after "error_already_set"
		//
		std::string ParsePythonException();

		// Extract the string from a boost::python object
		//
		std::string ExtractString(PyObject * pObj);

		// Template to test all input parameter data types
		//
		template<class SQLType, SQLSMALLINT dataType>
		void TestParameter(
			SQLType paramValue,
			bool isNull = false);

		// Test a string parameter
		//
		void TestStringParameter(
			const char *paramValue,
			SQLULEN    paramSize,
			bool       isFixedType);

		// Test a binary parameter
		//
		void TestRawParameter(
			const SQLCHAR paramValue[],
			SQLINTEGER    strLenOrInd,
			SQLULEN       paramSize,
			bool          isFixedType);

		// Fill a contiguous array columnData with members from the given columnVector
		// having lengths defined in strLenOrInd, unless it is SQL_NULL_DATA.
		//
		template<class SQLType>
		std::vector<SQLType> GenerateContiguousData(
			std::vector<const SQLType*> columnVector,
			SQLINTEGER                  *strLenOrInd);

		// Template function to Test Execute with default script
		//
		template<class SQLType, SQLSMALLINT dataType, class NullType>
		void TestExecute(
			SQLULEN                  rowsNumber,
			void                     **dataSet,
			SQLINTEGER               **strLen_or_Ind,
			std::vector<std::string> columnNames,
			const NullType           valueForNull = NULL);

		// Template function to compare the given column and data for equality.
		//
		template<class SQLType, SQLSMALLINT dataType, class NullType>
		void CheckColumnEquality(
			SQLULEN             expectedRowsNumber,
			boost::python::dict columnToTest,
			void                *expectedColumn,
			SQLINTEGER          *strLen_or_Ind,
			const NullType      valueForNull);

		// Compare a given boolean column with another for equality
		//
		void CheckBooleanColumnEquality(
			SQLULEN             expectedRowsNumber,
			boost::python::dict columnToTest,
			void                *expectedColumn,
			SQLINTEGER          *strLen_or_Ind);

		// Compare a given string column with another for equality
		//
		void CheckStringColumnEquality(
			SQLULEN             expectedRowsNumber,
			boost::python::dict columnToTest,
			void                *expectedColumn,
			SQLINTEGER          *strLen_or_Ind);

		// Compare a given binary column with another for equality
		//
		void CheckRawColumnEquality(
			SQLULEN             expectedRowsNumber,
			boost::python::dict columnToTest,
			void                *expectedColumn,
			SQLINTEGER          *strLen_or_Ind);

		// Objects declared here can be used by all tests in the test suite.
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

		SQLCHAR *m_inputDataName;
		std::string m_inputDataNameString;
		SQLSMALLINT m_inputDataNameLength;

		SQLCHAR *m_outputDataName;
		std::string m_outputDataNameString;
		SQLSMALLINT m_outputDataNameLength;

		const std::string m_printMessage = "Hello PythonExtension!";

		const double m_doubleNull = NAN;
		const int m_intNull = 0;
		const bool m_boolNull = false;
		const char m_charNull = '0';

		// The boost python module; python will run in this object
		//
		boost::python::object m_mainModule;

		// The boost python namespace; dictionary containing all python variables
		//
		boost::python::object m_mainNamespace;
	};
}
