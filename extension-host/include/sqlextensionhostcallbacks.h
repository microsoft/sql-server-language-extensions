//*********************************************************************
//      Copyright (c) Microsoft Corporation.
//
// Host callbacks protocol between Exthost and a 3rd party extension.
// Optional API surface introduced alongside v3 of the Extension API.
//
// @File: sqlextensionhostcallbacks.h
//
//*********************************************************************

#ifndef __SQLEXTENSIONHOSTCALLBACKS
#define __SQLEXTENSIONHOSTCALLBACKS

#include "sqlexternallanguage.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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

#endif/* __SQLEXTENSIONHOSTCALLBACKS */
