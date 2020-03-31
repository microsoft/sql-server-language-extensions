//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JavaLibraryUtils.cpp
//
// Purpose:
//  Cross-platform utility functions to handle library management for
//  the Java extension.
//
//*********************************************************************
#include "JavaLibraryUtils.h"
#include "JavaPathSettings.h"
#include "Logger.h"

using namespace std;

//--------------------------------------------------------------------------------------------------
// Name: JavaLibraryUtils::GetLibrariesClassPath
//
// Description:
//	Gets the string to include in the classpath for external libraries. This is done by searching
//	for every file under the directory and adding it to the path. This is needed, because JVM expects
//	that the full path for jars in the class path.
//
// Notes:
//	First private libraries are added to the classpath then public libraries.
//
// Returns:
//  The classpath of all the jars in the library management directories
//
string JavaLibraryUtils::GetLibrariesClassPath()
{
	string classPath = "";

	// Check if private library path was sent from SQL Server.
	//
	const string & privateLibraryPath = JavaPathSettings::GetPrivateLibraryPath();

	if (!privateLibraryPath.empty())
	{
		FindAppendFileNames(privateLibraryPath, classPath);
	}

	// Check if the public library path was sent from SQL Server.
	//
	const string & publicLibraryPath = JavaPathSettings::GetPublicLibraryPath();

	if (!publicLibraryPath.empty())
	{
		FindAppendFileNames(publicLibraryPath, classPath);
	}

	return classPath;
}
