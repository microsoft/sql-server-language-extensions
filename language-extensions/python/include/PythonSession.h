//*********************************************************************
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
//*********************************************************************

#pragma once
#include "Column.h"
#include "Common.h"

// Data pertaining to a session
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
		SQLCHAR		  *script,
		SQLULEN		  scriptLength,
		SQLUSMALLINT  inputSchemaColumnsNumber,
		SQLUSMALLINT  parametersNumber,
		SQLCHAR		  *inputDataName,
		SQLUSMALLINT  inputDataNameLength,
		SQLCHAR		  *outputDataName,
		SQLUSMALLINT  outputDataNameLength);

	// Init the input column
	//
	void InitColumn(
		SQLUSMALLINT  ColumnNumber,
		const SQLCHAR *ColumnName,
		SQLSMALLINT	  ColumnNameLength,
		SQLSMALLINT	  DataType,
		SQLULEN		  ColumnSize,
		SQLSMALLINT	  DecimalDigits,
		SQLSMALLINT	  Nullable,
		SQLSMALLINT	  PartitionByNumber,
		SQLSMALLINT	  OrderByNumber);

	// Init the input parameter
	//
	void InitParam(
		SQLUSMALLINT  ParamNumber,
		const SQLCHAR *ParamName,
		SQLSMALLINT	  ParamNameLength,
		SQLSMALLINT	  DataType,
		SQLULEN		  ArgSize,
		SQLSMALLINT	  DecimalDigits,
		SQLPOINTER	  ArgValue,
		SQLINTEGER	  StrLen_or_Ind,
		SQLSMALLINT	  InputOutputType);

	// Execute the workflow for the session
	//
	void ExecuteWorkflow(
		SQLULEN		 RowsNumber,
		SQLPOINTER	 *Data,
		SQLINTEGER	 **StrLen_or_Ind,
		SQLUSMALLINT *OutputSchemaColumnsNumber);

	// Get the metadata for the output column
	//
	void GetResultColumn(
		SQLUSMALLINT ColumnNumber,
		SQLSMALLINT  *DataType,
		SQLULEN		 *ColumnSize,
		SQLSMALLINT  *DecimalDigits,
		SQLSMALLINT  *Nullable);

	// Get the results
	//
	void GetResults(
		SQLULEN	   *RowsNumber,
		SQLPOINTER **Data,
		SQLINTEGER ***StrLen_or_Ind);

	// Get the the output parameter
	//
	void GetOutputParam(
		SQLUSMALLINT ParamNumber,
		SQLPOINTER	 *ParamValue,
		SQLINTEGER	 *StrLen_or_Ind);

	// Cleanup session
	//
	void Cleanup();

private:
	boost::python::object m_mainModule;		// The boost python module; python will run in this object
	boost::python::object m_mainNamespace;	// The boost python namespace; dictionary containing all python variables

	SQLUSMALLINT m_inputSchemaColumnsNumber;
	std::vector<Column> m_inputColumns;

	SQLGUID m_sessionId;					// Session ID

	// Parameters
	//
	SQLUSMALLINT m_parametersNumber;

	// TODO: Work item 13292456: Create a DataSet class to wrap input/output dataset
};
