//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonSession.h
//
// Purpose:
//  Class encapsulating operations performed per session
//
//*************************************************************************************************

#pragma once
#include "Common.h"
#include "PythonColumn.h"
#include "PythonDataSet.h"
#include "PythonParam.h"
#include "PythonParamContainer.h"

// data pertaining to a session
//
class PythonSession
{
public:

	// Init the session
	//
	void Init(
		const SQLGUID *sessionId,
		SQLUSMALLINT  taskId,
		SQLUSMALLINT  numTasks,
		SQLCHAR       *script,
		SQLULEN       scriptLength,
		SQLUSMALLINT  inputSchemaColumnsNumber,
		SQLUSMALLINT  parametersNumber,
		SQLCHAR       *inputDataName,
		SQLUSMALLINT  inputDataNameLength,
		SQLCHAR       *outputDataName,
		SQLUSMALLINT  outputDataNameLength);

	// Init the input column
	//
	void InitColumn(
		SQLUSMALLINT  columnNumber,
		const SQLCHAR *columnName,
		SQLSMALLINT   columnNameLength,
		SQLSMALLINT   dataType,
		SQLULEN       columnSize,
		SQLSMALLINT   decimalDigits,
		SQLSMALLINT   nullable,
		SQLSMALLINT   partitionByNumber,
		SQLSMALLINT   orderByNumber);

	// Init the input parameter
	//
	void InitParam(
		SQLUSMALLINT  paramNumber,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   dataType,
		SQLULEN       paramSize,
		SQLSMALLINT   decimalDigits,
		SQLPOINTER    paramValue,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   InputOutputType);

	// Execute the workflow for the session
	//
	void ExecuteWorkflow(
		SQLULEN      rowsNumber,
		SQLPOINTER   *data,
		SQLINTEGER   **strLen_or_Ind,
		SQLUSMALLINT *outputSchemaColumnsNumber);

	// Get the metadata for the output column
	//
	void GetResultColumn(
		SQLUSMALLINT columnNumber,
		SQLSMALLINT  *dataType,
		SQLULEN      *columnSize,
		SQLSMALLINT  *decimalDigits,
		SQLSMALLINT  *nullable);

	// Get the results
	//
	void GetResults(
		SQLULEN    *rowsNumber,
		SQLPOINTER **data,
		SQLINTEGER ***strLen_or_Ind);

	// Get the the output parameter
	//
	void GetOutputParam(
		SQLUSMALLINT paramNumber,
		SQLPOINTER   *paramValue,
		SQLINTEGER   *strLen_or_Ind);

	// Cleanup session
	//
	void Cleanup();

private:
	boost::python::object m_mainModule; // The boost python module which contains the namespace.

	// The underlying boost::python namespace, which contains all the python variables.
	// We execute any python scripts on this namespace.
	//
	boost::python::object m_mainNamespace;

	SQLGUID m_sessionId{ 0, 0, 0, {0} };
	SQLUSMALLINT m_taskId = 0;
	SQLUSMALLINT m_numTasks = 0;

	std::string m_script;
	SQLULEN m_scriptLength;

	PythonInputDataSet m_inputDataSet;
	PythonOutputDataSet m_outputDataSet;

	// Parameters
	//
	PythonParamContainer m_paramContainer;

};
