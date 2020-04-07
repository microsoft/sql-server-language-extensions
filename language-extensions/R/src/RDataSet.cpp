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
// @File: RDataset.cpp
//
// Purpose:
// Class handling loading and retrieving data from an R Dataframe.
//
//*************************************************************************************************

#include "Common.h"
#include <sqlext.h>

#include "Logger.h"
#include "RColumn.h"
#include "RDataSet.h"
#include "RTypeUtils.h"

#include "Rcpp.h"

using namespace std;

unordered_set<SQLSMALLINT> RDataSet::m_supportedDataTypes =
{
	SQL_C_SLONG,    // INT
	SQL_C_BIT,      // BIT
	SQL_C_FLOAT,    // REAL
	SQL_C_DOUBLE,   // FLOAT(53)
	SQL_C_SBIGINT,  // BIGINT
	SQL_C_UTINYINT, // TINYINT
	SQL_C_SSHORT,   // SMALLINT
	SQL_C_CHAR      // CHAR(n), VARCHAR(n), VARCHAR(max)
};

//-------------------------------------------------------------------------------------------------
// Name: RDataSet::Init
//
// Description:
// Initialize the DataSet with name and number of columns.
//
void RDataSet::Init(
	const SQLCHAR  *dataName,
	SQLUSMALLINT   dataNameLength,
	SQLUSMALLINT   schemaColumnsNumber)
{
	LOG("RDataSet::Init");

	if (dataName == nullptr)
	{
		throw invalid_argument("Invalid DataSet name, it cannot be nullptr.");
	}

	const char *name = static_cast<const char*>(
			static_cast<const void*>(dataName));

	// dataNameLength does not include the null terminator.
	//
#if defined(_DEBUG)
	if (static_cast<size_t>(dataNameLength) != strlen(name))
	{
		throw invalid_argument("Invalid DataSet name length, it doesn't match string length.");
	}
#endif

	m_name = string(name, dataNameLength);

	// Set the size of the columns vector to the given schema columns number.
	//
	m_columns.resize(schemaColumnsNumber);
	m_columnNullMap.resize(schemaColumnsNumber);
}

//-------------------------------------------------------------------------------------------------
// Name: RInputDataSet::InitColumn
//
// Description:
// Initializes each RColumn of the member vector m_columns.
//
void RInputDataSet::InitColumn(
	SQLUSMALLINT   columnNumber,
	const SQLCHAR  *columnName,
	SQLSMALLINT    columnNameLength,
	SQLSMALLINT    dataType,
	SQLULEN        columnSize,
	SQLSMALLINT    decimalDigits,
	SQLSMALLINT    nullable)
{
	LOG("RDataSet::InitColumn " + to_string(columnNumber));

	if (columnName == nullptr)
	{
		throw invalid_argument("Invalid input column name supplied");
	}
	else if (columnNumber >= GetVectorColumnsNumber())
	{
		throw invalid_argument("Invalid input column id supplied: " + to_string(columnNumber));
	}

	if (RDataSet::m_supportedDataTypes.find(dataType) == RDataSet::m_supportedDataTypes.end())
	{
		throw invalid_argument("Unsupported data type " + to_string(dataType) + " encountered for "
			"column id " + to_string(columnNumber) + " in input data.");
	}

	if (m_columns[columnNumber] != nullptr)
	{
		throw invalid_argument("Input column " + to_string(columnNumber) + " was already created,"
			" and cannot be reinitialized.");
	}

	// columnNumber is the column index - i.e. 0 based
	//
	m_columns[columnNumber] = make_unique<RColumn>(
		columnName,
		columnNameLength,
		dataType,
		columnSize,
		decimalDigits,
		nullable);
}

//-------------------------------------------------------------------------------------------------
// Name: RInputDataSet::AddColumnsToDataFrame
//
// Description:
// Add columns to the underlying R DataFrame with the given rowsNumber and data.
//
void RInputDataSet::AddColumnsToDataFrame(
	SQLULEN      rowsNumber,
	SQLPOINTER   *data,
	SQLINTEGER   **strLen_or_Ind)
{
	LOG("RInputDataSet::AddColumnsToDataFrame");

	SQLUSMALLINT numberOfCols = GetVectorColumnsNumber();
	for (SQLUSMALLINT columnIndex = 0; columnIndex < numberOfCols; columnIndex++)
	{
		SQLPOINTER colData = nullptr;

		if (strLen_or_Ind != nullptr)
		{
			m_columnNullMap[columnIndex] = strLen_or_Ind[columnIndex];
		}

		if (data != nullptr)
		{
			colData = data[columnIndex];
		}

		AddColumnToDataFrame(
			columnIndex,
			rowsNumber,
			colData);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: RInputDataSet::AddColumnToDataFrame
//
// Description:
// Adds a single column of values into the R DataFrame
//
void RInputDataSet::AddColumnToDataFrame(
	SQLSMALLINT columnIndex,
	SQLULEN     rowsNumber,
	SQLPOINTER  data)
{
	LOG("RInputDataSet::AddColumnToDataFrame");

	if (m_columns[columnIndex] == nullptr)
	{
		throw runtime_error("InitColumn not called for columnNumber " + to_string(columnIndex));
	}

	string name = m_columns[columnIndex].get()->Name();
	SQLSMALLINT dataType = m_columns[columnIndex].get()->DataType();
	SQLINTEGER *strLen_or_Ind = m_columnNullMap[columnIndex];
	switch (dataType)
	{
		case SQL_C_SLONG:
		{
			m_dataFrame[name.c_str()] =
				RTypeUtils::CreateVector<SQLINTEGER, Rcpp::IntegerVector, int>(
					rowsNumber,
					data,
					strLen_or_Ind,
					NA_INTEGER);
			break;
		}
		case SQL_C_BIT:
		{
			m_dataFrame[name.c_str()] =
				RTypeUtils::CreateLogicalVector(
					rowsNumber,
					data,
					strLen_or_Ind);
			break;
		}
		case SQL_C_FLOAT:
		{
			m_dataFrame[name.c_str()] =
				RTypeUtils::CreateVector<SQLREAL, Rcpp::NumericVector, double>(
					rowsNumber,
					data,
					strLen_or_Ind,
					NA_REAL);
			break;
		}
		case SQL_C_DOUBLE:
		{
			m_dataFrame[name.c_str()] =
				RTypeUtils::CreateVector<SQLDOUBLE, Rcpp::NumericVector, double>(
					rowsNumber,
					data,
					strLen_or_Ind,
					NA_REAL);
			break;
		}
		case SQL_C_SBIGINT:
		{
			m_dataFrame[name.c_str()] =
				RTypeUtils::CreateVector<SQLBIGINT, Rcpp::NumericVector, double>(
					rowsNumber,
					data,
					strLen_or_Ind,
					NA_REAL);
			break;
		}
		case SQL_C_UTINYINT:
		{
			m_dataFrame[name.c_str()] =
				RTypeUtils::CreateVector<SQLCHAR, Rcpp::IntegerVector, int>(
					rowsNumber,
					data,
					strLen_or_Ind,
					NA_INTEGER);
			break;
		}
		case SQL_C_SSHORT:
		{
			m_dataFrame[name.c_str()] =
				RTypeUtils::CreateVector<SQLSMALLINT, Rcpp::IntegerVector, int>(
					rowsNumber,
					data,
					strLen_or_Ind,
					NA_INTEGER);
			break;
		}
		case SQL_C_CHAR:
		{
			m_dataFrame[name.c_str()] =
				RTypeUtils::CreateCharacterVector(
					rowsNumber,
					data,
					strLen_or_Ind);
			break;
		}
		case SQL_C_BINARY:
		case SQL_C_WCHAR:
		case SQL_C_GUID:
		case SQL_C_TYPE_DATE:
		case SQL_C_NUMERIC:
		case SQL_C_TYPE_TIMESTAMP:
		default:
			throw invalid_argument("Unsupported input column type");
	}
}

//--------------------------------------------------------------------------------------------------
// Name: RInputDataSet::AddDataFrameToEmbeddedR
//
// Description:
// Add the underlying R DataFrame to the embedded R environment.
//
void RInputDataSet::AddDataFrameToEmbeddedR()
{
	LOG("RInputDataSet::AddDataFrameToEmbeddedR");

	// Add the "m_name" named DataFrame to embedded R
	//
	(*g_embeddedRPtr)[m_name.c_str()] = m_dataFrame;
}

//--------------------------------------------------------------------------------------------------
// Name: ROutputDataSet::RetrieveDataFrameFromEmbeddedR
//
// Description:
// Retrieves the DataFrame with m_name from embedded R if it exists.
//
void ROutputDataSet::RetrieveDataFrameFromEmbeddedR()
{
	LOG("ROutputDataset::RetrieveDataFrameFromEmbeddedR");

	Rcpp::Environment globalEnv = Rcpp::Environment::global_env();

	if (globalEnv.exists(m_name.c_str()))
	{
		// Get a binding to the DataFrame created in the R environment.
		//
		m_dataFrame = (*g_embeddedRPtr)[m_name.c_str()];
	}
}
