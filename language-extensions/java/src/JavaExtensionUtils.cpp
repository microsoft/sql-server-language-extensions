//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JavaExtensionUtils.cpp
//
// Purpose:
//  Platform independent utility functions used throughout the
//  extension
//
//*********************************************************************
#include "JavaExtensionUtils.h"
#include "JavaLibraryUtils.h"
#include "Logger.h"
#include <sstream>
#include <cstring> // Needed for memset in Linux

using namespace std;

// Pointer to the JVM (Java Virtual Machine)
//
static JavaVM *g_jvm = nullptr;

//----------------------------------------------------------------------------
// Name: JavaExtensionUtils::GetClassPath
//
// Description:
//  Construct the classpath from the enviroment variables CLASSPATH, the public
//  and private external library paths
//
// Returns:
//  String of the classpath, if there was no paths found then "." is returned
//
// Notes:
//  Platform specific functions are called to handle retrieving enviroment
//  variables
//
string JavaExtensionUtils::GetClassPath()
{
	string classpath = "";

	// Check if the 'CLASSPATH' enviroment variable exists
	//
	string envValue = GetEnvVariable("CLASSPATH");

	// Get the external libraries path
	//
	string librariesPath = JavaLibraryUtils::GetLibrariesClassPath();

	// A CLASSPATH could not be determined, so the use the current directory
	//
	if (envValue.empty() && librariesPath.empty())
	{
		classpath = ".";
	}
	else
	{
		// Append the enviroment variable first, then the libraries path
		//
		if (!envValue.empty())
		{
			classpath = envValue;

			if (!librariesPath.empty())
			{
				classpath += JavaExtensionUtils::GetClassPathSeparator() + librariesPath;
			}
		}
		else
		{
			classpath = librariesPath;
		}
	}

	LOG("CLASSPATH=" + classpath);

	return classpath;
}

//----------------------------------------------------------------------------
// Name: JavaExtensionUtils::GetJvmPath
//
// Description:
//  Tries to construct a JVM path from the JRE_HOME enviroment variable
//
// Returns:
//  String to the full path to the JVM library if valid, else the filename of
//  the JVM library.
//
string JavaExtensionUtils::GetJvmPath()
{
	string jvmDllPath = "";

	// Get the enviroment variable JRE_HOME
	//
	string jreHomeValue = GetEnvVariable("JRE_HOME");

	// If the enviroment variable exists then check if the JVM exists
	//
	if (!jreHomeValue.empty())
	{
		// Combine the well known directory to the JVM library from the JRE_HOME
		//
		jvmDllPath = CombinePath(jreHomeValue, GetRelativeJvmPath());

		if (!IsValidFile(jvmDllPath))
		{
			LOG("Could not find the JVM library from the JRE_HOME path: " + jvmDllPath);

			// The full path to the JVM library is not valid, clear the path.
			//
			jvmDllPath.clear();
		}
	}

	if (jvmDllPath.empty())
	{
		// A JVM library to load could not be found based on the JRE_HOME
		// path, try to load the JVM without a path in case it is
		// added to the global PATH enviroment variable
		//
		jvmDllPath = GetJvmFilename();

		LOG("A full path to the JVM library could not be determined");
	}

	return jvmDllPath;
}

//----------------------------------------------------------------------------
// Name: JavaExtensionUtils::CombinePath
//
// Description:
//  Combines the two paths
//
// Returns:
//  String of the combined paths
//
// Notes:
//  This function only checks if the base path ends in the path seperator,
//  which means the path to add is assumed to not start with a path seperator.
//
string JavaExtensionUtils::CombinePath(const string &basePath, const string &pathToAdd)
{
	string path = basePath;

	if (!path.empty())
	{
		if (path.back() != GetPathSeparator())
		{
			path += GetPathSeparator();
		}
	}

	path += pathToAdd;

	return path;
}

//--------------------------------------------------------------------------------------------------
// Name: JavaExtensionUtils::SplitString
//
// Description:
//  Split a string based on a delimiter
//
// Returns:
//  vector of strings that represent the tokenized input string
//
vector<string> JavaExtensionUtils::SplitString(const string &str, const char delimiter)
{
	vector<string> out;
	stringstream ss(str); // Turn the string into a stream.
	string split;

	while (getline(ss, split, delimiter))
	{
		out.push_back(split);
	}

	return out;
}

//----------------------------------------------------------------------------
// Name: JavaExtensionUtils::CreateJvm
//
// Description:
//  Creates the JVM enviroment by loading the JVM library and calling
//  JNICreate_JVM
//
// Returns:
//  Pointer to JNIEnv created if successful
//
// Throws:
//  Throws if the failed to create the JVM enviroment
//
JNIEnv* JavaExtensionUtils::CreateJvm()
{
	JNIEnv *result = nullptr;

	// Load the JVM and return the address for JNICreate_JVM function
	//
	fn_createJvm fnCreateJvm = LoadJvm(GetJvmPath());

	if (fnCreateJvm == nullptr)
	{
		throw runtime_error("Failed to find function JNI_CreateJVM in JVM library");
	}

	// Get all Java options from JAVA_OPTS environment variable.
	// If there are multiple options, they should be separated with spaces.
	//
	string javaOpts = GetEnvVariable("JAVA_OPTS");
	vector<string> splitOptions = SplitString(javaOpts, ' ');
	int numOptions = splitOptions.size();

	// Prepare loading for the JVM by declaring the initialization
	// arguments and the invocation options
	//
	JavaVMInitArgs vm_args;
	vector<JavaVMOption> options(numOptions + 2); // +2 to make space for classpath and file encoding

	string classPathPrefix = "-Djava.class.path";
	string optionStringClassPath = classPathPrefix + "=" + GetClassPath();
	options[0].optionString = const_cast<char*>(optionStringClassPath.c_str());
	options[0].extraInfo = 0;

	// The default encoding used by jvm is system encoding which in case of
	// Linux is UTF-8 and for Windows is Cp1252. The following code makes it uniform
	// across both Windows and Linux by explicitly setting the encoding to UTF-8.
	//
	string fileEncodingPrefix = "-Dfile.encoding";
	string optionStringFileEncoding = fileEncodingPrefix + "=UTF-8";
	options[1].optionString = const_cast<char*>(optionStringFileEncoding.c_str());
	options[1].extraInfo = 0;

	// Set all the other options
	//
	for (int i = 0; i < numOptions; ++i)
	{
		int index = i + 2; // +2 to account for the two default options
		string optionString = splitOptions[i];

		// Throw error if user set java class path or file encoding
		//
		if (optionString.compare(0, classPathPrefix.length(), classPathPrefix) == 0 ||
			optionString.compare(0, fileEncodingPrefix.length(), fileEncodingPrefix) == 0)
		{
			throw runtime_error("Cannot add java.class.path or file.encoding as JAVA_OPTS "
				"because they are preset in the java extension");
		}

		options[index].optionString = const_cast<char*>(splitOptions[i].c_str());
		options[index].extraInfo = 0;
	}

	// We set the Java version we want here and specify the number of options
	//
	memset(&vm_args, 0, sizeof(vm_args));
	vm_args.version = JNI_VERSION_10;
	vm_args.nOptions = options.size();
	vm_args.options = options.data();
	vm_args.ignoreUnrecognized = JNI_FALSE;

	// Load and initialize Java VM and JNI interface
	//
	jint rc = fnCreateJvm(&g_jvm, reinterpret_cast<void**>(&result), &vm_args);
	if (rc == JNI_OK)
	{
		// Display JVM version. Since the version is displayed like 0x90000, we right
		// shift to get the major version (it is and'ed with 0x0f to zero out the upper
		// 4 digits). We zero out the upper 4 digits as well to get the minor version.
		//
		jint ver = result->GetVersion();
		string version = to_string((ver >> 16) & 0x0f) + "." + to_string(ver & 0x0f);

		string msg = "JVM load succeeded: Version " + version + ".";
		LOG(msg);
	}
	else
	{
		string msg = "Failed to create Java Virtual Machine. JNI error code: " +
					 to_string(rc) + ".";

		throw runtime_error(msg);
	}

	return result;
}

//----------------------------------------------------------------------------
// Name: JavaExtensionUtils::CleanupJvm
//
// Description:
//  Cleans up all JVM resources, if valid. Destroys the JVM enviroment,
//  and unloads the JVM library
//
void JavaExtensionUtils::CleanupJvm()
{
	// Call platform specific function to shutdown the JVM
	//
	ShutdownJvm(g_jvm);

	// Call platform specific function to unload JVM library
	//
	UnloadJvm();
}
