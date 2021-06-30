//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: nativecsharpextension.cpp
//
// Purpose:
//  Implement the native extensibility APIs
//
//*********************************************************************
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <string>

// Header files copied from https://github.com/dotnet/core-setup
#include <coreclr_delegates.h>
#include <hostfxr.h>
#include "DotnetEnvironment.h"

#include <windows.h>
#include <sql.h>
#include "sqlexternallanguage.h"
#include "sqlexternallibrary.h"

#define nameof(x) #x
#define _export __declspec( dllexport )

using namespace std;

static DotnetEnvironment* g_dotnet_runtime = nullptr;

//--------------------------------------------------------------------------------------------------
// Name: UTF8PtrToStr
//
// Description:
//  Returns the string with the address of the first character and length
//
// Returns:
//  utf8 string
//
std::string UTF8PtrToStr(SQLCHAR* str, SQLULEN len)
{
    return std::string(reinterpret_cast<char*>(str), len);
}

//--------------------------------------------------------------------------------------------------
// Name: GetInterfaceVersion
//
// Description:
//  Returns the API interface version for the extension
//
// Returns:
//  EXTERNAL_LANGUAGE_EXTENSION_API
//
SQLUSMALLINT GetInterfaceVersion()
{
    return EXTERNAL_LANGUAGE_EXTENSION_API;
}

//--------------------------------------------------------------------------------------------------
// Name: Init
//
// Description:
//  Initializes the .NET Core C# Extension.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
_export SQLRETURN Init(
    SQLCHAR* languageParams,
    SQLULEN languageParamsLen,
    SQLCHAR* languagePath,
    SQLULEN languagePathLen,
    SQLCHAR* publicLibraryPath,
    SQLULEN publicLibraryPathLen,
    SQLCHAR* privateLibraryPath,
    SQLULEN privateLibraryPathLen)
{

    g_dotnet_runtime = new DotnetEnvironment(
        UTF8PtrToStr(languageParams, languageParamsLen),
        UTF8PtrToStr(languagePath, languagePathLen),
        UTF8PtrToStr(publicLibraryPath, publicLibraryPathLen),
        UTF8PtrToStr(privateLibraryPath, privateLibraryPathLen));

    SQLRETURN err = g_dotnet_runtime->Init();
    return err != 0 ? err :
        g_dotnet_runtime->call_managed_method<decltype(&Init)>(nameof(Init),
            languageParams,
            languageParamsLen,
            languagePath,
            languagePathLen,
            publicLibraryPath,
            publicLibraryPathLen,
            privateLibraryPath,
            privateLibraryPathLen);
}

// --------------------------------------------------------------------------------------------------
// Name: InitSession

// Description:
//  Initializes session-specific data. We store the schema, parameter info,
//  output input data info here.

// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
_export SQLRETURN InitSession(
    SQLGUID			sessionId,
    SQLUSMALLINT	taskId,
    SQLUSMALLINT	numTasks,
    SQLCHAR* script,
    SQLULEN			scriptLength,
    SQLUSMALLINT	inputSchemaColumnsNumber,
    SQLUSMALLINT	parametersNumber,
    SQLCHAR* inputDataName,
    SQLUSMALLINT	inputDataNameLength,
    SQLCHAR* outputDataName,
    SQLUSMALLINT	outputDataNameLength
)
{
    return g_dotnet_runtime->call_managed_method<decltype(&InitSession)>(nameof(InitSession),
        sessionId,
        taskId,
        numTasks,
        script,
        scriptLength,
        inputSchemaColumnsNumber,
        parametersNumber,
        inputDataName,
        inputDataNameLength,
        outputDataName,
        outputDataNameLength);
}

//--------------------------------------------------------------------------------------------------
// Name: InitColumn
//
// Description:
//  Initializes column-specific data. We store the name and the data type of the column here.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
_export SQLRETURN InitColumn(
    SQLGUID      SessionId,
    SQLUSMALLINT TaskId,
    SQLUSMALLINT ColumnNumber,
    SQLCHAR      *ColumnName,
    SQLSMALLINT  ColumnNameLength,
    SQLSMALLINT  DataType,
    SQLULEN      ColumnSize,
    SQLSMALLINT  DecimalDigits,
    SQLSMALLINT  Nullable,
    SQLSMALLINT  PartitionByNumber,
    SQLSMALLINT  OrderByNumber
    )
{
    return g_dotnet_runtime->call_managed_method<decltype(&InitColumn)>(nameof(InitColumn),
        SessionId,
        TaskId,
        ColumnNumber,
        ColumnName,
        ColumnNameLength,
        DataType,
        ColumnSize,
        DecimalDigits,
        Nullable,
        PartitionByNumber,
        OrderByNumber);
}

//--------------------------------------------------------------------------------------------------
// Name: InitParam
//
// Description:
//  Initializes parameter-specific data.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
_export SQLRETURN InitParam(
    SQLGUID      SessionId,
    SQLUSMALLINT TaskId,
    SQLUSMALLINT ParamNumber,
    SQLCHAR      *ParamName,
    SQLSMALLINT  ParamNameLength,
    SQLSMALLINT  DataType,
    SQLULEN      ArgSize,
    SQLSMALLINT  DecimalDigits,
    SQLPOINTER   ArgValue,
    SQLINTEGER   StrLen_or_Ind,
    SQLSMALLINT  InputOutputType
    )
{
    return g_dotnet_runtime->call_managed_method<decltype(&InitParam)>(nameof(InitParam),
        SessionId,
        TaskId,
        ParamNumber,
        ParamName,
        ParamNameLength,
        DataType,
        ArgSize,
        DecimalDigits,
        ArgValue,
        StrLen_or_Ind,
        InputOutputType);
}

//--------------------------------------------------------------------------------------------------
// Name: Execute
//
// Description:
//  Given the data from ExtHost, convert and populate the arrays in the user script. Then,
//  invoke the specified script and retrieve the output schema and convert the data back to
//  ODBC types.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
_export SQLRETURN Execute(
    SQLGUID      SessionId,
    SQLUSMALLINT TaskId,
    SQLULEN      RowsNumber,
    SQLPOINTER   *Data,
    SQLINTEGER   **StrLen_or_Ind,
    SQLUSMALLINT *OutputSchemaColumnsNumber
    )
{
    return g_dotnet_runtime->call_managed_method<decltype(&Execute)>(nameof(Execute),
        SessionId,
        TaskId,
        RowsNumber,
        Data,
        StrLen_or_Ind,
        OutputSchemaColumnsNumber);
}

//--------------------------------------------------------------------------------------------------
// Name: GetResultColumn
//
// Description:
//  Returns information about the output column
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
_export SQLRETURN GetResultColumn(
    SQLGUID      SessionId,
    SQLUSMALLINT TaskId,
    SQLUSMALLINT ColumnNumber,
    SQLSMALLINT  *DataType,
    SQLULEN      *ColumnSize,
    SQLSMALLINT  *DecimalDigits,
    SQLSMALLINT  *Nullable
    )
{
    return g_dotnet_runtime->call_managed_method<decltype(&GetResultColumn)>(nameof(GetResultColumn),
        SessionId,
        TaskId,
        ColumnNumber,
        DataType,
        ColumnSize,
        DecimalDigits,
        Nullable);
}

//--------------------------------------------------------------------------------------------------
// Name: GetResults
//
// Description:
//  Returns the output data as well as the null map retrieved from the user program
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
_export SQLRETURN GetResults(
    SQLGUID      SessionId,
    SQLUSMALLINT TaskId,
    SQLULEN      *RowsNumber,
    SQLPOINTER   **Data,
    SQLINTEGER   ***StrLen_or_Ind
    )
{
    return g_dotnet_runtime->call_managed_method<decltype(&GetResults)>(nameof(GetResults),
        SessionId,
        TaskId,
        RowsNumber,
        Data,
        StrLen_or_Ind);
}

//--------------------------------------------------------------------------------------------------
// Name: GetOutputParam
//
// Description:
//  Returns the output parameter's data.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
_export SQLRETURN GetOutputParam(
    SQLGUID      SessionId,
    SQLUSMALLINT TaskId,
    SQLUSMALLINT ParamNumber,
    SQLPOINTER   *ParamValue,
    SQLINTEGER   *StrLen_or_Ind)
{
    return g_dotnet_runtime->call_managed_method<decltype(&GetOutputParam)>(nameof(GetOutputParam),
        SessionId,
        TaskId,
        ParamNumber,
        ParamValue,
        StrLen_or_Ind);
}

//--------------------------------------------------------------------------------------------------
// Name: CleanupSession
//
// Description:
//  Cleans up the output data buffers that we persist for
//  ExtHost to finish processing the data
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
_export SQLRETURN CleanupSession(SQLGUID SessionId, SQLUSMALLINT TaskId)
{
    return g_dotnet_runtime->call_managed_method<decltype(&CleanupSession)>(nameof(CleanupSession),
        SessionId,
        TaskId);
}

//--------------------------------------------------------------------------------------------------
// Name: Cleanup
//
// Description:
//  Completely cleans up the extension
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
_export SQLRETURN Cleanup()
{
    delete g_dotnet_runtime;

    return SQL_SUCCESS;
}