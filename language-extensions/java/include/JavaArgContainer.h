//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: JavaArgContainer.h
// @Owner: brnieb
//
// Purpose:
// Java extension input/output parameter wrappers, along with container
// for consolidating them.
//
//*********************************************************************
#pragma once

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
		_In_ SQLUSMALLINT id,
		_In_ SQLSMALLINT  type,
		_In_ SQLULEN	  size,
		_In_ SQLSMALLINT  decimalDigits,
		_In_ SQLPOINTER	  value,
		_In_ SQLINTEGER	  strLen_or_Ind,
		_In_ SQLSMALLINT  inputOutputType);

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
		_In_ SQLSMALLINT	  type,
		_In_ SQLINTEGER		  size,
		_In_ const SQLPOINTER value
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
	void Init(_In_ const SQLUSMALLINT numOfArgs);

	// Construct and store argument
	//
	SQLRETURN AddArg(
		_In_ SQLUSMALLINT	  id,
		_In_ const SQLCHAR	  *paramName,
		_In_ SQLSMALLINT	  paramNameLength,
		_In_ SQLSMALLINT	  type,
		_In_ SQLULEN		  size,
		_In_ SQLSMALLINT	  decimalDigits,
		_In_ const SQLPOINTER value,
		_In_ SQLINTEGER		  strLen_or_Ind,
		_In_ SQLSMALLINT	  inputOutputType);

	// Get the arg from the name
	//
	JavaArg* GetArg(_In_ const std::string &name);

	// Get the arg from the id
	//
	JavaArg* GetArg(_In_ const SQLUSMALLINT id);

	// Gets the parameter name for the argument ID
	//
	const std::string& GetParamName(_In_ const SQLUSMALLINT id)
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
	jobject CreateArgMap(_In_ JNIEnv *env);

private:
	// Creates a Java object for the argument
	//
	jobject CreateJavaArgObject(_In_ JNIEnv *env, _In_ const JavaArg *arg);

	// Names of parameters, stored in order of parameter id. This
	// is used for accessing parameters by id, in order to get the
	// name for constant time lookup in the arg map.
	//
	std::vector<std::string>  m_argNames;

	// Map of names to parameter value
	//
	std::unordered_map<std::string, std::unique_ptr<JavaArg>> m_argMap;
};
