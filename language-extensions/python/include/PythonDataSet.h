//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonDataSet.h
//
// Purpose:
// Classes handling loading and retrieving data from a DataFrame.
//
//*************************************************************************************************

#pragma once
#include "Common.h"
#include "PythonColumn.h"
#include "PythonExtensionUtils.h"

#include <unordered_map>

//-------------------------------------------------------------------------------------------------
// Name: PythonDataSet
//
// Description:
//  Base class storing information about the PythonExtension DataSet.
//
class PythonDataSet
{
public:

	// Initialize the PythonDataSet.
	//
	void Init(
		const SQLCHAR         *dataName,
		SQLUSMALLINT          dataNameLength,
		SQLUSMALLINT          schemaColumnsNumber,
		boost::python::object mainNamespace);

	// Returns the number of columns in the PythonDataSet from the vector of PythonColumns
	// at the time this is called.
	// Note: Once Init() resizes the vector to schemaColumnsNumber, its size
	// doesn't increase when InitColumn is done for each column.
	//
	SQLUSMALLINT GetVectorColumnsNumber()
	{
		return m_columns.size();
	}

	// Getter for m_columns.
	//
	const std::vector<std::unique_ptr<PythonColumn>>& Columns() const
	{
		return m_columns;
	}

protected:

	// A protected constructor to stop instantiation of PythonDataSet
	// but allow derived classes to be instantiated.
	//
	PythonDataSet()
	{
	}

	// data type of simple python objects
	//
	const boost::python::numpy::dtype m_ObjType =
		boost::python::numpy::array(boost::python::object()).get_dtype();

	// Maps the ODBC C type to python type
	//
	static const std::unordered_map<std::string, SQLSMALLINT> m_pythonToOdbcTypeMap;
	typedef std::unordered_map<std::string, SQLSMALLINT> pythonToOdbcTypeMap;

	// The underlying boost::python namespace.
	//
	boost::python::object m_mainNamespace;

	// Name of the dataset in python.
	//
	std::string m_name;

	// A vector of PythonColumn objects representing the column info.
	//
	std::vector<std::unique_ptr<PythonColumn>> m_columns;

	// A vector of pointers to strlen_or_Ind info of each column.
	// For a column, strLen_or_Ind is specified for each row.
	//
	std::vector<SQLINTEGER*> m_columnNullMap;
};

//-------------------------------------------------------------------------------------------------
// Name: PythonInputDataSet
//
// Description:
//  Class representing an input PythonDataSet for data load
//  from PythonExtension to the namespace environment.
//
class PythonInputDataSet : public PythonDataSet
{
public:

	// Initializes each Column of the member vector m_columns.
	//
	void InitColumn(
		SQLUSMALLINT   columnNumber,
		const SQLCHAR  *columnName,
		SQLSMALLINT    columnNameLength,
		SQLSMALLINT    dataType,
		SQLULEN        columnSize,
		SQLSMALLINT    decimalDigits,
		SQLSMALLINT    nullable);

	// Returns the number of columns in the PythonDataSet that have been added to
	// to the underlying DataFrame at the time this is called.
	// Note: The length returned increases as new columns are added to the DataFrame.
	//
	SQLUSMALLINT GetDataFrameColumnsNumber() const
	{
		return boost::python::len(m_dataDict.keys());
	}

	// Add columns to the underlying boost python dictionary with the given rowsNumber and data.
	//
	void AddColumnsToDictionary(
		SQLULEN    rowsNumber,
		SQLPOINTER *data,
		SQLINTEGER **strLen_or_Ind);

	// Adds the underlying boost::python dictionary to namespace as a pandas DataFrame.
	//
	void AddDictionaryToNamespace();

private:
	// Adds a column of values into the python dictionary
	// Valid for integer, simple numeric, and boolean dataTypes.
	//
	template<class SQLType, class NullType>
	void AddColumnToDictionary(
		SQLSMALLINT columnNumber,
		SQLULEN     rowsNumber,
		SQLPOINTER  data,
		SQLINTEGER  *strLen_or_Ind);

	// Adds a column of string values into the python dictionary
	//
	void AddStringColumnToDictionary(
		SQLSMALLINT columnNumber,
		SQLULEN     rowsNumber,
		SQLPOINTER  data,
		SQLINTEGER  *strLen_or_Ind);

	// Adds a column of raw values into the python dictionary
	//
	void AddRawColumnToDictionary(
		SQLSMALLINT columnNumber,
		SQLULEN     rowsNumber,
		SQLPOINTER  data,
		SQLINTEGER  *strLen_or_Ind);


	// Add column function pointer definition
	//
	using fnAddColumn = void (PythonInputDataSet::*)(
		SQLSMALLINT columnNumber,
		SQLULEN     rowsNumber,
		SQLPOINTER  data,
		SQLINTEGER  *strLen_or_Ind);

	// Function map to add columns to the data frame and its typedef
	//
	static const std::unordered_map<SQLSMALLINT, fnAddColumn> m_fnAddColumnMap;
	typedef std::unordered_map<SQLSMALLINT, fnAddColumn> AddColumnFnMap;

	// The underlying boost::python dictionary.
	//
	boost::python::dict m_dataDict;
};

//-------------------------------------------------------------------------------------------------
// Name: PythonOutputDataSet
//
// Description:
//  Class representing an output PythonDataSet for data retrieval from namespace to PythonExtension.
//
class PythonOutputDataSet : public PythonDataSet
{
public:

	// Initialize a DataFrame for OutputDataSet to the python namespace.
	//
	void InitializeDataFrameInNamespace();

	// Get the number of columns in the underlying DataFrame
	//
	SQLUSMALLINT GetDataFrameColumnsNumber();

	// Returns the list of names of the columns of the DataFrame
	//
	boost::python::list GetColumnNames();

	// Gets column data from the DataFrame and stores it in the instance.
	//
	void RetrieveColumnsFromDataFrame();

	// Get one of the columns from the underlying pandas DataFrame
	//
	boost::python::numpy::ndarray ExtractArrayFromDataFrame(const std::string columnName);

	// Finds the data type of all columns in the DataFrame.
	//
	void PopulateColumnsDataType();

	// Populate the m_rowsNumber field from the DataFrame
	//
	void PopulateNumberOfRows();

	// Call CleanupColumn on each column.
	//
	void CleanupColumns();

private:
	// Gets the column information, adds data to m_data and nullmap to m_columnNullMap
	//
	template<class SQLType, class NullType, SQLSMALLINT DataType>
	void RetrieveColumnFromDataFrame(
		std::string columnName,
		SQLULEN     &columnSize,
		SQLSMALLINT &decimalDigits,
		SQLSMALLINT &nullable);

	// Gets the string column information, adds data to m_data and nullmap to m_columnNullMap
	//
	void RetrieveStringColumnFromDataFrame(
		std::string columnName,
		SQLULEN     &columnSize,
		SQLSMALLINT &decimalDigits,
		SQLSMALLINT &nullable);

	// Gets the raw column information, adds data to m_data and nullmap to m_columnNullMap
	//
	void RetrieveRawColumnFromDataFrame(
		std::string columnName,
		SQLULEN     &columnSize,
		SQLSMALLINT &decimalDigits,
		SQLSMALLINT &nullable);

	// Determine the data type of the given columnNumber.
	//
	SQLSMALLINT PopulateColumnDataType(SQLUSMALLINT columnNumber) const;

	// Cleanup data buffer and nullmap.
	//
	template<class SQLType>
	void CleanupColumn(SQLUSMALLINT columnNumber);

	// GetColumn function pointer definition
	//
	using fnRetrieveColumn = void (PythonOutputDataSet::*)(
		std::string columnName,
		SQLULEN     &columnSize,
		SQLSMALLINT &decimalDigits,
		SQLSMALLINT &nullable);

	// CleanupColumn function pointer definition.
	//
	using fnCleanupColumn = void (PythonOutputDataSet::*)(SQLUSMALLINT columnNumber);

	// Function map for getting column from DataSet
	//
	static const std::unordered_map<SQLSMALLINT, fnRetrieveColumn> m_fnRetrieveColumnMap;

	// Function map for Cleanup
	//
	static const std::unordered_map<SQLSMALLINT, fnCleanupColumn> m_fnCleanupColumnMap;

	// Function map typedefs.
	//
	typedef std::unordered_map<SQLSMALLINT, fnRetrieveColumn> GetColumnFnMap;
	typedef std::unordered_map<SQLSMALLINT, fnCleanupColumn> CleanupColumnFnMap;

	// Vector of pointers to data from all columns to be sent back to ExtHost.
	//
	std::vector<SQLPOINTER> m_data;

	// List of column names
	//
	boost::python::list m_columnNames;

	SQLULEN m_columnsNumber = 0;

	// Number of rows in the DataSet.
	//
	SQLULEN m_rowsNumber = 0;

	// A vector of ODBC C data type of all columns.
	//
	std::vector<SQLSMALLINT> m_columnsDataType;
};
