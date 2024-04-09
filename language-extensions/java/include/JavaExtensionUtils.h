//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JavaExtensionUtils.h
//
// Purpose:
//  Utility functions used by the Java extension.
//
//*********************************************************************
#pragma once

#include "Common.h"

//---------------------------------------------------------------------
// Description:
//	General utility functions for the Java extension
//
class JavaExtensionUtils
{
public:
	// Get the value of an enviroment variable
	//
	static std::string GetEnvVariable(const std::string &name);

	// Get the platform specifc classpath seperator
	//
	static const char GetClassPathSeparator();

	// Get the platform specifc path seperator
	//
	static const char GetPathSeparator();

	// Combines two paths
	//
	static std::string CombinePath(const std::string &basePath, const std::string &pathToAdd);

	// Determines if the file path is valid
	//
	static bool IsValidFile(const std::string &file);

	// Split a string based on a delimiter
	//
	static std::vector<std::string> SplitString(const std::string &str, const char delimiter);

	// Creates a JVM
	//
	static JNIEnv* CreateJvm();

	// Cleans up JVM
	//
	static void CleanupJvm();

private:
	// Function pointer definition for JNI_CreateJVM
	//
	using fn_createJvm = jint(*)(JavaVM **pvm, void **penv, void *args);

	// Constructs the classpath from the enviroment variable,
	// and external library paths
	//
	static std::string GetClassPath();

	// Constructs the path to the JVM library to load
	//
	static std::string GetJvmPath();

	// Constructs the platform specific relative path to the JVM with respect to
	// the JRE_HOME path
	//
	static const std::string& GetRelativeJvmPath();

	// Returns the platform specific name of the JVM library
	//
	static const std::string& GetJvmFilename();

	// Loads the JVM library and returns a function pointer to the JNICreate_JVM
	//
	static fn_createJvm LoadJvm(const std::string &jvmPath);

	// Cleans up the JVM instance
	//
	static void ShutdownJvm(JavaVM *jvm);

	// Unloads the JVM library
	//
	static void UnloadJvm();
};
