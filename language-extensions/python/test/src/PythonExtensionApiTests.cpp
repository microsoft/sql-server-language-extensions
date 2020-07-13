//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonExtensionApiTests.cpp
//
// Purpose:
//  Define the common member functions of the PythonExtensionApiTests class
//
//*************************************************************************************************
#include "PythonExtensionApiTests.h"

using namespace std;
namespace py = boost::python;

namespace ExtensionApiTest
{
	// Function map - maps a SQL data type to the appropriate function that
	// adds a column to the dictionary
	//
	const PythonExtensionApiTests::CheckColumnEqualityFnMap PythonExtensionApiTests::sm_FnCheckColumnEqualityMap =
	{
		{static_cast<SQLSMALLINT>(SQL_C_BIT),
		 static_cast<fnCheckColumnEquality>(&PythonExtensionApiTests::CheckBooleanColumnEquality)},
		{static_cast<SQLSMALLINT>(SQL_C_SLONG),
		 static_cast<fnCheckColumnEquality>(&PythonExtensionApiTests::CheckColumnEquality<SQLINTEGER>)},
		{static_cast<SQLSMALLINT>(SQL_C_DOUBLE),
		 static_cast<fnCheckColumnEquality>(&PythonExtensionApiTests::CheckColumnEquality<SQLDOUBLE>)},
		{static_cast<SQLSMALLINT>(SQL_C_FLOAT),
		 static_cast<fnCheckColumnEquality>(&PythonExtensionApiTests::CheckColumnEquality<SQLREAL>)},
		{static_cast<SQLSMALLINT>(SQL_C_SSHORT),
		 static_cast<fnCheckColumnEquality>(&PythonExtensionApiTests::CheckColumnEquality<SQLSMALLINT>)},
		{static_cast<SQLSMALLINT>(SQL_C_UTINYINT),
		 static_cast<fnCheckColumnEquality>(&PythonExtensionApiTests::CheckColumnEquality<SQLCHAR>)},
		{static_cast<SQLSMALLINT>(SQL_C_SBIGINT),
		 static_cast<fnCheckColumnEquality>(&PythonExtensionApiTests::CheckColumnEquality<SQLBIGINT>)},
		{static_cast<SQLSMALLINT>(SQL_C_CHAR),
		 static_cast<fnCheckColumnEquality>(&PythonExtensionApiTests::CheckStringColumnEquality)},
		{static_cast<SQLSMALLINT>(SQL_C_WCHAR),
		 static_cast<fnCheckColumnEquality>(&PythonExtensionApiTests::CheckWStringColumnEquality)},
		{static_cast<SQLSMALLINT>(SQL_C_BINARY),
		 static_cast<fnCheckColumnEquality>(&PythonExtensionApiTests::CheckRawColumnEquality)},
	};

	// Code here will be called immediately after the constructor (right
	// before each test).
	//
	void PythonExtensionApiTests::SetUp()
	{
		Py_Initialize();

		boost::python::numpy::initialize();

		SetupVariables();
	}

	// Code here will be called immediately after each test (right
	// before the destructor).
	//
	void PythonExtensionApiTests::TearDown()
	{
		DoCleanup();
	}

	// Set up default, valid variables for use in tests
	//
	void PythonExtensionApiTests::SetupVariables()
	{
		m_sessionId = new SQLGUID();
		m_taskId = 0;
		m_numTasks = 1;

		m_scriptString = "print('" + m_printMessage + "');"
			"OutputDataSet = InputDataSet;"
			"print('InputDataSet:'); print(InputDataSet);"
			"print('OutputDataSet:'); print(OutputDataSet);";
		m_script = static_cast<SQLCHAR *>(static_cast<void *>(const_cast<char *>(m_scriptString.c_str())));
		m_scriptLength = m_scriptString.length();

		m_inputDataNameString = "InputDataSet";
		m_inputDataName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_inputDataNameString.c_str())));
		m_inputDataNameLength = m_inputDataNameString.length();

		m_outputDataNameString = "OutputDataSet";
		m_outputDataName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(m_outputDataNameString.c_str())));
		m_outputDataNameLength = m_outputDataNameString.length();

		const SQLINTEGER intSize = sizeof(SQLINTEGER);
		m_integerInfo = make_unique<ColumnInfo<SQLINTEGER>>(
			"IntegerColumn1",
			vector<SQLINTEGER>{ 0, 1, 2, 3, 4},
			vector<SQLINTEGER>(ColumnInfo<SQLINTEGER>::m_rowsNumber, intSize),
			"IntegerColumn2",
			vector<SQLINTEGER>{ m_MaxInt, m_MinInt, 0, 0, -1 },
			vector<SQLINTEGER>{ intSize, intSize, SQL_NULL_DATA, SQL_NULL_DATA, intSize });

		const SQLINTEGER booleanSize = sizeof(SQLCHAR);
		m_booleanInfo = make_unique<ColumnInfo<SQLCHAR>>(
			"BooleanColumn1",
			vector<SQLCHAR>{ '1', '0', '1', '0', '1' },
			vector<SQLINTEGER>(ColumnInfo<SQLCHAR>::m_rowsNumber, booleanSize),
			"BooleanColumn2",
			vector<SQLCHAR>{ '\0', '2', '1', '0', '\0' },
			vector<SQLINTEGER>{ SQL_NULL_DATA, booleanSize, booleanSize, booleanSize, SQL_NULL_DATA });

		const SQLINTEGER realSize = sizeof(SQLREAL);
		m_realInfo = make_unique<ColumnInfo<SQLREAL>>(
			"RealColumn1",
			vector<SQLREAL>{ 0.34F, 1.33F, m_MaxReal, m_MinReal, 68e10F },
			vector<SQLINTEGER>(ColumnInfo<SQLREAL>::m_rowsNumber, realSize),
			"RealColumn2",
			vector<SQLREAL>{  0, -1, NAN, NAN, NAN },
			vector<SQLINTEGER>{ realSize, realSize, SQL_NULL_DATA, SQL_NULL_DATA, SQL_NULL_DATA });

		const SQLINTEGER doubleSize = sizeof(SQLDOUBLE);
		m_doubleInfo = make_unique<ColumnInfo<SQLDOUBLE>>(
			"DoubleColumn1",
			vector<SQLDOUBLE>{ -1.79e301, 1.33, m_MaxDouble, m_MinDouble, 1.79e30 },
			vector<SQLINTEGER>(ColumnInfo<SQLDOUBLE>::m_rowsNumber, doubleSize),
			"DoubleColumn2",
			vector<SQLDOUBLE>{  0, -1, NAN, NAN, NAN },
			vector<SQLINTEGER>{ doubleSize, doubleSize, SQL_NULL_DATA, SQL_NULL_DATA, SQL_NULL_DATA });

		const SQLINTEGER bigIntSize = sizeof(SQLBIGINT);
		m_bigIntInfo = make_unique<ColumnInfo<SQLBIGINT>>(
			"BigIntColumn1",
			vector<SQLBIGINT>{ m_MaxBigInt, 1, 88883939, m_MinBigInt, -622280108 },
			vector<SQLINTEGER>(ColumnInfo<SQLBIGINT>::m_rowsNumber, bigIntSize),
			"BigIntColumn2",
			vector<SQLBIGINT>{0, 0, 0, 12341512213, -12341512213 },
			vector<SQLINTEGER>{ SQL_NULL_DATA, SQL_NULL_DATA,
			SQL_NULL_DATA, bigIntSize, bigIntSize });

		const SQLINTEGER smallIntSize = sizeof(SQLSMALLINT);
		m_smallIntInfo = make_unique<ColumnInfo<SQLSMALLINT>>(
			"SmallIntColumn1",
			vector<SQLSMALLINT>{ 223, 33, 9811, -725, 6810 },
			vector<SQLINTEGER>(ColumnInfo<SQLSMALLINT>::m_rowsNumber, smallIntSize),
			"SmallIntColumn2",
			vector<SQLSMALLINT>{ m_MaxSmallInt, m_MinSmallInt, 0, 0, 3'276 },
			vector<SQLINTEGER>{ smallIntSize, smallIntSize,
			SQL_NULL_DATA, SQL_NULL_DATA, smallIntSize });

		const SQLINTEGER tinyIntSize = sizeof(SQLCHAR);
		m_tinyIntInfo = make_unique<ColumnInfo<SQLCHAR>>(
			"TinyIntColumn1",
			vector<SQLCHAR>{ 34, 133, 98, 72, 10 },
			vector<SQLINTEGER>(ColumnInfo<SQLCHAR>::m_rowsNumber, tinyIntSize),
			"TinyIntColumn2",
			vector<SQLCHAR>{ m_MaxTinyInt, m_MinTinyInt, 0, 0, 128 },
			vector<SQLINTEGER>{ tinyIntSize, tinyIntSize,
			SQL_NULL_DATA, SQL_NULL_DATA, tinyIntSize });

		try
		{
			m_mainModule = py::import("__main__");
			m_mainNamespace = m_mainModule.attr("__dict__");
		}
		catch (py::error_already_set&)
		{
			throw runtime_error("Error loading main module and namespace");
		}

		// Check that the module and namespace are populated, not None objects
		//
		if (m_mainModule == boost::python::object() ||
			m_mainNamespace == boost::python::object())
		{
			throw runtime_error("Main module or namespace was None");
		}
	}

	// Initialize a valid, default session for non-Init tests
	// Tests InitSession API
	//
	void PythonExtensionApiTests::InitializeSession(
		SQLUSMALLINT parametersNumber,
		SQLUSMALLINT inputSchemaColumnsNumber,
		string       scriptString)
	{
		SQLRETURN result = SQL_ERROR;

		SQLCHAR *script = static_cast<SQLCHAR*>(
			static_cast<void*>(const_cast<char*>(scriptString.c_str())));

		result = Init(
			nullptr, // Extension Params
			0,       // Extension Params Length
			nullptr, // Extension Path
			0,       // Extension Path Length
			nullptr, // Public Library Path
			0,       // Public Library Path Length
			nullptr, // Private Library Path
			0        // Private Library Path Length
		);
		EXPECT_EQ(result, SQL_SUCCESS);

		result = InitSession(
			*m_sessionId,
			m_taskId,
			m_numTasks,
			script,
			scriptString.length(),
			inputSchemaColumnsNumber,
			parametersNumber,
			m_inputDataName,
			m_inputDataNameLength,
			m_outputDataName,
			m_outputDataNameLength
		);
		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Call Cleanup on the PythonExtension.
	// Testing if Cleanup is implemented correctly.
	//
	void PythonExtensionApiTests::DoCleanup()
	{
		SQLRETURN result = SQL_ERROR;

		result = CleanupSession(*m_sessionId, m_taskId);
		EXPECT_EQ(result, SQL_SUCCESS);

		result = Cleanup();
		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Name: InitializeColumns
	//
	// Description:
	//  Template function to call InitializeColumn for all columns.
	//
	template<class SQLType, SQLSMALLINT dataType>
	void PythonExtensionApiTests::InitializeColumns(ColumnInfo<SQLType> *ColumnInfo)
	{
		SQLUSMALLINT inputSchemaColumnsNumber = ColumnInfo->GetColumnsNumber();
		for (SQLUSMALLINT columnNumber = 0; columnNumber < inputSchemaColumnsNumber; ++columnNumber)
		{
			InitializeColumn(columnNumber,
				ColumnInfo->m_columnNames[columnNumber],
				dataType,
				sizeof(SQLType));
		}
	}

	// Template instantiations
	//
	template void PythonExtensionApiTests::InitializeColumns<SQLINTEGER, SQL_C_SLONG>(
		ColumnInfo<SQLINTEGER> *ColumnInfo);
	template void PythonExtensionApiTests::InitializeColumns<SQLCHAR, SQL_C_BIT>(
		ColumnInfo<SQLCHAR> *ColumnInfo);
	template void PythonExtensionApiTests::InitializeColumns<SQLREAL, SQL_C_FLOAT>(
		ColumnInfo<SQLREAL> *ColumnInfo);
	template void PythonExtensionApiTests::InitializeColumns<SQLDOUBLE, SQL_C_DOUBLE>(
		ColumnInfo<SQLDOUBLE> *ColumnInfo);
	template void PythonExtensionApiTests::InitializeColumns<SQLBIGINT, SQL_C_SBIGINT>(
		ColumnInfo<SQLBIGINT> *ColumnInfo);
	template void PythonExtensionApiTests::InitializeColumns<SQLSMALLINT, SQL_C_SSHORT>(
		ColumnInfo<SQLSMALLINT> *ColumnInfo);
	template void PythonExtensionApiTests::InitializeColumns<SQLCHAR, SQL_C_UTINYINT>(
		ColumnInfo<SQLCHAR> *ColumnInfo);

	// Name: InitializeColumn
	//
	// Description:
	//  Call InitColumn for the given columnNumber, columnName, dataType and columnSize.
	//
	void PythonExtensionApiTests::InitializeColumn(
		SQLSMALLINT columnNumber,
		string      columnNameString,
		SQLSMALLINT dataType,
		SQLULEN     columnSize)
	{
		SQLCHAR *columnName = static_cast<SQLCHAR *>(
			static_cast<void *>(const_cast<char *>(columnNameString.c_str()))
			);

		SQLRETURN result = SQL_ERROR;

		result = InitColumn(
			*m_sessionId,
			m_taskId,
			columnNumber,
			columnName,
			columnNameString.length(),
			dataType,
			columnSize,
			0,         // decimalDigits
			1,         // nullable
			-1,        // partitionByNumber
			-1);       // orderByNumber

		EXPECT_EQ(result, SQL_SUCCESS);
	}

	// Name: GenerateContiguousData
	//
	// Description:
	//  Fill a contiguous array columnData with members from the given columnVector
	//  having lengths defined in strLenOrInd, unless it is SQL_NULL_DATA.
	//
	template<class SQLType>
	vector<SQLType> PythonExtensionApiTests::GenerateContiguousData(
		vector<const SQLType*> columnVector,
		SQLINTEGER             *strLenOrInd)
	{
		vector<SQLType> retVal;

		for (SQLULEN index = 0; index < columnVector.size(); ++index)
		{
			if (strLenOrInd[index] != SQL_NULL_DATA)
			{
				SQLINTEGER strLen = strLenOrInd[index] / sizeof(SQLType);
				vector<SQLType> data(columnVector[index], columnVector[index] + strLen);
				retVal.insert(retVal.end(), data.begin(), data.end());
			}
		}

		return retVal;
	}

	template vector<char> PythonExtensionApiTests::GenerateContiguousData(
		vector<const char*> columnVector,
		SQLINTEGER          *strLenOrInd);
	template vector<SQLCHAR> PythonExtensionApiTests::GenerateContiguousData(
		vector<const SQLCHAR*> columnVector,
		SQLINTEGER             *strLenOrInd);
	template vector<wchar_t> PythonExtensionApiTests::GenerateContiguousData(
		vector<const wchar_t*> columnVector,
		SQLINTEGER             *strLenOrInd);

	// Name: GetMaxLength
	//
	// Description:
	//  Get max length of all strings from strLenOrInd.
	//
	SQLINTEGER PythonExtensionApiTests::GetMaxLength(
		SQLINTEGER *strLenOrInd,
		SQLULEN    rowsNumber)
	{
		SQLINTEGER maxLen = 0;
		for (SQLULEN index = 0; index < rowsNumber; ++index)
		{
			if (strLenOrInd[index] != SQL_NULL_DATA && maxLen < strLenOrInd[index])
			{
				maxLen = strLenOrInd[index];
			}
		}

		return maxLen;
	}

	// Name: ColumnInfo
	//
	// Description:
	//  Template constructor for the type information.
	//  Useful for ColumnInfo of integer, basic numeric and boolean types.
	//
	template<class SQLType>
	ColumnInfo<SQLType>::ColumnInfo(
		string column1Name, vector<SQLType> column1, vector<SQLINTEGER> col1StrLenOrInd,
		string column2Name, vector<SQLType> column2, vector<SQLINTEGER> col2StrLenOrInd)
	{
		m_columnNames = { column1Name, column2Name };
		m_column1 = column1;
		m_column2 = column2;
		m_dataSet = { m_column1.data(), m_column2.data() };
		m_col1StrLenOrInd = col1StrLenOrInd;
		if (m_col1StrLenOrInd.empty())
		{
			m_strLen_or_Ind.push_back(nullptr);
		}
		else
		{
			m_strLen_or_Ind.push_back(m_col1StrLenOrInd.data());
		}

		m_col2StrLenOrInd = col2StrLenOrInd;
		if (m_col2StrLenOrInd.empty())
		{
			m_strLen_or_Ind.push_back(nullptr);
		}
		else
		{
			m_strLen_or_Ind.push_back(m_col2StrLenOrInd.data());
		}

	}

	// Name: CheckColumnEquality
	//
	// Description:
	//  Template function to compare the given int/float columns for equality
	//
	template<class SQLType>
	void PythonExtensionApiTests::CheckColumnEquality(
		SQLULEN    expectedRowsNumber,
		py::dict   columnToTest,
		void       *expectedColumn,
		SQLINTEGER *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<SQLULEN>(py::len(columnToTest)), expectedRowsNumber);

		for (SQLULEN index = 0; index < expectedRowsNumber; ++index)
		{
			py::object val = columnToTest[index];

			if (strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA)
			{
				EXPECT_TRUE(val.is_none());
			}
			else
			{
				SQLType typeVal = py::extract<SQLType>(val);
				SQLType expectedValue = static_cast<SQLType*>(expectedColumn)[index];
				EXPECT_EQ(typeVal, expectedValue);
			}
		}
	}

	// Name: CheckBooleanColumnEquality
	//
	// Description:
	//  Check boolean columns for equality
	//
	void PythonExtensionApiTests::CheckBooleanColumnEquality(
		SQLULEN    expectedRowsNumber,
		py::dict   columnToTest,
		void       *expectedColumn,
		SQLINTEGER *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<SQLULEN>(py::len(columnToTest)), expectedRowsNumber);

		for (SQLULEN index = 0; index < expectedRowsNumber; ++index)
		{
			py::object val = columnToTest[index];

			if (strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA)
			{
				EXPECT_TRUE(val.is_none());
			}
			else
			{
				bool expectedValue = static_cast<bool*>(expectedColumn)[index];
				bool typeVal = py::extract<bool>(val);
				EXPECT_EQ(typeVal, expectedValue);
			}
		}
	}

	// Name: CheckStringColumnEquality
	//
	// Description:
	//  Compare string column with the given data and corresponding strLen_or_Ind.
	//  The expectedData is input as a void*, hence we input the expectedRowsNumber as well.
	//  Where strLen_or_Ind == SQL_NULL_DATA, check for None.
	//
	void PythonExtensionApiTests::CheckStringColumnEquality(
		SQLULEN    expectedRowsNumber,
		py::dict   columnToTest,
		void       *expectedColumn,
		SQLINTEGER *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<SQLULEN>(py::len(columnToTest)), expectedRowsNumber);

		SQLINTEGER cumulativeLength = 0;

		for (SQLULEN index = 0; index < expectedRowsNumber; ++index)
		{
			py::object val = columnToTest[index];
			if (strLen_or_Ind == nullptr ||
				(strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA))
			{
				EXPECT_TRUE(val.is_none());
			}
			else
			{
				string typeVal = py::extract<string>(val);
				string expectedString = string(
					static_cast<char*>(expectedColumn) + cumulativeLength,
					strLen_or_Ind[index]);

				EXPECT_EQ(typeVal, expectedString);
				cumulativeLength += strLen_or_Ind[index];
			}
		}
	}

	// Name: CheckWStringColumnEquality
	//
	// Description:
	//  Compare wstring column with the given data and corresponding strLen_or_Ind.
	//  The expectedData is input as a void*, hence we input the expectedRowsNumber as well.
	//  Where strLen_or_Ind == SQL_NULL_DATA, check for None.
	//  We have to compare byte by byte because EXPECT_EQ/STREQ do not 
	//  work properly for wstrings in Linux.
	//
	void PythonExtensionApiTests::CheckWStringColumnEquality(
		SQLULEN    expectedRowsNumber,
		py::dict   columnToTest,
		void       *expectedColumn,
		SQLINTEGER *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<SQLULEN>(py::len(columnToTest)), expectedRowsNumber);

		SQLINTEGER cumulativeLength = 0;

		for (SQLULEN index = 0; index < expectedRowsNumber; ++index)
		{
			py::object val = columnToTest[index];
			if (strLen_or_Ind == nullptr ||
				(strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA))
			{
				EXPECT_TRUE(val.is_none());
			}
			else
			{
				// Get length of the unicode object in val and make sure it is the size we expect
				//
				SQLINTEGER size = PyUnicode_GET_LENGTH(val.ptr());
				SQLINTEGER strLen = strLen_or_Ind[index] / sizeof(wchar_t);

				EXPECT_EQ(strLen, size);

				// Get a byte representation of the string as UTF16.
				// PyUnicode_AsUTF16String adds a 2-byte BOM to the front of every string, so we ignore it.
				//
				char *paramBytes = PyBytes_AsString(PyUnicode_AsUTF16String(val.ptr())) + 2;

				char *expectedParamBytes = static_cast<char*>(expectedColumn) + cumulativeLength;

				// Compare the two wstrings byte by byte
				//
				for (SQLINTEGER i = 0; i < strLen_or_Ind[index]; ++i)
				{
					EXPECT_EQ(paramBytes[i], expectedParamBytes[i]);
				}

				cumulativeLength += strLen_or_Ind[index];
			}
		}
	}

	// Name: CheckRawColumnEquality
	//
	// Description:
	//  Compare raw column with the given data and corresponding strLen_or_Ind.
	//  The expectedData is input as a void*, hence we input the expectedRowsNumber as well.
	//  Where strLen_or_Ind == SQL_NULL_DATA, check for None.
	//
	void PythonExtensionApiTests::CheckRawColumnEquality(
		SQLULEN    expectedRowsNumber,
		py::dict   columnToTest,
		void       *expectedColumn,
		SQLINTEGER *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<SQLULEN>(py::len(columnToTest)), expectedRowsNumber);

		SQLINTEGER cumulativeLength = 0;

		for (SQLULEN index = 0; index < expectedRowsNumber; ++index)
		{
			py::object val = columnToTest[index];

			if (strLen_or_Ind == nullptr ||
				(strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA))
			{
				EXPECT_TRUE(val.is_none());
			}
			else
			{
				SQLCHAR *expectedValue = static_cast<SQLCHAR*>(expectedColumn) + cumulativeLength;
				SQLCHAR *bytes = static_cast<SQLCHAR*>(static_cast<void*>(PyBytes_AsString(val.ptr())));

				for (SQLINTEGER i = 0; i < strLen_or_Ind[index]; ++i)
				{
					EXPECT_EQ(bytes[i], expectedValue[i]);
				}

				cumulativeLength += strLen_or_Ind[index];
			}
		}
	}

	// Parses the value of the active python exception
	// Type, value, and traceback are in separate pointers
	//
	string PythonExtensionApiTests::ParsePythonException()
	{
		PyObject *pType = NULL;
		PyObject *pValue = NULL;
		PyObject *pTraceback = NULL;

		// Fetch the exception info from the Python C API
		//
		PyErr_Fetch(&pType, &pValue, &pTraceback);

		// Fallback error
		//
		string ret("Unfetchable Python error");

		// If the fetch got a type pointer, parse the type into the exception string
		//
		if (pType != NULL)
		{
			string type = ExtractString(pType);

			// If a valid string extraction is available, use it
			// otherwise use fallback string
			//
			if (type.empty())
			{
				ret = "Unknown exception type";
			}
			else
			{
				ret = type;
			}
		}

		// Do the same for the exception value (the stringification of the exception)
		//
		if (pValue != NULL)
		{
			string value = ExtractString(pValue);

			if (value.empty())
			{
				ret += string(": Unparseable Python error: ");
			}
			else
			{
				ret += ": " + value;
			}
		}

		// Parse lines from the traceback using the Python traceback module
		//
		if (pTraceback != NULL)
		{
			py::handle<> handleTrace(pTraceback);

			// Load the traceback module and the format_tb function
			//
			py::object traceback(py::import("traceback"));
			py::object format_tb(traceback.attr("format_tb"));

			// Call format_tb to get a list of traceback strings
			//
			py::object traceList(format_tb(handleTrace));

			// Join the traceback strings into a single string
			//
			py::object tracePyStr(py::str("\n").join(traceList));

			// Extract the string, check the extraction, and fallback if necessary
			//
			string trace = ExtractString(tracePyStr.ptr());

			if (trace.empty())
			{
				ret += string(": Unparseable Python traceback");
			}
			else
			{
				ret += ": " + trace;
			}
		}

		return ret;
	}

	// Extract the string from a boost::python object
	//
	string PythonExtensionApiTests::ExtractString(PyObject *pObj)
	{
		string ret;
		py::handle<> handle(pObj);
		py::str pyStr(handle);

		// Extract the string from the boost::python object
		//
		py::extract<string> extracted(pyStr);

		// If a valid string extraction is available, use it
		// otherwise return empty string
		//
		if (extracted.check())
		{
			ret = extracted();
		}

		return ret;
	}

}
