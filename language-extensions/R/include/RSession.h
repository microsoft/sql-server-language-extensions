//******************************************************************************************************
// RExtension : A language extension implementing the SQL Server external language communication protocol.
// Copyright (C) 2019 Microsoft Corporation.

// This file is part of RExtension.

// RExtension is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// RExtension is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with RExtension.  If not, see <https://www.gnu.org/licenses/>.

// @File: RSession.h
//
// Purpose:
//  Class encapsulating operations performed per R script session
//
//******************************************************************************************************
#pragma once

// Forward Declarations
//
class Column;
class Parameter;

// Data pertaining to a session
//
class RSession
{
public:
	// Constructor to initialize the members
	//
	RSession()
		:	m_sessionId({ 0, 0, 0, {0} }),
		m_taskId(0),
		m_numTasks(0),
		m_inputSchemaColumnsNumber(0),
		m_numberOfOutputRows(0)
	{}

	// Init the session
	//
	void Init(
		const SQLGUID &sessionId,
		SQLUSMALLINT   taskId,
		SQLUSMALLINT   numTasks,
		const SQLCHAR *script,
		SQLULEN        scriptLength,
		SQLUSMALLINT   inputSchemaColumnsNumber,
		SQLUSMALLINT   parametersNumber,
		const SQLCHAR *inputDataName,
		SQLUSMALLINT   inputDataNameLength,
		const SQLCHAR *outputDataName,
		SQLUSMALLINT   outputDataNameLength);

	// Init the input column
	//
	void InitColumn(
		SQLUSMALLINT   columnNumber,
		const SQLCHAR *columnName,
		SQLSMALLINT    columnNameLength,
		SQLSMALLINT    dataType,
		SQLULEN        columnSize,
		SQLSMALLINT    decimalDigits,
		SQLSMALLINT    nullable,
		SQLSMALLINT    partitionByNumber,
		SQLSMALLINT    orderByNumber);

	// Init the input parameter
	//
	void InitParam(
		SQLUSMALLINT   paramNumber,
		const SQLCHAR *paramName,
		SQLSMALLINT    paramNameLength,
		SQLSMALLINT    dataType,
		SQLULEN        argSize,
		SQLSMALLINT    decimalDigits,
		SQLPOINTER     argValue,
		SQLINTEGER     strLen_or_Ind,
		SQLSMALLINT    inputOutputType);

	// Execute the workflow for the session
	//
	void ExecuteWorkflow(
		SQLULEN       rowsNumber,
		SQLPOINTER   *data,
		SQLINTEGER  **strLen_or_Ind,
		SQLUSMALLINT *outputSchemaColumnsNumber);

	// Get the metadata for the output column
	//
	void GetResultColumn(
		SQLUSMALLINT columnNumber,
		SQLSMALLINT *dataType,
		SQLULEN     *columnSize,
		SQLSMALLINT *decimalDigits,
		SQLSMALLINT *nullable);

	// Get the results
	//
	void GetResults(
		SQLULEN      *rowsNumber,
		SQLPOINTER  **data,
		SQLINTEGER ***strLen_or_Ind);

	// Get the the output parameter
	//
	void GetOutputParam(
		SQLUSMALLINT paramNumber,
		SQLPOINTER  *paramValue,
		SQLINTEGER  *strLen_or_Ind);

	// Cleanup session
	//
	void Cleanup();

private:

	SQLGUID m_sessionId;                  // Session ID
	SQLUSMALLINT m_taskId;                // Task ID for this session
	SQLUSMALLINT m_numTasks;              // Number of tasks for this session

	std::string m_script;
	SQLULEN m_scriptLength;               // Script Length = this counts the null terminator as well

	std::string m_inputDataSetName;       // Input DataSet class name for the executor class
	SQLULEN m_inputDataSetNameLength;     // Input DataSet Name Length = this counts the null terminator as well

	std::string m_outputDataSetName;      // Output Dataset class name for the executor class
	SQLULEN m_outputDataSetNameLength;    // Output DataSet Name Length = this counts the null terminator as well

	// Input data information (schema, null map, etc.)
	//
	SQLUSMALLINT m_inputSchemaColumnsNumber;
	std::vector<Column*> m_inputColumns;

	std::vector<SQLSMALLINT> m_partitionByIndexes;
	std::vector<SQLSMALLINT> m_orderByIndexes;

	// Parameters
	//
	SQLUSMALLINT m_parametersNumber;

	// Output data information (schema, null map, etc.)
	//
	SQLULEN m_numberOfOutputRows;
	SQLUSMALLINT m_outputSchemaColumnsNumber;
	std::vector<Column*> m_outputColumns;
	std::vector<SQLINTEGER*> m_outputNullMap;
	std::vector<SQLPOINTER> m_outputData;
};
