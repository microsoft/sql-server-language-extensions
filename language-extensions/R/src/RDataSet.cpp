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
//  Class handling loading and retrieving data from an R Dataframe.
//
//*************************************************************************************************

#include "Common.h"
#include <limits>
#include <unordered_map>
#include <unordered_set>

#include "RColumn.h"
#include "RDataSet.h"
#include "RTypeUtils.h"

#include "Rcpp.h"

using namespace std;

//-------------------------------------------------------------------------------------------------
// Function map - maps a ODBC C data type to the function for adding a column
//
const RInputDataSet::AddColumnFnMap RInputDataSet::sm_FnAddColumnMap =
{
	{static_cast<SQLSMALLINT>(SQL_C_SLONG),                           // INT
		static_cast<fnAddColumn>(&RInputDataSet::AddColumnToDataFrame
		<SQLINTEGER, Rcpp::IntegerVector, int, SQL_C_SLONG>)},
	{static_cast<SQLSMALLINT>(SQL_C_SBIGINT),                         // BIGINT
		static_cast<fnAddColumn>(&RInputDataSet::AddColumnToDataFrame
		<SQLBIGINT, Rcpp::NumericVector, double, SQL_C_SBIGINT>)},
	{static_cast<SQLSMALLINT>(SQL_C_FLOAT),                           // REAL
		static_cast<fnAddColumn>(&RInputDataSet::AddColumnToDataFrame
		<SQLREAL, Rcpp::NumericVector, double, SQL_C_FLOAT>)},
	{static_cast<SQLSMALLINT>(SQL_C_DOUBLE),                          // FLOAT (53)
		static_cast<fnAddColumn>(&RInputDataSet::AddColumnToDataFrame
		<SQLDOUBLE, Rcpp::NumericVector, double, SQL_C_DOUBLE>)},
	{static_cast<SQLSMALLINT>(SQL_C_SSHORT),                          // SMALLINT
		static_cast<fnAddColumn>(&RInputDataSet::AddColumnToDataFrame
		<SQLSMALLINT, Rcpp::IntegerVector, int, SQL_C_SSHORT>)},
	{static_cast<SQLSMALLINT>(SQL_C_UTINYINT),                       // TINYINT
		static_cast<fnAddColumn>(&RInputDataSet::AddColumnToDataFrame
		<SQLCHAR, Rcpp::IntegerVector, int, SQL_C_UTINYINT>)},
	{static_cast<SQLSMALLINT>(SQL_C_BIT),                            // BIT
		static_cast<fnAddColumn>(&RInputDataSet::AddColumnToDataFrame
		<SQLCHAR, Rcpp::LogicalVector, int, SQL_C_BIT>)},
	{static_cast<SQLSMALLINT>(SQL_C_CHAR),                           // CHAR(n), VARCHAR(n), VARCHAR(max)
		static_cast<fnAddColumn>(&RInputDataSet::AddCharacterColumnToDataFrame)},
};

// Map of function pointers for getting a column information.
//
const ROutputDataSet::GetColumnFnMap ROutputDataSet::sm_FnGetColumnMap =
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
const ROutputDataSet::CleanupColumnFnMap ROutputDataSet::sm_FnCleanupColumnMap =
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
//  Initialize the DataSet with name and number of columns.
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
// Name: RDataSet::Cleanup
//
// Description:
//  Cleanup the DataFrame from the global R environment.
//
void RDataSet::Cleanup()
{
	LOG("RDataSet::Cleanup");

	Rcpp::Environment globalEnv = Rcpp::Environment::global_env();

	if (m_name.length() > 0 && globalEnv.exists(m_name.c_str()))
	{
		string cleanupScript = "rm(" + m_name + ");";

		// Execute the cleanup script to remove the DataFrame from the global R environment.
		//
		(*g_embeddedRPtr).parseEvalQ(cleanupScript.c_str());
	}
}

//-------------------------------------------------------------------------------------------------
// Name: RInputDataSet::Init
//
// Description:
//  Call the base Init and create the underlying DataFrame with stringsAsFactors = false.
//  This makes sure when character columns are added, they are not converted into factors.
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
//  Initializes each RColumn of the member vector m_columns.
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

	if (sm_FnAddColumnMap.find(dataType) == sm_FnAddColumnMap.end())
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
//  Add columns to the underlying R DataFrame with the given rowsNumber and data.
//
void RInputDataSet::AddColumnsToDataFrame(
	SQLULEN      rowsNumber,
	SQLPOINTER   *data,
	SQLINTEGER   **strLen_or_Ind)
{
	LOG("RInputDataSet::AddColumnsToDataFrame");

	SQLUSMALLINT numberOfCols = GetVectorColumnsNumber();
	for (SQLUSMALLINT columnNumber = 0; columnNumber < numberOfCols; ++columnNumber)
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

		SQLSMALLINT dataType = m_columns[columnNumber].get()->DataType();
		AddColumnFnMap::const_iterator it = sm_FnAddColumnMap.find(dataType);

		if (it == sm_FnAddColumnMap.end())
		{
			throw runtime_error("Unsupported input column type encountered when adding column #"
				+ to_string(columnNumber));
		}

		(this->*it->second)(
			columnNumber,
			rowsNumber,
			colData);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: RInputDataSet::AddColumnToDataFrame
//
// Description:
//  Adds a single column of values into the R DataFrame
//
template<class SQLType, class RType, class NAType, SQLSMALLINT DataType>
void RInputDataSet::AddColumnToDataFrame(
	SQLSMALLINT columnNumber,
	SQLULEN     rowsNumber,
	SQLPOINTER  data)
{
	LOG("RInputDataSet::AddColumnToDataFrame");

	if (m_columns[columnNumber] == nullptr)
	{
		throw runtime_error("InitColumn not called for column #" + to_string(columnNumber));
	}

	string name = m_columns[columnNumber].get()->Name();
	SQLINTEGER *strLen_or_Ind = m_columnNullMap[columnNumber];

	m_dataFrame[name.c_str()] = RTypeUtils::CreateVector<SQLType, RType, NAType, DataType>(
		rowsNumber,
		data,
		strLen_or_Ind);
}

//--------------------------------------------------------------------------------------------------
// Name: RInputDataSet::AddCharacterColumnToDataFrame
//
// Description:
//  Adds a single column of character values into the R DataFrame.
//
void RInputDataSet::AddCharacterColumnToDataFrame(
	SQLSMALLINT columnNumber,
	SQLULEN     rowsNumber,
	SQLPOINTER  data)
{
	LOG("RInputDataSet::AddCharacterColumnToDataFrame");

	if (m_columns[columnNumber] == nullptr)
	{
		throw runtime_error("InitColumn not called for column #" + to_string(columnNumber));
	}

	string name = m_columns[columnNumber].get()->Name();
	SQLINTEGER *strLen_or_Ind = m_columnNullMap[columnNumber];

	m_dataFrame[name.c_str()] = RTypeUtils::CreateCharacterVector<char>(
		rowsNumber,
		data,
		strLen_or_Ind);
}

//--------------------------------------------------------------------------------------------------
// Name: RInputDataSet::AddDataFrameToEmbeddedR
//
// Description:
//  Add the underlying R DataFrame to the embedded R environment.
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
//  Retrieves the DataFrame with m_name from embedded R if it exists.
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
//  Gets columns from the DataFrame and stores their data, nullmap and other information.
//
void ROutputDataSet::GetColumnsFromDataFrame()
{
	LOG("ROutputDataset::GetColumnsFromDataFrame");

	SQLUSMALLINT numberOfCols = GetDataFrameColumnsNumber();

	Rcpp::CharacterVector columnNames = m_dataFrame.names();
	for(SQLUSMALLINT columnNumber = 0; columnNumber < numberOfCols; ++columnNumber)
	{
		const char *columnName = columnNames[columnNumber];
		SQLSMALLINT dataType = m_columnsDataType[columnNumber];
		SQLULEN columnSize = 0;
		SQLSMALLINT decimalDigits = 0;
		SQLSMALLINT nullable = SQL_NO_NULLS;

		// Look up the GetColumn function
		//
		GetColumnFnMap::const_iterator it = sm_FnGetColumnMap.find(dataType);
		if (it == sm_FnGetColumnMap.end())
		{
			throw invalid_argument("Unsupported data type "
				+ to_string(dataType) + "in output data for column #" + to_string(columnNumber));
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
//  Templatized function to get the column information from the underlying m_dataFrame,
//  adds data to m_data and nullmap to m_columnNullMap.
//  Templated for integer, numeric and logical R class types.
//
template<class RType, class SQLType, SQLSMALLINT DataType>
void ROutputDataSet::GetColumnFromDataFrame(
	SQLUSMALLINT columnNumber,
	SQLULEN      &columnSize,
	SQLSMALLINT  &decimalDigits,
	SQLSMALLINT  &nullable)
{
	LOG("ROutputDataSet::GetColumnFromDataFrame");

	decimalDigits = 0;
	nullable = SQL_NO_NULLS;
	columnSize = sizeof(SQLType);

	vector<SQLType> *columnData = nullptr;
	SQLINTEGER *strLenOrInd = nullptr;

	if(m_rowsNumber > 0)
	{
		columnData = new vector<SQLType>();
		strLenOrInd = new SQLINTEGER[m_rowsNumber];

		RType column = m_dataFrame[columnNumber];
		RTypeUtils::FillDataFromRVector<SQLType, RType, DataType>(
			m_rowsNumber,
			column,
			columnData,
			strLenOrInd,
			nullable);
		m_data.push_back(static_cast<SQLPOINTER>(columnData->data()));
	}
	else
	{
		m_data.push_back(nullptr);
	}

	m_columnNullMap.push_back(strLenOrInd);
}

//--------------------------------------------------------------------------------------------------
// Name: ROutputDataSet::GetCharacterColumnFromDataFrame
//
// Description:
//  Get character column information from the underlying m_dataFrame,
//  adds data to m_data and nullmap to m_columnNullMap.
//
void ROutputDataSet::GetCharacterColumnFromDataFrame(
	SQLUSMALLINT columnNumber,
	SQLULEN      &columnSize,
	SQLSMALLINT  &decimalDigits,
	SQLSMALLINT  &nullable)
{
	LOG("ROutputDataSet::GetCharacterColumnFromDataFrame");

	decimalDigits = 0;
	nullable = SQL_NO_NULLS;

	vector<SQLCHAR> *columnData = nullptr;
	SQLINTEGER *strLenOrInd = nullptr;
	SQLULEN maxLen = 0;

	if(m_rowsNumber > 0)
	{
		columnData = new vector<SQLCHAR>();
		strLenOrInd = new SQLINTEGER[m_rowsNumber];

		Rcpp::CharacterVector column = m_dataFrame[columnNumber];
		RTypeUtils::FillDataFromCharacterVector(
			m_rowsNumber,
			column,
			numeric_limits<SQLULEN>::max(),
			columnData,
			strLenOrInd,
			nullable,
			maxLen);

		m_data.push_back(static_cast<SQLPOINTER>(columnData->data()));
	}
	else
	{
		m_data.push_back(nullptr);
	}

	columnSize = maxLen;
	m_columnNullMap.push_back(strLenOrInd);
}

//--------------------------------------------------------------------------------------------------
// Name: ROutputDataSet::GetRawColumnFromDataFrame
//
// Description:
//  Get raw column information from the underlying m_dataFrame,
//  adds data to m_data and nullmap to m_columnNullMap.
//
void ROutputDataSet::GetRawColumnFromDataFrame(
	SQLUSMALLINT columnNumber,
	SQLULEN      &columnSize,
	SQLSMALLINT  &decimalDigits,
	SQLSMALLINT  &nullable)
{
	LOG("ROutputDataSet::GetRawColumnFromDataFrame");

	decimalDigits = 0;
	nullable = SQL_NO_NULLS;

	vector<SQLCHAR> *columnData = nullptr;
	SQLINTEGER *strLenOrInd = nullptr;

	Rcpp::RawVector column = m_dataFrame[columnNumber];
	columnSize = column.size();

	if (m_rowsNumber > 0)
	{
		// If there are rows in the DataFrame, they are all grouped together in
		// a single row for raw column i.e. m_rowsNumber = 1.
		// And if m_rowsNumber > 0, it means columnSize > 0
		//
		columnData = new vector<SQLCHAR>();
		strLenOrInd = new SQLINTEGER[m_rowsNumber];

		RTypeUtils::FillDataFromRawVector(
			column,
			numeric_limits<SQLULEN>::max(),
			columnData,
			strLenOrInd);

		m_data.push_back(static_cast<SQLPOINTER>(columnData->data()));
	}
	else
	{
		m_data.push_back(nullptr);
		nullable = SQL_NULLABLE;
	}

	m_columnNullMap.push_back(strLenOrInd);
}

//--------------------------------------------------------------------------------------------------
// Name: ROutputDataSet::GetColumnsDataType
//
// Description:
//  Finds the data type for each column and stores it in the member m_columnsDataType.
//  If a binary data type is found when numberOfColumns is > 1, an exception is thrown
//  since OutputData having a binary column is not supported when it has more than 1 columns.
//
void ROutputDataSet::GetColumnsDataType()
{
	LOG("ROutputDataSet::GetColumnsDataType");

	SQLUSMALLINT numberOfCols = GetDataFrameColumnsNumber();

	for(SQLUSMALLINT columnNumber = 0; columnNumber < numberOfCols; ++columnNumber)
	{
		SQLSMALLINT dataType = GetColumnDataType(columnNumber);

		if (dataType == SQL_C_BINARY && numberOfCols > 1)
		{
			throw invalid_argument("Binary data type for column #"
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
//  First, evaluates class(m_name[1,columnNumber+1]) to give the R class.
//  Then gets the column data type by looking up the map with the R class as the key.
//
// Returns:
//  The ODBC C data type of the column.
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
		throw invalid_argument("Unsupported data type in output data for column #"
			+ to_string(columnNumber) + ".");
	}

	SQLSMALLINT dataType = it->second;
	return dataType;
}

//--------------------------------------------------------------------------------------------------
// Name: ROutputDataSet::PopulateRowsNumber
//
// Description:
//  Set the number of rows from the underlying DataFrame.
//  If there is a binary raw column, number of rows is set to 1 even if the underlying DataFrame has more
//  rows since all the bytes are returned in a single row.
//
void ROutputDataSet::PopulateRowsNumber()
{
	LOG("ROutputDataSet::PopulateRowsNumber");

	vector<SQLSMALLINT>::const_iterator it =
		find(m_columnsDataType.begin(),m_columnsDataType.end(), SQL_C_BINARY);

	if (it == m_columnsDataType.end())
	{
		// No binary column found; set the DataFrame rows as the number of rows for ROutputDataSet.
		//
		m_rowsNumber = GetDataFrameRowsNumber();
	}
	else if(GetDataFrameRowsNumber() > 0)
	{
		// Raw binary column found; set number of rows of ROutputDatSet = 1
		// only if DataFrame has at least 1 row.
		//
		m_rowsNumber = 1;
	}
}

//--------------------------------------------------------------------------------------------------
// Name: ROutputDataSet::CleanupColumns
//
// Description:
//  Looks up the CleanupColumnFnMap to find the the respective CleanupColumn function for every column
//  and calls that.
//
void ROutputDataSet::CleanupColumns()
{
	LOG("ROutputDataSet::CleanupColumns");

	for (SQLUSMALLINT columnNumber = 0; columnNumber < m_data.size(); ++columnNumber)
	{
		SQLSMALLINT dataType = m_columnsDataType[columnNumber];

		CleanupColumnFnMap::const_iterator it = sm_FnCleanupColumnMap.find(dataType);

		if (it == sm_FnCleanupColumnMap.end())
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
//  For the given columnNumber, cleans up the data buffer used to hold the data
//  before being sent to ExtHost. Also cleans up the columnNullMap.
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
