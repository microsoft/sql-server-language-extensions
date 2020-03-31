//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JavaSession.h
//
// Purpose:
//  Class encapsulating operations performed per session
//
//*********************************************************************
#pragma once

#include "Common.h"
#include "JavaArgContainer.h"

// Data pertaining to a session
//
class JavaSession
{
public:
	// Constructor to initialize the members
	//
	JavaSession()
		: m_userClass(nullptr),
		m_userObject(nullptr),
		m_env(nullptr),
		m_argMap(nullptr),
		m_mainMethodId(nullptr),
		m_sessionId({0, 0, 0, {0}}),
		m_taskId(0),
		m_numTasks(0),
		m_inputSchemaColumnsNumber(0),
		m_numberOfOutputRows(0)
	{}

	// Destructor, releases global references
	//
	~JavaSession()
	{
		CleanupOutputDataBuffers();

		if (m_env != nullptr && m_userObject != nullptr)
		{
			m_env->DeleteGlobalRef(m_userObject);
		}

		if (m_env != nullptr && m_userClass != nullptr)
		{
			m_env->DeleteGlobalRef(m_userClass);
		}

		if (m_env != nullptr && m_argMap != nullptr)
		{
			m_env->DeleteGlobalRef(m_argMap);
		}

		m_argMap = nullptr;
		m_userObject = nullptr;
		m_userClass = nullptr;
		m_env = nullptr;
	}

	// Init the session
	//
	void Init(
		JNIEnv        *env,
		const SQLGUID &sessionId,
		SQLUSMALLINT  taskId,
		SQLUSMALLINT  numTasks,
		const SQLCHAR *script,
		SQLULEN       scriptLength,
		SQLUSMALLINT  inputSchemaColumnsNumber,
		SQLUSMALLINT  parametersNumber,
		const SQLCHAR *inputDataName,
		SQLUSMALLINT  inputDataNameLength,
		const SQLCHAR *outputDataName,
		SQLUSMALLINT  outputDataNameLength);

	// Init the input column
	//
	void InitColumn(
		SQLUSMALLINT  columnNumber,
		const SQLCHAR *columnName,
		SQLSMALLINT   columnNameLength,
		SQLSMALLINT   dataType,
		SQLULEN       columnSize,
		SQLSMALLINT   decimalDigits,
		SQLSMALLINT   nullable,
		SQLSMALLINT   partitionByNumber,
		SQLSMALLINT   orderByNumber);

	// Init the input parameter
	//
	void InitParam(
		SQLUSMALLINT  paramNumber,
		const SQLCHAR *paramName,
		SQLSMALLINT   paramNameLength,
		SQLSMALLINT   dataType,
		SQLULEN       argSize,
		SQLSMALLINT   decimalDigits,
		SQLPOINTER    argValue,
		SQLINTEGER    strLen_or_Ind,
		SQLSMALLINT   inputOutputType);

	// Execute the workflow for the session
	//
	void ExecuteWorkflow(
		SQLULEN      rowsNumber,
		SQLPOINTER   *data,
		SQLINTEGER   **strLen_or_Ind,
		SQLUSMALLINT *outputSchemaColumnsNumber);

	// Get the metadata for the output column
	//
	void GetResultColumn(
		SQLUSMALLINT columnNumber,
		SQLSMALLINT  *dataType,
		SQLULEN      *columnSize,
		SQLSMALLINT  *decimalDigits,
		SQLSMALLINT  *nullable);

	// Get the results
	//
	void GetResults(
		SQLULEN    *rowsNumber,
		SQLPOINTER **data,
		SQLINTEGER ***strLen_or_Ind);

	// Get the the output parameter
	//
	void GetOutputParam(
		SQLUSMALLINT paramNumber,
		SQLPOINTER   *paramValue,
		SQLINTEGER   *strLen_or_Ind);

	// Cleanup session
	//
	void Cleanup();

private:
	// Call init() on the user object
	//
	void CallUserInit();

	// Call execute() on the user object
	//
	void CallUserExecute(
		SQLULEN      rowsNumber,
		SQLPOINTER   *data,
		SQLINTEGER   **strLen_or_Ind,
		SQLUSMALLINT *outputSchemaColumnsNumber);

	// Call cleanup() on the user object
	//
	void CallUserCleanup();

	// Gets the user class from the supplied script
	//
	static std::string GetUserDefinedClass(const SQLCHAR *sqlScript, SQLULEN sqlScriptLength);

	// Find method id of the execute function
	//
	jmethodID FindUserExecuteMethod();

	// Create user object
	//
	void InitUserClassObject();

	// Gets info about the user class
	//
	void GetUserClassInfo();

	// Cleanup output buffers
	//
	void CleanupOutputDataBuffers();

	SQLGUID m_sessionId;                  // Session ID
	SQLUSMALLINT m_taskId;                // Task ID for this session
	SQLUSMALLINT m_numTasks;              // Number of tasks for this session

	jclass m_userClass;                   // Global reference to the user supplied executor class
	jmethodID m_mainMethodId;             // Method ID of the execute function in executor class
	JNIEnv *m_env;                        // JNI enviroment
	jobject m_userObject;                 // Global reference of the instantiated object of the user class

	JavaArgContainer m_args;              // User input and output arguments
	jobject m_argMap;                     // Global reference of the hash map for the execution parameters

	std::string m_mainClassName;          // User executor class name
	std::string m_inputDatasetClassName;  // Input dataset class name for the executor class
	std::string m_outputDatasetClassName; // Output dataset class name for the executor class

	// Input data information (schema, null map, etc.)
	//
	SQLUSMALLINT m_inputSchemaColumnsNumber;
	std::vector<std::string> m_inputColumnNames;
	std::vector<SQLSMALLINT> m_inputDataTypes;
	std::vector<SQLULEN> m_inputColumnSizes;
	std::vector<SQLSMALLINT> m_inputNullColumns;
	std::vector<SQLSMALLINT> m_inputDecimalDigits;
	std::vector<SQLSMALLINT> m_partitionByIndexes;
	std::vector<SQLSMALLINT> m_orderByIndexes;

	// Output data information (schema, null map, etc.)
	//
	SQLULEN m_numberOfOutputRows;
	SQLUSMALLINT m_outputSchemaColumnsNumber;
	std::vector<SQLSMALLINT> m_outputDataTypes;
	std::vector<SQLULEN> m_outputColumnSizes;
	std::vector<SQLPOINTER> m_outputData;
	std::vector<SQLINTEGER*> m_outputNullMap;
	std::vector<SQLSMALLINT> m_outputNullColumns;
	std::vector<SQLSMALLINT> m_outputDecimalDigits;
};
