//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JavaExtensionUtils_win.cpp
//
// Purpose:
//  Windows specific utility functions used by the Java extension.
//
//*********************************************************************
#include "JavaExtensionUtils.h"
#include "Logger.h"

using namespace std;

const char x_WindowsClassPathSeparator = ';';  // CLASSPATH seperator on Windows
const char x_WindowsFilePathSeparator = '\\';  // Path seperator on Windows
const string x_WindowsJvmFilename = "jvm.dll"; // Filename of the JVM library on Windows

// Platform specific relative path to the JVM library on Windows
//
const string x_WindowsJvmPath = "bin\\server\\" + x_WindowsJvmFilename;

// Handle to the loaded JVM library, the JVM dll should only be loaded once. And extension Init()
// is guranteed to be called only once before executing any session thus there is no need
// for a lock on initialization.
//
static HMODULE g_jvmDll = nullptr;

//--------------------------------------------------------------------------------------------------
// Name: JavaExtensionUtils::GetClassPathSeparator
//
// Description:
//  Return Windows specific classpath separator
//
const char JavaExtensionUtils::GetClassPathSeparator()
{
	return x_WindowsClassPathSeparator;
}

//--------------------------------------------------------------------------------------------------
// Name: JavaExtensionUtils::GetClassPathSeparator
//
// Description:
//  Return Windows specific path separator
//
const char JavaExtensionUtils::GetPathSeparator()
{
	return x_WindowsFilePathSeparator;
}

//--------------------------------------------------------------------------------------------------
// Name: JavaExtensionUtils::GetPlatformJvmPath
//
// Description:
//  Gets the platform specific path to the jvm.dll with respect to JRE_HOME
//
// Returns:
//  Returns a string to the relative path to the jvm.dll with respect to JRE_HOME
//
const string& JavaExtensionUtils::GetRelativeJvmPath()
{
	return x_WindowsJvmPath;
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
	return x_WindowsJvmFilename;
}

//--------------------------------------------------------------------------------------------------
// Name: JavaExtensionUtils::IsValidFile
//
// Description:
//  Determines if a file path is valid
//
// Returns:
//  true if the file is valid, else false
//
bool JavaExtensionUtils::IsValidFile(const string &file)
{
	return GetFileAttributesA(file.c_str()) != INVALID_FILE_ATTRIBUTES;
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
	string result = "";

	// Check if the enviroment variable exists
	//
	DWORD valueLen = GetEnvironmentVariableA(name.c_str(), nullptr, 0);
	if (valueLen != 0)
	{
		unique_ptr<char[]> value = std::make_unique<char[]>(valueLen);

		DWORD valueLen2 = GetEnvironmentVariableA(name.c_str(), value.get(), valueLen);
		if (valueLen2 + 1 == valueLen)
		{
			result = value.get();
		}
	}

	return result;
}

//----------------------------------------------------------------------------
// Name: JavaExtensionUtils::LoadJvm
//
// Description:
//  Attempts to the load the jvm.dll from the path provided
//
// RETURNS:
//  Function pointer to JNICreate_JVM function
//
// NOTES:
//  If loading the JVM fails, then an exception is thrown
//
JavaExtensionUtils::fn_createJvm JavaExtensionUtils::LoadJvm(const string& jvmPath)
{
	g_jvmDll = LoadLibraryExA(jvmPath.c_str(), nullptr, 0);

	if (g_jvmDll == nullptr)
	{
		throw runtime_error(
				  "Failed to load jvm.dll from path " + jvmPath +
				  ". Ensure JRE_HOME is set.");
	}

	return reinterpret_cast<fn_createJvm>(GetProcAddress(g_jvmDll, "JNI_CreateJavaVM"));
}

//----------------------------------------------------------------------------
// Name: JavaExtensionUtils::ShutdownJvm
//
// Description:
//  Cleans up JVM resources by calling the Invocation API function: DestroyJavaVM.
//
void JavaExtensionUtils::ShutdownJvm(JavaVM *jvm)
{
	// Destroy the JVM
	//
	LOG("Shutting down JVM");
	if (jvm != nullptr)
	{
		int rc = jvm->DestroyJavaVM();
		if (rc == 0)
		{
			jvm = nullptr;
		}
		else
		{
			string msg = "Failed to destroy JVM. JNI error code: " + to_string(rc) + ".";
			LOG_ERROR(msg);
		}
	}
}

//----------------------------------------------------------------------------
// Name: JavaExtensionUtils::UnloadJvm
//
// Description:
//  Frees the HANDLE to the jvm.dll if valid
//
void JavaExtensionUtils::UnloadJvm()
{
	// If the extension was loaded, free it
	//
	if (g_jvmDll != nullptr)
	{
		FreeLibrary(g_jvmDll);
		g_jvmDll = nullptr;
	}
}
