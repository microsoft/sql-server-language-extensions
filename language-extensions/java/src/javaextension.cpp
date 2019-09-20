//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: javaextension.cpp
// @Owner: brnieb
//
// Purpose:
//	 Java extension DLL that can be loaded by ExtHost. This library loads
//	 JVM, handles communication with ExtHost, and executes user-specified
//	 Java method
//
//*********************************************************************

#ifdef _WIN64
#include <windows.h>
#endif
#include <assert.h>
#include <jni.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sqltypes.h>
#include <sqlext.h>
#include <algorithm>
#include <unordered_map>
#include <cassert>
#include <memory>
#include <locale>
#include <codecvt>
#include <stdlib.h>
#ifndef _WIN64
#include <linux/limits.h>
#include <string.h>
// These sal include headers must follow the standard c++ headers, or there
// will be compilation issues. This is because headers like iostream/algorithm use
// variables like __in which are the same as a SAL annotation causing redefinition issues.
//
#include <sal_def.h>
#include <xplat_sal.h>
#endif
#include "Logger.h"
#include "JavaExtensionUtils.h"
#include "JavaArgContainer.h"
#include "JavaLibraryUtils.h"
#include "JniHelper.h"
#include "JniTypeHelper.h"
#include "JavaDataset.h"
#include "JavaSession.h"
#include "JavaPathSettings.h"
#include "sqlexternallanguage.h"

using namespace std;

// Pointer to native interface
//
static JNIEnv *g_env = nullptr;

// A global object to keep track of the input data information,
// java class information, and output data information
//
static JavaSession *g_sessionData = nullptr;

//--------------------------------------------------------------------------------------------------
// Name: CheckSessionEnvInitialized
//
// Description:
//	Throws a runtime_error exception of a function is called before the session environment has
//	been initialized
//
void CheckSessionEnvInitialized(string &&FuncName)
{
	if (g_env == nullptr)
	{
		throw runtime_error("Function " + FuncName + " called before extension is initialized");
	}
	else if (g_sessionData == nullptr)
	{
		throw runtime_error("Function " + FuncName + " called before session is initialized");
	}
}

//--------------------------------------------------------------------------------------------------
// Name: GetInterfaceVersion
//
// Description:
//	Returns the API interface version for the extension
//
// Returns:
//	EXTERNAL_LANGUAGE_EXTENSION_API
//
SQLUSMALLINT GetInterfaceVersion()
{
	return EXTERNAL_LANGUAGE_EXTENSION_API;
}

//--------------------------------------------------------------------------------------------------
// Name: Init
//
// Description:
//	Initialize the java extension. Until registration, nothing is needed here.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN Init(
	_In_ SQLCHAR *languageParams,
	_In_ SQLULEN languageParamsLen,
	_In_ SQLCHAR *languagePath,
	_In_ SQLULEN languagePathLen,
	_In_ SQLCHAR *publicLibraryPath,
	_In_ SQLULEN publicLibraryPathLen,
	_In_ SQLCHAR *privateLibraryPath,
	_In_ SQLULEN privateLibraryPathLen)
{
	SQLRETURN result = SQL_SUCCESS;

	LOG("JavaExtension::Init");

	try
	{
		JavaPathSettings::Init(
			languageParams,
			languagePath,
			publicLibraryPath,
			privateLibraryPath);
		g_env = JavaExtensionUtils::CreateJvm();
	}
	catch (exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function Init()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: InitSession
//
// Description:
//	Initializes session-specific data. We store the schema and find the main class and
//	method to execute here.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN InitSession(
	_In_ SQLGUID	  SessionId,
	_In_ SQLUSMALLINT TaskId,
	_In_ SQLUSMALLINT NumTasks,
	_In_ SQLCHAR	  *Script,
	_In_ SQLULEN	  ScriptLength,
	_In_ SQLUSMALLINT InputSchemaColumnsNumber,
	_In_ SQLUSMALLINT ParametersNumber,
	_In_ SQLCHAR	  *InputDataName,
	_In_ SQLUSMALLINT InputDataNameLength,
	_In_ SQLCHAR	  *OutputDataName,
	_In_ SQLUSMALLINT OutputDataNameLength
	)
{
	string msg = "JavaExtension::InitSession";
	LOG(msg);

	SQLRETURN result = SQL_SUCCESS;

	try
	{
		if (g_env == nullptr)
		{
			throw runtime_error("Function InitSession() called before extension is initialized");
		}

		g_sessionData = new JavaSession();

		string guidToString = JniTypeHelper::ConvertGuidToString(&SessionId);

		msg = "Starting session: " + guidToString + " with task id: " +
			  to_string(TaskId) + ".";
		LOG(msg);

		g_sessionData->Init(g_env,
							SessionId,
							TaskId,
							NumTasks,
							Script,
							ScriptLength,
							InputSchemaColumnsNumber,
							ParametersNumber,
							InputDataName,
							InputDataNameLength,
							OutputDataName,
							OutputDataNameLength);
	}
	catch (java_exception_error &ex)
	{
		result = SQL_ERROR;

		LOG_JAVA_EXCEPTION(ex.what());

		JniHelper::LogJavaException(g_env);
	}
	catch (exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function InitSession()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: InitColumn
//
// Description:
//	Initializes column-specific data. We store the name and the data type of the column
//  here.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN InitColumn(
	_In_ SQLGUID	  SessionId,
	_In_ SQLUSMALLINT TaskId,
	_In_ SQLUSMALLINT ColumnNumber,
	_In_ SQLCHAR	  *ColumnName,
	_In_ SQLSMALLINT  ColumnNameLength,
	_In_ SQLSMALLINT  DataType,
	_In_ SQLULEN	  ColumnSize,
	_In_ SQLSMALLINT  DecimalDigits,
	_In_ SQLSMALLINT  Nullable,
	_In_ SQLSMALLINT  PartitionByNumber,
	_In_ SQLSMALLINT  OrderByNumber
	)
{
	string msg = "JavaExtension::InitColumn";
	LOG(msg);

	SQLRETURN result = SQL_SUCCESS;

	try
	{
		CheckSessionEnvInitialized("InitColumn");

		g_sessionData->InitColumn(
			ColumnNumber,
			ColumnName,
			ColumnNameLength,
			DataType,
			ColumnSize,
			DecimalDigits,
			Nullable,
			PartitionByNumber,
			OrderByNumber);
	}
	catch (java_exception_error &ex)
	{
		result = SQL_ERROR;

		LOG_JAVA_EXCEPTION(ex.what());

		JniHelper::LogJavaException(g_env);
	}
	catch (exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function InitColumn()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: InitParam
//
// Description:
//	Initializes parameter-specific data.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN InitParam(
	SQLGUID		 SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ParamNumber,
	SQLCHAR		 *ParamName,
	SQLSMALLINT	 ParamNameLength,
	SQLSMALLINT	 DataType,
	SQLULEN		 ArgSize,
	SQLSMALLINT	 DecimalDigits,
	SQLPOINTER	 ArgValue,
	SQLINTEGER	 StrLen_or_Ind,
	SQLSMALLINT	 InputOutputType
	)
{
	string msg = "JavaExtension::InitParam";
	LOG(msg);

	SQLRETURN result = SQL_SUCCESS;

	try
	{
		CheckSessionEnvInitialized("InitParam");

		g_sessionData->InitParam(
			ParamNumber,
			ParamName,
			ParamNameLength,
			DataType,
			ArgSize,
			DecimalDigits,
			ArgValue,
			StrLen_or_Ind,
			InputOutputType);
	}
	catch (java_exception_error &ex)
	{
		result = SQL_ERROR;

		LOG_JAVA_EXCEPTION(ex.what());

		JniHelper::LogJavaException(g_env);
	}
	catch (exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function InitParam()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: Execute
//
// Description:
//  Given the data from ExtHost, convert and populate the arrays in the user java program. Then,
//  invoke the specified function and retrieve the output schema and convert the data back to
//  ODBC types.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN Execute(
	_In_ SQLGUID		SessionId,
	_In_ SQLUSMALLINT	TaskId,
	_In_ SQLULEN		RowsNumber,
	_In_opt_ SQLPOINTER *Data,
	_In_opt_ SQLINTEGER **StrLen_or_Ind,
	_Out_ SQLUSMALLINT	*OutputSchemaColumnsNumber
	)
{
	string msg = "JavaExtension::Execute";
	LOG(msg);

	SQLRETURN result = SQL_SUCCESS;
	*OutputSchemaColumnsNumber = 0;

	try
	{
		CheckSessionEnvInitialized("Execute");

		g_sessionData->ExecuteWorkflow(
			RowsNumber,
			Data,
			StrLen_or_Ind,
			OutputSchemaColumnsNumber);
	}
	catch (java_exception_error &ex)
	{
		result = SQL_ERROR;

		LOG_JAVA_EXCEPTION(ex.what());

		JniHelper::LogJavaException(g_env);
	}
	catch (exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function Execute()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: GetResultColumn
//
// Description:
//  Returns information about the output column
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN GetResultColumn(
	_In_ SQLGUID	  SessionId,
	_In_ SQLUSMALLINT TaskId,
	_In_ SQLUSMALLINT ColumnNumber,
	_Out_ SQLSMALLINT *DataType,
	_Out_ SQLULEN	  *ColumnSize,
	_Out_ SQLSMALLINT *DecimalDigits,
	_Out_ SQLSMALLINT *Nullable
	)
{
	LOG("JavaExtension::GetResultColumn");

	SQLRETURN result = SQL_SUCCESS;

	try
	{
		CheckSessionEnvInitialized("GetResultColumn");

		g_sessionData->GetResultColumn(
			ColumnNumber,
			DataType,
			ColumnSize,
			DecimalDigits,
			Nullable);
	}
	catch (exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function GetResultColumn()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: GetResults
//
// Description:
//	Returns the output data as well as the null map retrieved from the user program
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN GetResults(
	_In_ SQLGUID		SessionId,
	_In_ SQLUSMALLINT	TaskId,
	_Out_ SQLULEN		*RowsNumber,
	_Outptr_ SQLPOINTER **Data,
	_Outptr_ SQLINTEGER ***StrLen_or_Ind
	)
{
	LOG("JavaExtension::GetResults");

	SQLRETURN result = SQL_SUCCESS;

	try
	{
		CheckSessionEnvInitialized("GetResults");

		g_sessionData->GetResults(
			RowsNumber,
			Data,
			StrLen_or_Ind);
	}
	catch (exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function GetResults()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: GetOutputParam
//
// Description:
//	Returns the output parameter's data.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN GetOutputParam(
	_In_ SQLGUID	  SessionId,
	_In_ SQLUSMALLINT TaskId,
	_In_ SQLUSMALLINT ParamNumber,
	_Out_ SQLPOINTER  *ParamValue,
	_Out_ SQLINTEGER  *StrLen_or_Ind)
{
	LOG("JavaExtension::GetOutputParam");

	SQLRETURN result = SQL_SUCCESS;

	try
	{
		CheckSessionEnvInitialized("GetOutputParam");

		g_sessionData->GetOutputParam(
			ParamNumber,
			ParamValue,
			StrLen_or_Ind);
	}
	catch (const java_exception_error &ex)
	{
		result = SQL_ERROR;

		LOG_JAVA_EXCEPTION(ex.what());

		JniHelper::LogJavaException(g_env);
	}
	catch (const exception &ex)
	{
		result = SQL_ERROR;

		LOG_ERROR(ex.what());
	}
	catch (...)
	{
		result = SQL_ERROR;

		LOG_ERROR("Unexpected exception occurred in function GetOutputParam()");
	}

	return result;
}

//--------------------------------------------------------------------------------------------------
// Name: CleanupSession
//
// Description:
//	Cleans up the output data buffers that we persist for
//	ExtHost to finish processing the data
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN CleanupSession(
	_In_ SQLGUID	  SessionId,
	_In_ SQLUSMALLINT TaskId
	)
{
	LOG("JavaExtension::CleanupSession");

	// Clean up the session
	//
	if (g_sessionData != nullptr)
	{
		g_sessionData->Cleanup();

		delete g_sessionData;
		g_sessionData = nullptr;
	}

	return SQL_SUCCESS;
}

//--------------------------------------------------------------------------------------------------
// Name: Cleanup
//
// Description:
//	Completely clean up the extension and destroy the JVM
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN Cleanup()
{
	string msg = "Calling cleanup";
	LOG(msg);

	// Cleanup JVM
	//
	JavaExtensionUtils::CleanupJvm();

	return SQL_SUCCESS;
}

