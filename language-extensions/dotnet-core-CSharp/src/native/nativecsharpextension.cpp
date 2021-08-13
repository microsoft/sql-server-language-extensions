//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: nativecsharpextension.cpp
//
// Purpose:
//  Implement the native language extensions APIs
//
//*********************************************************************
#include "nativecsharpextension.h"
#include "Logger.h"

#define nameof(x) #x

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
SQLRETURN Init(
    SQLCHAR  *languageParams,
    SQLULEN  languageParamsLen,
    SQLCHAR  *languagePath,
    SQLULEN  languagePathLen,
    SQLCHAR  *publicLibraryPath,
    SQLULEN  publicLibraryPathLen,
    SQLCHAR  *privateLibraryPath,
    SQLULEN  privateLibraryPathLen
)
{
    LOG("nativecsharpextension::Init");
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
SQLRETURN InitSession(
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
    SQLUSMALLINT outputDataNameLength
)
{
    LOG("nativecsharpextension::InitSession");
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
SQLRETURN InitColumn(
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
    SQLSMALLINT  orderByNumber
)
{
    LOG("nativecsharpextension::InitColumn");
    return g_dotnet_runtime->call_managed_method<decltype(&InitColumn)>(nameof(InitColumn),
        sessionId,
        taskId,
        columnNumber,
        columnName,
        columnNameLength,
        dataType,
        columnSize,
        decimalDigits,
        nullable,
        partitionByNumber,
        orderByNumber);
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
SQLRETURN InitParam(
    SQLGUID      sessionId,
    SQLUSMALLINT taskId,
    SQLUSMALLINT paramnumber,
    SQLCHAR      *paramName,
    SQLSMALLINT  paramNameLength,
    SQLSMALLINT  dataType,
    SQLULEN      paramSize,
    SQLSMALLINT  decimalDigits,
    SQLPOINTER   paramValue,
    SQLINTEGER   strLen_or_Ind,
    SQLSMALLINT  inputOutputType
)
{
    LOG("nativecsharpextension::InitParam");
    return g_dotnet_runtime->call_managed_method<decltype(&InitParam)>(nameof(InitParam),
        sessionId,
        taskId,
        paramnumber,
        paramName,
        paramNameLength,
        dataType,
        paramSize,
        decimalDigits,
        paramValue,
        strLen_or_Ind,
        inputOutputType);
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
SQLRETURN Execute(
    SQLGUID      sessionId,
    SQLUSMALLINT taskId,
    SQLULEN      rowsNumber,
    SQLPOINTER   *data,
    SQLINTEGER   **strLen_or_Ind,
    SQLUSMALLINT *outputSchemaColumnsNumber
)
{
    LOG("nativecsharpextension::Execute");
    return g_dotnet_runtime->call_managed_method<decltype(&Execute)>(nameof(Execute),
        sessionId,
        taskId,
        rowsNumber,
        data,
        strLen_or_Ind,
        outputSchemaColumnsNumber);
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
SQLRETURN GetResultColumn(
    SQLGUID      sessionId,
    SQLUSMALLINT taskId,
    SQLUSMALLINT columnNumber,
    SQLSMALLINT  *dataType,
    SQLULEN      *columnSize,
    SQLSMALLINT  *decimalDigits,
    SQLSMALLINT  *nullable
)
{
    LOG("nativecsharpextension::GetResultColumn");
    return g_dotnet_runtime->call_managed_method<decltype(&GetResultColumn)>(nameof(GetResultColumn),
        sessionId,
        taskId,
        columnNumber,
        dataType,
        columnSize,
        decimalDigits,
        nullable);
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
SQLRETURN GetResults(
    SQLGUID      sessionId,
    SQLUSMALLINT taskId,
    SQLULEN      *rowsNumber,
    SQLPOINTER   **data,
    SQLINTEGER   ***strLen_or_Ind
)
{
    LOG("nativecsharpextension::GetResults");
    return g_dotnet_runtime->call_managed_method<decltype(&GetResults)>(nameof(GetResults),
        sessionId,
        taskId,
        rowsNumber,
        data,
        strLen_or_Ind);
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
SQLRETURN GetOutputParam(
    SQLGUID      sessionId,
    SQLUSMALLINT taskId,
    SQLUSMALLINT paramnumber,
    SQLPOINTER   *paramValue,
    SQLINTEGER   *strLen_or_Ind
)
{
    LOG("nativecsharpextension::GetOutputParam");
    return g_dotnet_runtime->call_managed_method<decltype(&GetOutputParam)>(nameof(GetOutputParam),
        sessionId,
        taskId,
        paramnumber,
        paramValue,
        strLen_or_Ind);
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
SQLRETURN CleanupSession(SQLGUID sessionId, SQLUSMALLINT taskId)
{
    LOG("nativecsharpextension::CleanupSession");
    return g_dotnet_runtime->call_managed_method<decltype(&CleanupSession)>(nameof(CleanupSession),
        sessionId,
        taskId);
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
SQLRETURN Cleanup()
{
    LOG("nativecsharpextension::Cleanup");
    delete g_dotnet_runtime;
    return SQL_SUCCESS;
}