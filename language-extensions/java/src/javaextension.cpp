//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: javaextension.cpp
//
// Purpose:
//  Java extension DLL that can be loaded by ExtHost. This library loads
//  JVM, handles communication with ExtHost, and executes user-specified
//  Java method
//
//*********************************************************************
#include "Common.h"
#include "JavaExtensionUtils.h"
#include "JniHelper.h"
#include "JniTypeHelper.h"
#include "JavaSession.h"
#include "JavaPathSettings.h"
#include "Logger.h"
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
//	Initialize the java extension.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN Init(
	SQLCHAR *languageParams,
	SQLULEN languageParamsLen,
	SQLCHAR *languagePath,
	SQLULEN languagePathLen,
	SQLCHAR *publicLibraryPath,
	SQLULEN publicLibraryPathLen,
	SQLCHAR *privateLibraryPath,
	SQLULEN privateLibraryPathLen)
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
	SQLGUID      SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT NumTasks,
	SQLCHAR      *Script,
	SQLULEN      ScriptLength,
	SQLUSMALLINT InputSchemaColumnsNumber,
	SQLUSMALLINT ParametersNumber,
	SQLCHAR      *InputDataName,
	SQLUSMALLINT InputDataNameLength,
	SQLCHAR      *OutputDataName,
	SQLUSMALLINT OutputDataNameLength
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
	SQLGUID      SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ColumnNumber,
	SQLCHAR      *ColumnName,
	SQLSMALLINT  ColumnNameLength,
	SQLSMALLINT  DataType,
	SQLULEN      ColumnSize,
	SQLSMALLINT  DecimalDigits,
	SQLSMALLINT  Nullable,
	SQLSMALLINT  PartitionByNumber,
	SQLSMALLINT  OrderByNumber
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
	SQLGUID      SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ParamNumber,
	SQLCHAR      *ParamName,
	SQLSMALLINT  ParamNameLength,
	SQLSMALLINT  DataType,
	SQLULEN      ArgSize,
	SQLSMALLINT  DecimalDigits,
	SQLPOINTER   ArgValue,
	SQLINTEGER   StrLen_or_Ind,
	SQLSMALLINT  InputOutputType
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
	SQLGUID      SessionId,
	SQLUSMALLINT TaskId,
	SQLULEN      RowsNumber,
	SQLPOINTER   *Data,
	SQLINTEGER   **StrLen_or_Ind,
	SQLUSMALLINT *OutputSchemaColumnsNumber
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
	SQLGUID      SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ColumnNumber,
	SQLSMALLINT  *DataType,
	SQLULEN      *ColumnSize,
	SQLSMALLINT  *DecimalDigits,
	SQLSMALLINT  *Nullable
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
	SQLGUID      SessionId,
	SQLUSMALLINT TaskId,
	SQLULEN      *RowsNumber,
	SQLPOINTER   **Data,
	SQLINTEGER   ***StrLen_or_Ind
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
	SQLGUID      SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ParamNumber,
	SQLPOINTER   *ParamValue,
	SQLINTEGER   *StrLen_or_Ind)
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
SQLRETURN CleanupSession(SQLGUID SessionId, SQLUSMALLINT TaskId)
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
