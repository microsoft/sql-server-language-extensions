//*********************************************************************
//      Copyright (c) Microsoft Corporation.
//
// Communication protocol between Exthost and a 3rd party extension
//
// @File: sqlexternallanguage.h
//
//*********************************************************************

#ifndef __SQLEXTERNALLANGUAGE
#define __SQLEXTERNALLANGUAGE

// Use ODBC 3.80
//
#define ODBCVER 0x0380

#include <sqltypes.h>

#ifdef _WIN32
#define SQLEXTENSION_INTERFACE __declspec(dllexport)
#elif __linux__
#define SQLEXTENSION_INTERFACE __attribute__((visibility("default")))
#else
#define SQLEXTENSION_INTERFACE
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define EXTERNAL_LANGUAGE_EXTENSION_API 3

SQLEXTENSION_INTERFACE
SQLUSMALLINT GetInterfaceVersion();

SQLEXTENSION_INTERFACE
SQLRETURN Init(
	SQLCHAR *ExtensionParams,
	SQLULEN ExtensionParamsLength,
	SQLCHAR *ExtensionPath,
	SQLULEN ExtensionPathLength,
	SQLCHAR *PublicLibraryPath,
	SQLULEN PublicLibraryPathLength,
	SQLCHAR *PrivateLibraryPath,
	SQLULEN PrivateLibraryPathLength
	);

SQLEXTENSION_INTERFACE
SQLRETURN InitSession(
	SQLGUID      SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT NumTasks,
	SQLCHAR      *Script,
	SQLULEN      ScriptLength,
	SQLUSMALLINT InputSchemaColumnsNumber,
	SQLUSMALLINT ParametersNumber,
	SQLCHAR      *InputDataName,
	SQLUSMALLINT InputDataNameLength,
	SQLCHAR      *OutputDataName,
	SQLUSMALLINT OutputDataNameLength
	);

SQLEXTENSION_INTERFACE
SQLRETURN InitColumn(
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
	);

SQLEXTENSION_INTERFACE
SQLRETURN InitParam(
	SQLGUID      SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ParamNumber,
	SQLCHAR      *ParamName,
	SQLSMALLINT  ParamNameLength,
	SQLSMALLINT  DataType,
	SQLULEN      ParamSize,
	SQLSMALLINT  DecimalDigits,
	SQLPOINTER   ParamValue,
	SQLINTEGER   StrLen_or_Ind,
	SQLSMALLINT  InputOutputType);

SQLEXTENSION_INTERFACE
SQLRETURN Execute(
	SQLGUID      SessionId,
	SQLUSMALLINT TaskId,
	SQLULEN      RowsNumber,
	SQLPOINTER   *Data,
	SQLINTEGER   **StrLen_or_Ind,
	SQLUSMALLINT *OutputSchemaColumnsNumber
	);

SQLEXTENSION_INTERFACE
SQLRETURN GetResultColumn(
	SQLGUID      SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ColumnNumber,
	SQLSMALLINT  *DataType,
	SQLULEN      *ColumnSize,
	SQLSMALLINT  *DecimalDigits,
	SQLSMALLINT  *Nullable
	);

SQLEXTENSION_INTERFACE
SQLRETURN GetResults(
	SQLGUID      SessionId,
	SQLUSMALLINT TaskId,
	SQLULEN      *RowsNumber,
	SQLPOINTER   **Data,
	SQLINTEGER   ***StrLen_or_Ind
	);

SQLEXTENSION_INTERFACE
SQLRETURN GetOutputParam(
	SQLGUID      SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ParamNumber,
	SQLPOINTER   *ParamValue,
	SQLINTEGER   *StrLen_or_Ind
	);

SQLEXTENSION_INTERFACE
SQLRETURN CleanupSession(
	SQLGUID      SessionId,
	SQLUSMALLINT TaskId
	);

SQLEXTENSION_INTERFACE
SQLRETURN Cleanup();

// Trace levels for events logged via LogXEvent function.
// Lowest numeric value is the most severe, matching the Windows ETW
// TRACE_LEVEL_* convention.
//
enum ExtensionTraceLevel
{
	Extension_Critical    = 1,
	Extension_Error       = 2,
	Extension_Warning     = 3,
	Extension_Information = 4,
	Extension_Verbose     = 5
};

// Callback function provided by the host for logging XEvents from Extension.
//
typedef void (*PFunc_ExtensionLogXEvent)(
	const SQLCHAR	*ExtensionName,
	SQLULEN		ExtensionNameLength,
	SQLGUID		SessionId,
	SQLUSMALLINT	TaskId,
	SQLUSMALLINT	TraceLevel,
	SQLINTEGER	ErrorCode,
	const SQLCHAR	*Message,
	SQLULEN		MessageLength
	);

// Host callbacks structure passed from host to Extension.
//
#define SQLEXTENSION_HOST_CALLBACKS_VERSION_1 1
#define SQLEXTENSION_HOST_CALLBACKS_MAX_SUPPORTED_VERSION SQLEXTENSION_HOST_CALLBACKS_VERSION_1

typedef struct _SQLEXTENSION_HOST_CALLBACKS
{
	SQLUSMALLINT Version;
	PFunc_ExtensionLogXEvent LogXEvent;
} SQLEXTENSION_HOST_CALLBACKS;

// Optional API (v3+)
// Receives host callback functions from the host.
//
SQLEXTENSION_INTERFACE
SQLRETURN SetHostCallbacks(
	SQLEXTENSION_HOST_CALLBACKS *Callbacks
	);

#ifdef __cplusplus
} /* End of extern "C" { */
#endif/* __cplusplus */

#endif/* __SQLEXTERNALLANGUAGE */
