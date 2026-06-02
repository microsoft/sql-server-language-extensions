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

// Single definitions for the globals declared (extern) in the header.
//
DotnetEnvironment*           g_dotnet_runtime = nullptr;
SQLEXTENSION_HOST_CALLBACKS* g_hostCallbacks  = nullptr;

// g_hostCallbacks points at internal copy of the host's callbacks
// struct g_hostCallbacksCopy rather than the caller-owned memory, so the
// pointer can never dangle if the host passed a stack-allocated struct to
// SetHostCallbacks.
//
static SQLEXTENSION_HOST_CALLBACKS g_hostCallbacksCopy = {};

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

    // Clear the managed-side callbacks delegates before tearing down the
    // runtime so any thread that races into callbacks during shutdown
    // cannot invoke a host function pointer whose backing implementation is
    // about to be freed.
    //
    if (g_dotnet_runtime != nullptr)
    {
        SQLEXTENSION_HOST_CALLBACKS nullCallbacks = {};
        nullCallbacks.Version   = SQLEXTENSION_HOST_CALLBACKS_VERSION_1;
        nullCallbacks.LogXEvent = nullptr;

        g_dotnet_runtime->call_managed_method<decltype(&SetHostCallbacks)>(
            nameof(SetHostCallbacks),
            &nullCallbacks);
    }

    g_hostCallbacks  = nullptr;
    delete g_dotnet_runtime;
    g_dotnet_runtime = nullptr;
    return SQL_SUCCESS;
}

//--------------------------------------------------------------------------------------------------
// Name: SetLibraryError
//
// Description:
//  Helper to populate the library error output parameters.
//
static void SetLibraryError(
    const std::string &errorString,
    SQLCHAR    **libraryError,
    SQLINTEGER *libraryErrorLength)
{
    // Guard against null out-parameters. The managed SetLibraryError
    // (CSharpExtension.cs) does the same null-check; without it, a caller
    // passing null libraryError / libraryErrorLength would dereference null
    // and crash the host process.
    if (libraryError == nullptr || libraryErrorLength == nullptr)
    {
        return;
    }

    if (!errorString.empty())
    {
        // Length excludes null terminator -- ExtHost adds +1 when copying
        // (see Utf8ToNullTerminatedUtf16Le / strcpy_s in the host).
        *libraryErrorLength = static_cast<SQLINTEGER>(errorString.length());

        // Ownership of the buffer transfers to ExtHost; it frees the pointer
        // via the C runtime (free()) after consuming the message. Mirrors
        // the managed SetLibraryError (CSharpExtension.cs), which uses
        // Marshal.AllocHGlobal for the same reason. The previous
        // implementation -- new std::string(errorString) + returning
        // c_str() -- leaked the string AND handed back a pointer into a
        // std::string's internal buffer, which is undefined behavior the
        // moment ExtHost calls free() on it.
        size_t bufLen = errorString.length() + 1;
        char *buf = static_cast<char*>(malloc(bufLen));
        if (buf == nullptr)
        {
            // Out of memory; surface a "no error" state rather than crash
            // (the original failure will still be logged by the caller).
            *libraryError = nullptr;
            *libraryErrorLength = 0;
            return;
        }
        memcpy(buf, errorString.c_str(), bufLen);
        *libraryError = reinterpret_cast<SQLCHAR*>(buf);
    }
    else
    {
        // Explicitly clear the out-parameters so callers that don't
        // pre-initialize them see a well-defined "no error" state.
        *libraryError = nullptr;
        *libraryErrorLength = 0;
    }
}

//--------------------------------------------------------------------------------------------------
// Name: InstallExternalLibrary
//
// Description:
//  Installs an external library to the specified directory.
//  The library file may be a ZIP archive or a raw DLL.
//  If a ZIP, and it contains an inner zip, that inner zip is extracted to the
//  install directory. Otherwise, all files are copied directly.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN InstallExternalLibrary(
    SQLGUID    setupSessionId,
    SQLCHAR    *libraryName,
    SQLINTEGER libraryNameLength,
    SQLCHAR    *libraryFile,
    SQLINTEGER libraryFileLength,
    SQLCHAR    *libraryInstallDirectory,
    SQLINTEGER libraryInstallDirectoryLength,
    SQLCHAR    **libraryError,
    SQLINTEGER *libraryErrorLength)
{
    LOG("nativecsharpextension::InstallExternalLibrary");

    SQLRETURN result = SQL_ERROR;

    if (g_dotnet_runtime == nullptr)
    {
        SetLibraryError(
            "Extension not initialized. Call Init before InstallExternalLibrary.",
            libraryError,
            libraryErrorLength);
    }
    else
    {
        result = g_dotnet_runtime->call_managed_method<decltype(&InstallExternalLibrary)>(
            nameof(InstallExternalLibrary),
            setupSessionId,
            libraryName,
            libraryNameLength,
            libraryFile,
            libraryFileLength,
            libraryInstallDirectory,
            libraryInstallDirectoryLength,
            libraryError,
            libraryErrorLength);
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
// Name: UninstallExternalLibrary
//
// Description:
//  Uninstalls an external library from the specified directory.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN UninstallExternalLibrary(
    SQLGUID    setupSessionId,
    SQLCHAR    *libraryName,
    SQLINTEGER libraryNameLength,
    SQLCHAR    *libraryInstallDirectory,
    SQLINTEGER libraryInstallDirectoryLength,
    SQLCHAR    **libraryError,
    SQLINTEGER *libraryErrorLength)
{
    LOG("nativecsharpextension::UninstallExternalLibrary");

    SQLRETURN result = SQL_ERROR;

    if (g_dotnet_runtime == nullptr)
    {
        SetLibraryError(
            "Extension not initialized. Call Init before UninstallExternalLibrary.",
            libraryError,
            libraryErrorLength);
    }
    else
    {
        result = g_dotnet_runtime->call_managed_method<decltype(&UninstallExternalLibrary)>(
            nameof(UninstallExternalLibrary),
            setupSessionId,
            libraryName,
            libraryNameLength,
            libraryInstallDirectory,
            libraryInstallDirectoryLength,
            libraryError,
            libraryErrorLength);
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
// Name: SetHostCallbacks
//
// Description:
//  Receives a pointer to the host callbacks structure.
//  Stores the pointer natively and forwards to managed code so the
//  managed layer can call back into the host.
//
// Returns:
//  SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN SetHostCallbacks(
    SQLEXTENSION_HOST_CALLBACKS *hostCallbacks
)
{
    LOG("nativecsharpextension::SetHostCallbacks");

    if (hostCallbacks == nullptr)
    {
        LOG_ERROR("SetHostCallbacks called with null pointer");
        return SQL_ERROR;
    }

    if (g_dotnet_runtime == nullptr)
    {
        LOG_ERROR("SetHostCallbacks called before Init() or after Cleanup()");
        return SQL_ERROR;
    }

    // Validate the struct version before reading any version-gated fields.
    //
    if (hostCallbacks->Version < SQLEXTENSION_HOST_CALLBACKS_MIN_SUPPORTED_VERSION)
    {
        LOG_ERROR("SetHostCallbacks called with unsupported host callbacks version");
        return SQL_ERROR;
    }

    // Take a shallow copy of the caller's struct so g_hostCallbacks cannot
    // dangle if the host passed a stack-allocated SQLEXTENSION_HOST_CALLBACKS.
    //
    g_hostCallbacksCopy = *hostCallbacks;
    g_hostCallbacks     = &g_hostCallbacksCopy;

    return g_dotnet_runtime->call_managed_method<decltype(&SetHostCallbacks)>(
        nameof(SetHostCallbacks),
        hostCallbacks);
}