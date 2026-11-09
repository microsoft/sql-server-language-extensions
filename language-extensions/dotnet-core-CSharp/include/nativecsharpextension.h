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

#if defined(_WIN32) || defined(WINDOWS)
#include <windows.h>
#endif

#include <sql.h>
#include "sqlexternallanguage.h"
#include "sqlexternallibrary.h"
#include "sqlextensionhostcallbacks.h"
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

//  Installs an external library to the specified directory.
//
//  Dispatch is by the registered library name (libraryName), not the
//  contents of libraryFile:
//    - libraryName ending in ".zip"  -> ZIP archive install. If the archive
//      contains a single inner zip, that inner zip is extracted to the
//      install directory; otherwise the outer archive's files are copied
//      directly. A "{libName}.manifest" listing every extracted file is
//      written so UninstallExternalLibrary can clean up exactly what was
//      installed.
//    - libraryName ending in ".dll"  -> raw DLL install. libraryFile is
//      copied verbatim to "{installDir}\{libraryName}" and a one-entry
//      manifest is written.
//    - libraryName with neither extension -> falls back to libraryFile's
//      extension (preserves the legacy contract for callers that register
//      libraries by bare name and point libraryFile at a "*.zip" or
//      "*.dll" fixture).
//
SQLEXTENSION_INTERFACE SQLRETURN InstallExternalLibrary(
    SQLGUID    setupSessionId,
    SQLCHAR    *libraryName,
    SQLINTEGER libraryNameLength,
    SQLCHAR    *libraryFile,
    SQLINTEGER libraryFileLength,
    SQLCHAR    *libraryInstallDirectory,
    SQLINTEGER libraryInstallDirectoryLength,
    SQLCHAR    **libraryError,
    SQLINTEGER *libraryErrorLength);

//  Uninstalls an external library from the specified directory.
//
SQLEXTENSION_INTERFACE SQLRETURN UninstallExternalLibrary(
    SQLGUID    setupSessionId,
    SQLCHAR    *libraryName,
    SQLINTEGER libraryNameLength,
    SQLCHAR    *libraryInstallDirectory,
    SQLINTEGER libraryInstallDirectoryLength,
    SQLCHAR    **libraryError,
    SQLINTEGER *libraryErrorLength);

//  Receives host callback function pointers from the host.
//  Optional API, supported since v3 of the Extension API.
//
SQLEXTENSION_INTERFACE SQLRETURN SetHostCallbacks(
    SQLEXTENSION_HOST_CALLBACKS *hostCallbacks);

//  Dotnet environment pointer. Defined in nativecsharpextension.cpp.
//
extern DotnetEnvironment* g_dotnet_runtime;

//  Host callbacks pointer provided by the host via SetHostCallbacks.
//  Defined in nativecsharpextension.cpp.
//
extern SQLEXTENSION_HOST_CALLBACKS* g_hostCallbacks;