//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: Dataset.cpp
//
// Purpose:
// Class handling loading and retrieving data from a DataFrame.
//
//*************************************************************************************************

#include "PythonColumn.h"
#include "PythonDataSet.h"
#include "Logger.h"

#include <sqlext.h>

using namespace std;
namespace py = boost::python;

// Function map - maps a SQL data type to the appropriate function that
// adds a column to the dictionary
//
unordered_map<SQLSMALLINT, PythonInputDataSet::fnAddColumn> PythonInputDataSet::m_fnAddColumnMap =
{
	{static_cast<SQLSMALLINT>(SQL_C_BIT),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddColumnToDictionary<SQLCHAR>)},
	{static_cast<SQLSMALLINT>(SQL_C_SLONG),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddColumnToDictionary<SQLINTEGER>)},
	{static_cast<SQLSMALLINT>(SQL_C_DOUBLE),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddColumnToDictionary<SQLDOUBLE>)},
	{static_cast<SQLSMALLINT>(SQL_C_FLOAT),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddColumnToDictionary<SQLREAL>)},
	{static_cast<SQLSMALLINT>(SQL_C_SSHORT),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddColumnToDictionary<SQLSMALLINT>)},
	{static_cast<SQLSMALLINT>(SQL_C_UTINYINT),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddColumnToDictionary<SQLCHAR>)},
	{static_cast<SQLSMALLINT>(SQL_C_SBIGINT),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddColumnToDictionary<SQLBIGINT>)},
	{static_cast<SQLSMALLINT>(SQL_C_CHAR),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddStringColumnToDictionary)},
	{static_cast<SQLSMALLINT>(SQL_C_BINARY),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddRawColumnToDictionary)},
};

//-------------------------------------------------------------------------------------------------
// Name: PythonDataSet::Init
//
// Description:
// Initialize the PythonDataSet with name and number of columns.
//
void PythonDataSet::Init(
	const SQLCHAR  *dataName,
	SQLUSMALLINT   dataNameLength,
	SQLUSMALLINT   schemaColumnsNumber)
{
	LOG("PythonDataSet::Init");

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
// Name: PythonInputDataSet::InitColumn
//
// Description:
//  Initializes each PythonColumn of the member vector m_columns.
//
void PythonInputDataSet::InitColumn(
	SQLUSMALLINT   columnNumber,
	const SQLCHAR  *columnName,
	SQLSMALLINT    columnNameLength,
	SQLSMALLINT    dataType,
	SQLULEN        columnSize,
	SQLSMALLINT    decimalDigits,
	SQLSMALLINT    nullable)
{
	LOG("PythonInputDataSet::InitColumn #" + to_string(columnNumber));

	if (columnName == nullptr)
	{
		throw invalid_argument("Invalid input column name supplied");
	}
	else if (columnNumber >= GetVectorColumnsNumber())
	{
		throw invalid_argument("Invalid input column id supplied: " + to_string(columnNumber));
	}


	AddColumnFnMap::const_iterator it = m_fnAddColumnMap.find(dataType);

	if (it == m_fnAddColumnMap.end())
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
	m_columns[columnNumber] = make_unique<PythonColumn>(
		columnName,
		columnNameLength,
		dataType,
		columnSize,
		decimalDigits,
		nullable);
}

//-------------------------------------------------------------------------------------------------
// Name: PythonInputDataSet::AddColumnsToDictionary
//
// Description:
//  Add columns to the underlying boost python dictionary with the given rowsNumber and data.
//
void PythonInputDataSet::AddColumnsToDictionary(
	SQLULEN      rowsNumber,
	SQLPOINTER   *data,
	SQLINTEGER   **strLen_or_Ind)
{
	LOG("PythonInputDataSet::AddColumnsToDictionary");

	SQLUSMALLINT numberOfCols = GetVectorColumnsNumber();

	for (SQLUSMALLINT columnIndex = 0; columnIndex < numberOfCols; columnIndex++)
	{
		SQLPOINTER colData = nullptr;
		SQLINTEGER *colNullMap = nullptr;

		if (strLen_or_Ind != nullptr)
		{
			colNullMap = strLen_or_Ind[columnIndex];
		}

		if (data != nullptr)
		{
			colData = data[columnIndex];
		}

		if (m_columns[columnIndex] == nullptr)
		{
			throw runtime_error("InitColumn not called for columnNumber " + to_string(columnIndex));
		}

		SQLSMALLINT dataType = m_columns[columnIndex].get()->DataType();
		AddColumnFnMap::const_iterator it = m_fnAddColumnMap.find(dataType);

		if (it == m_fnAddColumnMap.end())
		{
			throw runtime_error("Unsupported column type encountered when adding column #" + to_string(columnIndex));
		}

		(this->*it->second)(
			columnIndex,
			rowsNumber,
			colData,
			colNullMap);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: PythonInputDataSet::AddColumnToDictionary
//
// Description:
//  Add a column to the python dictionary that will be the DataFrame.
//  Works for simple types like numbers and logical
//
template<class SQLType>
void PythonInputDataSet::AddColumnToDictionary(
	SQLSMALLINT columnIndex,
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind)
{
	LOG("PythonInputDataSet::AddColumnToDictionary");

	string name = m_columns[columnIndex].get()->Name();
	SQLSMALLINT dataType = m_columns[columnIndex].get()->DataType();

	py::list dataList = py::list();

	for (SQLULEN j = 0; j < rowsNumber; j++)
	{
		SQLType* t = static_cast<SQLType *>(data);
		SQLType value = t[j];

		if (dataType == SQL_C_BIT)
		{
			 value = value != '0' ? true : false;
		}

		if (strLen_or_Ind != nullptr && strLen_or_Ind[j] == SQL_NULL_DATA)
		{
			dataList.append(py::object());
		}
		else
		{
			dataList.append(value);
		}
	}

	m_dataDict[name] = py::numpy::array(dataList);
}

//--------------------------------------------------------------------------------------------------
// Name: PythonInputDataSet::AddStringColumnToDictionary
//
// Description:
//  Add a string column to the python dictionary that will be the DataFrame
//
void PythonInputDataSet::AddStringColumnToDictionary(
	SQLSMALLINT columnIndex,
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind)
{
	LOG("PythonInputDataSet::AddStringColumnToDictionary");

	string name = m_columns[columnIndex].get()->Name();

	py::list dataList = py::list();

	char *strArray = reinterpret_cast<char*>(data);
	int length = 0;

	for (SQLULEN j = 0; j < rowsNumber; j++)
	{
		if ((strLen_or_Ind == nullptr) ||
			(strLen_or_Ind != nullptr && strLen_or_Ind[j] == SQL_NULL_DATA))
		{
			dataList.append(py::object());
		}
		else
		{
			char *str = strArray + length;
			SQLINTEGER strlen = strLen_or_Ind[j] / sizeof(CHAR);
			string value(str, strlen);
			dataList.append(value);
			length += strlen;
		}
	}

	m_dataDict[name] = py::numpy::array(dataList);
}

//--------------------------------------------------------------------------------------------------
// Name: PythonInputDataSet::AddRawColumnToDictionary
//
// Description:
//  Add a raw column to the python dictionary that will be the DataFrame
//
void PythonInputDataSet::AddRawColumnToDictionary(
	SQLSMALLINT columnIndex,
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind)
{
	LOG("PythonInputDataSet::AddRawColumnToDictionary");

	string name = m_columns[columnIndex].get()->Name();

	py::list dataList = py::list();

	char *rawArray = reinterpret_cast<char*>(data);
	int length = 0;

	for (SQLULEN j = 0; j < rowsNumber; j++)
	{
		if ((strLen_or_Ind == nullptr) ||
			(strLen_or_Ind != nullptr && strLen_or_Ind[j] == SQL_NULL_DATA))
		{
			dataList.append(py::object());
		}
		else
		{
			char *rawVal = rawArray + length;
			SQLINTEGER strlen = strLen_or_Ind[j] / sizeof(SQLCHAR);

			// Create a Python bytes object from binary
			//
			py::object rawObj = py::object(py::handle<>(
				PyBytes_FromObject(PyMemoryView_FromMemory(
					static_cast<char*>(rawVal), strlen, PyBUF_READ
				))
			));

			dataList.append(rawObj);
			length += strlen;
		}
	}

	m_dataDict[name] = py::numpy::array(dataList);
}

//--------------------------------------------------------------------------------------------------
// Name: PythonInputDataSet::AddDictionaryToNamespace
//
// Description:
//  Adds the underlying boost::python dictionary to namespace as a pandas DataFrame.
//
void PythonInputDataSet::AddDictionaryToNamespace(py::object m_mainNamespace)
{
	LOG("PythonInputDataSet::AddDictionaryToNamespace");

	try
	{
		// Create the InputDataSet DataFrame in the namespace
		//
		m_mainNamespace[m_name] = m_dataDict;

		string pandasDataFrame = "from pandas import DataFrame";
		string createDataFrameScript = m_name + " = DataFrame(" + m_name + ")";

		py::exec(pandasDataFrame.c_str(), m_mainNamespace);
		py::exec(createDataFrameScript.c_str(), m_mainNamespace);

	}
	catch (py::error_already_set&)
	{
		string pyError = PythonExtensionUtils::ParsePythonException();
		LOG_ERROR(pyError);
		throw runtime_error(pyError);
	}
	catch (exception &ex)
	{
		LOG_ERROR(ex.what());
	}
}
