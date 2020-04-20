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
#include <unordered_map>
#include <unordered_set>

#include "Logger.h"
#include "RColumn.h"
#include "RDataSet.h"
#include "RTypeUtils.h"

#include "Rcpp.h"

using namespace std;

// A set of supported column data types
//
const unordered_set<SQLSMALLINT> RInputDataSet::m_supportedDataTypes =
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

// Map of function pointers for getting a column information.
//
const ROutputDataSet::GetColumnFnMap ROutputDataSet::m_fnGetColumnMap =
{
	{static_cast<SQLSMALLINT>(SQL_C_BIT),
	 static_cast<fnGetColumn>(
		 &ROutputDataSet::GetColumnFromDataFrame<Rcpp::LogicalVector, SQLCHAR, SQL_C_BIT>)},
	{static_cast<SQLSMALLINT>(SQL_C_SLONG),
	 static_cast<fnGetColumn>(
		 &ROutputDataSet::GetColumnFromDataFrame<Rcpp::IntegerVector, SQLINTEGER, SQL_C_SLONG>)},
	{static_cast<SQLSMALLINT>(SQL_C_DOUBLE),
	 static_cast<fnGetColumn>(
		 &ROutputDataSet::GetColumnFromDataFrame<Rcpp::NumericVector, SQLDOUBLE, SQL_C_DOUBLE>)},
	{static_cast<SQLSMALLINT>(SQL_C_CHAR),
	 static_cast<fnGetColumn>(
		 &ROutputDataSet::GetCharacterColumnFromDataFrame)},
	{static_cast<SQLSMALLINT>(SQL_C_BINARY),
	 static_cast<fnGetColumn>(
		 &ROutputDataSet::GetRawColumnFromDataFrame)}
};

// Map of function pointers for cleaning up output data buffers and null map.
//
const ROutputDataSet::CleanupColumnFnMap ROutputDataSet::m_fnCleanupColumnMap =
{
	{static_cast<SQLSMALLINT>(SQL_C_BIT),
	 static_cast<fnCleanupColumn>(
		 &ROutputDataSet::CleanupColumn<SQLCHAR>)},
	{static_cast<SQLSMALLINT>(SQL_C_SLONG),
	 static_cast<fnCleanupColumn>(
		 &ROutputDataSet::CleanupColumn<SQLINTEGER>)},
	{static_cast<SQLSMALLINT>(SQL_C_DOUBLE),
	 static_cast<fnCleanupColumn>(
		 &ROutputDataSet::CleanupColumn<SQLDOUBLE>)},
	{static_cast<SQLSMALLINT>(SQL_C_CHAR),
	 static_cast<fnCleanupColumn>(
		 &ROutputDataSet::CleanupColumn<SQLCHAR>)},
	{static_cast<SQLSMALLINT>(SQL_C_BINARY),
	 static_cast<fnCleanupColumn>(
		 &ROutputDataSet::CleanupColumn<SQLCHAR>)}
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
// Name: RInputDataSet::Init
//
// Description:
// Call the base Init and create the underlying DataFrame with stringsAsFactors = false.
// This makes sure when character columns are added, they are not converted into factors.
//
void RInputDataSet::Init(
	const SQLCHAR  *dataName,
	SQLUSMALLINT   dataNameLength,
	SQLUSMALLINT   schemaColumnsNumber)
{
	RDataSet::Init(dataName, dataNameLength, schemaColumnsNumber);
	m_dataFrame = Rcpp::DataFrame::create(Rcpp::Named("stringsAsFactors") = false);
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

	if (m_supportedDataTypes.find(dataType) == m_supportedDataTypes.end())
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
	for (SQLUSMALLINT columnNumber = 0; columnNumber < numberOfCols; columnNumber++)
	{
		SQLPOINTER colData = nullptr;

		if (strLen_or_Ind != nullptr)
		{
			m_columnNullMap[columnNumber] = strLen_or_Ind[columnNumber];
		}

		if (data != nullptr)
		{
			colData = data[columnNumber];
		}

		AddColumnToDataFrame(
			columnNumber,
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
	SQLSMALLINT columnNumber,
	SQLULEN     rowsNumber,
	SQLPOINTER  data)
{
	LOG("RInputDataSet::AddColumnToDataFrame");

	if (m_columns[columnNumber] == nullptr)
	{
		throw runtime_error("InitColumn not called for columnNumber " + to_string(columnNumber));
	}

	string name = m_columns[columnNumber].get()->Name();
	SQLSMALLINT dataType = m_columns[columnNumber].get()->DataType();
	SQLINTEGER *strLen_or_Ind = m_columnNullMap[columnNumber];
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

	// Add the "m_name" named variable to embedded R; this gets added as a list.
	//
	(*g_embeddedRPtr)[m_name.c_str()] = m_dataFrame;

	// Cast the list into a DataFrame
	//
	string castToDataFrame = m_name + "<- as.data.frame(" + m_name + ", stringsAsFactors = FALSE)";
	(*g_embeddedRPtr).parseEvalQ(castToDataFrame);
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

//--------------------------------------------------------------------------------------------------
// Name: ROutputDataset::GetColumnsFromDataFrame
//
// Description:
// Gets columns from the DataFrame and stores their data, nullmap and other information.
//
void ROutputDataSet::GetColumnsFromDataFrame()
{
	LOG("ROutputDataset::GetColumnsFromDataFrame");

	SQLUSMALLINT numberOfCols = GetDataFrameColumnsNumber();

	Rcpp::CharacterVector columnNames = m_dataFrame.names();
	for(SQLUSMALLINT columnNumber = 0; columnNumber < numberOfCols; columnNumber++)
	{
		const char *columnName = columnNames[columnNumber];
		SQLSMALLINT dataType = m_columnsDataType[columnNumber];
		SQLULEN columnSize = 0;
		SQLSMALLINT decimalDigits = 0;
		SQLSMALLINT nullable = SQL_NO_NULLS;

		// Look up the GetColumn function
		//
		GetColumnFnMap::const_iterator it = m_fnGetColumnMap.find(dataType);
		if (it == m_fnGetColumnMap.end())
		{
			throw invalid_argument("Unsupported data type "
				+ to_string(dataType) + "in output data for column # " + to_string(columnNumber));
		}

		// Gets the column information, adds data to m_data and nullmap to m_columnNullMap
		//
		(this->*it->second)(
			columnNumber,
			columnSize,
			decimalDigits,
			nullable);

		// Store the column information obtained above in m_columns.
		//
		auto *unsignedColumnName = static_cast<const SQLCHAR*>(
				static_cast<const void*>(columnName));
		m_columns.push_back(
			make_unique<RColumn>(
				unsignedColumnName,
				strlen(columnName),
				dataType,
				columnSize,
				decimalDigits,
				nullable));
	}
}

//--------------------------------------------------------------------------------------------------
// Name: ROutputDataSet::GetColumnFromDataFrame
//
// Description:
// Templatized function to get the column information from the underlying m_dataFrame,
// adds data to m_data and nullmap to m_columnNullMap.
// Templated for integer, numeric and logical R class types.
//
template<class RType, class SQLType, SQLSMALLINT dataType>
void ROutputDataSet::GetColumnFromDataFrame(
	SQLUSMALLINT columnNumber,
	SQLULEN      &columnSize,
	SQLSMALLINT  &decimalDigits,
	SQLSMALLINT  &nullable)
{
	LOG("ROutputDataSet::GetColumnFromDataFrame");

	SQLType *columnData = nullptr;
	SQLINTEGER *nullMap = nullptr;

	if(m_numberOfRows > 0)
	{
		columnData = new SQLType[m_numberOfRows];
		nullMap = new SQLINTEGER[m_numberOfRows];
	}

	decimalDigits = 0;
	nullable = SQL_NO_NULLS;

	RType column = m_dataFrame[columnNumber];
	columnSize = sizeof(SQLType);

	for(SQLULEN index = 0 ; index < m_numberOfRows; index++)
	{
		if (!RType::is_na(column[index]))
		{
			if (dataType != SQL_C_BIT)
			{
				columnData[index] = static_cast<SQLType>(column[index]);
			}
			else
			{
				columnData[index] = column[index] ? '1' : '0';
			}

			nullMap[index] = sizeof(SQLType);
		}
		else
		{
			nullMap[index] = SQL_NULL_DATA;
			nullable = SQL_NULLABLE;
			columnData[index] = 0;
		}
	}

	m_data.push_back(static_cast<SQLPOINTER>(columnData));
	m_columnNullMap.push_back(nullMap);
}

//--------------------------------------------------------------------------------------------------
// Name: ROutputDataSet::GetCharacterColumnFromDataFrame
//
// Description:
// Get character column information from the underlying m_dataFrame,
// adds data to m_data and nullmap to m_columnNullMap.
//
void ROutputDataSet::GetCharacterColumnFromDataFrame(
	SQLUSMALLINT columnNumber,
	SQLULEN      &columnSize,
	SQLSMALLINT  &decimalDigits,
	SQLSMALLINT  &nullable)
{
	LOG("ROutputDataSet::GetCharacterColumnFromDataFrame");

	vector<SQLCHAR> *columnData = nullptr;
	SQLINTEGER *strLenOrNullMap = nullptr;
	if(m_numberOfRows > 0)
	{
		columnData = new vector<SQLCHAR>();
		strLenOrNullMap = new SQLINTEGER[m_numberOfRows];
	}

	decimalDigits = 0;
	nullable = SQL_NO_NULLS;

	Rcpp::CharacterVector column = m_dataFrame[columnNumber];

	// Insert the character column into the columnData vector contiguously.
	//
	SQLULEN maxLen = 0;
	for(SQLULEN index = 0 ; index < m_numberOfRows; index++)
	{
		if (!Rcpp::CharacterVector::is_na(column[index]))
		{
			strLenOrNullMap[index] = strlen(column[index]);

			if (maxLen < static_cast<SQLULEN>(strLenOrNullMap[index]))
			{
				maxLen = strLenOrNullMap[index];
			}

			string stringToCopy(column[index]);
			columnData->insert(columnData->end(), stringToCopy.begin(), stringToCopy.end());
		}
		else
		{
			nullable = SQL_NULLABLE;
			strLenOrNullMap[index] = SQL_NULL_DATA;
		}
	}

	columnSize = maxLen;
	if (m_numberOfRows > 0)
	{
		m_data.push_back(static_cast<SQLPOINTER>(columnData->data()));
	}
	else
	{
		m_data.push_back(nullptr);
	}

	m_columnNullMap.push_back(strLenOrNullMap);
}

//--------------------------------------------------------------------------------------------------
// Name: ROutputDataSet::GetRawColumnFromDataFrame
//
// Description:
// Get raw column information from the underlying m_dataFrame,
// adds data to m_data and nullmap to m_columnNullMap.
//
void ROutputDataSet::GetRawColumnFromDataFrame(
	SQLUSMALLINT columnNumber,
	SQLULEN      &columnSize,
	SQLSMALLINT  &decimalDigits,
	SQLSMALLINT  &nullable)
{
	LOG("ROutputDataSet::GetRawColumnFromDataFrame");

	SQLCHAR *columnData = nullptr;

	decimalDigits = 0;
	nullable = SQL_NO_NULLS;

	Rcpp::RawVector column = m_dataFrame[columnNumber];
	columnSize = column.size();

	if (columnSize > 0)
	{
		// All the bytes returned in the column are returned in a single row.
		//
		columnData = new SQLCHAR[columnSize];
		for(SQLULEN index = 0 ; index < columnSize; index++)
		{
			columnData[index] = column[index];
		}

	}
	else
	{
		nullable = SQL_NULLABLE;
	}

	SQLINTEGER *strLenOrNullMap = nullptr;

	if (m_numberOfRows > 0)
	{
		strLenOrNullMap = new SQLINTEGER[columnSize];

		// If there are rows in the DataFrame, they are all clubbed together in
		// a single row for raw column.
		//
		strLenOrNullMap[0] = columnSize;
	}

	m_data.push_back(static_cast<SQLPOINTER>(columnData));
	m_columnNullMap.push_back(strLenOrNullMap);
}

//--------------------------------------------------------------------------------------------------
// Name: ROutputDataSet::GetColumnsDataType
//
// Description:
// Finds the data type for each column and stores it in the member m_columnsDataType.
// If a binary data type is found when numberOfColumns is > 1, an exception is thrown
// since OutputData having a binary column is not supported when it has more than 1 columns.
//
void ROutputDataSet::GetColumnsDataType()
{
	LOG("ROutputDataSet::GetColumnsDataType");

	SQLUSMALLINT numberOfCols = GetDataFrameColumnsNumber();

	for(SQLUSMALLINT columnNumber = 0; columnNumber < numberOfCols; columnNumber++)
	{
		SQLSMALLINT dataType = GetColumnDataType(columnNumber);

		if (dataType == SQL_C_BINARY && numberOfCols > 1)
		{
			throw invalid_argument("Binary data type for column # "
				+ to_string(columnNumber) + " not supported when "
				" output data has more than 1 column.");
		}

		m_columnsDataType.push_back(dataType);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: ROutputDataSet::GetColumnDataType
//
// Description:
// First, evaluates class(m_name[1,columnNumber+1]) to give the R class.
// Then gets the column data type by looking up the map with the R class as the key.
//
SQLSMALLINT ROutputDataSet::GetColumnDataType(SQLUSMALLINT columnNumber)
{
	LOG("ROutputDataSet::GetColumnDataType");

	// R follows 1-based indices so need to add 1 to columnNumber.
	//
	SQLUSMALLINT columnNumberInR = columnNumber + 1;

	// Construct the script to find class in R.
	//
	string scriptToFindRClass = "class(" +  m_name + "[1," + to_string(columnNumberInR) + "])";

	// Evaluate the script and store the result in a string
	//
	Rcpp::CharacterVector classInR(static_cast<SEXP>(
		(*g_embeddedRPtr).parseEval(scriptToFindRClass)));

	// Look up the map with the key classInR.
	//
	RTypeUtils::RToOdbcTypeMap::const_iterator it =
		RTypeUtils::m_classInRToOdbcTypeMap.find(string(classInR[0]));

	if (it == RTypeUtils::m_classInRToOdbcTypeMap.end())
	{
		throw invalid_argument("Unsupported data type in output data for column # "
			+ to_string(columnNumber) + ".");
	}

	SQLSMALLINT dataType = (*it).second;
	return dataType;
}

//--------------------------------------------------------------------------------------------------
// Name: ROutputDataSet::PopulateNumberOfRows
//
// Description:
// Set the number of rows from the underlying DataFrame.
// If there is a binary column, number of rows is set to 1 even if the underlying DataFrame has more
// rows since all the bytes are returned in a single row.
//
void ROutputDataSet::PopulateNumberOfRows()
{
	LOG("ROutputDataSet::PopulateNumberOfRows");

	vector<SQLSMALLINT>::const_iterator it =
		find(m_columnsDataType.begin(),m_columnsDataType.end(), SQL_C_BINARY);

	if (it == m_columnsDataType.end())
	{
		// No binary column found; set the DataFrame rows as the number of rows for ROutputDataSet.
		//
		m_numberOfRows = GetDataFrameRowsNumber();
	}
	else if(GetDataFrameRowsNumber() > 0)
	{
		// Binary column found; set number of rows of ROutputDatSet = 1
		// only if DataFrame has at least 1 row.
		//
		m_numberOfRows = 1;
	}
}

//--------------------------------------------------------------------------------------------------
// Name: ROutputDataSet::CleanupColumns
//
// Description:
// Looks up the CleanupColumnFnMap to find the the respective CleanupColumn function for every column
// and calls that.
//
void ROutputDataSet::CleanupColumns()
{
	LOG("ROutputDataSet::CleanupColumns");

	for (SQLUSMALLINT columnNumber = 0; columnNumber < m_data.size(); columnNumber++)
	{
		SQLSMALLINT dataType = m_columnsDataType[columnNumber];

		CleanupColumnFnMap::const_iterator it = m_fnCleanupColumnMap.find(dataType);

		if (it == m_fnCleanupColumnMap.end())
		{
			throw invalid_argument("In cleanup, unsupported data type "
				+ to_string(dataType) + " in output data for column # "
				+ to_string(columnNumber));
		}

		(this->*it->second)(columnNumber);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: ROutputDataSet::CleanupColumn
//
// Description:
// For the given columnNumber, cleans up the data buffer used to hold the data
// before being sent to ExtHost. Also cleans up the columnNullMap.
//
template<class SQLType>
void ROutputDataSet::CleanupColumn(SQLUSMALLINT columnNumber)
{
	LOG("ROutputDataSet::CleanupColumn");

	if (m_data[columnNumber] != nullptr)
	{
		delete[] reinterpret_cast<SQLType*>(m_data[columnNumber]);
		m_data[columnNumber] = nullptr;
	}

	if (m_columnNullMap[columnNumber] != nullptr)
	{
		delete[] m_columnNullMap[columnNumber];
		m_columnNullMap[columnNumber] = nullptr;
	}
}
