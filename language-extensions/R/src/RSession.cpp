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

#include "RColumn.h"
#include "RDataSet.h"
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
	SQLUSMALLINT  taskId,
	SQLUSMALLINT  numTasks,
	const SQLCHAR *script,
	SQLULEN       scriptLength,
	SQLUSMALLINT  inputSchemaColumnsNumber,
	SQLUSMALLINT  parametersNumber,
	const SQLCHAR *inputDataName,
	SQLUSMALLINT  inputDataNameLength,
	const SQLCHAR *outputDataName,
	SQLUSMALLINT  outputDataNameLength)
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
	m_script = string(static_cast<const char*>(
		static_cast<const void *>(script)), scriptLength);

	// Initialize the InputDataSet
	//
	m_inputDataSet.Init(inputDataName, inputDataNameLength, inputSchemaColumnsNumber);

	// Initialize the OutputDataSet. It is initialized with 0 columns.
	// Columns will be added after the script is evaluated and as per what the
	// the script execution defines the OutputDataSet to be.
	//
	m_outputDataSet.Init(outputDataName,
		outputDataNameLength,
		0);    // schemaColumnsNumber

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
	SQLUSMALLINT  columnNumber,
	const SQLCHAR *columnName,
	SQLSMALLINT   columnNameLength,
	SQLSMALLINT   dataType,
	SQLULEN       columnSize,
	SQLSMALLINT   decimalDigits,
	SQLSMALLINT   nullable,
	SQLSMALLINT   partitionByNumber,
	SQLSMALLINT   orderByNumber)
{
	LOG("RSession::InitColumn " + to_string(columnNumber));

	m_inputDataSet.InitColumn(
		columnNumber,
		columnName,
		columnNameLength,
		dataType,
		columnSize,
		decimalDigits,
		nullable);
}

//-------------------------------------------------------------------------------------------------
// Name: RSession::InitParam
//
// Description:
//  Initializes the parameter for this session
//
void RSession::InitParam(
	SQLUSMALLINT  paramNumber,
	const SQLCHAR *paramName,
	SQLSMALLINT   paramNameLength,
	SQLSMALLINT   dataType,
	SQLULEN       paramSize,
	SQLSMALLINT   decimalDigits,
	SQLPOINTER    paramValue,
	SQLINTEGER    strLen_or_Ind,
	SQLSMALLINT   inputOutputType)
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

	// Add parameter to the container and embedded R environment.
	//
	m_paramContainer.AddParamToEmbeddedR(
		paramNumber,
		paramName,
		paramNameLength,
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
	SQLULEN      rowsNumber,
	SQLPOINTER   *data,
	SQLINTEGER   **strLen_or_Ind,
	SQLUSMALLINT *outputSchemaColumnsNumber)
{
	LOG("RSession::ExecuteWorkflow");

	// Add columns to the input DataFrame.
	//
	m_inputDataSet.AddColumnsToDataFrame(rowsNumber, data, strLen_or_Ind);

	// Add the DataFrame for InputDataSet to the R environment.
	//
	m_inputDataSet.AddDataFrameToEmbeddedR();

	// Execute the script, any standard output or error is flushed to the console.
	//
	(*g_embeddedRPtr).parseEvalQ(m_script.c_str());

	// In case of streaming clean up the previous stream batch's output buffers
	//
	m_outputDataSet.CleanupColumns();

	// After script evaluation, retrieve the DataFrame for OutputDataSet.
	//
	m_outputDataSet.RetrieveDataFrameFromEmbeddedR();

	// Get the column number from the underlying DataFrame
	// and set it to be the outputSchemaColumnsNumber.
	//
	*outputSchemaColumnsNumber = m_outputDataSet.GetDataFrameColumnsNumber();

	if (*outputSchemaColumnsNumber > 0)
	{
		m_outputDataSet.GetColumnsDataType();
		m_outputDataSet.PopulateNumberOfRows();
		m_outputDataSet.GetColumnsFromDataFrame();
	}
}

//-------------------------------------------------------------------------------------------------
// Name: RSession::GetResultColumn
//
// Description:
//  Returns information about the output column
//
void RSession::GetResultColumn(
	SQLUSMALLINT columnNumber,
	SQLSMALLINT  *dataType,
	SQLULEN      *columnSize,
	SQLSMALLINT  *decimalDigits,
	SQLSMALLINT  *nullable)
{
	LOG("RSession::GetResultColumn for column # " + to_string(columnNumber));

	*dataType = SQL_UNKNOWN_TYPE;
	*columnSize = 0;
	*decimalDigits = 0;
	*nullable = 0;

	if (columnNumber >= m_outputDataSet.GetVectorColumnsNumber())
	{
		throw invalid_argument("Invalid column # " + to_string(columnNumber));
	}

	vector<unique_ptr<RColumn>>& resultColumns = m_outputDataSet.Columns();
	RColumn *resultColumn = resultColumns[columnNumber].get();

	*dataType = resultColumn->DataType();
	*columnSize = resultColumn->Size();
	*decimalDigits = resultColumn->DecimalDigits();
	*nullable = resultColumn->Nullable();
}

//-------------------------------------------------------------------------------------------------
// Name: RSession::GetResults
//
// Description:
//	Returns the output data and the null map retrieved from the user program
//
void RSession::GetResults(
	SQLULEN    *rowsNumber,
	SQLPOINTER **data,
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
	SQLPOINTER   *paramValue,
	SQLINTEGER   *strLen_or_Ind)
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

	m_outputDataSet.CleanupColumns();
}
