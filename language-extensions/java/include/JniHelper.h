//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JniHelper.h
//
// Purpose:
//  Wrappers around commonly used JNI functions
//
//*********************************************************************
#pragma once

#include "Common.h"
#include <cassert>

//---------------------------------------------------------------------
// Description:
// Wrapper class around commonly used JNI functions
//
class JniHelper
{
public:
	// Checks if an Java exception occurred and throws an exception
	//
	static void ThrowOnJavaException(JNIEnv *env);

	// Checks if an Java exception occurred and throws an exception
	//
	static void ThrowOnJavaException(JNIEnv *env, std::string &&errorMsg);

	// Logs the Java exception
	//
	static void LogJavaException(JNIEnv *env);

	// Finds the method ID for the class, name, and signature
	//
	static jmethodID FindMethod(
		JNIEnv            *env,
		jclass            jClass,
		const std::string &funcName,
		const std::string &funcSignature);
};

//---------------------------------------------------------------------
// Description:
// In JNI, local references are associated with the current local frame,
// when call native code from Java these are created and managed automatically.
// But when calling Java from native, there is no local frame created by default.
// This class manages a local frame pushing and popping, when this object is
// destroyed all the JNI local references created during the lifetime of this
// object are released.
//
class AutoJniLocalFrame
{
public:
	AutoJniLocalFrame(JNIEnv *env, jint capacity)
	{
		assert(env != nullptr);

		m_env = env;

		m_env->PushLocalFrame(capacity);
	}

	~AutoJniLocalFrame()
	{
		m_env->PopLocalFrame(nullptr);
	}

private:
	JNIEnv *m_env;
};

//---------------------------------------------------------------------
// Description:
// Specific runtime exception indicating a Java exception has occurred
//
class java_exception_error : public std::runtime_error
{
	using runtime_error::runtime_error;
};
