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

#include <datetime.h>

using namespace std;
namespace bp = boost::python;

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
		{static_cast<SQLSMALLINT>(SQL_C_TYPE_TIMESTAMP),
		 static_cast<fnCheckColumnEquality>(&PythonExtensionApiTests::CheckDateTimeColumnEquality<SQL_TIMESTAMP_STRUCT>)},
		{static_cast<SQLSMALLINT>(SQL_C_TYPE_DATE),
		 static_cast<fnCheckColumnEquality>(&PythonExtensionApiTests::CheckDateTimeColumnEquality<SQL_DATE_STRUCT>)},
	};

	// Name: SetUp
	//
	// Description:
	//  Code here will be called immediately after the constructor (right
	//  before each test).
	//
	void PythonExtensionApiTests::SetUp()
	{
		Py_Initialize();

		boost::python::numpy::initialize();

		SetupVariables();
	}

	// Name: TearDown
	//
	// Description:
	//  Code here will be called immediately after each test 
	//  (right before the destructor).
	//
	void PythonExtensionApiTests::TearDown()
	{
		DoCleanup();
	}

	// Name: SetupVariables
	//
	// Description:
	//  Set up default, valid variables for use in tests
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

		m_integerInfo = make_unique<ColumnInfo<SQLINTEGER>>(
			"IntegerColumn1",
			vector<SQLINTEGER>{ 0, 1, 2, 3, 4},
			vector<SQLINTEGER>(ColumnInfo<SQLINTEGER>::sm_rowsNumber, m_IntSize),
			"IntegerColumn2",
			vector<SQLINTEGER>{ m_MaxInt, m_MinInt, 0, 0, -1 },
			vector<SQLINTEGER>{ m_IntSize, m_IntSize, SQL_NULL_DATA, SQL_NULL_DATA, m_IntSize });

		m_booleanInfo = make_unique<ColumnInfo<SQLCHAR>>(
			"BooleanColumn1",
			vector<SQLCHAR>{ '1', '0', '1', 0, 1 },
			vector<SQLINTEGER>(ColumnInfo<SQLCHAR>::sm_rowsNumber, m_BooleanSize),
			"BooleanColumn2",
			vector<SQLCHAR>{ '\0', '2', '1', '0', '\0' },
			vector<SQLINTEGER>{ SQL_NULL_DATA, m_BooleanSize, m_BooleanSize, m_BooleanSize, SQL_NULL_DATA });

		m_realInfo = make_unique<ColumnInfo<SQLREAL>>(
			"RealColumn1",
			vector<SQLREAL>{ 0.34F, 1.33F, m_MaxReal, m_MinReal, 68e10F },
			vector<SQLINTEGER>(ColumnInfo<SQLREAL>::sm_rowsNumber, m_RealSize),
			"RealColumn2",
			vector<SQLREAL>{  0, -1, NAN, NAN, NAN },
			vector<SQLINTEGER>{ m_RealSize, m_RealSize, SQL_NULL_DATA, SQL_NULL_DATA, SQL_NULL_DATA });

		m_doubleInfo = make_unique<ColumnInfo<SQLDOUBLE>>(
			"DoubleColumn1",
			vector<SQLDOUBLE>{ -1.79e301, 1.33, m_MaxDouble, m_MinDouble, 1.79e30 },
			vector<SQLINTEGER>(ColumnInfo<SQLDOUBLE>::sm_rowsNumber, m_DoubleSize),
			"DoubleColumn2",
			vector<SQLDOUBLE>{  0, -1, NAN, NAN, NAN },
			vector<SQLINTEGER>{ m_DoubleSize, m_DoubleSize, SQL_NULL_DATA, SQL_NULL_DATA, SQL_NULL_DATA });

		m_bigIntInfo = make_unique<ColumnInfo<SQLBIGINT>>(
			"BigIntColumn1",
			vector<SQLBIGINT>{ m_MaxBigInt, 1, 88883939, m_MinBigInt, -622280108 },
			vector<SQLINTEGER>(ColumnInfo<SQLBIGINT>::sm_rowsNumber, m_BigIntSize),
			"BigIntColumn2",
			vector<SQLBIGINT>{0, 0, 0, 12341512213, -12341512213 },
			vector<SQLINTEGER>{ SQL_NULL_DATA, SQL_NULL_DATA,
			SQL_NULL_DATA, m_BigIntSize, m_BigIntSize });

		m_smallIntInfo = make_unique<ColumnInfo<SQLSMALLINT>>(
			"SmallIntColumn1",
			vector<SQLSMALLINT>{ 223, 33, 9811, -725, 6810 },
			vector<SQLINTEGER>(ColumnInfo<SQLSMALLINT>::sm_rowsNumber, m_SmallIntSize),
			"SmallIntColumn2",
			vector<SQLSMALLINT>{ m_MaxSmallInt, m_MinSmallInt, 0, 0, 3'276 },
			vector<SQLINTEGER>{ m_SmallIntSize, m_SmallIntSize,
			SQL_NULL_DATA, SQL_NULL_DATA, m_SmallIntSize });

		m_tinyIntInfo = make_unique<ColumnInfo<SQLCHAR>>(
			"TinyIntColumn1",
			vector<SQLCHAR>{ 34, 133, 98, 72, 10 },
			vector<SQLINTEGER>(ColumnInfo<SQLCHAR>::sm_rowsNumber, m_TinyIntSize),
			"TinyIntColumn2",
			vector<SQLCHAR>{ m_MaxTinyInt, m_MinTinyInt, 0, 0, 128 },
			vector<SQLINTEGER>{ m_TinyIntSize, m_TinyIntSize,
			SQL_NULL_DATA, SQL_NULL_DATA, m_TinyIntSize });

		m_dateTimeInfo = make_unique<ColumnInfo<SQL_TIMESTAMP_STRUCT>>(
			"DateTimeColumn1",
			vector<SQL_TIMESTAMP_STRUCT>{
				{ 9518, 8, 25, 19, 11, 40, 528934000 },
				{ 5712, 3, 9, 2, 24, 32, 770483000 },
				{ 1470, 7, 27, 17, 47, 52, 123456000 },
				{ 2020, 4, 16, 15, 5, 12, 169012000 },
				{ 231, 2, 14, 22, 36, 18, 489102000 },
			},
			vector<SQLINTEGER>(ColumnInfo<SQL_TIMESTAMP_STRUCT>::sm_rowsNumber, m_DateTimeSize),
			"DateTimeColumn2",
			vector<SQL_TIMESTAMP_STRUCT>{
				{ 9999, 12, 31, 23, 59, 59, 999999000 },
				{ 1,1,1,0,0,0,0 },
				{},
				{},
				{}
			},
			vector<SQLINTEGER>{ m_DateTimeSize, m_DateTimeSize,
			SQL_NULL_DATA, SQL_NULL_DATA, SQL_NULL_DATA });

		m_dateInfo = make_unique<ColumnInfo<SQL_DATE_STRUCT>>(
			"DateColumn1",
			vector<SQL_DATE_STRUCT>{
				{ 9518, 8, 25 },
				{ 5712, 3, 9 },
				{ 1470, 7, 27 },
				{ 2020, 4, 16 },
				{ 231, 2, 14, },
			},
			vector<SQLINTEGER>(ColumnInfo<SQL_DATE_STRUCT>::sm_rowsNumber, m_DateSize),
			"DateColumn2",
			vector<SQL_DATE_STRUCT>{
				{ 9999, 12, 31 },
				{ 1,1,1 },
				{},
				{},
				{}
			},
			vector<SQLINTEGER>{ m_DateSize, m_DateSize,
			SQL_NULL_DATA, SQL_NULL_DATA, SQL_NULL_DATA });

		try
		{
			m_mainModule = bp::import("__main__");
			m_mainNamespace = m_mainModule.attr("__dict__");
		}
		catch (bp::error_already_set&)
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

	// Name: InitializeSession
	//
	// Description:
	//  Initialize a valid, default session for non-Init tests
	//  Tests InitSession API
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


	// Name: DoCleanup
	//
	// Description:
	//  Call Cleanup on the PythonExtension.
	//  Testing if Cleanup is implemented correctly.
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
	template void PythonExtensionApiTests::InitializeColumns<SQL_TIMESTAMP_STRUCT, SQL_C_TYPE_TIMESTAMP>(
		ColumnInfo<SQL_TIMESTAMP_STRUCT> *ColumnInfo);
	template void PythonExtensionApiTests::InitializeColumns<SQL_DATE_STRUCT, SQL_C_TYPE_DATE>(
		ColumnInfo<SQL_DATE_STRUCT> *ColumnInfo);

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

	// Name: GetWStringLength
	//
	// Description:
	//  Utility function to get the length of a wchar_t *.
	//  wcslen does not work in Linux with -fshort-wchar, so we use this function instead.
	//
	SQLULEN PythonExtensionApiTests::GetWStringLength(const wchar_t *wstr)
	{
		SQLULEN distance = -1;

		// If nullptr, return
		//
		if (wstr)
		{
			// Get distance from end of string to beginning
			//
			const wchar_t *newstr = wstr;
			while (*newstr)
			{
				++newstr;
			}

			distance = newstr - wstr;
		}

		return distance;
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
		bp::dict   columnToTest,
		void       *expectedColumn,
		SQLINTEGER *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<SQLULEN>(bp::len(columnToTest)), expectedRowsNumber);

		for (SQLULEN index = 0; index < expectedRowsNumber; ++index)
		{
			bp::object val = columnToTest[index];

			if (strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA)
			{
				EXPECT_TRUE(val.is_none());
			}
			else
			{
				SQLType typeVal = bp::extract<SQLType>(val);
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
		bp::dict   columnToTest,
		void       *expectedColumn,
		SQLINTEGER *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<SQLULEN>(bp::len(columnToTest)), expectedRowsNumber);

		for (SQLULEN index = 0; index < expectedRowsNumber; ++index)
		{
			bp::object val = columnToTest[index];

			if (strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA)
			{
				EXPECT_TRUE(val.is_none());
			}
			else
			{
				bool expectedValue = static_cast<bool*>(expectedColumn)[index];
				bool typeVal = bp::extract<bool>(val);
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
		bp::dict   columnToTest,
		void       *expectedColumn,
		SQLINTEGER *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<SQLULEN>(bp::len(columnToTest)), expectedRowsNumber);

		SQLINTEGER cumulativeLength = 0;

		for (SQLULEN index = 0; index < expectedRowsNumber; ++index)
		{
			bp::object val = columnToTest[index];
			if (strLen_or_Ind == nullptr ||
				(strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA))
			{
				EXPECT_TRUE(val.is_none());
			}
			else
			{
				string typeVal = bp::extract<string>(val);
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
		bp::dict   columnToTest,
		void       *expectedColumn,
		SQLINTEGER *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<SQLULEN>(bp::len(columnToTest)), expectedRowsNumber);

		SQLINTEGER cumulativeLength = 0;

		for (SQLULEN index = 0; index < expectedRowsNumber; ++index)
		{
			bp::object val = columnToTest[index];
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
		bp::dict   columnToTest,
		void       *expectedColumn,
		SQLINTEGER *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<SQLULEN>(bp::len(columnToTest)), expectedRowsNumber);

		SQLINTEGER cumulativeLength = 0;

		for (SQLULEN index = 0; index < expectedRowsNumber; ++index)
		{
			bp::object val = columnToTest[index];

			if (strLen_or_Ind == nullptr ||
				(strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA))
			{
				EXPECT_TRUE(val.is_none());
			}
			else
			{
				SQLCHAR *expectedValue = static_cast<SQLCHAR *>(expectedColumn) + cumulativeLength;
				SQLCHAR *bytes = static_cast<SQLCHAR *>(static_cast<void *>(PyBytes_AsString(val.ptr())));

				for (SQLINTEGER i = 0; i < strLen_or_Ind[index]; ++i)
				{
					EXPECT_EQ(bytes[i], expectedValue[i]);
				}

				cumulativeLength += strLen_or_Ind[index];
			}
		}
	}

	// Name: CheckDateTimeColumnEquality
	//
	// Description:
	//  Compare datetime column with the given data and corresponding strLen_or_Ind.
	//
	template<class DateTimeStruct>
	void PythonExtensionApiTests::CheckDateTimeColumnEquality(
		SQLULEN    expectedRowsNumber,
		bp::dict   columnToTest,
		void       *expectedColumn,
		SQLINTEGER *strLen_or_Ind)
	{
		ASSERT_EQ(static_cast<SQLULEN>(bp::len(columnToTest)), expectedRowsNumber);

		DateTimeStruct *expectedDateTimeColumn = static_cast<DateTimeStruct *>(expectedColumn);

		for (SQLULEN index = 0; index < expectedRowsNumber; ++index)
		{
			bp::object val = columnToTest[index];

			if (strLen_or_Ind != nullptr && strLen_or_Ind[index] == SQL_NULL_DATA)
			{
				EXPECT_TRUE(val.is_none() || 
					strcmp(val.ptr()->ob_type->tp_name, "NaTType") == 0);
			}
			else
			{
				// Import the PyDateTime API
				//
				PyDateTime_IMPORT;

				PyObject *dateObject = val.ptr();

				DateTimeStruct expectedValue = expectedDateTimeColumn[index];

				SQLSMALLINT year = PyDateTime_GET_YEAR(dateObject);
				SQLUSMALLINT month = PyDateTime_GET_MONTH(dateObject);
				SQLUSMALLINT day = PyDateTime_GET_DAY(dateObject);

				if constexpr (is_same_v<DateTimeStruct, SQL_TIMESTAMP_STRUCT>)
				{
					EXPECT_TRUE(PyDateTime_CheckExact(dateObject));
					SQLUSMALLINT hour = PyDateTime_DATE_GET_HOUR(dateObject);
					SQLUSMALLINT minute = PyDateTime_DATE_GET_MINUTE(dateObject);
					SQLUSMALLINT second = PyDateTime_DATE_GET_SECOND(dateObject);
					SQLUINTEGER usec = PyDateTime_DATE_GET_MICROSECOND(dateObject);

					EXPECT_EQ(expectedValue.year, year);
					EXPECT_EQ(expectedValue.month, month);
					EXPECT_EQ(expectedValue.day, day);
					EXPECT_EQ(expectedValue.hour, hour);
					EXPECT_EQ(expectedValue.minute, minute);
					EXPECT_EQ(expectedValue.second, second);
					EXPECT_EQ(expectedValue.fraction, usec * 1000);
				}
				else
				{
					EXPECT_TRUE(PyDate_CheckExact(dateObject));

					EXPECT_EQ(expectedValue.year, year);
					EXPECT_EQ(expectedValue.month, month);
					EXPECT_EQ(expectedValue.day, day);
				}
			}
		}
	}
}
