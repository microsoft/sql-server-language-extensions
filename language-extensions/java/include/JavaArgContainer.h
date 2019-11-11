//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JavaArgContainer.h
//
// Purpose:
//  Java extension input/output parameter wrappers, along with container
//  for consolidating them.
//
//*********************************************************************
#pragma once

#include "Common.h"

//---------------------------------------------------------------------
// Description:
//	Stores information about the Java argument
//
class JavaArg
{
	friend class JavaArgContainer;

public:
	// Constructor to initialize the members
	//
	JavaArg(
		SQLUSMALLINT	id,
		SQLSMALLINT		type,
		SQLULEN			size,
		SQLSMALLINT		decimalDigits,
		SQLPOINTER		value,
		SQLINTEGER		strLen_or_Ind,
		SQLSMALLINT		inputOutputType);

	// Destructor
	//
	~JavaArg();

	// Gets id
	//
	SQLUSMALLINT GetId() const
	{
		return m_id;
	}

	// Gets type
	//
	SQLSMALLINT GetType() const
	{
		return m_type;
	}

	// Gets size
	//
	SQLULEN GetSize() const
	{
		return m_size;
	}

	// Gets decimal digits
	//
	SQLSMALLINT GetDecimalDigits() const
	{
		return m_decimalDigits;
	}

	// Gets value
	//
	SQLPOINTER GetValue() const
	{
		return m_value;
	}

	// Gets string length or null indicator
	//
	SQLINTEGER GetStrLenOrInd() const
	{
		return m_strLenOrInd;
	}

	// Gets input/output type
	//
	SQLSMALLINT GetInputOutputType() const
	{
		return m_inputOutputType;
	}

private:
	// Deep copy argument
	//
	void DeepCopyValue(
		SQLSMALLINT			type,
		SQLINTEGER			size,
		const SQLPOINTER 	value
		);

	// Cleans up memory used to store argument
	//
	void Cleanup();

	SQLUSMALLINT m_id;				// Id of the argument
	SQLSMALLINT m_type;				// Data type of the argument
	SQLULEN m_size;					// Size of the argument
	SQLSMALLINT m_decimalDigits;	// Decimal digits of the argument
	SQLPOINTER m_value;				// Value of the argument
	SQLINTEGER m_strLenOrInd;		// Indicates string length or null
	SQLSMALLINT m_inputOutputType;	// Argument type (Input/Output)
};

//---------------------------------------------------------------------
// Description:
//	Container for all Java arguments
//
class JavaArgContainer
{
public:
	// Constructor
	//
	JavaArgContainer()
	{}

	// Destructor
	//
	~JavaArgContainer();

	// Initialize argument container
	//
	void Init(const SQLUSMALLINT numOfArgs);

	// Construct and store argument
	//
	SQLRETURN AddArg(
		SQLUSMALLINT		id,
		const SQLCHAR		*paramName,
		SQLSMALLINT			paramNameLength,
		SQLSMALLINT			type,
		SQLULEN				size,
		SQLSMALLINT			decimalDigits,
		const SQLPOINTER	value,
		SQLINTEGER			strLen_or_Ind,
		SQLSMALLINT			inputOutputType);

	// Get the arg from the name
	//
	JavaArg* GetArg(const std::string &name);

	// Get the arg from the id
	//
	JavaArg* GetArg(const SQLUSMALLINT id);

	// Gets the parameter name for the argument ID
	//
	const std::string& GetParamName(const SQLUSMALLINT id)
	{
		return m_argNames[id];
	}

	// Get the number of arguments
	//
	inline SQLUSMALLINT GetCount() const
	{
		return static_cast<SQLUSMALLINT>(m_argNames.size());
	}

	// Creates a Java HashMap and populates it with all arguments
	// in the container
	//
	jobject CreateArgMap(JNIEnv *env);

	// Replaces the parameter's ODBC-format input value stored in the container
	// with the updated output value from execution parameters' hash map
	//
	void ReplaceArgValue(
		JNIEnv			*env,
		SQLUSMALLINT	id,
		jobject			javaArgMap,
		SQLPOINTER		*value,
		SQLINTEGER		*strLen_or_Ind);

private:
	// Creates a Java object for the argument
	//
	jobject CreateJavaArgObject(JNIEnv *env, const JavaArg *arg);

	// Creates an ODBC object for the argument
	//
	void CreateOdbcArgObject(JNIEnv	*env, jobject jObj, JavaArg *arg);

	// Verify that the given output parameter jObj is of the expected java class
	//
	void ValidateOutputClass(
		JNIEnv 			*env,
		SQLUSMALLINT	paramId,
		jobject 		jObj,
		jclass			objectClass,
		std::string		&&objectClassName);

	// Names of parameters, stored in order of parameter id. This
	// is used for accessing parameters by id, in order to get the
	// name for constant time lookup in the arg map.
	//
	std::vector<std::string>  m_argNames;

	// Map of names to parameter value
	//
	std::unordered_map<std::string, std::unique_ptr<JavaArg>> m_argMap;
};
