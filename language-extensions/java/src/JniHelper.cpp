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

//--------------------------------------------------------------------------------------------------
// Name: JniHelper::ThrownOnJavaException
//
// Description:
//  Throw exception when a Java exception has occurred.
//
void JniHelper::ThrowOnJavaException(_In_ JNIEnv *env)
{
	ThrowOnJavaException(env, "");
}

//--------------------------------------------------------------------------------------------------
// Name: JniHelper::ThrownOnJavaException
//
// Description:
//  Throw exception with the provided error message when a Java exception has occurred.
//
void JniHelper::ThrowOnJavaException(_In_ JNIEnv *env, _In_ std::string &&errorMsg)
{
	jboolean isException = env->ExceptionCheck();

	if (isException == JNI_TRUE)
	{
		throw java_exception_error(errorMsg);
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
	jboolean isException = env->ExceptionCheck();

	if (isException == JNI_TRUE)
	{
		env->ExceptionDescribe();
		env->ExceptionClear();
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
