//*********************************************************************
//      Copyright (c) Microsoft Corporation.
//
// Host callbacks protocol between the host and extensions.
// Optional API surface introduced alongside v3 of the Extension API.
//
// @File: sqlextensionhostcallbacks.h
//
//*********************************************************************

#ifndef __SQLEXTENSIONHOSTCALLBACKS
#define __SQLEXTENSIONHOSTCALLBACKS

#include "sqlexternallanguage.h"
#include "sqlextensiontracelevel.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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
	// Highest SQLEXTENSION_HOST_CALLBACKS_VERSION_* the host populates.
	// Extension must validate before reading version-gated fields.
	//
	SQLUSMALLINT Version;

	// Explicit padding so SizeInBytes is naturally 4-byte aligned regardless
	// of compiler packing settings. Must be zero.
	//
	SQLUSMALLINT Reserved0;

	// sizeof(SQLEXTENSION_HOST_CALLBACKS) as the host saw it at build time.
	// Extension must validate this is greater or equal the size of every field it intends
	// to read. Lets a newer Extension safely run against an older host that supplied a smaller struct.
	//
	SQLUINTEGER SizeInBytes;

	// Version 1 callbacks.
	//
	PFunc_ExtensionLogXEvent LogXEvent;

	// Reserved for future expansion. Zero-initialized by the host. Extension must not read or call these.
	//
	void *Reserved1;
	void *Reserved2;
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
