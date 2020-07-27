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

// datetime.h includes macros that define the PyDateTime APIs
//
#include <datetime.h>
#include <sqlext.h>

using namespace std;
namespace py = boost::python;
namespace np = py::numpy;

// Maps the ODBC C type to python type
//
const unordered_map<string, SQLSMALLINT> PythonDataSet::m_pythonToOdbcTypeMap =
{
	{"bool", SQL_C_BIT},
	{"uint8", SQL_C_UTINYINT},
	{"int16", SQL_C_SSHORT},
	{"int32", SQL_C_SLONG},
	{"int64", SQL_C_SBIGINT},
	{"float32", SQL_C_FLOAT},
	{"float64", SQL_C_DOUBLE},
	{"str", SQL_C_CHAR},
	{"bytes", SQL_C_BINARY},
	{"datetime64[ns]", SQL_C_TYPE_TIMESTAMP},
	{"datetime.datetime", SQL_C_TYPE_TIMESTAMP},
	{"datetime.date", SQL_C_TYPE_DATE},

	// Default types for when the array dtype is "object"
	//
	{"int", SQL_C_SBIGINT},
	{"float", SQL_C_DOUBLE},
	{"NoneType", SQL_C_CHAR}
};

// Function map - maps a SQL data type to the appropriate function that
// adds a column to the dictionary
//
const PythonInputDataSet::AddColumnFnMap PythonInputDataSet::sm_FnAddColumnMap =
{
	{static_cast<SQLSMALLINT>(SQL_C_BIT),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddBooleanColumnToDictionary)},
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
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddStringColumnToDictionary<char>)},
	{static_cast<SQLSMALLINT>(SQL_C_WCHAR),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddStringColumnToDictionary<wchar_t>)},
	{static_cast<SQLSMALLINT>(SQL_C_BINARY),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddRawColumnToDictionary)},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_TIMESTAMP),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddDateTimeColumnToDictionary<SQL_C_TYPE_TIMESTAMP>)},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_DATE),
	 static_cast<fnAddColumn>(&PythonInputDataSet::AddDateTimeColumnToDictionary<SQL_C_TYPE_DATE>)},
};

// Function map - maps a SQL data type to the appropriate function that
// adds a column to the dictionary
//
const PythonOutputDataSet::GetColumnFnMap PythonOutputDataSet::sm_FnRetrieveColumnMap =
{
	{static_cast<SQLSMALLINT>(SQL_C_BIT),
	 static_cast<fnRetrieveColumn>(&PythonOutputDataSet::RetrieveBooleanColumnFromDataFrame)},
	{static_cast<SQLSMALLINT>(SQL_C_SLONG),
	 static_cast<fnRetrieveColumn>(&PythonOutputDataSet::RetrieveColumnFromDataFrame<SQLINTEGER, int, SQL_C_SLONG>)},
	{static_cast<SQLSMALLINT>(SQL_C_DOUBLE),
	 static_cast<fnRetrieveColumn>(&PythonOutputDataSet::RetrieveColumnFromDataFrame<SQLDOUBLE, float, SQL_C_DOUBLE>)},
	{static_cast<SQLSMALLINT>(SQL_C_FLOAT),
	 static_cast<fnRetrieveColumn>(&PythonOutputDataSet::RetrieveColumnFromDataFrame<SQLREAL, float, SQL_C_FLOAT>)},
	{static_cast<SQLSMALLINT>(SQL_C_SSHORT),
	 static_cast<fnRetrieveColumn>(&PythonOutputDataSet::RetrieveColumnFromDataFrame<SQLSMALLINT, int, SQL_C_SSHORT>)},
	{static_cast<SQLSMALLINT>(SQL_C_UTINYINT),
	 static_cast<fnRetrieveColumn>(&PythonOutputDataSet::RetrieveColumnFromDataFrame<SQLCHAR, int, SQL_C_UTINYINT>)},
	{static_cast<SQLSMALLINT>(SQL_C_SBIGINT),
	 static_cast<fnRetrieveColumn>(&PythonOutputDataSet::RetrieveColumnFromDataFrame<SQLBIGINT, int, SQL_C_SBIGINT>)},
	{static_cast<SQLSMALLINT>(SQL_C_CHAR),
	 static_cast<fnRetrieveColumn>(&PythonOutputDataSet::RetrieveStringColumnFromDataFrame)},
	{static_cast<SQLSMALLINT>(SQL_C_BINARY),
	 static_cast<fnRetrieveColumn>(&PythonOutputDataSet::RetrieveRawColumnFromDataFrame)},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_TIMESTAMP),
	 static_cast<fnRetrieveColumn>(&PythonOutputDataSet::RetrieveDateTimeColumnFromDataFrame<SQL_TIMESTAMP_STRUCT>)},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_DATE),
	 static_cast<fnRetrieveColumn>(&PythonOutputDataSet::RetrieveDateTimeColumnFromDataFrame<SQL_DATE_STRUCT>)},
};

// Map of function pointers for cleaning up output data buffers and null map.
//
const PythonOutputDataSet::CleanupColumnFnMap PythonOutputDataSet::sm_FnCleanupColumnMap =
{
	{static_cast<SQLSMALLINT>(SQL_C_BIT),
	 static_cast<fnCleanupColumn>(&PythonOutputDataSet::CleanupColumn<SQLCHAR>)},
	{static_cast<SQLSMALLINT>(SQL_C_SLONG),
	 static_cast<fnCleanupColumn>(&PythonOutputDataSet::CleanupColumn<SQLINTEGER>)},
	{static_cast<SQLSMALLINT>(SQL_C_DOUBLE),
	 static_cast<fnCleanupColumn>(&PythonOutputDataSet::CleanupColumn<SQLDOUBLE>)},
	{static_cast<SQLSMALLINT>(SQL_C_FLOAT),
	 static_cast<fnCleanupColumn>(&PythonOutputDataSet::CleanupColumn<SQLREAL>)},
	{static_cast<SQLSMALLINT>(SQL_C_SSHORT),
	 static_cast<fnCleanupColumn>(&PythonOutputDataSet::CleanupColumn<SQLSMALLINT>)},
	{static_cast<SQLSMALLINT>(SQL_C_UTINYINT),
	 static_cast<fnCleanupColumn>(&PythonOutputDataSet::CleanupColumn<SQLCHAR>)},
	{static_cast<SQLSMALLINT>(SQL_C_SBIGINT),
	 static_cast<fnCleanupColumn>(&PythonOutputDataSet::CleanupColumn<SQLBIGINT>)},
	{static_cast<SQLSMALLINT>(SQL_C_CHAR),
	 static_cast<fnCleanupColumn>(&PythonOutputDataSet::CleanupColumn<SQLCHAR>)},
	{static_cast<SQLSMALLINT>(SQL_C_BINARY),
	 static_cast<fnCleanupColumn>(&PythonOutputDataSet::CleanupColumn<SQLCHAR>)},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_TIMESTAMP),
	 static_cast<fnCleanupColumn>(&PythonOutputDataSet::CleanupColumn<SQL_TIMESTAMP_STRUCT>)},
	{static_cast<SQLSMALLINT>(SQL_C_TYPE_DATE),
	 static_cast<fnCleanupColumn>(&PythonOutputDataSet::CleanupColumn<SQL_DATE_STRUCT>)},
};

//-------------------------------------------------------------------------------------------------
// Name: PythonDataSet::Init
//
// Description:
//  Initializes the PythonDataSet with name and number of column
//  and a reference to the python namespace.
//
void PythonDataSet::Init(
	const SQLCHAR  *dataName,
	SQLUSMALLINT   dataNameLength,
	SQLUSMALLINT   schemaColumnsNumber,
	py::object     mainNamespace)
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

	m_mainNamespace = mainNamespace;

	// Set the size of the columns vector to the given schema columns number.
	//
	m_columns.resize(schemaColumnsNumber);
	m_columnNullMap.resize(schemaColumnsNumber);
}

//-------------------------------------------------------------------------------------------------
// Name: PythonDataSet::HasNulls
//
// Description:
//  Check whether there are any SQL_NULL_DATA in the strLen_or_Ind
//
bool PythonDataSet::HasNulls(
	SQLULEN    rowsNumber,
	SQLINTEGER *strLen_or_Ind) const
{
	LOG("PythonDataSet::HasNulls");

	bool hasNulls = false;

	// For basic types, if strLen_or_Ind is nullptr we take that to mean there are no NULL values.
	//
	if(strLen_or_Ind != nullptr)
	{
		for (SQLULEN row = 0; row < rowsNumber; ++row)
		{
			if (strLen_or_Ind[row] == SQL_NULL_DATA)
			{
				hasNulls = true;
				break;
			}
		}
	}

	return hasNulls;
}

//-------------------------------------------------------------------------------------------------
// Name: PythonDataSet::Cleanup
//
// Description:
//  Cleanup the dataset in python namespace.
//
void PythonDataSet::Cleanup()
{
	LOG("PythonDataSet::Cleanup");

	if (m_name.length() > 0)
	{
		string cleanupScript = "if '" + m_name + "' in globals(): del " + m_name;

		// Execute the cleanup script to reset the dataset variable to None.
		//
		py::exec(cleanupScript.c_str(), m_mainNamespace);
	}
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

	AddColumnFnMap::const_iterator it = sm_FnAddColumnMap.find(dataType);

	if (it == sm_FnAddColumnMap.end())
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
//  Adds columns to the underlying boost python dictionary with the given rowsNumber and data.
//
void PythonInputDataSet::AddColumnsToDictionary(
	SQLULEN    rowsNumber,
	SQLPOINTER *data,
	SQLINTEGER **strLen_or_Ind)
{
	LOG("PythonInputDataSet::AddColumnsToDictionary");

	SQLUSMALLINT numberOfCols = GetVectorColumnsNumber();

	for (SQLUSMALLINT columnNumber = 0; columnNumber < numberOfCols; ++columnNumber)
	{
		SQLPOINTER colData = nullptr;
		SQLINTEGER *colNullMap = nullptr;

		if (strLen_or_Ind != nullptr)
		{
			colNullMap = strLen_or_Ind[columnNumber];
		}

		if (data != nullptr)
		{
			colData = data[columnNumber];
		}

		if (m_columns[columnNumber] == nullptr)
		{
			throw runtime_error("InitColumn not called for columnNumber " + to_string(columnNumber));
		}

		SQLSMALLINT dataType = m_columns[columnNumber].get()->DataType();
		AddColumnFnMap::const_iterator it = sm_FnAddColumnMap.find(dataType);

		if (it == sm_FnAddColumnMap.end())
		{
			throw runtime_error("Unsupported column type encountered when adding column #" + to_string(columnNumber));
		}

		(this->*it->second)(
			columnNumber,
			rowsNumber,
			colData,
			colNullMap);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: PythonInputDataSet::AddColumnToDictionary
//
// Description:
//  Adds a column to the python dictionary that will be the DataFrame.
//  Works for simple types like numbers and boolean
//
template<class SQLType>
void PythonInputDataSet::AddColumnToDictionary(
	SQLSMALLINT columnNumber,
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind)
{
	LOG("PythonInputDataSet::AddColumnToDictionary");

	string name = m_columns[columnNumber].get()->Name();
	SQLType* dataArray = static_cast<SQLType *>(data);

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

	// Check if there are actually any nulls in this nullable column
	//
	bool hasNulls = false;
	if (m_columns[columnNumber].get()->Nullable() == SQL_NULLABLE)
	{
		hasNulls = HasNulls(rowsNumber, strLen_or_Ind);
	}

	// If there are no NULLs in the input data, then we can use a more memory efficient way
	// of constructing the numpy array.
	// If there ARE NULLs, then we need to create python objects for each value
	// and use None for the NULLs
	//
	if(!hasNulls)
	{
		// Create a numpy array that points directly to the C++ data array.
		// This DOES NOT copy the data, so numpy will directly point to the data location.
		//
		np::ndarray npDataArray = np::from_data(dataArray, dt, shape, stride, own);
		m_dataDict[name] = npDataArray;
	}
	else
	{
		np::ndarray nArray = np::empty(shape, m_ObjType);

		for (SQLULEN row = 0; row < rowsNumber; ++row)
		{
			if (strLen_or_Ind[row] == SQL_NULL_DATA)
			{
				// Use None object for NULLs
				//
				nArray[row] = py::object();
			}
			else
			{
				nArray[row] = dataArray[row];
			}
		}

		m_dataDict[name] = nArray;
	}
}

//--------------------------------------------------------------------------------------------------
// Name: PythonInputDataSet::AddBooleanColumnToDictionary
//
// Description:
//  Adds a boolean column to the python dictionary that will be the DataFrame.
//
void PythonInputDataSet::AddBooleanColumnToDictionary(
	SQLSMALLINT columnNumber,
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind)
{
	LOG("PythonInputDataSet::AddBooleanColumnToDictionary");

	string name = m_columns[columnNumber].get()->Name();

	// Properties of the numpy array that will be populated in from_data below.
	// dt     - Numpy DataType
	// stride - size of each element of the array, so numpy knows how to separate elements
	// shape  - shape of the array as a tuple of sizes of each dimension (we only have 1 dimension)
	// own    - python object to hold a reference until it is transferred to the namespace
	//
	np::dtype dt = np::dtype::get_builtin<bool>();
	py::tuple stride = py::make_tuple(sizeof(bool));
	py::tuple shape = py::make_tuple(rowsNumber);
	py::object own;

	// Check if there are actually any nulls in this nullable column
	//
	bool hasNulls = false;
	if (m_columns[columnNumber].get()->Nullable() == SQL_NULLABLE)
	{
		hasNulls = HasNulls(rowsNumber, strLen_or_Ind);
	}

	SQLCHAR* dataArray = static_cast<SQLCHAR *>(data);

	// If there are no NULLs in the input data, then we can use a more memory efficient way
	// of constructing the numpy array.
	// If there ARE NULLs, then we need to create python objects for each value
	// and use None for the NULLs
	//
	if (!hasNulls)
	{
		// We modify the data array that is passed in by ExtHost in place so that
		// numpy can use that memory location for its data and not need to copy it out.
		//
		for (SQLULEN row = 0; row < rowsNumber; ++row)
		{
			dataArray[row] = dataArray[row] != '0';
		}

		// Create a numpy array that points directly to the C++ data array.
		// This DOES NOT copy the data, so numpy will directly point to the data location.
		//
		np::ndarray npDataArray = np::from_data(dataArray, dt, shape, stride, own);
		m_dataDict[name] = npDataArray;
	}
	else
	{
		np::ndarray nArray = np::empty(shape, m_ObjType);

		for (SQLULEN row = 0; row < rowsNumber; ++row)
		{
			if (strLen_or_Ind[row] == SQL_NULL_DATA)
			{
				// Use None object for NULLs
				//
				nArray[row] = py::object();
			}
			else
			{
				dataArray[row] = dataArray[row] != '0';
				nArray[row] = bool(dataArray[row]);
			}
		}

		m_dataDict[name] = nArray;
	}
}

//--------------------------------------------------------------------------------------------------
// Name: PythonInputDataSet::AddStringColumnToDictionary
//
// Description:
//  Adds a string column to the python dictionary that will be the DataFrame
//
template<class CharType>
void PythonInputDataSet::AddStringColumnToDictionary(
	SQLSMALLINT columnNumber,
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind)
{
	LOG("PythonInputDataSet::AddStringColumnToDictionary");

	string name = m_columns[columnNumber].get()->Name();

	CharType *strArray = reinterpret_cast<CharType*>(data);

	int length = 0;

	// Create an empty numpy array of type python object
	//
	py::tuple shape = py::make_tuple(rowsNumber);
	np::ndarray nArray = np::empty(shape, m_ObjType);

	for (SQLULEN row = 0; row < rowsNumber; ++row)
	{
		if (strLen_or_Ind == nullptr || strLen_or_Ind[row] == SQL_NULL_DATA)
		{
			// If this string should be NULL, then we set it to the Python None object.
			//
			nArray[row] = py::object();
		}
		else
		{
			CharType *str = strArray + length;
			Py_ssize_t strlen = strLen_or_Ind[row] / sizeof(CharType);

			// Create a string PyObject from the str and strLen.
			// This DOES copy the underlying string into a new buffer and null terminates it.
			// Then, convert to a boost object so that boost handles ref counting.
			//
			py::object strObj = py::object(py::handle<>(
				PyUnicode_FromKindAndData(sizeof(CharType), str, strlen)
			));

			nArray[row] = strObj;
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
//  Adds a raw column to the python dictionary that will be the DataFrame
//
void PythonInputDataSet::AddRawColumnToDictionary(
	SQLSMALLINT columnNumber,
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind)
{
	LOG("PythonInputDataSet::AddRawColumnToDictionary");

	string name = m_columns[columnNumber].get()->Name();

	char *rawArray = reinterpret_cast<char*>(data);
	int length = 0;

	// Create an empty numpy array of type python object
	//
	py::tuple shape = py::make_tuple(rowsNumber);
	np::ndarray nArray = np::empty(shape, m_ObjType);

	for (SQLULEN row = 0; row < rowsNumber; ++row)
	{
		if (strLen_or_Ind == nullptr || strLen_or_Ind[row] == SQL_NULL_DATA)
		{
			// If this string should be NULL, then we set it to the Python None object.
			//
			nArray[row] = py::object();
		}
		else
		{
			char *rawVal = rawArray + length;
			SQLINTEGER strlen = strLen_or_Ind[row] / sizeof(CHAR);

			// Create a bytes PyObject from the raw bytes and strLen.
			// We create a PyMemoryViewObject (which does not copy the underlying data) then
			// convert to a Bytes object. This step DOES copy the data.
			// Then, convert to a boost object so that boost handles ref counting.
			//
			py::object bytesObj = py::object(py::handle<>(
				PyBytes_FromObject(PyMemoryView_FromMemory(rawVal, strlen, PyBUF_READ))
			));

			nArray[row] = bytesObj;
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
// Name: PythonInputDataSet::AddDateTimeColumnToDictionary
//
// Description:
//  Adds a datetime column to the python dictionary that will be the DataFrame
//
template<SQLSMALLINT DataType>
void PythonInputDataSet::AddDateTimeColumnToDictionary(
	SQLSMALLINT columnNumber,
	SQLULEN     rowsNumber,
	SQLPOINTER  data,
	SQLINTEGER  *strLen_or_Ind)
{
	LOG("PythonInputDataSet::AddDateTimeColumnToDictionary");

	string name = m_columns[columnNumber].get()->Name();

	// Create an empty numpy array of type python object
	//
	py::tuple shape = py::make_tuple(rowsNumber);
	np::ndarray nArray = np::empty(shape, m_ObjType);

	for (SQLULEN row = 0; row < rowsNumber; ++row)
	{
		if (strLen_or_Ind == nullptr || strLen_or_Ind[row] == SQL_NULL_DATA)
		{
			// If this string should be NULL, then we set it to the Python None object.
			//
			nArray[row] = py::object();
		}
		else
		{
			// Use the PyDateTime_IMPORT macro to get the Python Date/Time APIs
			//
			PyDateTime_IMPORT;
			PyObject *dtObject = Py_None;

			if (DataType == SQL_C_TYPE_DATE)
			{
				SQL_DATE_STRUCT dateParam = *(static_cast<SQL_DATE_STRUCT *>(data) + row);

				// Create a Python Date object
				//
				dtObject = PyDate_FromDate(dateParam.year, dateParam.month, dateParam.day);
			}
			else if (DataType == SQL_C_TYPE_TIMESTAMP)
			{
				SQL_TIMESTAMP_STRUCT timeStampParam = *(static_cast<SQL_TIMESTAMP_STRUCT *>(data) + row);

				// "fraction" is stored in nanoseconds, we need microseconds.
				//
				SQLUINTEGER usec = timeStampParam.fraction / 1000;

				// Create a Python DateTime object
				//
				dtObject = PyDateTime_FromDateAndTime(timeStampParam.year, timeStampParam.month, timeStampParam.day,
					timeStampParam.hour, timeStampParam.minute, timeStampParam.second, usec);
			}

			nArray[row] = py::object(py::handle<>(dtObject));
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
void PythonInputDataSet::AddDictionaryToNamespace()
{
	LOG("PythonInputDataSet::AddDictionaryToNamespace");

	// Create the InputDataSet DataFrame in the namespace
	//
	m_mainNamespace[m_name] = m_dataDict;

	string createDataFrameScript = m_name + " = DataFrame(" + m_name + ", copy=False)";

	py::exec(createDataFrameScript.c_str(), m_mainNamespace);
}

//--------------------------------------------------------------------------------------------------
// Name: PythonOutputDataSet::InitializeDataFrameInNamespace
//
// Description:
//  Initializes the OutputDataSet DataFrame as an empty boost::python dictionary.
//
void PythonOutputDataSet::InitializeDataFrameInNamespace()
{
	LOG("PythonOutputDataSet::InitializeDataFrameInNamespace");

	string createDataFrameScript = m_name + " = DataFrame()";

	py::exec(createDataFrameScript.c_str(), m_mainNamespace);
}

//--------------------------------------------------------------------------------------------------
// Name: PythonOutputDataSet::GetDataFrameColumnsNumber
//
// Description:
//  Gets the number of columns in the DataFrame
//
SQLUSMALLINT PythonOutputDataSet::GetDataFrameColumnsNumber()
{
	LOG("PythonOutputDataSet::GetDataFrameColumnsNumber");

	if(m_columnsNumber == 0)
	{
		string getColumnsNumberScript = "len("+ m_name +".columns)";

		m_columnsNumber = py::extract<SQLUSMALLINT>(py::eval(getColumnsNumberScript.c_str(), m_mainNamespace));
	}

	return m_columnsNumber;
}

//--------------------------------------------------------------------------------------------------
// Name: PythonOutputDataSet::GetColumnNames
//
// Description:
//  Returns the list of names of the columns of the DataFrame
//
py::list PythonOutputDataSet::GetColumnNames()
{
	LOG("PythonOutputDataSet::GetColumnNames");

	if(py::len(m_columnNames) == 0)
	{
		string getColumnNamesScript = "list(" + m_name + ".columns)";
		m_columnNames = py::extract<py::list>(py::eval(getColumnNamesScript.c_str(), m_mainNamespace));
	}

	return m_columnNames;
}

//--------------------------------------------------------------------------------------------------
// Name: PythonOutputDataSet::RetrieveColumnsFromDataFrame
//
// Description:
//  Gets columns from the DataFrame and stores their data, nullmap and other information.
//
void PythonOutputDataSet::RetrieveColumnsFromDataFrame()
{
	LOG("PythonOutputDataSet::RetrieveColumnsFromDataFrame");

	py::list columnNames = GetColumnNames();

	for (SQLUSMALLINT columnNumber = 0; columnNumber < py::len(columnNames); columnNumber++)
	{
		string columnName = py::extract<string>(columnNames[columnNumber]);
		SQLSMALLINT dataType = m_columnsDataType[columnNumber];
		SQLULEN columnSize = 0;
		SQLSMALLINT decimalDigits = 0;
		SQLSMALLINT nullable = SQL_NO_NULLS;

		// Gets the column information, add data to m_data and nullmap to m_columnNullMap
		//
		GetColumnFnMap::const_iterator it = sm_FnRetrieveColumnMap.find(dataType);

		if (it == sm_FnRetrieveColumnMap.end())
		{
			throw invalid_argument("Unsupported data type "
				+ to_string(dataType) + " in output data for column # " + to_string(columnNumber));
		}

		(this->*it->second)(
			columnName,
			columnSize,
			decimalDigits,
			nullable);

		// Store the column information obtained above in m_columns.
		//
		const SQLCHAR *unsignedColumnName = static_cast<const SQLCHAR*>(
			static_cast<const void*>(columnName.c_str()));

		m_columns.push_back(make_unique<PythonColumn>(
			unsignedColumnName,
			static_cast<SQLSMALLINT>(columnName.length()),
			dataType,
			columnSize,
			decimalDigits,
			nullable));
	}
}

//--------------------------------------------------------------------------------------------------
// Name: PythonOutputDataSet::RetrieveColumnFromDataFrame
//
// Description:
//  Templatized function to get the column information from the underlying DataFrame,
//  adds data to m_data and nullmap to m_columnNullMap.
//  Templated for integer and simple numeric types.
//
template<class SQLType, class NullType, SQLSMALLINT DataType>
void PythonOutputDataSet::RetrieveColumnFromDataFrame(
	string       columnName,
	SQLULEN      &columnSize,
	SQLSMALLINT  &decimalDigits,
	SQLSMALLINT  &nullable)
{
	LOG("PythonOutputDataSet::RetrieveColumnFromDataFrame");

	SQLType *columnData = nullptr;
	SQLINTEGER *nullMap = nullptr;
	NullType valueForNull = *(static_cast<const NullType*>(
		PythonExtensionUtils::m_DataTypeToNullMap.at(DataType)));

	if (m_rowsNumber > 0)
	{
		columnData = new SQLType[m_rowsNumber];
		nullMap = new SQLINTEGER[m_rowsNumber];
	}

	columnSize = sizeof(SQLType);
	decimalDigits = 0;
	nullable = SQL_NO_NULLS;

	// Get the column of values
	//
	np::ndarray column = ExtractArrayFromDataFrame(columnName);

	for (SQLULEN row = 0; row < m_rowsNumber; ++row)
	{
		bool isNull = true;
		py::object pyObj = column[row];

		// Make sure the object is not pointing at Python None, or else it will crash on extract
		//
		if (!pyObj.is_none())
		{
			// Extract the data value from the iterator
			//
			py::extract<SQLType> extractedData(pyObj);

			// Check to make sure the extracted data exists and is of the correct type
			//
			if (extractedData.check())
			{
				SQLType data = extractedData;

				// If the data is not NAN or INF, we set it to the extracted data.
				//
				if(!(is_same<NullType, float>::value && (isnan(data) || isinf(data))))
				{
					columnData[row] = data;
					nullMap[row] = sizeof(SQLType);
					isNull = false;
				}
			}
		}

		// If data is None, NAN, INF, or a bad type, we set it to NULL_DATA.
		//
		if(isNull)
		{
			// If there are any nulls, nullable is set to SQL_NULLABLE for the whole column
			//
			nullMap[row] = SQL_NULL_DATA;
			nullable = SQL_NULLABLE;
			columnData[row] = valueForNull;
		}
	}

	m_data.push_back(static_cast<SQLPOINTER>(columnData));
	m_columnNullMap.push_back(nullMap);
}

//--------------------------------------------------------------------------------------------------
// Name: PythonOutputDataSet::RetrieveBooleanColumnFromDataFrame
//
// Description:
//  Gets boolean column information from the underlying DataFrame,
//  adds data to m_data and nullmap to m_columnNullMap.
//
void PythonOutputDataSet::RetrieveBooleanColumnFromDataFrame(
	string      columnName,
	SQLULEN     &columnSize,
	SQLSMALLINT &decimalDigits,
	SQLSMALLINT &nullable)
{
	LOG("PythonOutputDataSet::RetrieveBooleanColumnFromDictionary");

	bool *columnData = nullptr;
	SQLINTEGER *nullMap = nullptr;

	if (m_rowsNumber > 0)
	{
		columnData = new bool[m_rowsNumber];
		nullMap = new SQLINTEGER[m_rowsNumber];
	}

	columnSize = sizeof(bool);
	decimalDigits = 0;
	nullable = SQL_NO_NULLS;

	// Get the column of values
	//
	np::ndarray column = ExtractArrayFromDataFrame(columnName);

	for (SQLULEN row = 0; row < m_rowsNumber; ++row)
	{
		py::object pyObj = column[row];

		// Make sure the object is not pointing at Python None, or else it will crash on extract
		//
		if (!pyObj.is_none())
		{
			// Extract the data value from the iterator
			//
			py::extract<bool> extractedData(pyObj);

			// Check to make sure the extracted data exists and is of the correct type
			//
			if (extractedData.check())
			{
				bool data = extractedData;

				columnData[row] = data;
			}
			else
			{
				columnData[row] = false;
			}

			nullMap[row] = sizeof(SQLCHAR);
		}
		else
		{
			// If there are any nulls, nullable is set to SQL_NULLABLE for the whole column
			//
			nullMap[row] = SQL_NULL_DATA;
			nullable = SQL_NULLABLE;
			columnData[row] = false;
		}
	}

	m_data.push_back(static_cast<SQLPOINTER>(columnData));
	m_columnNullMap.push_back(nullMap);
}

//--------------------------------------------------------------------------------------------------
// Name: PythonOutputDataSet::RetrieveStringColumnFromDataFrame
//
// Description:
//  Gets string column information from the underlying DataFrame,
//  adds data to m_data and nullmap to m_columnNullMap.
//
void PythonOutputDataSet::RetrieveStringColumnFromDataFrame(
	string      columnName,
	SQLULEN     &columnSize,
	SQLSMALLINT &decimalDigits,
	SQLSMALLINT &nullable)
{
	LOG("PythonOutputDataSet::RetrieveStringColumnFromDataFrame");

	vector<SQLCHAR> *columnData = nullptr;
	SQLINTEGER *strLenOrNullMap = nullptr;
	if (m_rowsNumber > 0)
	{
		columnData = new vector<SQLCHAR>();
		strLenOrNullMap = new SQLINTEGER[m_rowsNumber];
	}

	decimalDigits = 0;
	nullable = SQL_NO_NULLS;

	np::ndarray column = ExtractArrayFromDataFrame(columnName);

	// Insert the string column into the columnData vector contiguously.
	//
	SQLINTEGER maxLen = 0;
	for (SQLULEN row = 0; row < m_rowsNumber; ++row)
	{
		py::object pyObj = column[row];

		// Make sure the iterator is not pointing at Python None, or else it will crash on extract
		//
		if (!pyObj.is_none())
		{
			py::extract<string> extractedData(pyObj);

			// Check to make sure the extracted data exists and is of the correct type
			//
			if (extractedData.check())
			{
				string data = extractedData;
				strLenOrNullMap[row] = data.length();

				// Concatenate the string data into the full column data
				//
				columnData->insert(columnData->end(), data.begin(), data.end());

				// Store the maximum length to find the widest the column needs to be
				//
				if (maxLen < strLenOrNullMap[row])
				{
					maxLen = strLenOrNullMap[row];
				}
			}
			else
			{
				strLenOrNullMap[row] = SQL_NULL_DATA;
				nullable = SQL_NULLABLE;
			}
		}
		else
		{
			strLenOrNullMap[row] = SQL_NULL_DATA;
			nullable = SQL_NULLABLE;
		}
	}

	columnSize = maxLen;
	if (m_rowsNumber > 0)
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
// Name: PythonOutputDataSet::RetrieveRawColumnFromDataFrame
//
// Description:
//  Gets raw column information from the underlying DataFrame,
//  adds data to m_data and nullmap to m_columnNullMap.
//
void PythonOutputDataSet::RetrieveRawColumnFromDataFrame(
	string      columnName,
	SQLULEN     &columnSize,
	SQLSMALLINT &decimalDigits,
	SQLSMALLINT &nullable)
{
	LOG("PythonOutputDataSet::RetrieveRawColumnFromDataFrame");

	vector<SQLCHAR> *columnData = nullptr;
	SQLINTEGER *strLenOrNullMap = nullptr;
	if (m_rowsNumber > 0)
	{
		columnData = new vector<SQLCHAR>();
		strLenOrNullMap = new SQLINTEGER[m_rowsNumber];
	}

	decimalDigits = 0;
	nullable = SQL_NO_NULLS;

	np::ndarray column = ExtractArrayFromDataFrame(columnName);

	// Insert the raw column into the columnData vector contiguously.
	//
	SQLINTEGER maxLen = 0;
	for (SQLULEN row = 0; row < m_rowsNumber; ++row)
	{
		py::object pyObj = column[row];

		// Make sure the iterator is not pointing at Python None, or else it will crash on extract
		//
		if (!pyObj.is_none())
		{
			// Extract the size and bytes of the pyObj
			//
			PyObject *baseObj = pyObj.ptr();

			int size = PyBytes_Size(baseObj);
			SQLCHAR *bytes = static_cast<SQLCHAR*>(static_cast<void*>(PyBytes_AsString(baseObj)));

			strLenOrNullMap[row] = size;

			// Append the bytes data into the full column data
			//
			columnData->insert(columnData->end(), bytes, bytes + size);

			// Store the maximum length to find the widest the column needs to be
			//
			if (maxLen < strLenOrNullMap[row])
			{
				maxLen = strLenOrNullMap[row];
			}
		}
		else
		{
			strLenOrNullMap[row] = SQL_NULL_DATA;
			nullable = SQL_NULLABLE;
		}
	}

	columnSize = maxLen;
	if (m_rowsNumber > 0)
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
// Name: PythonOutputDataSet::RetrieveDateTimeColumnFromDataFrame
//
// Description:
//  Gets date and datetime column information from the underlying DataFrame,
//  adds data to m_data and nullmap to m_columnNullMap. 
//
template<class DateTimeStruct>
void PythonOutputDataSet::RetrieveDateTimeColumnFromDataFrame(
	string      columnName,
	SQLULEN     &columnSize,
	SQLSMALLINT &decimalDigits,
	SQLSMALLINT &nullable)
{
	LOG("PythonOutputDataSet::RetrieveDateTimeColumnFromDataFrame");

	DateTimeStruct *columnData = nullptr;
	SQLINTEGER *strLenOrNullMap = nullptr;
	if (m_rowsNumber > 0)
	{
		columnData = new DateTimeStruct[m_rowsNumber];
		strLenOrNullMap = new SQLINTEGER[m_rowsNumber];
	}

	decimalDigits = 0;
	nullable = SQL_NO_NULLS;
	
	// Get the column as a list of Timestamp objects.
	//
	string script = "list(" + m_name + "['" + columnName + "'])";
	py::list column = py::extract<py::list>(py::eval(script.c_str(), m_mainNamespace));

	for (SQLULEN row = 0; row < m_rowsNumber; ++row)
	{
		py::object pyObj = column[row];

		// Make sure the iterator is not pointing at Python None, or else it will crash on extract
		//
		if (!pyObj.is_none())
		{
			SQL_TIMESTAMP_STRUCT timestamp = ExtractTimestampFromPyObject(pyObj.ptr());
			
			if (is_same<DateTimeStruct, SQL_TIMESTAMP_STRUCT>::value)
			{
				// Cast columnData pointer to SQL_TIMESTAMP_STRUCT so we can assign a TIMESTAMP_STRUCT to it.
				// This prevents compile errors where DateTimeStruct is SQL_DATE_STRUCT.
				//
				SQL_TIMESTAMP_STRUCT *typedColumnData = reinterpret_cast<SQL_TIMESTAMP_STRUCT *>(columnData);

				// TIMESTAMP_STRUCT stores "fraction" as nanoseconds, we change microseconds to nanosec
				//
				typedColumnData[row] = timestamp;
			}
			else 
			{
				SQL_DATE_STRUCT *typedColumnData = reinterpret_cast<SQL_DATE_STRUCT *>(columnData);

				typedColumnData[row] = { timestamp.year, timestamp.month, timestamp.day };
			}
		
			strLenOrNullMap[row] = sizeof(DateTimeStruct);
		}
		else
		{
			strLenOrNullMap[row] = SQL_NULL_DATA;
			nullable = SQL_NULLABLE;
		}
	}

	columnSize = sizeof(DateTimeStruct);
	if (m_rowsNumber > 0)
	{
		m_data.push_back(static_cast<SQLPOINTER>(columnData));
	}
	else
	{
		m_data.push_back(nullptr);
	}

	m_columnNullMap.push_back(strLenOrNullMap);
}

//--------------------------------------------------------------------------------------------------
// Name: PythonOutputDataSet::ExtractArrayFromDataFrame
//
// Description:
//  Extracts a numpy ndarray from the pandas DataFrame in the python namespace
//
np::ndarray PythonOutputDataSet::ExtractArrayFromDataFrame(const string columnName)
{
	string getNumpyArrayScript = "np.array(" + m_name + "['" + columnName + "'], copy=False)";

	return py::extract<np::ndarray>(py::eval(getNumpyArrayScript.c_str(), m_mainNamespace));
}

//--------------------------------------------------------------------------------------------------
// Name: PythonOutputDataSet::ExtractTimestampFromPyObject
//
// Description:
//  Extract all the time stamp data from a PyObject and return a TIMESTAMP_STRUCT.
//  Because TIMESTAMP is the most general, we return TIMESTAMP even for Date objects, with defaults.
//
SQL_TIMESTAMP_STRUCT PythonOutputDataSet::ExtractTimestampFromPyObject(const PyObject *dateObject)
{
	// Import the PyDateTime API
	//
	PyDateTime_IMPORT;

	// Make sure this PyObject is in the PyDateTime family so that we can extract data
	//
	if (!PyDate_Check(dateObject) &&
		!PyDateTime_Check(dateObject))
	{
		throw runtime_error("Failed to extract timestamp from python object");
	}

	// Set default values in case of DATE object
	//
	SQLSMALLINT year = 1970;
	SQLUSMALLINT month = 1;
	SQLUSMALLINT day = 1;
	SQLUSMALLINT hour = 0;
	SQLUSMALLINT minute = 0;
	SQLUSMALLINT second = 0;
	SQLUINTEGER usec = 0;

	// If this is a PyDate, we only need year/month/day values.
	// PyDateTime is also a PyDate so will return true on this check.
	//
	if (PyDate_Check(dateObject))
	{
		year = PyDateTime_GET_YEAR(dateObject);
		month = PyDateTime_GET_MONTH(dateObject);
		day = PyDateTime_GET_DAY(dateObject);
	}
	
	// If dateObject is a Date (not DateTime) then we can't extract the time values.
	//
	if (PyDateTime_Check(dateObject))
	{
		hour = PyDateTime_DATE_GET_HOUR(dateObject);
		minute = PyDateTime_DATE_GET_MINUTE(dateObject);
		second = PyDateTime_DATE_GET_SECOND(dateObject);
		usec = PyDateTime_DATE_GET_MICROSECOND(dateObject);
	}

	// TIMESTAMP_STRUCT stores "fraction" as nanoseconds, so multiply microseconds by 1000
	//
	return { year, month, day, hour, minute, second, usec * 1000 };
}

//--------------------------------------------------------------------------------------------------
// Name: PythonOutputDataSet::PopulateColumnsDataType
//
// Description:
//  Finds the data type for each column and stores it in the member m_columnsDataType.
//
void PythonOutputDataSet::PopulateColumnsDataType()
{
	LOG("PythonOutputDataSet::PopulateColumnsDataType");

	SQLUSMALLINT numberOfCols = GetDataFrameColumnsNumber();

	for (SQLUSMALLINT columnNumber = 0; columnNumber < numberOfCols; ++columnNumber)
	{
		SQLSMALLINT dataType = PopulateColumnDataType(columnNumber);

		m_columnsDataType.push_back(dataType);
	}
}

//--------------------------------------------------------------------------------------------------
// Name: PythonOutputDataSet::PopulateColumnDataType
//
// Description:
//  Gets the python type from the value in the column,
//  then get the column data type by looking up the python to odbc type map.
//
SQLSMALLINT PythonOutputDataSet::PopulateColumnDataType(SQLUSMALLINT columnNumber) const
{
	LOG("PythonOutputDataSet::PopulateColumnDataType");

	string getColumnString = "np.array(" + m_name + "[" + m_name + ".columns[" +
		to_string(columnNumber) + "]], copy=False)";

	np::ndarray column = py::extract<np::ndarray>(py::eval(getColumnString.c_str(), m_mainNamespace));

	np::dtype dType = column.get_dtype();
	string type = "NoneType";

	if(!np::equivalent(dType, m_ObjType))
	{
		type = py::extract<string>(py::str(dType));
	}
	else
	{
		// Create an iterator over the column values.
		// The uninitialized "end" iterator is equivalent to the end of the list.
		//
		py::stl_input_iterator<py::object> itVal(column), end;
		while(type == "NoneType" && itVal != end)
		{
			type = itVal->ptr()->ob_type->tp_name;
			++itVal;
		}
	}

	PythonDataSet::pythonToOdbcTypeMap::const_iterator it =
		PythonDataSet::m_pythonToOdbcTypeMap.find(type);

	if (it == PythonDataSet::m_pythonToOdbcTypeMap.end())
	{
		throw invalid_argument("Unsupported data type " + type + " in output data for column # "
			+ to_string(columnNumber) + ".");
	}

	SQLSMALLINT dataType = it->second;

	return dataType;
}

//--------------------------------------------------------------------------------------------------
// Name: PythonOutputDataSet::PopulateNumberOfRows
//
// Description:
//  Set the number of rows from the underlying DataFrame
//
void PythonOutputDataSet::PopulateNumberOfRows()
{
	LOG("PythonOutputDataSet::PopulateNumberOfRows");
	string getNumRowsScript = "len(" + m_name + ".index)";
	m_rowsNumber = py::extract<int>(py::eval(getNumRowsScript.c_str(), m_mainNamespace));
}

//--------------------------------------------------------------------------------------------------
// Name: PythonOutputDataSet::CleanupColumns
//
// Description:
//  Looks up the CleanupColumnFnMap to find the the respective CleanupColumn function for every
//  column and calls that.
//
void PythonOutputDataSet::CleanupColumns()
{
	LOG("PythonOutputDataSet::CleanupColumns");

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
// Name: PythonOutputDataSet::CleanupColumn
//
// Description:
//  For the given columnNumber and SQLType, cleans up the data buffer used to hold the data
//  before being sent to ExtHost. Also cleans up the columnNullMap.
//
template<class SQLType>
void PythonOutputDataSet::CleanupColumn(SQLUSMALLINT columnNumber)
{
	LOG("PythonOutputDataSet::CleanupColumn");

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
