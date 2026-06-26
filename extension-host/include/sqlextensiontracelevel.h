//*********************************************************************
//      Copyright (c) Microsoft Corporation.
//
// ExtensionTraceLevel enum shared between the host and extensions.
//
// @File: sqlextensiontracelevel.h
//
//*********************************************************************

#ifndef __SQLEXTENSIONTRACELEVEL
#define __SQLEXTENSIONTRACELEVEL

// Trace levels for events logged via the LogXEvent host callback.
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

#endif /* __SQLEXTENSIONTRACELEVEL */
