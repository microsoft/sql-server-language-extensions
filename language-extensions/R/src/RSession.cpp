//*************************************************************************************************
// RExtension : A language extension implementing the SQL Server
// external language communication protocol for R.
// Copyright (C) 2019 Microsoft Corporation.
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
// @File: RSession.cpp
//
// Purpose:
//  Class encapsulating operations performed per R script
//
//*************************************************************************************************
#include "Common.h"

#include <memory>
#include <stdexcept>

#include "Column.h"
#include "Logger.h"
#include "RParam.h"
#include "RParamContainer.h"
#include "RSession.h"
#include "RTypeUtils.h"

using namespace std;

//-------------------------------------------------------------------------------------------------
// Name: RSession::Init
//
// Description:
//  Initializes the R session, by storing all the information passed in.
//
void RSession::Init(
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
	SQLUSMALLINT   outputDataNameLength)
{
	LOG("RSession::Init");

	m_sessionId = sessionId;
	m_taskId = taskId;
	m_numTasks = numTasks;

	// Initialize the script
	//
	if (script == nullptr)
	{
		throw invalid_argument("Invalid Script, the Script value cannot be nullptr");
	}

	// scriptLength does not include the null terminator.
	//
	m_script = string(reinterpret_cast<const char*>(script), scriptLength);

	// Initialize the InputDataSet
	//
	if (inputDataName == nullptr)
	{
		throw invalid_argument("Invalid InputDataName, the InputDataName value cannot be nullptr");
	}

	// inputDataNameLength does not include the null terminator.
	//
	m_inputDataSetName = string(reinterpret_cast<const char*>(inputDataName),
							inputDataNameLength);

	// Initialize the OutputDataSet
	//
	if (outputDataName == nullptr)
	{
		throw invalid_argument("Invalid OutputDataName, the OutputDataName value cannot be nullptr");
	}

	// outputDataNameLength does not include the null terminator.
	//
	m_outputDataSetName = string(reinterpret_cast<const char*>(outputDataName),
							outputDataNameLength);

	// Initialize the rest of the parameters like storing the schema
	//
	m_inputSchemaColumnsNumber = inputSchemaColumnsNumber;

	// Initialize the parameters container.
	//
	m_paramContainer.Init(parametersNumber);
}

//-------------------------------------------------------------------------------------------------
// Name: RSession::Init
//
// Description:
//  Initializes the input column for this session
//
void RSession::InitColumn(
	SQLUSMALLINT   columnNumber,
	const SQLCHAR *columnName,
	SQLSMALLINT    columnNameLength,
	SQLSMALLINT    dataType,
	SQLULEN        columnSize,
	SQLSMALLINT    decimalDigits,
	SQLSMALLINT    nullable,
	SQLSMALLINT    partitionByNumber,
	SQLSMALLINT    orderByNumber)
{
	LOG("RSession::InitColumn " + to_string(columnNumber));

	if (columnName == nullptr)
	{
		throw invalid_argument("Invalid input column name supplied");
	}
	else if (columnNumber >= m_inputSchemaColumnsNumber)
	{
		throw invalid_argument("Invalid input column id supplied: " + to_string(columnNumber));
	}

	// Store the information for this column
	//
	string name(reinterpret_cast<const char*>(columnName), columnNameLength);

	m_inputColumns.push_back(make_unique<Column>(name, dataType, columnSize, nullable, decimalDigits));
}

//-------------------------------------------------------------------------------------------------
// Name: RSession::InitParam
//
// Description:
//  Initializes the parameter for this session
//
void RSession::InitParam(
	SQLUSMALLINT   paramNumber,
	const SQLCHAR *paramName,
	SQLSMALLINT    paramNameLength,
	SQLSMALLINT    dataType,
	SQLULEN        paramSize,
	SQLSMALLINT    decimalDigits,
	SQLPOINTER     paramValue,
	SQLINTEGER     strLen_or_Ind,
	SQLSMALLINT    inputOutputType)
{
	LOG("RSession::InitParam");

	if (paramName == nullptr)
	{
		throw invalid_argument("Invalid input parameter name supplied");
	}
	else if (paramNumber >= m_paramContainer.GetSize())
	{
		throw invalid_argument("Invalid input param id supplied: " + to_string(paramNumber));
	}

	// +1 points to the next character after @ in front of the parameter name.
	// paramNameLength includes @ so do a -1 to exclude it.
	//
	string name(reinterpret_cast<const char*>((paramName + 1)), paramNameLength - 1);

	LOG("RSession::InitParam: Initializing parameter " + name);

	// Add parameter to the container and embedded R environment.
	//
	m_paramContainer.AddParamToEmbeddedR(
		paramNumber,
		name,
		dataType,
		paramSize,
		decimalDigits,
		paramValue,
		strLen_or_Ind,
		inputOutputType);
}

//-------------------------------------------------------------------------------------------------
// Name: RSession::ExecuteWorkflow
//
// Description:
//  Execute the workflow for the session
//
void RSession::ExecuteWorkflow(
	SQLULEN       rowsNumber,
	SQLPOINTER   *data,
	SQLINTEGER  **strLen_or_Ind,
	SQLUSMALLINT *outputSchemaColumnsNumber)
{

}

//-------------------------------------------------------------------------------------------------
// Name: RSession::GetResultColumn
//
// Description:
//  Returns information about the output column
//
void RSession::GetResultColumn(
	SQLUSMALLINT  columnNumber,
	SQLSMALLINT  *dataType,
	SQLULEN      *columnSize,
	SQLSMALLINT  *decimalDigits,
	SQLSMALLINT  *nullable)
{

}

//-------------------------------------------------------------------------------------------------
// Name: RSession::GetResults
//
// Description:
//	Returns the output data and the null map retrieved from the user program
//
void RSession::GetResults(
	SQLULEN      *rowsNumber,
	SQLPOINTER  **data,
	SQLINTEGER ***strLen_or_Ind)
{

}

//-------------------------------------------------------------------------------------------------
// Name: RSession::GetOutputParam
//
// Description:
//  Returns the data and size of the output parameter
//
void RSession::GetOutputParam(
	SQLUSMALLINT paramNumber,
	SQLPOINTER  *paramValue,
	SQLINTEGER  *strLen_or_Ind)
{

}

//-------------------------------------------------------------------------------------------------
// Name: RSession::Cleanup()
//
// Description:
//	Cleans up the R session
//
void RSession::Cleanup()
{
	LOG("RSession::Cleanup");
}
