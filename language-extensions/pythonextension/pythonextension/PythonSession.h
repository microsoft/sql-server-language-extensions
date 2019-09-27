//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: PythonSession.h
// @Owner: joz
//
// Purpose:
//  Class encapsulating operations performed per session
//
//*********************************************************************
#include <boost/python.hpp>
#include <Python.h>
#include <wchar.h>
#include <stdio.h>
#include <string.h>
#include <chrono>
#include <iostream>
#include <list>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <assert.h>
#ifdef MPLAT_UNIX
#include <sqldkcmnpch.h>
#else
#include <windows.h>
#endif
#include <strsafe.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

// Use macros to account for changes in Python 2 and 3:
// - Python's C API for embedding requires different naming conventions for
//   module initialization functions.
// - The builtins module was renamed.
//#if PY_VERSION_HEX >= 0x03000000
//    #define MODULE_INIT_FN(name) BOOST_PP_CAT(PyInit_, name)
//    #define PYTHON_BUILTINS "builtins"
//#else
//    #define MODULE_INIT_FN(name) BOOST_PP_CAT(init, name)
//    #define PYTHON_BUILTINS "__builtin__"
//#endif

// Data pertaining to a session
//
class PythonSession
{
public:
    // Constructor to initialize the members
    //
    PythonSession();

    // Destructor, releases global references
    //
    ~PythonSession();

    // Init the session
    //
    void Init(
        _In_ const SQLGUID  *sessionId,
        _In_ SQLUSMALLINT   taskId,
        _In_ SQLUSMALLINT   numTasks,
        _In_ SQLCHAR	    *script,
        _In_ SQLULEN	    scriptLength,
        _In_ SQLUSMALLINT   inputSchemaColumnsNumber,
        _In_ SQLUSMALLINT   parametersNumber,
        _In_ SQLCHAR        *inputDataName,
        _In_ SQLUSMALLINT   inputDataNameLength,
        _In_ SQLCHAR        *outputDataName,
        _In_ SQLUSMALLINT   outputDataNameLength);

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
        _In_ SQLSMALLINT   OrderByNumber);

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
        SQLSMALLINT	  InputOutputType);

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

    // Get the the output parameter
    //
    void GetOutputParam(
        _In_ SQLUSMALLINT ParamNumber,
        _Out_ SQLPOINTER  *ParamValue,
        _Out_ SQLINTEGER  *StrLen_or_Ind);

    // Cleanup session
    //
    void Cleanup();

private:

    void HandlePythonError(
        _In_ std::string expectedError,
        _In_ std::exception& exceptionToThrow);


    // Cleanup output buffers
    //
    void CleanupOutputDataBuffers();

    boost::python::dict m_builtinNamespace;
    boost::python::object m_mainModule;
    boost::python::object m_mainNamespace;

    SQLGUID m_sessionId;                // Session ID
    SQLUSMALLINT m_taskId;              // Task ID for this session
    SQLUSMALLINT m_numTasks;            // Number of tasks for this session

	CHAR *m_script;                     // Script
	SQLULEN m_scriptLength;             // Script Length

    std::string m_mainClassName;        // User executor class name

    boost::python::dict m_inputDataSet; // Input DataSet
    CHAR *m_inputDataSetName;           // Input DataSet Name
    SQLULEN m_inputDataSetNameLength;   // Input DataSet Name Length

    boost::python::dict m_outputDataSet;// Output DataSet
    CHAR *m_outputDataSetName;          // Output DataSet Name
    SQLULEN m_outputDataSetNameLength;  // Output DataSet Name Length

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
