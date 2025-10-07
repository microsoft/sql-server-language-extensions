//**************************************************************************************************
// ONNXRuntime-extension : A language extension implementing the SQL Server AIRuntime extension
// for ONNX Runtime.
// Copyright (C) 2025 Microsoft Corporation.
//
// @File: main.cpp
//
// Purpose:
//  This file contains the necessary methods for the AIRuntimeHost process.
//
//**************************************************************************************************

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <locale>
#include <memory>
// Windows headers
#if defined(_WIN32)
#include <strsafe.h>
#include <windows.h>
#endif
#include "sqlexternallanguage.h"
#include "sqlexternallibrary.h"
#include <cassert>
#include <sql.h>
#include <sqlext.h>
#include "util.h"
#include "OnnxSession.h"
#include "nlohmann/json.hpp"

using namespace std;

// Global variables

// Enables verbose logging for debugging and diagnostics.
// Set to 'true' to enable detailed trace logs.
BOOL g_fVerboseTraceEnabled = true;

// Global unique pointer to the OnnxSession object.
// This manages the ONNX model session for the extension.
// Lifetime: Created during Init(), reset in CleanupSession(), valid for the session.
std::unique_ptr<OnnxSession> g_onnxsession = nullptr;

// Buffer to store the output vector embedding as a wide-character (WCHAR) string.
// Used for returning results to SQL Server.
// Lifetime: Allocated in GetOutputParam(), reset in CleanupSession(), valid for the session.
std::unique_ptr<WCHAR[]> g_outputBuffer = nullptr;

#pragma region HelperMethods

// Wrapper function for logging as Info messages.
// Usually this would show up in the SQL Server logs ExtensibilityData
// e.g. (`C:\Program Files\Microsoft SQL Server\MSSQL17.MSSQLSERVER\MSSQL\ExtensibilityData\AppContainer1\<sessionId>.txt`)
static void TRACELOG(const char *message)
{
	if (g_fVerboseTraceEnabled)
	{
		LogMessage(message, LogLevel::INFO);
	}
}

#pragma endregion

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
//	Initializes the AIRuntime extension. This initializes the global OnnxSession object.
//
// Returns:
//	SQL_SUCCESS on success, otherwise SQL_ERROR
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

	TRACELOG("OnnxExtension::Init");

	try
	{
		// languagePath should contain the LOCAL_RUNTIME_PATH having onnxruntime.dll
		g_onnxsession = std::make_unique<OnnxSession>(reinterpret_cast<char *>(languagePath));
	}
	catch (exception e)
	{
		TRACELOG(e.what());
		TRACELOG("Most of the times the issue is with file permission like: Load model from all-MiniLM-L6-v2.onnx failed:system error number 13");
		return SQL_ERROR;
	}
	catch (...)
	{
		TRACELOG("Unknown exception in Init.");
		return SQL_ERROR;
	}

	return SQL_SUCCESS;
}

//--------------------------------------------------------------------------------------------------
// Name: InitSession
//
// Description:
//	Initializes session-specific data.
//	This method is not used by AIRuntime, but it is required by the Extensibility Framework API for SQL Server
//
// Returns:
//	SQL_SUCCESS on success, otherwise SQL_ERROR
SQLRETURN InitSession(
	SQLGUID SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT NumTasks,
	SQLCHAR *Script,
	SQLULEN ScriptLength,
	SQLUSMALLINT InputSchemaColumnsNumber,
	SQLUSMALLINT ParametersNumber,
	SQLCHAR *InputDataName,
	SQLUSMALLINT InputDataNameLength,
	SQLCHAR *OutputDataName,
	SQLUSMALLINT OutputDataNameLength)
{
	return SQL_SUCCESS;
}

//--------------------------------------------------------------------------------------------------
// Name: InitColumn
//
// Description:
//	This method is not used by AIRuntime, but it is required by the Extensibility Framework API for SQL Server
//
// Returns:
//	SQL_SUCCESS on success, otherwise SQL_ERROR
SQLRETURN InitColumn(
	SQLGUID SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ColumnNumber,
	SQLCHAR *ColumnName,
	SQLSMALLINT ColumnNameLength,
	SQLSMALLINT DataType,
	SQLULEN ColumnSize,
	SQLSMALLINT DecimalDigits,
	SQLSMALLINT Nullable,
	SQLSMALLINT PartitionByNumber,
	SQLSMALLINT OrderByNumber)
{
	try
	{
		TRACELOG("OnnxExtension::InitColumn");
		return SQL_SUCCESS;
	}
	catch (...)
	{
		TRACELOG("Unknown exception in InitColumn.");
		return SQL_ERROR;
	}
}

//--------------------------------------------------------------------------------------------------
// Name: InitParam
//
// Description:
// In AIRuntime, model-related information is communicated through parameters.
// The parameters are passed to the extension in the InitParam function.
// The extension can then use these parameters to set up the model and perform inference.
//
// @param:
//  SessionId: Unique identifier (GUID) for the ai_generate_embedding execution session. Remains the same
//			 for a single execution request from SQL Server, even if there are multiple tasks.
//  TaskId:	Small integer identifying a specific task within a session. SQL Server may split work
//			 into multiple tasks for parallel execution; each task within the same session will have
//			 a unique TaskId.
//  ParamNumber: Small integer identifying a specific parameter within a task. Each parameter within
//			   a task will have a unique ParamNumber.
//  ParamName:   String representing the name of the parameter. Used to identify the parameter and its
//			   purpose.
//  ParamNameLength: Length of the ParamName string in characters.
//  DataType:	Small integer indicating the data type of the parameter value (e.g., SQL_WCHAR,
//			   SQL_CHAR, etc.).
//  ArgSize:	 Size of the parameter value in bytes. Indicates how much memory to allocate for the
//			   parameter.
//  DecimalDigits: Small integer indicating the number of decimal digits for numeric data types. Not
//				 used for string data types.
//  ArgValue:	Pointer to the actual value of the parameter. Can be a string, numeric value, or any
//			   other data type.
//  StrLen_or_Ind: Integer indicating the length of the ArgValue string in characters. If the value is
//				 SQL_NULL_DATA, it indicates that the parameter is null.
//  InputOutputType: Small integer indicating whether the parameter is an input or output parameter.
//				   Can be SQL_PARAM_INPUT, SQL_PARAM_OUTPUT, or SQL_PARAM_INPUT_OUTPUT.
SQLRETURN InitParam(
	SQLGUID SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ParamNumber,
	SQLCHAR *ParamName,
	SQLSMALLINT ParamNameLength,
	SQLSMALLINT DataType,
	SQLULEN ArgSize,
	SQLSMALLINT DecimalDigits,
	SQLPOINTER ArgValue,
	SQLINTEGER StrLen_or_Ind,
	SQLSMALLINT InputOutputType)
{
	try
	{
		TRACELOG(("OnnxExtension::InitParam with ParamName: " + string((char *)ParamName)).c_str());

		// Check if the session is already initialized
		if (!g_onnxsession)
		{
			TRACELOG("OnnxSession is not initialized.");
			return SQL_ERROR;
		}

		if (DataType != SQL_WCHAR)
		{
			TRACELOG("DataType is not SQL_WCHAR");
			return SQL_ERROR;
		}

		// Validate the parameter name and argument value.
		if (ParamNameLength <= 0 || !ArgValue)
		{
			TRACELOG("Invalid parameter name length or argument value.");
			return SQL_ERROR;
		}

		// Set the model location, input text for embedding generation, or default parameters based on the parameter name.
		// The parameter names are case-sensitive and should match the expected names exactly.
		// The expected parameter names are:
		// 1. MODEL_LOCATION: Specifies the location of the ONNX model file.
		// 2. INPUT_TEXT_FOR_EMBEDDING_GENERATION: Specifies the input text for generating embeddings.
		// 3. MODEL_DEFAULT_PARAMETERS: Specifies the default parameters for the model.
		if (ParamNameLength == strlen("MODEL_LOCATION") &&
			strncmp("MODEL_LOCATION", (const char *)ParamName, ParamNameLength) == 0)
		{
			g_onnxsession->SetModelLocation((WCHAR *)ArgValue, StrLen_or_Ind);
		}
		else if (ParamNameLength == strlen("INPUT_TEXT_FOR_EMBEDDING_GENERATION") &&
				 strncmp("INPUT_TEXT_FOR_EMBEDDING_GENERATION", (const char *)ParamName, ParamNameLength) == 0)
		{
			// This parameter is for converting an input wide-char string to a vector embedding.
			// Store the actual input in m_embedding_param and generate and store the embedding in output_str.
			ULONG cbBuffer = StrLen_or_Ind / sizeof(WCHAR) + 1;
			std::unique_ptr<WCHAR[]> buffer(new WCHAR[cbBuffer]);
			memset(buffer.get(), 0, sizeof(WCHAR) * cbBuffer);
			memcpy(buffer.get(), ArgValue, StrLen_or_Ind);
			buffer[cbBuffer - 1] = L'\0';

			// Use cross-platform utility for conversion
			std::string jsonString = WCHAR_to_utf8_string(buffer.get(), cbBuffer - 1);

			// Extract the value of the "prompt" key from the JSON string stored in m_embedding_param
			try
			{
				nlohmann::json jsonObject = nlohmann::json::parse(jsonString);

				if (jsonObject.contains("prompt") && jsonObject["prompt"].is_string())
				{
					std::string prompt = jsonObject["prompt"];
					if (!prompt.empty())
					{
						g_onnxsession->SetEmbeddingParam(const_cast<CHAR *>(prompt.c_str()), prompt.size());
					}
					else
					{
						TRACELOG("Error: Empty input for embedding generation.");
						return SQL_ERROR;
					}
				}
				else
				{
					TRACELOG("Error: JSON does not contain 'prompt' key or it is not a string.");
					return SQL_ERROR;
				}
			}
			catch (const nlohmann::json::parse_error &e)
			{
				TRACELOG("JSON parsing error: ");
				TRACELOG(e.what());
				return SQL_ERROR;
			}
			catch (const std::exception &e)
			{
				TRACELOG("Standard exception occurred: ");
				TRACELOG(e.what());
				return SQL_ERROR;
			}
			catch (...)
			{
				TRACELOG("An unknown error occurred while processing the JSON.");
				return SQL_ERROR;
			}

			// Assuming we have received model location now, start the Onnx session.
			// If the session is already started, reuse it.
			if (!g_onnxsession->SetupOnnxSession())
			{
				TRACELOG("Error: Failed to set up ONNX session.");
				return SQL_ERROR;
			}

			// Use m_embedding_param to generate the embedding and clean up that memory.
			if (!g_onnxsession->GenerateSingleEmbedding())
			{
				TRACELOG("Error: Failed to generate embedding.");
				return SQL_ERROR;
			}

			// The m_embedding_param is no longer needed, so clear it.
			g_onnxsession->ClearEmbeddingParam();
		}
		else if (ParamNameLength == strlen("MODEL_DEFAULT_PARAMETERS") &&
				 strncmp("MODEL_DEFAULT_PARAMETERS", (const char *)ParamName, ParamNameLength) == 0)
		{
			// Copy the PARAMETERS argument.
			g_onnxsession->SetDefaultParams((WCHAR *)ArgValue, StrLen_or_Ind);
		}

		return SQL_SUCCESS;
	}
	catch (...)
	{
		TRACELOG("Unknown exception in InitParam.");
		return SQL_ERROR;
	}
}

//--------------------------------------------------------------------------------------------------
// Name: Execute
//
// Description:
//	This method is not used by AIRuntime, but it is required by the Extensibility Framework API for SQL Server
//
// Returns:
//	SQL_SUCCESS on success, otherwise SQL_ERROR
SQLRETURN Execute(
	SQLGUID SessionId,
	SQLUSMALLINT TaskId,
	SQLULEN RowsNumber,
	SQLPOINTER *Data,
	SQLINTEGER **StrLen_or_Ind,
	SQLUSMALLINT *OutputSchemaColumnsNumber)
{
	try
	{
		TRACELOG("OnnxExtension::Execute");
		*OutputSchemaColumnsNumber = 0;
		return SQL_SUCCESS;
	}
	catch (...)
	{
		TRACELOG("Unknown exception in Execute.");
		return SQL_ERROR;
	}
}

//--------------------------------------------------------------------------------------------------
// Name: GetResultColumn
//
// Description:
//	This method is not used by AIRuntime, but it is required by the Extensibility Framework API for SQL Server
//
// Returns:
//	SQL_SUCCESS on success, otherwise SQL_ERROR
SQLRETURN GetResultColumn(
	SQLGUID SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ColumnNumber,
	SQLSMALLINT *DataType,
	SQLULEN *ColumnSize,
	SQLSMALLINT *DecimalDigits,
	SQLSMALLINT *Nullable)
{

	TRACELOG("OnnxExtension::GetResultColumn");
	return SQL_SUCCESS;

}

//--------------------------------------------------------------------------------------------------
// Name: GetResults
//
// Description:
//	This method is not used by AIRuntime, but it is required by the Extensibility Framework API for SQL Server
//
// Returns:
//	SQL_SUCCESS on success, otherwise SQL_ERROR
SQLRETURN GetResults(
	SQLGUID SessionId,
	SQLUSMALLINT TaskId,
	SQLULEN *RowsNumber,
	SQLPOINTER **Data,
	SQLINTEGER ***StrLen_or_Ind)
{
	TRACELOG("OnnxExtension::GetResults");
	return SQL_SUCCESS;
}

//--------------------------------------------------------------------------------------------------
// Name: GetOutputParam
//
// Description:
// Retrieves the result vector embedding for SQL Server.
// On Windows, returns the output buffer as a wide string (WCHAR, UTF-16) directly from m_output_str(outputStr).
// On Linux, std::wstring uses 4-byte wchar_t (UTF-32), but SQL Server expects 2-byte WCHAR (UTF-16),
// So the output is converted from UTF-32 to UTF-16 before returning.
//
// Returns:
//	SQL_SUCCESS on success, otherwise SQL_ERROR
SQLRETURN GetOutputParam(
	SQLGUID SessionId,
	SQLUSMALLINT TaskId,
	SQLUSMALLINT ParamNumber,
	SQLPOINTER *ParamValue,
	SQLINTEGER *StrLen_or_Ind)
{
	try
	{
		TRACELOG("OnnxExtension::GetOutputParam");

		if (ParamNumber == 0)
		{
			if (!g_onnxsession)
			{
				TRACELOG("OnnxSession is not initialized in GetOutputParam.");
				*ParamValue = nullptr;
				*StrLen_or_Ind = SQL_NULL_DATA;
				return SQL_ERROR;
			}

			const std::vector<std::wstring> &outputStr = g_onnxsession->GetOutputStr();
			if (!outputStr.empty() && !outputStr[0].empty())
			{
				// Reset the output buffer before reusing it
				g_outputBuffer.reset();

				size_t outputLength = outputStr[0].length();
#if defined(_WIN32)
				g_outputBuffer = std::make_unique<WCHAR[]>(outputLength + 1);
				wcscpy_s(g_outputBuffer.get(), outputLength + 1, outputStr[0].c_str());
				*ParamValue = reinterpret_cast<SQLPOINTER>(g_outputBuffer.get());
				*StrLen_or_Ind = static_cast<SQLINTEGER>(sizeof(WCHAR) * outputLength);
#else
				// On Linux, std::wstring is wchar_t (UTF-32), but SQL Server expects WCHAR (UTF-16, unsigned short)
				// Use a modern conversion utility to convert std::wstring (UTF-32) to UTF-16 (WCHAR)
				std::u16string utf16str;
				try
				{
					utf16str = ConvertUTF32ToUTF16(outputStr[0]);
				}
				catch (const std::exception &e)
				{
					TRACELOG("Exception in ConvertUTF32ToUTF16: ");
					TRACELOG(e.what());
					*ParamValue = nullptr;
					*StrLen_or_Ind = SQL_NULL_DATA;
					return SQL_ERROR;
				}
				g_outputBuffer = std::make_unique<WCHAR[]>(utf16str.length() + 1);
				for (size_t i = 0; i < utf16str.length(); ++i)
					g_outputBuffer[i] = static_cast<WCHAR>(utf16str[i]);
				g_outputBuffer[utf16str.length()] = 0;
				*ParamValue = reinterpret_cast<SQLPOINTER>(g_outputBuffer.get());
				*StrLen_or_Ind = static_cast<SQLINTEGER>(sizeof(WCHAR) * utf16str.length());
#endif
			}
			else
			{
				*ParamValue = nullptr;
				*StrLen_or_Ind = SQL_NULL_DATA;
			}
		}

		return SQL_SUCCESS;
	}
	catch (...)
	{
		TRACELOG("Unknown exception in GetOutputParam.");
		return SQL_ERROR;
	}
}

//--------------------------------------------------------------------------------------------------
// Name: CleanupSession
//
// Description:
//	Cleans up the session and frees resources. This method is called when the session is no longer needed during shutdown.
//
// Returns:
//	SQL_SUCCESS on success, otherwise SQL_ERROR
SQLRETURN CleanupSession(
	SQLGUID SessionId,
	SQLUSMALLINT TaskId)
{
	try
	{
		TRACELOG("OnnxExtension::CleanupSession");

		g_outputBuffer.reset();

		g_onnxsession.reset();

		return SQL_SUCCESS;
	}
	catch (...)
	{
		TRACELOG("Unknown exception in CleanupSession.");
		return SQL_ERROR;
	}
}

//--------------------------------------------------------------------------------------------------
// Name: Cleanup
//
// Description:
//	Cleans up the extension and frees resources.
//
// Returns:
//	SQL_SUCCESS on success, otherwise SQL_ERROR
SQLRETURN Cleanup()
{
	TRACELOG("OnnxExtension::Cleanup");
	return SQL_SUCCESS;
}