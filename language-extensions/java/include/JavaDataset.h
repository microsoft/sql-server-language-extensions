//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: JavaDataset.h
// @Owner: brnieb
//
// Purpose:
// Handles loading and retrieving data from the Dataset class
//
//*********************************************************************
#pragma once

class JavaDataset
{
public:
	// Constructor
	//
	JavaDataset() :
		m_object(nullptr), m_env(nullptr), m_class(nullptr)
	{}

	// Destructor, releases global references
	//
	~JavaDataset()
	{
		if (m_env != nullptr && m_object != nullptr)
		{
			m_env->DeleteGlobalRef(m_object);
		}

		if (m_env != nullptr && m_class != nullptr)
		{
			m_env->DeleteGlobalRef(m_class);
		}

		m_object = nullptr;
		m_class = nullptr;
		m_env = nullptr;
	}

	// Initializes the object
	//
	void Init(_In_ JNIEnv *env, const std::string &className);

	// Initializes the object from an existing Java object
	//
	void Init(_In_ JNIEnv *env, jobject obj);

	// Adds the column metadata
	//
	void AddColumnMetadata(_In_ jint			  colId,
						   _In_ const std::string &colName,
						   _In_ SQLSMALLINT		  colType,
						   _In_ SQLULEN			  colSize,
						   _In_ SQLSMALLINT		  decimalDigits);

	// Adds the column
	//
	void AddColumn(
		_In_ jint colId,
		_In_ SQLSMALLINT colType,
		_In_ SQLULEN colSize,
		_In_ SQLSMALLINT decimalDigits,
		_In_ SQLULEN numRows,
		_In_reads_opt_(numRows) const SQLPOINTER data,
		_In_reads_opt_(numRows) const SQLINTEGER * nullMap);

	// Adds each column
	//
	void AddColumns(
		_In_ SQLUSMALLINT numCols,
		_In_ SQLULEN numRows,
		_In_ std::vector<SQLSMALLINT>                   &colTypes,
		_In_ std::vector<std::string>                   &colNames,
		_In_ std::vector<SQLULEN>                       &colSizes,
		_In_ std::vector<SQLSMALLINT>                   &decimalDigits,
		_In_ std::vector<SQLSMALLINT>                   &nullableCols,
		_In_reads_opt_(numRows * numCols) SQLPOINTER    * data,
		_In_reads_opt_(numRows * numCols) SQLINTEGER    * *nullMap);

	// Gets the column type
	//
	void GetColumnType(_In_ jint colId, _Out_ SQLSMALLINT &colType);

	// Gets the column precision
	//
	void GetColumnPrecision(_In_ jint colId, _Out_ SQLULEN &colPrec);

	// Gets the column scale
	//
	void GetColumnScale(_In_ jint colId, _Out_ SQLSMALLINT &colScale);

	// Gets the column
	//
	void GetColumn(
		_In_ jint							 colId,
		_Out_ SQLSMALLINT					 &colType,
		_Out_ SQLULEN						 &colSize,
		_Out_ SQLSMALLINT					 &colDecimalDigits,
		_Out_ SQLULEN						 &numRows,
		_Outptr_result_maybenull_ SQLPOINTER *data,
		_Outptr_result_maybenull_ SQLINTEGER **nullMap);

	// Gets all columns
	//
	void GetColumns(_Out_ SQLUSMALLINT			   &numCols,
					_Out_ SQLULEN				   &numRows,
					_Out_ std::vector<SQLSMALLINT> &colTypes,
					_Out_ std::vector<SQLULEN>	   &colSizes,
					_Out_ std::vector<SQLSMALLINT> &colDecimalDigits,
					_Out_ std::vector<SQLSMALLINT> &nullableCols,
					_Out_ std::vector<SQLPOINTER>  &data,
					_Out_ std::vector<SQLINTEGER*> &nullMaps);

	// Get the jobject of the Java dataset
	//
	jobject GetJavaObject() const
	{
		return m_object;
	}

private:
	// Add column function pointer definition
	//
	typedef void (JavaDataset::*fnAddColumn)(
		_In_ jint								  colId,
		_In_ SQLSMALLINT						  colType,
		_In_ SQLULEN							  colSize,
		_In_ SQLSMALLINT						  decimalDigits,
		_In_ jsize								  numRows,
		_In_reads_ (numRows) SQLPOINTER			  data,
		_In_reads_opt_ (numRows) const SQLINTEGER *nullMap);

	// Get column function pointer definition
	//
	typedef void (JavaDataset::*fnGetColumn)(
		_In_ jint							 colId,
		_In_ SQLSMALLINT					 colType,
		_Out_ SQLULEN						 &numRows,
		_Out_ SQLULEN						 &dataSizeInBytes,
		_Out_ SQLSMALLINT					 &decimalDigits,
		_Outptr_result_maybenull_ SQLPOINTER *data,
		_Outptr_result_maybenull_ SQLINTEGER **nullMap);

	// Internal add column
	//
	template<typename jArrayType, typename jType, typename cType>
	void AddColumnInternal(
		_In_ jint colId,
		_In_ SQLSMALLINT colType,
		_In_ SQLULEN colSize,
		_In_ SQLSMALLINT decimalDigits,
		_In_ jsize numRows,
		_In_reads_(numRows) SQLPOINTER data,
		_In_reads_opt_(numRows) const SQLINTEGER      * nullMap);

	// Internal add string column
	//
	template<bool isUnicode>
	void AddStringColumnInternal(
		_In_ jint colId,
		_In_ SQLSMALLINT colType,
		_In_ SQLULEN colSize,
		_In_ SQLSMALLINT decimalDigits,
		_In_ jsize numRows,
		_In_reads_(numRows) SQLPOINTER data,
		_In_reads_opt_(numRows) const SQLINTEGER  * nullMap);

	// Internal add binary column
	//
	void AddBinaryColumnInternal(
		_In_ jint colId,
		_In_ SQLSMALLINT colType,
		_In_ SQLULEN colSize,
		_In_ SQLSMALLINT decimalDigits,
		_In_ jsize numRows,
		_In_reads_(numRows) SQLPOINTER data,
		_In_reads_opt_(numRows) const SQLINTEGER      * nullMap);

	// Internal add GUID column
	//
	void AddGuidColumnInternal(
		_In_ jint colId,
		_In_ SQLSMALLINT colType,
		_In_ SQLULEN colSize,
		_In_ SQLSMALLINT decimalDigits,
		_In_ jsize numRows,
		_In_reads_(numRows) SQLPOINTER data,
		_In_reads_opt_(numRows) const SQLINTEGER  * nullMap);

	// Internal add date column
	//
	void AddDateColumnInternal(
		_In_ jint colId,
		_In_ SQLSMALLINT colType,
		_In_ SQLULEN colSize,
		_In_ SQLSMALLINT decimalDigits,
		_In_ jsize numRows,
		_In_reads_(numRows) SQLPOINTER data,
		_In_reads_opt_(numRows) const SQLINTEGER  * nullMap);

	// Internal add numeric column
	//
	void AddNumericColumnInternal(
		_In_ jint colId,
		_In_ SQLSMALLINT colType,
		_In_ SQLULEN colSize,
		_In_ SQLSMALLINT decimalDigits,
		_In_ jsize numRows,
		_In_reads_(numRows) SQLPOINTER data,
		_In_reads_opt_(numRows) const SQLINTEGER  * nullMap);

	// Internal add timestamp column
	//
	void AddTimestampColumnInternal(
		_In_ jint colId,
		_In_ SQLSMALLINT colType,
		_In_ SQLULEN colSize,
		_In_ SQLSMALLINT decimalDigits,
		_In_ jsize numRows,
		_In_reads_(numRows) SQLPOINTER data,
		_In_reads_opt_(numRows) const SQLINTEGER  * nullMap);

	// Internal get column
	//
	template<typename jArrayType, typename jType, typename cType>
	void GetColumnInternal(
		_In_ jint							 colId,
		_In_ SQLSMALLINT					 colType,
		_Out_ SQLULEN						 &numRows,
		_Out_ SQLULEN						 &dataSizeInBytes,
		_Out_ SQLSMALLINT					 &decimalDigits,
		_Outptr_result_maybenull_ SQLPOINTER *data,
		_Outptr_result_maybenull_ SQLINTEGER **nullMap);

	// Internal get string column
	//
	template<bool isUTF8>
	void GetStringColumnInternal(
		_In_ jint							 colId,
		_In_ SQLSMALLINT					 colType,
		_Out_ SQLULEN						 &numRows,
		_Out_ SQLULEN						 &dataSizeInBytes,
		_Out_ SQLSMALLINT					 &decimalDigits,
		_Outptr_result_maybenull_ SQLPOINTER *data,
		_Outptr_result_maybenull_ SQLINTEGER **nullMap);

	// Internal get binary column
	//
	void GetBinaryColumnInternal(
		_In_ jint							 colId,
		_In_ SQLSMALLINT					 colType,
		_Out_ SQLULEN						 &numRows,
		_Out_ SQLULEN						 &dataSizeInBytes,
		_Out_ SQLSMALLINT					 &decimalDigits,
		_Outptr_result_maybenull_ SQLPOINTER *data,
		_Outptr_result_maybenull_ SQLINTEGER **nullMap);

	// Internal get date column
	//
	void GetDateColumnInternal(
		_In_ jint							 colId,
		_In_ SQLSMALLINT					 colType,
		_Out_ SQLULEN						 &numRows,
		_Out_ SQLULEN						 &dataSizeInBytes,
		_Out_ SQLSMALLINT					 &decimalDigits,
		_Outptr_result_maybenull_ SQLPOINTER *data,
		_Outptr_result_maybenull_ SQLINTEGER **nullMap);

	// Internal get numeric column
	//
	void GetNumericColumnInternal(
		_In_ jint							 colId,
		_In_ SQLSMALLINT					 colType,
		_Out_ SQLULEN						 &numRows,
		_Out_ SQLULEN						 &dataSizeInBytes,
		_Out_ SQLSMALLINT					 &decimalDigits,
		_Outptr_result_maybenull_ SQLPOINTER *data,
		_Outptr_result_maybenull_ SQLINTEGER **nullMap);

	// Internal get timestamp column
	//
	void GetTimestampColumnInternal(
		_In_ jint							 colId,
		_In_ SQLSMALLINT					 colType,
		_Out_ SQLULEN						 &numRows,
		_Out_ SQLULEN						 &dataSizeInBytes,
		_Out_ SQLSMALLINT					 &decimalDigits,
		_Outptr_result_maybenull_ SQLPOINTER *data,
		_Outptr_result_maybenull_ SQLINTEGER **nullMap);

	// Internal get column null map
	//
	void GetColumnNullMap(_In_ jint		   colId,
						  _Out_ SQLULEN	   &numRows,
						  _Out_ SQLINTEGER **nullMap);

	// Calculates max size of variable length output
	//
	template<typename jType, bool isUTF8 = false>
	void CalculateMaxForVarLengthOutputData(
		_In_ jobjectArray		 jArray,
		_Out_ unsigned long long &totalSizeInBytes,
		_Out_ SQLULEN			 &dataTypeSizeInBytes);

	// Finds Add<type>Column method in Dataset class
	//
	jmethodID FindAddColumnMethod(_In_ SQLSMALLINT colType);

	// Finds Get<type>Column method in Dataset class
	//
	jmethodID FindGetColumnMethod(_In_ SQLSMALLINT colType);

	// Finds Java method in Dataset class
	//
	jmethodID FindDatasetMethod(_In_ const std::string& funcName,
								_In_ const std::string& funcSignature);

	// Create column null map in JNI
	//
	jbooleanArray CreateJniNullMap(_In_ jsize numRows,
								   _In_reads_(numRows) const SQLINTEGER * values);

	// Creates null map to send to SQL
	//
	SQLINTEGER*CreateSqlNullMap(_In_ SQLULEN numRows, _In_reads_(numRows) jbooleanArray jArray);

	jobject m_object;														// Dataset object global reference
	JNIEnv *m_env;															// JNI enviroment
	jclass m_class;															// Dataset class reference
	static std::unordered_map<SQLSMALLINT, fnAddColumn> m_fnAddColumnMap;	// Function map for adding column to Dataset
	static std::unordered_map<SQLSMALLINT, fnGetColumn> m_fnGetColumnMap;	// Function map for getting column from Dataset

	// Function map typedefs
	//
	typedef std::unordered_map<SQLSMALLINT, fnGetColumn> GetColumnFnMap;
	typedef std::unordered_map<SQLSMALLINT, fnAddColumn> AddColumnFnMap;
};
