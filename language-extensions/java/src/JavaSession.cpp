//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: JavaSession.cpp
// @Owner: brnieb
//
// Purpose:
//  Class encapsulating operations performed per session
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
#include <regex>
#include <stdlib.h>
#ifndef _WIN64
// These sal include headers must follow the standard c++ headers, or there
// will be compilation issues. This is because headers like iostream/algorithm use
// variables like __in which are the same as a SAL annotation causing redefinition issues.
//
#include <sal_def.h>
#include <xplat_sal.h>
#endif
#include "Logger.h"
#include "JavaArgContainer.h"
#include "JavaExtensionUtils.h"
#include "JavaSqlTypeHelper.h"
#include "JniHelper.h"
#include "JniTypeHelper.h"
#include "JavaDataset.h"
#include "JavaSession.h"

using namespace std;

// Supported Java Extension versions
//
const jint x_javaExtensionVersion1 = 1;

// Members of AbstractSqlServerExtensionExecutor containing information
// about this Java session
//
const string x_inputDatasetMemberName = "executorInputDatasetClassName";
const string x_outputDatasetMemberName = "executorOutputDatasetClassName";
const string x_javaExtensionVersionMemberName = "executorExtensionVersion";

// SDK package name
//
const string x_javaSdkPackageName = "com/microsoft/sqlserver/javalangextension";

// Base classes for the Dataset and Executor
//
const string x_javaSdkBaseExecutorClass =
	x_javaSdkPackageName + "/AbstractSqlServerExtensionExecutor";
const string x_javaSdkBaseDatasetClass =
	x_javaSdkPackageName + "/AbstractSqlServerExtensionDataset";

//----------------------------------------------------------------------------
// Name: JavaSession::Init
//
// Description:
//  Initializes the Java session, by creating the Java object and verifying
//  validity of the user class
//
void JavaSession::Init(
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
	_In_ SQLUSMALLINT  OutputDataNameLength)
{
	if (env == nullptr)
	{
		throw invalid_argument("Invalid JNI enviroment");
	}

	m_env = env;
	m_sessionId = SessionId;
	m_taskId = TaskId;
	m_numTasks = NumTasks;

	// Initialize the rest of the parameters like storing the schema
	//
	m_inputSchemaColumnsNumber = InputSchemaColumnsNumber;

	// Allocate space for the vectors so they can be populated later
	//
	m_inputDataTypes.resize(InputSchemaColumnsNumber);
	m_inputColumnNames.resize(InputSchemaColumnsNumber);
	m_inputColumnSizes.resize(InputSchemaColumnsNumber);
	m_inputNullColumns.resize(InputSchemaColumnsNumber);
	m_inputDecimalDigits.resize(InputSchemaColumnsNumber);

	// Get the user defined class name from the script
	//
	m_mainClassName = GetUserDefinedClass(Script, ScriptLength);

	InitUserClassObject();

	GetUserClassInfo();

	CallUserInit();

	m_args.Init(ParametersNumber);
}

//----------------------------------------------------------------------------
// Name: JavaSession::Init
//
// Description:
//  Initializes the input column for this session
//
void JavaSession::InitColumn(
	_In_ SQLUSMALLINT  ColumnNumber,
	_In_ const SQLCHAR *ColumnName,
	_In_ SQLSMALLINT   ColumnNameLength,
	_In_ SQLSMALLINT   DataType,
	_In_ SQLULEN	   ColumnSize,
	_In_ SQLSMALLINT   DecimalDigits,
	_In_ SQLSMALLINT   Nullable,
	_In_ SQLSMALLINT   PartitionByNumber,
	_In_ SQLSMALLINT   OrderByNumber
	)
{
	LOG("Initializing column #" + to_string(ColumnNumber));

	if (ColumnName == nullptr)
	{
		throw invalid_argument("Invalid input column name supplied");
	}
	else if (ColumnNumber >= m_inputSchemaColumnsNumber || ColumnNumber < 0)
	{
		throw invalid_argument("Invalid input column id supplied: " + to_string(ColumnNumber));
	}

	// Store the information for this column
	//
	m_inputColumnNames[ColumnNumber] =
		string(reinterpret_cast<const char*>(ColumnName), ColumnNameLength);
	m_inputColumnSizes[ColumnNumber] = ColumnSize;
	m_inputDataTypes[ColumnNumber] = DataType;
	m_inputNullColumns[ColumnNumber] = Nullable;
	m_inputDecimalDigits[ColumnNumber] = DecimalDigits;
}

//----------------------------------------------------------------------------
// Name: JavaSession::InitParam
//
// Description:
//  Initializes the parameter for this session
//
void JavaSession::InitParam(
	SQLUSMALLINT  ParamNumber,
	const SQLCHAR *ParamName,
	SQLSMALLINT	  ParamNameLength,
	SQLSMALLINT	  DataType,
	SQLULEN		  ArgSize,
	SQLSMALLINT	  DecimalDigits,
	SQLPOINTER	  ArgValue,
	SQLINTEGER	  StrLen_or_Ind,
	SQLSMALLINT	  InputOutputType
	)
{
	LOG("Initializing input parameter #" + to_string(ParamNumber));

	if (ParamName == nullptr)
	{
		throw invalid_argument("Invalid input parameter name supplied");
	}
	else if (ParamNumber >= m_args.GetCount() || ParamNumber < 0)
	{
		throw invalid_argument("Invalid input parameter id supplied: " + to_string(ParamNumber));
	}

	SQLRETURN result = m_args.AddArg(
		ParamNumber,
		ParamName,
		ParamNameLength,
		DataType,
		ArgSize,
		DecimalDigits,
		ArgValue,
		StrLen_or_Ind,
		InputOutputType);

	// TODO (VSTS #12734107):
	// V1 still uses this AddArg code, once V1 is removed AddArg will switch
	// to throwing exceptions on errors
	//
	if (result != SQL_SUCCESS)
	{
		// AddArg() will have outputted the correct error
		//
		throw runtime_error("Error adding script argument");
	}
}

//----------------------------------------------------------------------------
// Name: JavaSession::InitParam
//
// Description:
//  Execute the workflow for the session
//
void JavaSession::ExecuteWorkflow(
	_In_ SQLULEN		RowsNumber,
	_In_opt_ SQLPOINTER *Data,
	_In_opt_ SQLINTEGER **StrLen_or_Ind,
	_Out_ SQLUSMALLINT	*OutputSchemaColumnsNumber)
{
	LOG("JavaSession::ExecuteWorkflow");

	*OutputSchemaColumnsNumber = 0;

	// In the streaming case clean up output buffers
	//
	CleanupOutputDataBuffers();

	CallUserExecute(RowsNumber, Data, StrLen_or_Ind, OutputSchemaColumnsNumber);
}

//--------------------------------------------------------------------------------------------------
// Name: JavaSession::Cleanup()
//
// Description:
//	Cleans up the session and calls cleanup on user object
//
void JavaSession::Cleanup()
{
	LOG("JavaSession::Cleanup");

	CleanupOutputDataBuffers();

	if (m_env != nullptr &&
		m_userClass != nullptr &&
		m_userObject != nullptr)
	{
		// If we have a user object, then call cleanup on it
		//
		CallUserCleanup();
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaSession::CallUserInit
//
// Description:
//	Calls init on user object
//
void JavaSession::CallUserInit()
{
	LOG("JavaSession::CallUserInit");

	jmethodID methodId = JniHelper::FindMethod(
		m_env,
		m_userClass,
		"init",
		"(Ljava/lang/String;II)V");

	// Convert the guid to string
	//
	std::string guidString = JniTypeHelper::ConvertGuidToString(&m_sessionId);

	// Create the java string
	//
	jstring jGuidStr = m_env->NewStringUTF(guidString.c_str());
	JniHelper::ThrowOnJavaException(m_env);

	m_env->CallVoidMethod(m_userObject,
						  methodId,
						  jGuidStr,
						  static_cast<jint>(m_taskId),
						  static_cast<jint>(m_numTasks));
	JniHelper::ThrowOnJavaException(m_env);

	m_env->DeleteLocalRef(jGuidStr);
}

//--------------------------------------------------------------------------------------------------
// Name: JavaSession::CallUserExecute
//
// Description:
//	Populates the input dataset, calls user execute, and extracts the output dataset
//
void JavaSession::CallUserExecute(
	_In_ SQLULEN		RowsNumber,
	_In_opt_ SQLPOINTER *Data,
	_In_opt_ SQLINTEGER **StrLen_or_Ind,
	_Out_ SQLUSMALLINT	*OutputSchemaColumnsNumber)
{
	LOG("JavaSession::CallUserExecute");

	*OutputSchemaColumnsNumber = 0;

	JavaDataset inputDataset;

	inputDataset.Init(m_env, m_inputDatasetClassName);

	// Add the input columns
	//
	inputDataset.AddColumns(
		m_inputSchemaColumnsNumber,
		RowsNumber,
		m_inputDataTypes,
		m_inputColumnNames,
		m_inputColumnSizes,
		m_inputDecimalDigits,
		m_inputNullColumns,
		Data,
		StrLen_or_Ind);

	// Create the map for input/output parameters
	//
	m_argMap = m_args.CreateArgMap(m_env);

	// Find the execute() method to call
	//
	jmethodID methodId = FindUserExecuteMethod();

	jobject outputDatasetObj = m_env->CallObjectMethod(m_userObject,
													   methodId,
													   inputDataset.GetJavaObject(),
													   m_argMap);

	JniHelper::ThrowOnJavaException(m_env);

	// Get the results if there are any
	//
	if (outputDatasetObj != nullptr)
	{
		JavaDataset outputDataset;

		outputDataset.Init(m_env, outputDatasetObj);

		outputDataset.GetColumns(
			m_outputSchemaColumnsNumber,
			m_numberOfOutputRows,
			m_outputDataTypes,
			m_outputColumnSizes,
			m_outputDecimalDigits,
			m_outputNullColumns,
			m_outputData,
			m_outputNullMap);

		*OutputSchemaColumnsNumber = m_outputSchemaColumnsNumber;
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaSession::CallUserCleanup
//
// Description:
//	Calls user cleanup
//
void JavaSession::CallUserCleanup()
{
	LOG("JavaSession::CallUserCleanup");

	jmethodID methodId = JniHelper::FindMethod(
		m_env,
		m_userClass,
		"cleanup",
		"()V");

	m_env->CallVoidMethod(m_userObject, methodId);

	JniHelper::ThrowOnJavaException(m_env);
}

//--------------------------------------------------------------------------------------------------
// Name: JavaSession::InitUserClassObject
//
// Description:
//	Initializes the user object and verifies inhierts from the SDK base class
//
void JavaSession::InitUserClassObject()
{
	jclass jUserClass = m_env->FindClass(m_mainClassName.c_str());
	if (jUserClass == nullptr)
	{
		JniHelper::ThrowOnJavaException(m_env, "Failed to find class " + m_mainClassName);
		throw runtime_error("Failed to find class " + m_mainClassName);
	}

	jclass jBaseClass = m_env->FindClass(x_javaSdkBaseExecutorClass.c_str());
	if (jBaseClass == nullptr)
	{
		JniHelper::ThrowOnJavaException(m_env, "Failed to find class " + x_javaSdkBaseExecutorClass);
		throw runtime_error("Failed to find class " + x_javaSdkBaseExecutorClass);
	}

	// Verify that the user class inhierts the SDK base class
	//
	if (JNI_FALSE == m_env->IsAssignableFrom(jUserClass, jBaseClass))
	{
		throw runtime_error(
				  "Class " + m_mainClassName + " is not a subclass of " +
				  x_javaSdkBaseExecutorClass);
	}

	// Create the object and call the default constructor
	//
	jmethodID methodId = m_env->GetMethodID(jUserClass, "<init>", "()V");
	if (methodId == nullptr)
	{
		JniHelper::ThrowOnJavaException(m_env, "Failed to find default constructor for class " + m_mainClassName);
		throw runtime_error("Failed to find default constructor for class " + m_mainClassName);
	}

	jobject jUserObj = m_env->NewObject(jUserClass, methodId);
	if (jUserObj == nullptr)
	{
		JniHelper::ThrowOnJavaException(m_env, "Failed to create object for class " + m_mainClassName);
		throw runtime_error("Failed to create object for class " + m_mainClassName);
	}

	// Create global references
	//
	m_userClass = static_cast<jclass>(m_env->NewGlobalRef(jUserClass));
	if (m_userClass == nullptr)
	{
		throw runtime_error("Failed to create global reference for class " + m_mainClassName);
	}

	m_userObject = m_env->NewGlobalRef(jUserObj);

	if (m_userObject == nullptr)
	{
		throw runtime_error("Failed to create global reference on object ");
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaSession::GetUserClassInfo
//
// Description:
//	Gets the info from the user class (input/output dataset and version)
//
void JavaSession::GetUserClassInfo()
{
	AutoJniLocalFrame jFrame(m_env, 4);

	jfieldID versionId =
		m_env->GetFieldID(m_userClass, x_javaExtensionVersionMemberName.c_str(), "I");
	jfieldID inputDatasetClassId = m_env->GetFieldID(m_userClass,
													 x_inputDatasetMemberName.c_str(),
													 "Ljava/lang/String;");
	jfieldID outputDatasetClassId = m_env->GetFieldID(m_userClass,
													  x_outputDatasetMemberName.c_str(),
													  "Ljava/lang/String;");

	if (versionId == nullptr || inputDatasetClassId == nullptr || outputDatasetClassId == nullptr)
	{
		// Since these are members of the abstract executor class, and we verified the user class
		// derives the base class this is unlikely to happen.
		//
		JniHelper::ThrowOnJavaException(
			m_env,
			"Failed to find class members " + x_javaExtensionVersionMemberName + "," + x_inputDatasetMemberName + "," +
			x_outputDatasetMemberName);

		throw runtime_error(
			"Failed to find class members " + x_javaExtensionVersionMemberName + "," + x_inputDatasetMemberName + "," +
			x_outputDatasetMemberName);
	}

	jint version = m_env->GetIntField(m_userObject, versionId);

	// Check the version supplied is supported
	//
	if (version != x_javaExtensionVersion1)
	{
		throw runtime_error("Unsupported executor version encountered " + to_string(version));
	}

	// Get the class to use for input data set
	//
	jstring jInputDataClassStr =
		static_cast<jstring>(m_env->GetObjectField(m_userObject, inputDatasetClassId));

	if (jInputDataClassStr == nullptr)
	{
		throw runtime_error(
				  "Invalid value encountered for executorInputDatasetClassName in class " +
				  m_mainClassName);
	}

	jsize sizeInBytes = m_env->GetStringUTFLength(jInputDataClassStr);
	const char *value = m_env->GetStringUTFChars(jInputDataClassStr, nullptr);
	JniHelper::ThrowOnJavaException(m_env);

	m_inputDatasetClassName = regex_replace(string(value, sizeInBytes), regex("\\."), "/");

	m_env->ReleaseStringUTFChars(jInputDataClassStr, value);

	// Get the class to use for output data set
	//
	// NOTE:
	// This class is only used for determining the function signature,
	// so null is acceptable. As the object is allocated and return to
	// the extension from execute()
	//
	jstring jOutputDataClassStr =
		static_cast<jstring>(m_env->GetObjectField(m_userObject, outputDatasetClassId));

	if (jOutputDataClassStr != nullptr)
	{
		sizeInBytes = m_env->GetStringUTFLength(jOutputDataClassStr);
		value = m_env->GetStringUTFChars(jOutputDataClassStr, nullptr);
		JniHelper::ThrowOnJavaException(m_env);

		m_outputDatasetClassName = regex_replace(string(value, sizeInBytes), regex("\\."), "/");

		m_env->ReleaseStringUTFChars(jOutputDataClassStr, value);
	}
	else
	{
		// Set to the default data set class
		//
		m_outputDatasetClassName = x_javaSdkBaseDatasetClass;
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaSession::FindUserExecuteMethod
//
// Description:
//  Finds the user method id to execute
//
// Returns:
//  The method id of the execute function to call
//
// Notes:
//  To allow the user not to deal with casting, we will try to find the method id with the signature
//  of the datasets they supply. If it is not found, then find the default signature, which should
//  always be found because verification of class inhiertance occurred during InitSession()
//
jmethodID JavaSession::FindUserExecuteMethod()
{
	// The signature for the DatasetClass execute(DatasetClass input, LinkedHashMap<String, Object> params)
	//
	// Construct the signature for using the user specific dataset classes, but fall back to the default
	// base class signature. This allows the user to not have to cast the dataset classes.
	//
	string specificDatasetSignature =
		"(L" + m_inputDatasetClassName + ";Ljava/util/LinkedHashMap;)L" + m_outputDatasetClassName +
		";";

	string defaultSignature =
		"(L" + x_javaSdkBaseDatasetClass + ";Ljava/util/LinkedHashMap;)L" +
		x_javaSdkBaseDatasetClass  +
		";";

	jmethodID methodId = nullptr;

	// Try to find the specialized dataset signature first
	//
	try
	{
		methodId = JniHelper::FindMethod(
			m_env,
			m_userClass,
			"execute",
			specificDatasetSignature);
	}
	catch (java_exception_error)
	{
		LOG("Did not find execute with signature " + specificDatasetSignature);

		// Clear the not found exception
		//
		m_env->ExceptionClear();
	}

	if (methodId == nullptr)
	{
		// Fall back and try to find the default signature
		//
		methodId = JniHelper::FindMethod(
			m_env,
			m_userClass,
			"execute",
			defaultSignature);
	}

	// InitSession() verified that the user class can be casted to the base class, so this check should never fail.
	//
	if (methodId == nullptr)
	{
		throw runtime_error("Failed to find method execute in class " + m_mainClassName);
	}

	return methodId;
}

//--------------------------------------------------------------------------------------------------
// Name: GetUserDefinedClass
//
// Description:
//  Parse the script passed in by the user that contains the fully qualified class name
//  (both package and class name) and the method name.
//
// Note:
//  Throws runtime_exception if script is invalid
//
string JavaSession::GetUserDefinedClass(
	_In_reads_(sqlScriptLength) const SQLCHAR *sqlScript,
	_In_ SQLULEN							  sqlScriptLength)
{
	if (sqlScript == nullptr)
	{
		throw invalid_argument("Invalid script, the script value cannot be NULL");
	}

	string className(reinterpret_cast<const char*>(sqlScript), sqlScriptLength);

	// Find the class name and replace the period separator with
	// the forward slashes required by JNI
	//
	replace(className.begin(), className.end(), '.', '/');

	return className;
}

//--------------------------------------------------------------------------------------------------
// Name: JavaSession::CleanUpOutputDataBuffers
//
// Description:
//	Cleans up the memory allocated for the output schema and
//	the output data
//
// TODO (VSTS #12734114):
//  These can be done with template function, but for now the V1 implementation is used
//
void JavaSession::CleanupOutputDataBuffers()
{
	// Clean up OutputNullMap and the output data buffers
	//
	for (unsigned int i = 0; i < m_outputData.size(); ++i)
	{
		if (m_outputData[i] != nullptr)
		{
			// Since we store the pointers to output data buffers as void pointers,
			// we have to recast them to their original types to destroy them. In the
			// future, the types may be implemented as objects to simplify the code here.
			//
			switch (m_outputDataTypes[i])
			{
			case SQL_C_SSHORT:
				delete[] reinterpret_cast<short*>(m_outputData[i]);
				break;
			case SQL_C_BIT:
				delete[] reinterpret_cast<bool*>(m_outputData[i]);
				break;
			case SQL_C_SBIGINT:
				delete[] reinterpret_cast<SQLBIGINT*>(m_outputData[i]);
				break;
			case SQL_C_SLONG:
				delete[] reinterpret_cast<SQLINTEGER*>(m_outputData[i]);
				break;
			case SQL_C_FLOAT:
				delete[] reinterpret_cast<float*>(m_outputData[i]);
				break;
			case SQL_C_DOUBLE:
				delete[] reinterpret_cast<double*>(m_outputData[i]);
				break;
			case SQL_C_UTINYINT:
				delete[] reinterpret_cast<unsigned short*>(m_outputData[i]);
				break;
			case SQL_C_WCHAR:
				delete[] reinterpret_cast<jchar*>(m_outputData[i]);
				break;
			case SQL_C_BINARY:
				delete[] reinterpret_cast<jbyte*>(m_outputData[i]);
				break;
			case SQL_C_TYPE_DATE:
				delete[] reinterpret_cast<SQL_DATE_STRUCT*>(m_outputData[i]);
				break;
			case SQL_C_NUMERIC:
				delete[] reinterpret_cast<SQL_NUMERIC_STRUCT*>(m_outputData[i]);
				break;
			case SQL_C_TYPE_TIMESTAMP:
				delete[] reinterpret_cast<SQL_TIMESTAMP_STRUCT*>(m_outputData[i]);
				break;
			}

			m_outputData[i] = nullptr;
		}

		// Clean up the space allocated for the output null map
		//
		if (m_outputNullMap[i] != nullptr)
		{
			delete[] m_outputNullMap[i];
			m_outputNullMap[i] = nullptr;
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaSession::GetResultColumn
//
// Description:
//  Returns information about the output column
//
void JavaSession::GetResultColumn(
	_In_ SQLUSMALLINT ColumnNumber,
	_Out_ SQLSMALLINT *DataType,
	_Out_ SQLULEN	  *ColumnSize,
	_Out_ SQLSMALLINT *DecimalDigits,
	_Out_ SQLSMALLINT *Nullable
	)
{
	LOG("Retrieving metadata for result column #" + to_string(ColumnNumber));

	*DataType = SQL_UNKNOWN_TYPE;
	*ColumnSize = 0;
	*Nullable = 0;
	*DecimalDigits = 0;

	if (0 <= ColumnNumber && ColumnNumber < m_outputDataTypes.size())
	{
		*DataType = m_outputDataTypes[ColumnNumber];
		*ColumnSize = m_outputColumnSizes[ColumnNumber];
		*DecimalDigits = m_outputDecimalDigits[ColumnNumber];
		*Nullable = m_outputNullColumns[ColumnNumber];
	}
	else
	{
		throw runtime_error("Invalid column id provided to GetResultColumn():" +
							to_string(ColumnNumber));
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaSession::GetResults
//
// Description:
//	Returns the output data and the null map retrieved from the user program
//
void JavaSession::GetResults(
	_Out_ SQLULEN		*RowsNumber,
	_Outptr_ SQLPOINTER **Data,
	_Outptr_ SQLINTEGER ***StrLen_or_Ind)
{
	LOG("JavaSession::GetResults");

	*RowsNumber = 0;
	*Data = nullptr;
	*StrLen_or_Ind = nullptr;

	if (RowsNumber != nullptr && Data != nullptr && StrLen_or_Ind != nullptr)
	{
		*RowsNumber = m_numberOfOutputRows;
		*Data = &m_outputData[0];
		*StrLen_or_Ind = &m_outputNullMap[0];
	}
	else
	{
		throw runtime_error("Invalid parameters provided to GetResults()");
	}
}

//--------------------------------------------------------------------------------------------------
// Name: JavaSession::GetOutputParam
//
// Description:
//  Returns the data and size of the output parameter
//
void JavaSession::GetOutputParam(
	_In_ SQLUSMALLINT ParamNumber,
	_Out_ SQLPOINTER  *ParamValue,
	_Out_ SQLINTEGER  *StrLen_or_Ind)
{
	LOG("Initializing output parameter #" + to_string(ParamNumber));

	if (0 <= ParamNumber && ParamNumber < m_args.GetCount())
	{
		// Replace the input parameter with the updated output value from the
		// execution parameters' hash map
		//
		m_args.ReplaceArgValue(m_env, ParamNumber, m_argMap, ParamValue, StrLen_or_Ind);
	}
	else
	{
		throw invalid_argument("Invalid output parameter id supplied to GetOutputParam(): " +
							   to_string(ParamNumber));
	}
}

