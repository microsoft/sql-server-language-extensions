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
namespace np = boost::python::numpy;

// Function map - maps a SQL data type to the appropriate function that
// adds a column to the dictionary
//
unordered_map<SQLSMALLINT, PythonInputDataSet::fnAddColumn> PythonInputDataSet::m_fnAddColumnMap =
{
	{static_cast<SQLSMALLINT>(SQL_C_BIT),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddColumnToDictionary<SQLCHAR, bool>)},
	{static_cast<SQLSMALLINT>(SQL_C_SLONG),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddColumnToDictionary<SQLINTEGER, int>)},
	{static_cast<SQLSMALLINT>(SQL_C_DOUBLE),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddColumnToDictionary<SQLDOUBLE, double>)},
	{static_cast<SQLSMALLINT>(SQL_C_FLOAT),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddColumnToDictionary<SQLREAL, double>)},
	{static_cast<SQLSMALLINT>(SQL_C_SSHORT),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddColumnToDictionary<SQLSMALLINT, int>)},
	{static_cast<SQLSMALLINT>(SQL_C_UTINYINT),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddColumnToDictionary<SQLCHAR, int>)},
	{static_cast<SQLSMALLINT>(SQL_C_SBIGINT),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddColumnToDictionary<SQLBIGINT, int>)},
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
	SQLUSMALLINT  columnNumber,
	const SQLCHAR *columnName,
	SQLSMALLINT   columnNameLength,
	SQLSMALLINT   dataType,
	SQLULEN       columnSize,
	SQLSMALLINT   decimalDigits,
	SQLSMALLINT   nullable)
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
	SQLULEN    rowsNumber,
	SQLPOINTER *data,
	SQLINTEGER **strLen_or_Ind)
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
template<class SQLType, class NullType>
void PythonInputDataSet::AddColumnToDictionary(
	SQLSMALLINT columnIndex,
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind)
{
	LOG("PythonInputDataSet::AddColumnToDictionary");

	string name = m_columns[columnIndex].get()->Name();
	SQLSMALLINT dataType = m_columns[columnIndex].get()->DataType();
	NullType valueForNull =
		*(static_cast<const NullType*>(PythonExtensionUtils::m_dataTypeToNullMap.at(dataType)));

	// Properties of the numpy array that will be populated in from_data below.
	// dt     - Numpy DataType
	// stride - size of each element of the array, so numpy knows how to separate elements
	// shape  - shape of the array as a tuple of sizes of each dimension (we only have 1 dimension)
	// own    - python object to hold a reference until it is transferred to the namespace
	//
	np::dtype dt = np::dtype::get_builtin<SQLType>();
	py::tuple stride = py::make_tuple(sizeof(SQLType));
	py::tuple shape = py::make_tuple(rowsNumber);
	py::object own;

	// We need to specify the type is boolean not SQL_CHAR for bools so python knows
	//
	if (dataType == SQL_C_BIT)
	{
		dt = np::dtype::get_builtin<bool>();
		stride = py::make_tuple(sizeof(bool));
	}

	SQLType* dataArray = static_cast<SQLType *>(data);

	// We modify the data array that is passed in by ExtHost in place so that
	// numpy can use that memory location for its data and not need to copy it out.
	//
	for (SQLULEN i = 0; i < rowsNumber; i++)
	{
		// We need to specify the type is boolean not SQL_CHAR for bools so python knows
		//
		if (dataType == SQL_C_BIT)
		{
			dataArray[i] = dataArray[i] != '0';
		}

		if (strLen_or_Ind != nullptr && strLen_or_Ind[i] == SQL_NULL_DATA)
		{
			dataArray[i] = valueForNull;
		}
	}

	// Create a numpy array that points directly to the C++ data array.
	// This DOES NOT copy the data, so numpy will directly point to the data location.
	//
	np::ndarray npDataArray = np::from_data(dataArray, dt, shape, stride, own);
	m_dataDict[name] = npDataArray;
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

	char *strArray = reinterpret_cast<char*>(data);
	int length = 0;

	// Create an empty numpy array of type python object
	//
	py::tuple shape = py::make_tuple(rowsNumber);
	np::ndarray nArray = np::empty(shape, objType);

	for (SQLULEN i = 0; i < rowsNumber; ++i)
	{
		if ((strLen_or_Ind == nullptr) ||
			(strLen_or_Ind != nullptr && strLen_or_Ind[i] == SQL_NULL_DATA))
		{
			// If this string should be NULL, then we set it to the Python None object.
			//
			nArray[i] = py::object();
		}
		else
		{
			char *str = strArray + length;
			SQLINTEGER strlen = strLen_or_Ind[i] / sizeof(CHAR);

			// Create a string PyObject from the str and strLen.
			// This DOES copy the underlying string into a new buffer and null terminates it.
			// Then, convert to a boost object so that boost handles ref counting.
			//
			py::object strObj = py::object(py::handle<>(
				PyUnicode_FromStringAndSize(str, strlen)
			));

			nArray[i] = strObj;
			length += strlen;
		}
	}

	// By assigning the boost python objects into the data dictionary, the memory will not be
	// deallocated because we keep at least one reference to the PyObjects that were created
	// in the loop above.
	//
	m_dataDict[name] = nArray;
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

	char *rawArray = reinterpret_cast<char*>(data);
	int length = 0;

	// Create an empty numpy array of type python object
	//
	py::tuple shape = py::make_tuple(rowsNumber);
	np::ndarray nArray = np::empty(shape, objType);

	for (SQLULEN i = 0; i < rowsNumber; ++i)
	{
		if ((strLen_or_Ind == nullptr) ||
			(strLen_or_Ind != nullptr && strLen_or_Ind[i] == SQL_NULL_DATA))
		{
			// If this string should be NULL, then we set it to the Python None object.
			//
			nArray[i] = py::object();
		}
		else
		{
			char *rawVal = rawArray + length;
			SQLINTEGER strlen = strLen_or_Ind[i] / sizeof(CHAR);

			// Create a bytes PyObject from the raw bytes and strLen.
			// We create a PyMemoryViewObject (which does not copy the underlying data) then
			// convert to a Bytes object. This step DOES copy the data.
			// Then, convert to a boost object so that boost handles ref counting.
			//
			py::object bytesObj = py::object(py::handle<>(
				PyBytes_FromObject(PyMemoryView_FromMemory(rawVal, strlen, PyBUF_READ))
			));

			nArray[i] = bytesObj;
			length += strlen;
		}
	}

	// By assigning the boost python objects into the data dictionary, the memory will not be
	// deallocated because we keep at least one reference to the PyObjects that were created
	// in the loop above.
	//
	m_dataDict[name] = nArray;
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

	// Create the InputDataSet DataFrame in the namespace
	//
	m_mainNamespace[m_name] = m_dataDict;

	string pandasDataFrame = "from pandas import DataFrame";
	string createDataFrameScript = m_name + " = DataFrame(" + m_name + ")";

	py::exec(pandasDataFrame.c_str(), m_mainNamespace);
	py::exec(createDataFrameScript.c_str(), m_mainNamespace);
}
