//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JniHelper.cpp
//
// Purpose:
//  Wrappers around commonly used JNI functions
//
//*********************************************************************
#include "JniHelper.h"
#include "Logger.h"

using namespace std;

//--------------------------------------------------------------------------------------------------
// Name: JniHelper::ThrownOnJavaException
//
// Description:
//  Throw exception when a Java exception has occurred.
//
void JniHelper::ThrowOnJavaException(JNIEnv *env)
{
	ThrowOnJavaException(env, "");
}

//--------------------------------------------------------------------------------------------------
// Name: JniHelper::ThrownOnJavaException
//
// Description:
//  Throw exception with the provided error message when a Java exception has occurred.
//
void JniHelper::ThrowOnJavaException(JNIEnv *env, string &&errorMsg)
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
void JniHelper::LogJavaException(JNIEnv *env)
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
	JNIEnv			*env,
	jclass			jClass,
	const string	&funcName,
	const string	&funcSignature)
{
	jmethodID result = env->GetMethodID(
		jClass,
		funcName.c_str(),
		funcSignature.c_str());

	ThrowOnJavaException(env);

	return result;
}
