//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: nativecsharpextension.h
//
// Purpose:
//  Implement the native language extensions APIs
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

#if defined(__WIN32) || defined(WINDOWS)
#include <windows.h>
#endif

#include <sql.h>
#include "sqlexternallanguage.h"
#include "sqlexternallibrary.h"
#include "DotnetEnvironment.h"

//  Returns the string with the address of the first character and length
//
std::string UTF8PtrToStr(SQLCHAR* str, SQLULEN len);

//  Returns the API interface version for the extension
//
SQLUSMALLINT GetInterfaceVersion();

//  Initializes the .NET Core C# Extension.
//
SQLEXTENSION_INTERFACE SQLRETURN Init(
    SQLCHAR  *languageParams,
    SQLULEN  languageParamsLen,
    SQLCHAR  *languagePath,
    SQLULEN  languagePathLen,
    SQLCHAR  *publicLibraryPath,
    SQLULEN  publicLibraryPathLen,
    SQLCHAR  *privateLibraryPath,
    SQLULEN  privateLibraryPathLen);

//  Initializes session-specific data. We store the schema, parameter info,
//  output input data info here.
//
SQLEXTENSION_INTERFACE SQLRETURN InitSession(
    SQLGUID      sessionId,
    SQLUSMALLINT taskId,
    SQLUSMALLINT numTasks,
    SQLCHAR      *script,
    SQLULEN      scriptLength,
    SQLUSMALLINT inputSchemaColumnsNumber,
    SQLUSMALLINT parametersNumber,
    SQLCHAR      *inputDataName,
    SQLUSMALLINT inputDataNameLength,
    SQLCHAR      *outputDataName,
    SQLUSMALLINT outputDataNameLength);

//  Initializes column-specific data. We store the name and the data type of the column here.
//
SQLEXTENSION_INTERFACE SQLRETURN InitColumn(
    SQLGUID      sessionId,
    SQLUSMALLINT taskId,
    SQLUSMALLINT columnNumber,
    SQLCHAR      *columnName,
    SQLSMALLINT  columnNameLength,
    SQLSMALLINT  dataType,
    SQLULEN      columnSize,
    SQLSMALLINT  decimalDigits,
    SQLSMALLINT  nullable,
    SQLSMALLINT  partitionByNumber,
    SQLSMALLINT  orderByNumber);

//  Initializes parameter-specific data.
//
SQLEXTENSION_INTERFACE SQLRETURN InitParam(
    SQLGUID      sessionId,
    SQLUSMALLINT taskId,
    SQLUSMALLINT paramNumber,
    SQLCHAR      *paramName,
    SQLSMALLINT  paramNameLength,
    SQLSMALLINT  dataType,
    SQLULEN      paramSize,
    SQLSMALLINT  decimalDigits,
    SQLPOINTER   parmValue,
    SQLINTEGER   strLen_or_Ind,
    SQLSMALLINT  inputOutputType);

//  Given the data from ExtHost, convert and populate the arrays in the user script. Then,
//  invoke the specified script and retrieve the output schema and convert the data back to
//  ODBC types.
//
SQLEXTENSION_INTERFACE SQLRETURN Execute(
    SQLGUID      sessionId,
    SQLUSMALLINT taskId,
    SQLULEN      rowsNumber,
    SQLPOINTER   *data,
    SQLINTEGER   **strLen_or_Ind,
    SQLUSMALLINT *outputSchemaColumnsNumber);

//  Returns information about the output column
//
SQLEXTENSION_INTERFACE SQLRETURN GetResultColumn(
    SQLGUID      sessionId,
    SQLUSMALLINT taskId,
    SQLUSMALLINT columnNumber,
    SQLSMALLINT  *dataType,
    SQLULEN      *columnSize,
    SQLSMALLINT  *decimalDigits,
    SQLSMALLINT  *nullable);

//  Returns the output data as well as the null map retrieved from the user program
//
SQLEXTENSION_INTERFACE SQLRETURN GetResults(
    SQLGUID      sessionId,
    SQLUSMALLINT taskId,
    SQLULEN      *rowsNumber,
    SQLPOINTER   **data,
    SQLINTEGER   ***strLen_or_Ind);

//  Returns the output parameter's data.
//
SQLEXTENSION_INTERFACE SQLRETURN GetOutputParam(
    SQLGUID      sessionId,
    SQLUSMALLINT taskId,
    SQLUSMALLINT paramNumber,
    SQLPOINTER   *paramValue,
    SQLINTEGER   *strLen_or_Ind);

//  Cleans up the output data buffers that we persist for
//  ExtHost to finish processing the data
//
SQLEXTENSION_INTERFACE SQLRETURN CleanupSession(SQLGUID sessionId, SQLUSMALLINT taskId);

//  Completely cleans up the extension
//
SQLEXTENSION_INTERFACE SQLRETURN Cleanup();

//  Dotnet environment pointer
//
static DotnetEnvironment* g_dotnet_runtime = nullptr;