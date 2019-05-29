//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: JavaSession.h
// @Owner: brnieb
//
// Purpose:
//  Class encapsulating operations performed per session
//
//*********************************************************************
#pragma once

class JavaArgContainer;

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
		_In_ JNIEnv		   *env,
		_In_ const SQLGUID &SessionId,
		_In_ SQLUSMALLINT  TaskId,
		_In_ SQLUSMALLINT  NumTasks,
		_In_ const SQLCHAR *Script,
		_In_ SQLULEN	   ScriptLength,
		_In_ SQLUSMALLINT  InputSchemaColumnsNumber,
		_In_ SQLUSMALLINT  ParametersNumber,
		_In_ const SQLCHAR *InputDataName,
		_In_ SQLUSMALLINT  InputDataNameLength,
		_In_ const SQLCHAR *OutputDataName,
		_In_ SQLUSMALLINT  OutputDataNameLength);

	// Init the input column
	//
	void InitColumn(
		_In_ SQLUSMALLINT  ColumnNumber,
		_In_ const SQLCHAR *ColumnName,
		_In_ SQLSMALLINT   ColumnNameLength,
		_In_ SQLSMALLINT   DataType,
		_In_ SQLULEN	   ColumnSize,
		_In_ SQLSMALLINT   DecimalDigits,
		_In_ SQLSMALLINT   Nullable,
		_In_ SQLSMALLINT   PartitionByNumber,
		_In_ SQLSMALLINT   OrderByNumber
		);

	// Init the input parameter
	//
	void InitParam(
		SQLUSMALLINT  ParamNumber,
		const SQLCHAR *ParamName,
		SQLSMALLINT	  ParamNameLength,
		SQLSMALLINT	  DataType,
		SQLULEN		  ArgSize,
		SQLSMALLINT	  DecimalDigits,
		SQLPOINTER	  ArgValue,
		SQLINTEGER	  StrLen_or_Ind,
		SQLSMALLINT	  InputOutputType
		);

	// Execute the workflow for the session
	//
	void ExecuteWorkflow(
		_In_ SQLULEN		RowsNumber,
		_In_opt_ SQLPOINTER *Data,
		_In_opt_ SQLINTEGER **StrLen_or_Ind,
		_Out_ SQLUSMALLINT	*OutputSchemaColumnsNumber);

	// Get the metadata for the output column
	//
	void GetResultColumn(
		_In_ SQLUSMALLINT ColumnNumber,
		_Out_ SQLSMALLINT *DataType,
		_Out_ SQLULEN	  *ColumnSize,
		_Out_ SQLSMALLINT *DecimalDigits,
		_Out_ SQLSMALLINT *Nullable);

	// Get the results
	//
	void GetResults(
		_Out_ SQLULEN		*RowsNumber,
		_Outptr_ SQLPOINTER **Data,
		_Outptr_ SQLINTEGER ***StrLen_or_Ind);

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
		_In_ SQLULEN		RowsNumber,
		_In_opt_ SQLPOINTER *Data,
		_In_opt_ SQLINTEGER **StrLen_or_Ind,
		_Out_ SQLUSMALLINT	*OutputSchemaColumnsNumber);

	// Call cleanup() on the user object
	//
	void CallUserCleanup();

	// Gets the user class from the supplied script
	//
	static std::string GetUserDefinedClass(
		_In_reads_(sqlScriptLength) const SQLCHAR   * sqlScript,
		_In_ SQLULEN sqlScriptLength);

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

	SQLGUID m_sessionId;	// Session ID
	SQLUSMALLINT m_taskId;	// Task ID for this session
	SQLUSMALLINT m_numTasks;// Number of tasks for this session

	jclass m_userClass;			// Global reference to the user supplied executor class
	jmethodID m_mainMethodId;	// Method ID of the execute function in executor class
	JNIEnv *m_env;				// JNI enviroment
	jobject m_userObject;		// Global reference of the instantiated object of the user class

	JavaArgContainer m_args;// User input and output arguments
	jobject m_argMap;		// Global reference of the hash map for the execution parameters

	std::string m_mainClassName;			// User executor class name
	std::string m_inputDatasetClassName;	// Input dataset class name for the executor class
	std::string m_outputDatasetClassName;	// Output dataset class name for the executor class

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