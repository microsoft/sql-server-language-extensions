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

std::string to_utf8_str(SQLCHAR* str, SQLULEN len)
{
    return std::string(reinterpret_cast<char*>(str), len);
}

SQLUSMALLINT GetInterfaceVersion()
{
    return EXTERNAL_LANGUAGE_EXTENSION_API;
}

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
        to_utf8_str(languageParams, languageParamsLen),
        to_utf8_str(languagePath, languagePathLen),
        to_utf8_str(publicLibraryPath, publicLibraryPathLen),
        to_utf8_str(privateLibraryPath, privateLibraryPathLen));

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

_export SQLRETURN CleanupSession(SQLGUID SessionId, SQLUSMALLINT TaskId)
{
    return g_dotnet_runtime->call_managed_method<decltype(&CleanupSession)>(nameof(CleanupSession),
        SessionId,
        TaskId);
}

_export SQLRETURN Cleanup()
{
    delete g_dotnet_runtime;

    return SQL_SUCCESS;
}