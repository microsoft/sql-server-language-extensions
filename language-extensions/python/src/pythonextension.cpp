//*********************************************************************
//                Copyright (C) Microsoft Corporation.
//
// @File: pythonextension.cpp
//
// Purpose:
//	 Python extension DLL that can be loaded by ExtHost. This library loads the
//	 Python dll, handles communication with ExtHost, and executes user-specified
//	 Python script
//
//*********************************************************************

// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifdef _WIN64
#include <windows.h>
#endif
#include <boost/python.hpp>
#include <iostream>
#include <string>
#include <ctime>
#include <sqltypes.h>
#include <sqlext.h>
#include "sqlexternallanguage.h"
#include "Logger.h"

using namespace std;

//--------------------------------------------------------------------------------------------------
// Name: GetInterfaceVersion
//
// Description:
//	Returns the API interface version for the extension
//
// Returns:
//	EXTERNAL_LANGUAGE_EXTENSION_API
//
SQLUSMALLINT
GetInterfaceVersion()
{
	return EXTERNAL_LANGUAGE_EXTENSION_API;
}

//--------------------------------------------------------------------------------------------------
// Name: Init
//
// Description:
//	Initialize the python extension. Until registration, nothing is needed here.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN Init(
	SQLCHAR *ExtensionParams,
	SQLULEN ExtensionParamsLength,
	SQLCHAR *ExtensionPath,
	SQLULEN ExtensionPathLength,
	SQLCHAR *PublicLibraryPath,
	SQLULEN PublicLibraryPathLength,
	SQLCHAR *PrivateLibraryPath,
	SQLULEN PrivateLibraryPathLength
)
{
	LOG("PythonExtension::Init");

	return SQL_SUCCESS;
}


//--------------------------------------------------------------------------------------------------
// Name: InitSession
//
// Description:
//	Initializes session-specific data. We store the schema and find the main class and
//	method to execute here.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN InitSession(
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT NumTasks,
	SQLCHAR		 *Script,
	SQLULEN		 ScriptLength,
	SQLUSMALLINT InputSchemaColumnsNumber,
	SQLUSMALLINT ParametersNumber,
	SQLCHAR		 *InputDataName,
	SQLUSMALLINT InputDataNameLength,
	SQLCHAR		 *OutputDataName,
	SQLUSMALLINT OutputDataNameLength
)
{
	LOG("InitSession");
	return SQL_SUCCESS;
}

//--------------------------------------------------------------------------------------------------
// Name: InitColumn
//
// Description:
//	Initializes column-specific data. We store the name and the data type of the column
//  here.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN InitColumn(
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ColumnNumber,
	SQLCHAR		 *ColumnName,
	SQLSMALLINT	 ColumnNameLength,
	SQLSMALLINT	 DataType,
	SQLULEN		 ColumnSize,
	SQLSMALLINT	 DecimalDigits,
	SQLSMALLINT	 Nullable,
	SQLSMALLINT	 PartitionByNumber,
	SQLSMALLINT	 OrderByNumber
)
{
	LOG("InitColumn");
	return SQL_SUCCESS;
}

//--------------------------------------------------------------------------------------------------
// Name: InitParam
//
// Description:
//	Initializes parameter-specific data.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN InitParam(
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ParamNumber,
	SQLCHAR		 *ParamName,
	SQLSMALLINT	 ParamNameLength,
	SQLSMALLINT	 DataType,
	SQLULEN		 ParamSize,
	SQLSMALLINT	 DecimalDigits,
	SQLPOINTER	 ParamValue,
	SQLINTEGER	 StrLen_or_Ind,
	SQLSMALLINT	 InputOutputType)
{
	LOG("InitParam");
	return SQL_SUCCESS;
}

//--------------------------------------------------------------------------------------------------
// Name: Execute
//
// Description:
//  Given the data from ExtHost, convert and populate the arrays in the user python program. Then,
//  execute the script and retrieve the output schema and convert the data back to
//  ODBC types.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN Execute(
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId,
	SQLULEN		 RowsNumber,
	SQLPOINTER	 *Data,
	SQLINTEGER	 **StrLen_or_Ind,
	SQLUSMALLINT *OutputSchemaColumnsNumber
)
{
	LOG("Execute");
	return SQL_SUCCESS;
}

SQLEXTENSION_INTERFACE
SQLRETURN GetResultColumn(
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ColumnNumber,
	SQLSMALLINT	 *DataType,
	SQLULEN		 *ColumnSize,
	SQLSMALLINT	 *DecimalDigits,
	SQLSMALLINT	 *Nullable
)
{
	LOG("GetResultColumn");
	return SQL_SUCCESS;
}

SQLEXTENSION_INTERFACE
SQLRETURN GetResults(
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId,
	SQLULEN		 *RowsNumber,
	SQLPOINTER	 **Data,
	SQLINTEGER	 ***StrLen_or_Ind
)
{
	LOG("GetResults");
	return SQL_SUCCESS;
}

SQLEXTENSION_INTERFACE
SQLRETURN GetOutputParam(
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ParamNumber,
	SQLPOINTER	 *ParamValue,
	SQLINTEGER	 *StrLen_or_Ind
)
{
	LOG("GetOutputParam");
	return SQL_SUCCESS;
}

SQLEXTENSION_INTERFACE
SQLRETURN CleanupSession(
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId
)
{
	LOG("CleanupSession");
	return SQL_SUCCESS;
}

SQLEXTENSION_INTERFACE
SQLRETURN Cleanup()
{
	LOG("Cleanup");
	return SQL_SUCCESS;
}
