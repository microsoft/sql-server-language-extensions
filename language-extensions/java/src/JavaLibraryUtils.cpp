//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: JavaLibraryUtils.cpp
// @Owner: brnieb
//
// Purpose:
//	 Cross-platform utility functions to handle library management for
//	 the Java extension.
//
//*********************************************************************
#ifdef _WIN64
#include <windows.h>
#else
#include <sal_def.h>
#include <xplat_sal.h>
#endif
#include <jni.h>
#include <string>
#include "JavaExtensionUtils.h"
#include "JavaLibraryUtils.h"

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

	// Check if the 'PrivateLibPath' enviroment variable exists.
	//
	string envValue = JavaExtensionUtils::GetEnvVariable("PrivateLibPath");

	if (!envValue.empty())
	{
		FindAppendFileNames(envValue, classPath);
	}

	// Check if the 'PublicLibPath' enviroment variable exists.
	//
	envValue = JavaExtensionUtils::GetEnvVariable("PublicLibPath");

	if (!envValue.empty())
	{
		FindAppendFileNames(envValue, classPath);
	}

	return classPath;
}