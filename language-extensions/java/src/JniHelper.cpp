//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: JniHelper.cpp
// @Owner: brnieb
//
// Purpose:
//	 Wrappers around commonly used JNI functions
//
//*********************************************************************
#ifdef _WIN64
#include <windows.h>
#endif
#include <assert.h>
#include <iostream>
#include <jni.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <string>
#ifndef _WIN64
// These sal include headers must follow the standard c++ headers, or there
// will be compilation issues. This is because headers like iostream/algorithm use
// variables like __in which are the same as a SAL annotation causing redefinition issues.
//
#include <sal_def.h>
#include <xplat_sal.h>
#endif
#include "Logger.h"
#include "JniHelper.h"

using namespace std;

// The name of the method to invoke to retrieve an exception's details
//
const std::string x_getExceptionMsgMethodName = "printStackTrace";

//--------------------------------------------------------------------------------------------------
// Name: JniHelper::HandleJavaException
//
// Description:
//  When an exception occurs in the user program, attempt to
//  print out relevant information given the exception
//
SQLRETURN JniHelper::HandleJavaException(_In_ JNIEnv *env)
{
	// Auto release local references
	// 1 for the exception object
	// 1 for the exception class
	//
	AutoJniLocalFrame jFrame(env, 2);

	SQLRETURN status = SQL_SUCCESS;

	jthrowable exception = env->ExceptionOccurred();

	if (exception != nullptr)
	{
		status = SQL_ERROR;

		// Clear the exception and attempt to log it if the exception class can be found
		// and the exception details can be found.
		//
		env->ExceptionClear();
		jclass exceptionClass = env->GetObjectClass(exception);
		if (exceptionClass != nullptr)
		{
			jmethodID getStringId = env->GetMethodID(exceptionClass,
													 x_getExceptionMsgMethodName.c_str(),
													 "()V");

			if (getStringId != nullptr)
			{
				LOG_JAVA_EXCEPTION("");
				env->CallObjectMethod(exception, getStringId);
			}
		}
	}

	return status;
}

//--------------------------------------------------------------------------------------------------
// Name: JniHelper::ThrownOnJavaException
//
// Description:
//  Throw exception when a Java exception has occurred.
//
void JniHelper::ThrowOnJavaException(_In_ JNIEnv *env)
{
	// Auto cleanup reference to the exception
	//
	AutoJniLocalFrame jFrame(env, 1);
	jthrowable exception = env->ExceptionOccurred();

	if (exception != nullptr)
	{
		throw java_exception_error("An exception has occurred in Java");
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JniHelper::LogJavaException
//
// Description:
//  Logs the Java exception if one has occurred
//
void JniHelper::LogJavaException(_In_ JNIEnv *env)
{
	// Auto release local references
	// 1 for the exception object
	// 1 for the exception class
	//
	AutoJniLocalFrame jFrame(env, 2);

	jthrowable exception = env->ExceptionOccurred();

	if (exception != nullptr)
	{
		// Clear the exception and attempt to log it if the exception class can be found
		// and the exception details can be found.
		//
		env->ExceptionClear();
		jclass exceptionClass = env->GetObjectClass(exception);

		if (exceptionClass != nullptr)
		{
			jmethodID getStringId = env->GetMethodID(exceptionClass,
													 x_getExceptionMsgMethodName.c_str(),
													 "()V");

			if (getStringId != nullptr)
			{
				env->CallObjectMethod(exception, getStringId);
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JniHelper::FindMethod
//
// Description:
// Attempts to finds a Java method through JNI
//
jmethodID JniHelper::FindMethod(
	_In_ JNIEnv		  *env,
	_In_ jclass		  jClass,
	_In_ const string &funcName,
	_In_ const string &funcSignature)
{
	jmethodID result = env->GetMethodID(
		jClass,
		funcName.c_str(),
		funcSignature.c_str());

	ThrowOnJavaException(env);

	return result;
}