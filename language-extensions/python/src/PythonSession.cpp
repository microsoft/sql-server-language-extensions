//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonSession.cpp
//
// Purpose:
//  Class encapsulating operations performed per session
//
//*************************************************************************************************

#include "Logger.h"
#include "PythonExtensionUtils.h"
#include "PythonNamespace.h"
#include "PythonPathSettings.h"
#include "PythonSession.h"

using namespace std;
namespace bp = boost::python;
namespace np = boost::python::numpy;

//-------------------------------------------------------------------------------------------------
// Name: PythonSession::Init
//
// Description:
//  Initializes the Python session, storing all the information passed in.
//
void PythonSession::Init(
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
	SQLUSMALLINT  outputDataNameLength)
{
	LOG("PythonSession::Init");

	m_mainNamespace = PythonNamespace::MainNamespace();

	// Initialize the script
	//
	if (script == nullptr)
	{
		throw invalid_argument("Invalid script, the script value cannot be NULL");
	}

	// Initialize and store the user script
	//
	m_script = string(reinterpret_cast<const char*>(script), scriptLength);
	m_scriptLength = scriptLength;

	// Initialize the parameters container.
	//
	m_paramContainer.Init(parametersNumber);

	// Initialize the InputDataSet
	//
	m_inputDataSet.Init(inputDataName, inputDataNameLength, inputSchemaColumnsNumber, m_mainNamespace);

	// Initialize the OutputDataSet
	//
	m_outputDataSet.Init(outputDataName, outputDataNameLength, 0, m_mainNamespace);
}

//-------------------------------------------------------------------------------------------------
// Name: PythonSession::Init
//
// Description:
//  Initializes the input column for this session
//
void PythonSession::InitColumn(
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
	LOG("PythonSession::InitColumn #" + to_string(columnNumber));

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
// Name: PythonSession::InitParam
//
// Description:
//  Initializes an input parameter for this session
//
void PythonSession::InitParam(
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
	LOG("PythonSession::InitParam #" + to_string(paramNumber));

	if (paramName == nullptr)
	{
		throw invalid_argument("Invalid input parameter name supplied");
	}
	else if (paramNumber >= m_paramContainer.GetSize())
	{
		throw invalid_argument("Invalid input param id supplied: " + to_string(paramNumber));
	}

	// Add parameter to the container and boost::python nameSpace.
	//
	m_paramContainer.AddParamToNamespace(
		m_mainNamespace,
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
// Name: PythonSession::ExecuteWorkflow
//
// Description:
//  Execute the workflow for the session
//
void PythonSession::ExecuteWorkflow(
	SQLULEN      rowsNumber,
	SQLPOINTER   *data,
	SQLINTEGER   **strLen_or_Ind,
	SQLUSMALLINT *outputSchemaColumnsNumber)
{
	LOG("PythonSession::ExecuteWorkflow");

	*outputSchemaColumnsNumber = 0;

	// Add columns to the input DataFrame.
	//
	m_inputDataSet.AddColumnsToDictionary(rowsNumber, data, strLen_or_Ind);

	// Add the dictionary for InputDataSet to the python namespace and convert to a DataFrame.
	//
	m_inputDataSet.AddDictionaryToNamespace();

	// Initialize a dictionary for OutputDataSet to the python namespace .
	//
	m_outputDataSet.InitializeDataFrameInNamespace();

	// Scripts to redirect stdout and stderr to variables to extract afterwards
	//
	string redirectPyOut = "import sys; from io import StringIO\n"
		"_temp_out_ = StringIO(); _temp_err_ = StringIO()\n"
		"sys.stdout = _temp_out_; sys.stderr = _temp_err_\n"
		"_original_stdout_ = sys.stdout; _original_stderr_ = sys.stderr";

	string resetPyOut = "sys.stdout = _original_stdout_\n"
		"sys.stderr = _original_stderr_\n"
		"_temp_out_ = _temp_out_.getvalue()\n"
		"_temp_err_ = _temp_err_.getvalue()";

	// Execute script and capture output
	//
	bp::exec(redirectPyOut.c_str(), m_mainNamespace);
	bp::exec(m_script.c_str(), m_mainNamespace);
	bp::exec(resetPyOut.c_str(), m_mainNamespace);

	string pyStdOut = bp::extract<string>(m_mainNamespace["_temp_out_"]);
	string pyStdErr = bp::extract<string>(m_mainNamespace["_temp_err_"]);

	cout << pyStdOut << endl;
	cerr << pyStdErr << endl;

	// In case of streaming clean up the previous stream batch's output buffers
	//
	m_outputDataSet.CleanupColumns();

	// Get the column number from the underlying DataFrame
	// and set it to be the outputSchemaColumnsNumber.
	//
	*outputSchemaColumnsNumber = m_outputDataSet.GetDataFrameColumnsNumber();

	if (*outputSchemaColumnsNumber > 0)
	{
		m_outputDataSet.PopulateColumnsDataType();
		m_outputDataSet.PopulateNumberOfRows();
		m_outputDataSet.RetrieveColumnsFromDataFrame();
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonSession::GetResultColumn
//
// Description:
//  Returns metadata information about the output column
//
void PythonSession::GetResultColumn(
	SQLUSMALLINT columnNumber,
	SQLSMALLINT  *dataType,
	SQLULEN      *columnSize,
	SQLSMALLINT  *decimalDigits,
	SQLSMALLINT  *nullable)
{
	LOG("PythonSession::GetResultColumn for column #" + to_string(columnNumber));

	*dataType = SQL_UNKNOWN_TYPE;
	*columnSize = 0;
	*decimalDigits = 0;
	*nullable = 0;

	if (columnNumber >= m_outputDataSet.GetVectorColumnsNumber())
	{
		throw invalid_argument("Invalid column #" + to_string(columnNumber)
			+ " provided to GetResultColumn().");
	}

	const vector<unique_ptr<PythonColumn>>& resultColumns = m_outputDataSet.Columns();
	PythonColumn *resultColumn = resultColumns[columnNumber].get();

	if(resultColumn != nullptr)
	{
		*dataType = resultColumn->DataType();
		*columnSize = resultColumn->Size();
		*decimalDigits = resultColumn->DecimalDigits();
		*nullable = resultColumn->Nullable();
	}
	else
	{
		throw runtime_error("ResultColumn #" + to_string(columnNumber) +
			" is not initialized for the output dataset");
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonSession::GetResults
//
// Description:
//  Returns the output data and the null map retrieved from the user program
//
void PythonSession::GetResults(
	SQLULEN    *rowsNumber,
	SQLPOINTER **data,
	SQLINTEGER ***strLen_or_Ind)
{
	LOG("PythonSession::GetResults");

	if (rowsNumber != nullptr && data != nullptr && strLen_or_Ind != nullptr)
	{
		*rowsNumber = m_outputDataSet.RowsNumber();
		*data = m_outputDataSet.GetData();
		*strLen_or_Ind = m_outputDataSet.GetColumnNullMap();
	}
	else
	{
		throw runtime_error("Invalid parameters provided to GetResults()");
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonSession::GetOutputParam
//
// Description:
//  Returns the data and size of the output parameter
//
void PythonSession::GetOutputParam(
	SQLUSMALLINT paramNumber,
	SQLPOINTER   *paramValue,
	SQLINTEGER   *strLen_or_Ind)
{
	LOG("PythonSession::GetOutputParam - initializing output parameter #"
		+ to_string(paramNumber));

	if (paramValue == nullptr || strLen_or_Ind == nullptr)
	{
		throw invalid_argument("Null arguments supplied to GetOutputParam().");
	}

	if (paramNumber < m_paramContainer.GetSize())
	{
		m_paramContainer.GetParamValueAndStrLenInd(
			m_mainNamespace,
			paramNumber,
			paramValue,
			strLen_or_Ind);
	}
	else
	{
		throw invalid_argument("Invalid output parameter id supplied to GetOutputParam(): " +
			to_string(paramNumber));
	}
}

//-------------------------------------------------------------------------------------------------
// Name: PythonSession::Cleanup()
//
// Description:
//  Cleans up the python session
//
void PythonSession::Cleanup()
{
	LOG("PythonSession::Cleanup");

	m_inputDataSet.Cleanup();

	m_outputDataSet.CleanupColumns();
	m_outputDataSet.Cleanup();
}
