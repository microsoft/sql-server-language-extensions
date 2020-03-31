//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JavaDataset.h
//
// Purpose:
//  Handles loading and retrieving data from the Dataset class
//
//*********************************************************************
#pragma once

#include "Common.h"

//---------------------------------------------------------------------
// Description:
//	Stores information about the Java dataset
//
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
	void Init(JNIEnv *env, const std::string &className);

	// Initializes the object from an existing Java object
	//
	void Init(JNIEnv *env, jobject obj);

	// Adds the column metadata
	//
	void AddColumnMetadata(
		jint              colId,
		const std::string &colName,
		SQLSMALLINT       colType,
		SQLULEN           colSize,
		SQLSMALLINT       decimalDigits);

	// Adds the column
	//
	void AddColumn(
		jint             colId,
		SQLSMALLINT      colType,
		SQLULEN          colSize,
		SQLSMALLINT      decimalDigits,
		SQLULEN          numRows,
		const SQLPOINTER data,
		const SQLINTEGER *nullMap);

	// Adds each column
	//
	void AddColumns(
		SQLUSMALLINT             numCols,
		SQLULEN                  numRows,
		std::vector<SQLSMALLINT> &colTypes,
		std::vector<std::string> &colNames,
		std::vector<SQLULEN>     &colSizes,
		std::vector<SQLSMALLINT> &decimalDigits,
		std::vector<SQLSMALLINT> &nullableCols,
		SQLPOINTER               *data,
		SQLINTEGER               **nullMap);

	// Gets the column type
	//
	void GetColumnType(jint colId, SQLSMALLINT &colType);

	// Gets the column precision
	//
	void GetColumnPrecision(jint colId, SQLULEN &colPrec);

	// Gets the column scale
	//
	void GetColumnScale(jint colId, SQLSMALLINT &colScale);

	// Gets the column
	//
	void GetColumn(
		jint        colId,
		SQLSMALLINT &colType,
		SQLULEN     &colSize,
		SQLSMALLINT &colDecimalDigits,
		SQLULEN     &numRows,
		SQLPOINTER  *data,
		SQLINTEGER  **nullMap);

	// Gets all columns
	//
	void GetColumns(
		SQLUSMALLINT             &numCols,
		SQLULEN                  &numRows,
		std::vector<SQLSMALLINT> &colTypes,
		std::vector<SQLULEN>     &colSizes,
		std::vector<SQLSMALLINT> &colDecimalDigits,
		std::vector<SQLSMALLINT> &nullableCols,
		std::vector<SQLPOINTER>  &data,
		std::vector<SQLINTEGER*> &nullMaps);

	// Get the jobject of the Java dataset
	//
	jobject GetJavaObject() const
	{
		return m_object;
	}

private:
	// Add column function pointer definition
	//
	using fnAddColumn = void (JavaDataset::*)(
		jint             colId,
		SQLSMALLINT      colType,
		SQLULEN          colSize,
		SQLSMALLINT      decimalDigits,
		jsize            numRows,
		SQLPOINTER       data,
		const SQLINTEGER *nullMap);

	// Get column function pointer definition
	//
	using fnGetColumn = void (JavaDataset::*)(
		jint        colId,
		SQLSMALLINT colType,
		SQLULEN     &numRows,
		SQLULEN     &dataSizeInBytes,
		SQLSMALLINT &decimalDigits,
		SQLPOINTER  *data,
		SQLINTEGER  **nullMap);

	// Internal add column
	//
	template<typename jArrayType, typename jType, typename cType>
	void AddColumnInternal(
		jint             colId,
		SQLSMALLINT      colType,
		SQLULEN          colSize,
		SQLSMALLINT      decimalDigits,
		jsize            numRows,
		SQLPOINTER       data,
		const SQLINTEGER *nullMap);

	// Internal add string column
	//
	template<bool isUTF8>
	void AddStringColumnInternal(
		jint             colId,
		SQLSMALLINT      colType,
		SQLULEN          colSize,
		SQLSMALLINT      decimalDigits,
		jsize            numRows,
		SQLPOINTER       data,
		const SQLINTEGER *nullMap);

	// Internal add binary column
	//
	void AddBinaryColumnInternal(
		jint             colId,
		SQLSMALLINT      colType,
		SQLULEN          colSize,
		SQLSMALLINT      decimalDigits,
		jsize            numRows,
		SQLPOINTER       data,
		const SQLINTEGER *nullMap);

	// Internal add GUID column
	//
	void AddGuidColumnInternal(
		jint             colId,
		SQLSMALLINT      colType,
		SQLULEN          colSize,
		SQLSMALLINT      decimalDigits,
		jsize            numRows,
		SQLPOINTER       data,
		const SQLINTEGER *nullMap);

	// Internal add date column
	//
	void AddDateColumnInternal(
		jint             colId,
		SQLSMALLINT      colType,
		SQLULEN          colSize,
		SQLSMALLINT      decimalDigits,
		jsize            numRows,
		SQLPOINTER       data,
		const SQLINTEGER *nullMap);

	// Internal add numeric column
	//
	void AddNumericColumnInternal(
		jint             colId,
		SQLSMALLINT      colType,
		SQLULEN          colSize,
		SQLSMALLINT      decimalDigits,
		jsize            numRows,
		SQLPOINTER       data,
		const SQLINTEGER *nullMap);

	// Internal add timestamp column
	//
	void AddTimestampColumnInternal(
		jint             colId,
		SQLSMALLINT      colType,
		SQLULEN          colSize,
		SQLSMALLINT      decimalDigits,
		jsize            numRows,
		SQLPOINTER       data,
		const SQLINTEGER *nullMap);

	// Internal get column
	//
	template<typename jArrayType, typename jType, typename cType>
	void GetColumnInternal(
		jint        colId,
		SQLSMALLINT colType,
		SQLULEN     &numRows,
		SQLULEN     &dataSizeInBytes,
		SQLSMALLINT &decimalDigits,
		SQLPOINTER  *data,
		SQLINTEGER  **nullMap);

	// Internal get string column
	//
	template<bool isUTF8>
	void GetStringColumnInternal(
		jint        colId,
		SQLSMALLINT colType,
		SQLULEN     &numRows,
		SQLULEN     &dataSizeInBytes,
		SQLSMALLINT &decimalDigits,
		SQLPOINTER  *data,
		SQLINTEGER  **nullMap);

	// Internal get binary column
	//
	void GetBinaryColumnInternal(
		jint        colId,
		SQLSMALLINT colType,
		SQLULEN     &numRows,
		SQLULEN     &dataSizeInBytes,
		SQLSMALLINT &decimalDigits,
		SQLPOINTER  *data,
		SQLINTEGER  **nullMap);

	// Internal get date column
	//
	void GetDateColumnInternal(
		jint        colId,
		SQLSMALLINT colType,
		SQLULEN     &numRows,
		SQLULEN     &dataSizeInBytes,
		SQLSMALLINT &decimalDigits,
		SQLPOINTER  *data,
		SQLINTEGER  **nullMap);

	// Internal get numeric column
	//
	void GetNumericColumnInternal(
		jint        colId,
		SQLSMALLINT colType,
		SQLULEN     &numRows,
		SQLULEN     &dataSizeInBytes,
		SQLSMALLINT &decimalDigits,
		SQLPOINTER  *data,
		SQLINTEGER  **nullMap);

	// Internal get timestamp column
	//
	void GetTimestampColumnInternal(
		jint        colId,
		SQLSMALLINT colType,
		SQLULEN     &numRows,
		SQLULEN     &dataSizeInBytes,
		SQLSMALLINT &decimalDigits,
		SQLPOINTER  *data,
		SQLINTEGER  **nullMap);

	// Internal get column null map
	//
	void GetColumnNullMap(jint colId, SQLULEN &numRows, SQLINTEGER **nullMap);

	// Calculates max size of variable length output
	//
	template<typename jType, bool isUTF8 = false>
	void CalculateMaxForVarLengthOutputData(
		jobjectArray       jArray,
		unsigned long long &totalSizeInBytes,
		SQLULEN            &dataTypeSizeInBytes);

	// Finds Add<type>Column method in Dataset class
	//
	jmethodID FindAddColumnMethod(SQLSMALLINT colType);

	// Finds Get<type>Column method in Dataset class
	//
	jmethodID FindGetColumnMethod(SQLSMALLINT colType);

	// Finds Java method in Dataset class
	//
	jmethodID FindDatasetMethod(const std::string& funcName, const std::string& funcSignature);

	// Create column null map in JNI
	//
	jbooleanArray CreateJniNullMap(jsize numRows, const SQLINTEGER *values);

	// Creates null map to send to SQL
	//
	SQLINTEGER* CreateSqlNullMap(SQLULEN numRows, jbooleanArray jArray);

	jobject m_object;                                                     // Dataset object global reference
	JNIEnv *m_env;                                                        // JNI enviroment
	jclass m_class;                                                       // Dataset class reference
	static std::unordered_map<SQLSMALLINT, fnAddColumn> m_fnAddColumnMap; // Function map for adding column to Dataset
	static std::unordered_map<SQLSMALLINT, fnGetColumn> m_fnGetColumnMap; // Function map for getting column from Dataset

	// Function map typedefs
	//
	typedef std::unordered_map<SQLSMALLINT, fnGetColumn> GetColumnFnMap;
	typedef std::unordered_map<SQLSMALLINT, fnAddColumn> AddColumnFnMap;
};
