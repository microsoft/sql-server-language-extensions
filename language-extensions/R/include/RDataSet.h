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
// @File: RDataSet.h
//
// Purpose:
//  Classes handling loading and retrieving data from an R Dataframe.
//
//*************************************************************************************************

#pragma once

//-------------------------------------------------------------------------------------------------
// Name: RDataSet
//
// Description:
//  Base class storing information about the RExtension DataSet.
//
class RDataSet
{
public:

	// Initialize the RDataSet.
	//
	virtual void Init(
		const SQLCHAR *dataName,
		SQLUSMALLINT  dataNameLength,
		SQLUSMALLINT  schemaColumnsNumber);

	// Cleanup the DataFrame from R environment.
	//
	virtual void Cleanup();

	// Returns the number of columns in the RDataSet from the vector of RColumns
	// at the time this is called.
	// Note: Once Init() resizes the vector to schemaColumnsNumber, its size
	// doesn't increase when InitColumn is done for each column.
	//
	SQLUSMALLINT GetVectorColumnsNumber() const
	{
		return m_columns.size();
	}

	// Returns the number of columns in the RDataSet that have been added to
	// to the underlying DataFrame at the time this is called.
	// Note: The length returned increases as new columns are added to the DataFrame.
	//
	SQLUSMALLINT GetDataFrameColumnsNumber() const
	{
		return m_dataFrame.length();
	}

	// Returns the number of rows in the RDataSet that have been added to
	// to the underlying DataFrame at the time this is called.
	// Note: The length returned increases as a single new column is added to the DataFrame,
	// then stays the same for further addition of columns.
	//
	SQLUSMALLINT GetDataFrameRowsNumber() const
	{
		return m_dataFrame.nrows();
	}

	// Getter for m_columns.
	//
	const std::vector<std::unique_ptr<RColumn>>& Columns() const
	{
		return m_columns;
	}

	// Get the underlying pointer of m_columnNullMap.
	//
	SQLINTEGER** GetColumnNullMap()
	{
		return m_columnNullMap.data();
	}

protected:

	// A protected constructor to stop instantiation of RDataSet
	// but allow derived classes to be instantiated.
	//
	RDataSet()
	{
	}

	// The underlying Rcpp DataFrame pointing to the DataFrame in R.
	//
	Rcpp::DataFrame m_dataFrame;

	// Name of the DataFrame in R.
	//
	std::string m_name;

	// A vector of RColumn objects representing the column info.
	//
	std::vector<std::unique_ptr<RColumn>> m_columns;

	// A vector of pointers to strlen_or_Ind info of each column.
	// For a column, strLen_or_Ind is specified for each row.
	//
	std::vector<SQLINTEGER*> m_columnNullMap;
};

//-------------------------------------------------------------------------------------------------
// Name: RInputDataSet
//
// Description:
//  Class representing an input DataSet for data load from RExtension to the Embedded R environment.
//
class RInputDataSet : public RDataSet
{
public:

	// Initializes each RColumn of the member vector m_columns.
	//
	void InitColumn(
		SQLUSMALLINT   columnNumber,
		const SQLCHAR  *columnName,
		SQLSMALLINT    columnNameLength,
		SQLSMALLINT    dataType,
		SQLULEN        columnSize,
		SQLSMALLINT    decimalDigits,
		SQLSMALLINT    nullable);

	// Add columns to the underlying R DataFrame with the given rowsNumber and data.
	//
	void AddColumnsToDataFrame(
		SQLULEN    rowsNumber,
		SQLPOINTER *data,
		SQLINTEGER **strLen_or_Ind);

	// Adds a single column of values into the R DataFrame
	// template for basic numeric and integer types.
	//
	template<class SQLType, class RType, class NAType, SQLSMALLINT DataType>
	void AddColumnToDataFrame(
		SQLSMALLINT columnNumber,
		SQLULEN     rowsNumber,
		SQLPOINTER  data);

	// Adds a single column of character values into the R DataFrame.
	//
	void AddCharacterColumnToDataFrame(
		SQLSMALLINT columnNumber,
		SQLULEN     rowsNumber,
		SQLPOINTER  data);

	// Adds the underlying R DataFrame to embedded R.
	//
	void AddDataFrameToEmbeddedR();

	void Init(
		const SQLCHAR *dataName,
		SQLUSMALLINT  dataNameLength,
		SQLUSMALLINT  schemaColumnsNumber) override;

	using fnAddColumn = void (RInputDataSet::*)(
		SQLSMALLINT columnNumber,
		SQLULEN     rowsNumber,
		SQLPOINTER  data);

private:
	// Function map for adding column to DataSet
	//
	static const std::unordered_map<SQLSMALLINT, fnAddColumn> m_fnAddColumnMap;

	// Map typedef.
	//
	typedef std::unordered_map<SQLSMALLINT, fnAddColumn> AddColumnFnMap;
};

//-------------------------------------------------------------------------------------------------
// Name: ROutputDataSet
//
// Description:
//  Class representing an output DataSet for data retrieval from Embedded R environment to RExtension.
//
class ROutputDataSet : public RDataSet
{
public:

	// Retrieves the DataFrame with m_name from embedded R if it exists.
	//
	void RetrieveDataFrameFromEmbeddedR();

	// Gets columns from the DataFrame and stores the data.
	//
	void GetColumnsFromDataFrame();

	// Looks up the map m_fnGetColumnMap to find and call GetColumnFromDataFrame.
	//
	void GetColumnFromDataFrameWrapper(
		SQLUSMALLINT columnNumber,
		SQLSMALLINT  dataType,
		SQLULEN      &columnSize,
		SQLSMALLINT  &decimalDigits,
		SQLSMALLINT  &nullable);

	// Gets the column information, adds data to m_data and nullmap to m_columnNullMap
	//
	template<class RType, class SQLType, SQLSMALLINT dataType>
	void GetColumnFromDataFrame(
		SQLUSMALLINT columnNumber,
		SQLULEN      &columnSize,
		SQLSMALLINT  &decimalDigits,
		SQLSMALLINT  &nullable);

	// Gets the character column information, adds data to m_data and nullmap to m_columnNullMap
	//
	void GetCharacterColumnFromDataFrame(
		SQLUSMALLINT columnNumber,
		SQLULEN      &columnSize,
		SQLSMALLINT  &decimalDigits,
		SQLSMALLINT  &nullable);

	// Gets the raw column information, adds data to m_data and nullmap to m_columnNullMap
	//
	void GetRawColumnFromDataFrame(
		SQLUSMALLINT columnNumber,
		SQLULEN      &columnSize,
		SQLSMALLINT  &decimalDigits,
		SQLSMALLINT  &nullable);

	// Gets the data type of all columns in the DataFrame.
	//
	void GetColumnsDataType();

	// Determine the data type of the given columnNumber.
	//
	SQLSMALLINT GetColumnDataType(SQLUSMALLINT columnNumber);

	// Populate numberOfRows based on dataType.
	//
	void PopulateNumberOfRows();

	// Getter for number of rows
	//
	SQLULEN RowsNumber() const
	{
		return m_numberOfRows;
	}

	// Get the underlying pointer of m_data.
	//
	SQLPOINTER* GetData()
	{
		return m_data.data();
	}

	// Call CleanupColumn on each column.
	//
	void CleanupColumns();

	// Cleanup data buffer and nullmap.
	//
	template<class SQLType>
	void CleanupColumn(SQLUSMALLINT columnNumber);

private:
	// Vector of pointers to data from all columns to be sent back to ExtHost.
	//
	std::vector<SQLPOINTER> m_data;

	// Number of rows in the DataSet.
	//
	SQLULEN m_numberOfRows = 0;

	// A vector of ODBC C data type of all columns.
	//
	std::vector<SQLSMALLINT> m_columnsDataType;

	// GetColumn function pointer definition.
	//
	using fnGetColumn = void (ROutputDataSet::*)(
		SQLUSMALLINT columnNumber,
		SQLULEN      &columnSize,
		SQLSMALLINT  &decimalDigits,
		SQLSMALLINT  &nullable);

	// CleanupColumn function pointer definition.
	//
	using fnCleanupColumn = void (ROutputDataSet::*)(
		SQLUSMALLINT columnNumber);

	// Function map for getting column from DataSet
	//
	static const std::unordered_map<SQLSMALLINT, fnGetColumn> m_fnGetColumnMap;

	// Function map for Cleanup
	//
	static const std::unordered_map<SQLSMALLINT, fnCleanupColumn> m_fnCleanupColumnMap;

	// Function map typedefs.
	//
	typedef std::unordered_map<SQLSMALLINT, fnGetColumn> GetColumnFnMap;
	typedef std::unordered_map<SQLSMALLINT, fnCleanupColumn> CleanupColumnFnMap;
};
