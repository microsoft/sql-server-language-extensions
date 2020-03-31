//*********************************************************************
//      Copyright (c) Microsoft Corporation.
//
// Library management protocol between Exthost and a 3rd party extension
//
// @File: sqlexternallibrary.h
//
//*********************************************************************

#ifndef __SQLEXTERNALLIBRARY
#define __SQLEXTERNALLIBRARY

#include "sqlexternallanguage.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Optional API
//
SQLEXTENSION_INTERFACE
SQLRETURN InstallExternalLibrary(
	SQLGUID    SetupSessionId,
	SQLCHAR    *LibraryName,
	SQLINTEGER LibraryNameLength,
	SQLCHAR    *LibraryFile,
	SQLINTEGER LibraryFileLength,
	SQLCHAR    *LibraryInstallDirectory,
	SQLINTEGER LibraryInstallDirectoryLength,
	SQLCHAR    **LibraryError,
	SQLINTEGER *LibraryErrorLength);

// Optional API
//
SQLEXTENSION_INTERFACE
SQLRETURN UninstallExternalLibrary(
	SQLGUID    SetupSessionId,
	SQLCHAR    *LibraryName,
	SQLINTEGER LibraryNameLength,
	SQLCHAR    *LibraryInstallDirectory,
	SQLINTEGER LibraryInstallDirectoryLength,
	SQLCHAR    **LibraryError,
	SQLINTEGER *LibraryErrorLength);

#ifdef __cplusplus
} /* End of extern "C" { */
#endif  /* __cplusplus */

#endif  /* __SQLEXTERNALLIBRARY */
