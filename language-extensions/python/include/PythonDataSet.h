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
		const SQLCHAR *dataName,
		SQLUSMALLINT  dataNameLength,
		SQLUSMALLINT  schemaColumnsNumber);

	// Returns the number of columns in the PythonDataSet from the vector of PythonColumns
	// at the time this is called.
	// Note: Once Init() resizes the vector to schemaColumnsNumber, its size
	// doesn't increase when InitColumn is done for each column.
	//
	SQLUSMALLINT GetVectorColumnsNumber()
	{
		return m_columns.size();
	}

	// Returns the number of columns in the PythonDataSet that have been added to
	// to the underlying DataFrame at the time this is called.
	// Note: The length returned increases as new columns are added to the DataFrame.
	//
	SQLUSMALLINT GetDataFrameColumnsNumber()
	{
		return boost::python::len(m_dataDict.keys());
	}

protected:

	// A protected constructor to stop instantiation of PythonDataSet
	// but allow derived classes to be instantiated.
	//
	PythonDataSet()
	{
	}

	// The underlying boost::python dictionary.
	//
	boost::python::dict m_dataDict;

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

	// Add columns to the underlying boost python dictionary with the given rowsNumber and data.
	//
	void AddColumnsToDictionary(
		SQLULEN    rowsNumber,
		SQLPOINTER *data,
		SQLINTEGER **strLen_or_Ind);

	// Adds a column of values into the python dictionary
	// Valid for integer, simple numeric, and logical dataTypes.
	//
	template<class SQLType, class NullType>
	void AddColumnToDictionary(
		SQLSMALLINT columnIndex,
		SQLULEN     rowsNumber,
		SQLPOINTER  data,
		SQLINTEGER  *strLen_or_Ind);

	// Adds a column of string values into the python dictionary
	//
	void AddStringColumnToDictionary(
		SQLSMALLINT columnIndex,
		SQLULEN     rowsNumber,
		SQLPOINTER  data,
		SQLINTEGER  *strLen_or_Ind);

	// Adds a column of raw values into the python dictionary
	//
	void AddRawColumnToDictionary(
		SQLSMALLINT columnIndex,
		SQLULEN     rowsNumber,
		SQLPOINTER  data,
		SQLINTEGER  *strLen_or_Ind);

	// Adds the underlying boost::python dictionary to namespace as a pandas DataFrame.
	//
	void AddDictionaryToNamespace(boost::python::object m_mainNamespace);

private:

	// dtype of simple python objects
	//
	const boost::python::numpy::dtype objType =
		boost::python::numpy::array(boost::python::object()).get_dtype();

	// Add column function pointer definition
	//
	using fnAddColumn = void (PythonInputDataSet::*)(
		SQLSMALLINT columnIndex,
		SQLULEN     rowsNumber,
		SQLPOINTER  data,
		SQLINTEGER  *strLen_or_Ind);

	// Function map to add columns to the data frame and its typedef
	//
	static std::unordered_map<SQLSMALLINT, fnAddColumn> m_fnAddColumnMap;
	typedef std::unordered_map<SQLSMALLINT, fnAddColumn> AddColumnFnMap;
};

//-------------------------------------------------------------------------------------------------
// Name: PythonOutputDataSet
//
// Description:
//  Class representing an output PythonDataSet for data retrieval from namespace to PythonExtension.
//
class PythonOutputDataSet : public PythonDataSet
{

};
