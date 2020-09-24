//**************************************************************************************************
// RExtension : A language extension implementing the SQL Server
// external language communication protocol for R.
// Copyright (C) 2020 Microsoft Corporation.
//
// This file is part of RExtension.
//
// RExtension is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RExtension is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RExtension.  If not, see <https://www.gnu.org/licenses/>.
//
// @File: RSession.h
//
// Purpose:
//  Class encapsulating operations performed per R script session
//
//**************************************************************************************************

#pragma once

// Forward Declarations
//
class Column;
class Parameter;

//--------------------------------------------------------------------------------------------------
// Name: RSession
//
// Description:
//  Stores data pertaining to a session.
//
class RSession
{
public:

	// Constructor to initialize the members
	//
	RSession()
		: m_sessionId({ 0, 0, 0, {0} }),
		m_taskId(0),
		m_numTasks(0)
	{}

	// Inits the session
	//
	void Init(
		const SQLGUID  &sessionId,
		SQLUSMALLINT   taskId,
		SQLUSMALLINT   numTasks,
		const SQLCHAR  *script,
		SQLULEN        scriptLength,
		SQLUSMALLINT   inputSchemaColumnsNumber,
		SQLUSMALLINT   parametersNumber,
		const SQLCHAR  *inputDataName,
		SQLUSMALLINT   inputDataNameLength,
		const SQLCHAR  *outputDataName,
		SQLUSMALLINT   outputDataNameLength);

	// Inits the input column
	//
	void InitColumn(
		SQLUSMALLINT   columnNumber,
		const SQLCHAR  *columnName,
		SQLSMALLINT    columnNameLength,
		SQLSMALLINT    dataType,
		SQLULEN        columnSize,
		SQLSMALLINT    decimalDigits,
		SQLSMALLINT    nullable,
		SQLSMALLINT    partitionByNumber,
		SQLSMALLINT    orderByNumber);

	// Inits the input parameter
	//
	void InitParam(
		SQLUSMALLINT   paramNumber,
		const SQLCHAR  *paramName,
		SQLSMALLINT    paramNameLength,
		SQLSMALLINT    dataType,
		SQLULEN        paramSize,
		SQLSMALLINT    decimalDigits,
		SQLPOINTER     paramValue,
		SQLINTEGER     strLen_or_Ind,
		SQLSMALLINT    inputOutputType);

	// Executes the workflow for the session
	//
	void ExecuteWorkflow(
		SQLULEN       rowsNumber,
		SQLPOINTER    *data,
		SQLINTEGER    **strLen_or_Ind,
		SQLUSMALLINT  *outputSchemaColumnsNumber);

	// Gets the metadata for the output column
	//
	void GetResultColumn(
		SQLUSMALLINT columnNumber,
		SQLSMALLINT  *dataType,
		SQLULEN      *columnSize,
		SQLSMALLINT  *decimalDigits,
		SQLSMALLINT  *nullable);

	// Gets the results
	//
	void GetResults(
		SQLULEN      *rowsNumber,
		SQLPOINTER   **data,
		SQLINTEGER   ***strLen_or_Ind);

	// Gets the the output parameter
	//
	void GetOutputParam(
		SQLUSMALLINT paramNumber,
		SQLPOINTER   *paramValue,
		SQLINTEGER   *strLen_or_Ind);

	// Cleans up session
	//
	void Cleanup();

private:

	// Session ID
	//
	SQLGUID m_sessionId {0, 0, 0, {0}};

	// Task ID for this session
	//
	SQLUSMALLINT m_taskId = 0;

	// Number of tasks for this session
	//
	SQLUSMALLINT m_numTasks = 0;

	// The R script.
	//
	std::string m_script;

	// Parameter container.
	//
	RParamContainer m_paramContainer;

	std::vector<SQLSMALLINT> m_partitionByIndexes;
	std::vector<SQLSMALLINT> m_orderByIndexes;

	// InputDataSet
	//
	RInputDataSet m_inputDataSet;

	// OutputDataSet
	//
	ROutputDataSet m_outputDataSet;
};
