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
#endif	/* __cplusplus */

#define EXTERNAL_LANGUAGE_EXTENSION_API 2

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
	SQLGUID			SessionId,
	SQLUSMALLINT	TaskId,
	SQLUSMALLINT	NumTasks,
	SQLCHAR			*Script,
	SQLULEN			ScriptLength,
	SQLUSMALLINT	InputSchemaColumnsNumber,
	SQLUSMALLINT	ParametersNumber,
	SQLCHAR			*InputDataName,
	SQLUSMALLINT	InputDataNameLength,
	SQLCHAR			*OutputDataName,
	SQLUSMALLINT	OutputDataNameLength
	);

SQLEXTENSION_INTERFACE
SQLRETURN InitColumn(
	SQLGUID			SessionId,
	SQLUSMALLINT	TaskId,
	SQLUSMALLINT	ColumnNumber,
	SQLCHAR			*ColumnName,
	SQLSMALLINT		ColumnNameLength,
	SQLSMALLINT		DataType,
	SQLULEN			ColumnSize,
	SQLSMALLINT		DecimalDigits,
	SQLSMALLINT		Nullable,
	SQLSMALLINT		PartitionByNumber,
	SQLSMALLINT		OrderByNumber
	);

SQLEXTENSION_INTERFACE
SQLRETURN InitParam(
	SQLGUID			SessionId,
	SQLUSMALLINT	TaskId,
	SQLUSMALLINT	ParamNumber,
	SQLCHAR			*ParamName,
	SQLSMALLINT		ParamNameLength,
	SQLSMALLINT		DataType,
	SQLULEN			ParamSize,
	SQLSMALLINT		DecimalDigits,
	SQLPOINTER		ParamValue,
	SQLINTEGER		StrLen_or_Ind,
	SQLSMALLINT		InputOutputType);

SQLEXTENSION_INTERFACE
SQLRETURN Execute(
	SQLGUID			SessionId,
	SQLUSMALLINT	TaskId,
	SQLULEN			RowsNumber,
	SQLPOINTER		*Data,
	SQLINTEGER		**StrLen_or_Ind,
	SQLUSMALLINT	*OutputSchemaColumnsNumber
	);

SQLEXTENSION_INTERFACE
SQLRETURN GetResultColumn(
	SQLGUID			SessionId,
	SQLUSMALLINT	TaskId,
	SQLUSMALLINT	ColumnNumber,
	SQLSMALLINT		*DataType,
	SQLULEN			*ColumnSize,
	SQLSMALLINT		*DecimalDigits,
	SQLSMALLINT		*Nullable
	);

SQLEXTENSION_INTERFACE
SQLRETURN GetResults(
	SQLGUID			SessionId,
	SQLUSMALLINT	TaskId,
	SQLULEN			*RowsNumber,
	SQLPOINTER		**Data,
	SQLINTEGER		***StrLen_or_Ind
	);

SQLEXTENSION_INTERFACE
SQLRETURN GetOutputParam(
	SQLGUID			SessionId,
	SQLUSMALLINT	TaskId,
	SQLUSMALLINT	ParamNumber,
	SQLPOINTER		*ParamValue,
	SQLINTEGER		*StrLen_or_Ind
	);

SQLEXTENSION_INTERFACE
SQLRETURN CleanupSession(
	SQLGUID			SessionId,
	SQLUSMALLINT	TaskId
	);

SQLEXTENSION_INTERFACE
SQLRETURN Cleanup();

#ifdef __cplusplus
}	/* End of extern "C" { */
#endif	/* __cplusplus */

#endif	/* __SQLEXTERNALLANGUAGE */
