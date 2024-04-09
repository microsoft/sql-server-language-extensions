//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JavaExtensionUtils_linux.cpp
//
// Purpose:
//  Linux specific utility functions used by the Java extension.
//
//*********************************************************************
#include "JavaExtensionUtils.h"
#include "Logger.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

const char x_LinuxClassPathSeparator = ':';    // CLASSPATH separator on Linux
const char x_LinuxFilePathSeparator = '/';     // File path separator on Linux
const string x_LinuxJvmFilename = "libjvm.so"; // Filename of the JVM library on Linux

// Linux specific relative path to the JVM library, based on the standard for the JRE_HOME
// enviroment variable.
//
const string x_LinuxJvmPath = "lib/server/" + x_LinuxJvmFilename;

// Pointer to the JVM module on Linux
//
static void *g_jvmLib = nullptr;

//--------------------------------------------------------------------------------------------------
// Name: JavaExtensionUtils::GetClassPathSeparator
//
// Description:
//  Return Windows classpath separator
//
const char JavaExtensionUtils::GetClassPathSeparator()
{
	return x_LinuxClassPathSeparator;
}

//--------------------------------------------------------------------------------------------------
// Name: JavaExtensionUtils::GetClassPathSeparator
//
// Description:
//  Return Windows classpath separator
//
const char JavaExtensionUtils::GetPathSeparator()
{
	return x_LinuxFilePathSeparator;
}

//--------------------------------------------------------------------------------------------------
// Name: JavaExtensionUtils::GetRelativeJvmPath
//
// Description:
//  Gets the platform specific relative path to the JVM library
//  with respect to JRE_HOME.
//
// Returns:
//  Returns a string to the relative path to libjvm.so with respect to JRE_HOME
//
const string& JavaExtensionUtils::GetRelativeJvmPath()
{
	return x_LinuxJvmPath;
}

//--------------------------------------------------------------------------------------------------
// Name: JavaExtensionUtils::GetJvmFilename
//
// Description:
//  Gets the platform specific filename for the JVM library
//
// Returns:
//  Returns a string of the platform specific filename
//
const string& JavaExtensionUtils::GetJvmFilename()
{
	return x_LinuxJvmFilename;
}


//--------------------------------------------------------------------------------------------------
// Name: JavaExtensionUtils::GetEnvVariable
//
// Description:
//  Get the enviroment variable
//
// Returns:
//  String of the enviroment variable if it exists, else an empty string
//
string JavaExtensionUtils::GetEnvVariable(const string &name)
{
	char* value = getenv(name.c_str());
	string result = "";

	if (value != nullptr)
	{
		result = value;
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: JavaExtensionUtils::IsValidFile
//
// Description:
//  Determines if a file path is valid
//
// Returns:
//  Returns true if the file is valid, else false
//
bool JavaExtensionUtils::IsValidFile(const string &file)
{
	struct stat st;
	bool result = false;

	if (stat(file.c_str(), &st) == 0)
	{
		if (S_ISREG(st.st_mode))
		{
			result = true;
		}
	}

	return result;
}

//----------------------------------------------------------------------------
// Name: JavaExtensionUtils::LoadJvm
//
// Description:
//	Tries to find a libjvm.so path from the PATH provided
//
// RETURNS:
//	Function pointer to JNICreate_JVM function
//
JavaExtensionUtils::fn_createJvm JavaExtensionUtils::LoadJvm(const string& jvmPath)
{
	// Load the JVM
	//
	// Flags:
	//  RTLD_NOW forces all symbols in the library to be resolved before dlopen() returns
	//  RTLD_LOCAL prevents subsequently loaded libraries from resolving symbols defined in this library
	//
	g_jvmLib = dlopen(jvmPath.c_str(), RTLD_NOW | RTLD_LOCAL);

	if (g_jvmLib == nullptr)
	{
		throw runtime_error(
				  "Failed to load JVM from path " + jvmPath +
				  ". Ensure JRE_HOME is set.");
	}
	else
	{
		LOG("Successfully loaded JVM from path: " + jvmPath);
	}

	return reinterpret_cast<fn_createJvm>(dlsym(g_jvmLib, "JNI_CreateJavaVM"));
}

//----------------------------------------------------------------------------
// Name: JavaExtensionUtils::ShutdownJvm
//
// Description:
//  Cleans up JVM resources. When running on Linux, clean up operation
//  doesn't call Invocation API function DestroyJavaVM() because 
//  ExtHost process exit defers to OS for resource cleanup 
//  and SPEES query doesn't hang.
//  Follow up investigation tracked via work item: 3090980
//
void JavaExtensionUtils::ShutdownJvm(JavaVM *jvm)
{
	LOG("Cleaning up JVM");
	jvm = nullptr;
}

//----------------------------------------------------------------------------
// Name: JavaExtensionUtils::UnloadJvm
//
// Description:
//  Closes the reference to libjvm.so
//
void JavaExtensionUtils::UnloadJvm()
{
	// If the extension was loaded, free it
	//
	if (g_jvmLib != nullptr)
	{
		dlclose(g_jvmLib);
		g_jvmLib = nullptr;
	}
}
