// pythonextension.cpp : Defines the exported functions for the DLL application.
//

#ifdef _WIN64
#include <windows.h>
#endif
#include <boost/python.hpp>
#include <iostream>
#include <string>
#include <ctime>
#include <sqltypes.h>
#include <sqlext.h>
#include "sqlexternallanguage.h"
#include "PythonSession.h"
#include "PythonExtensionUtils.h"
#include "Logger.h"
#include "Utilities.h"

using namespace std;

// Use macros to account for changes in Python 2 and 3:
// - Python's C API for embedding requires different naming conventions for
//   module initialization functions.
// - The builtins module was renamed.
//#if PY_VERSION_HEX >= 0x03000000
//#define MODULE_INIT_FN(name) BOOST_PP_CAT(PyInit_, name)
//#define PYTHON_BUILTINS "builtins"
//#else
//#define MODULE_INIT_FN(name) BOOST_PP_CAT(init, name)
//#define PYTHON_BUILTINS "__builtin__"
//#endif

static HMODULE g_pyDLL = nullptr;
static void *g_initFnAddress = nullptr;
//static PythonSession *g_pySession = nullptr;
static unordered_map<string, PythonSession *> g_pySessionMap;

typedef void(__cdecl *PYINIT)();

//--------------------------------------------------------------------------------------------------
// Name: GetInterfaceVersion
//
// Description:
//	Returns the API interface version for the extension
//
// Returns:
//	EXTERNAL_LANGUAGE_EXTENSION_API
//
SQLUSMALLINT
GetInterfaceVersion()
{
    return EXTERNAL_LANGUAGE_EXTENSION_API;
}

//--------------------------------------------------------------------------------------------------
// Name: Init
//
// Description:
//	Initialize the java extension. Until registration, nothing is needed here.
//
// Returns:
//	SQL_SUCCESS on success, else SQL_ERROR
//
SQLRETURN Init(
    SQLCHAR *ExtensionParams,
    SQLULEN ExtensionParamsLength,
    SQLCHAR *ExtensionPath,
    SQLULEN ExtensionPathLength,
    SQLCHAR *PublicLibraryPath,
    SQLULEN PublicLibraryPathLength,
    SQLCHAR *PrivateLibraryPath,
    SQLULEN PrivateLibraryPathLength
)
{
    cout << "PythonExtension::Init" << endl;

    std::string pyDllPath = PythonExtensionUtils::GetDLLPath();

    g_pyDLL = LoadLibraryExA(pyDllPath.c_str(), nullptr, 0);

    g_initFnAddress = Utilities::GetFunctionFromModule(&g_pyDLL, "Py_Initialize");

    cout << "PythonExtension::Python_Init" << endl;
    PYINIT py_init = (PYINIT)g_initFnAddress;
    py_init();

    return SQL_SUCCESS;
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
    SQLGUID		 SessionId,
    SQLUSMALLINT TaskId,
    SQLUSMALLINT NumTasks,
    SQLCHAR		 *Script,
    SQLULEN		 ScriptLength,
    SQLUSMALLINT InputSchemaColumnsNumber,
    SQLUSMALLINT ParametersNumber,
    SQLCHAR		 *InputDataName,
    SQLUSMALLINT InputDataNameLength,
    SQLCHAR		 *OutputDataName,
    SQLUSMALLINT OutputDataNameLength
)
{
    string session = Utilities::ConvertGuidToString(&SessionId);
    cout << "InitSession" << endl;
    g_pySessionMap[session] = new PythonSession();
    g_pySessionMap[session]->Init(
        &SessionId,
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
    return SQL_SUCCESS;
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
    SQLGUID		 SessionId,
    SQLUSMALLINT TaskId,
    SQLUSMALLINT ColumnNumber,
    SQLCHAR		 *ColumnName,
    SQLSMALLINT	 ColumnNameLength,
    SQLSMALLINT	 DataType,
    SQLULEN		 ColumnSize,
    SQLSMALLINT	 DecimalDigits,
    SQLSMALLINT	 Nullable,
    SQLSMALLINT	 PartitionByNumber,
    SQLSMALLINT	 OrderByNumber
)
{
    string session = Utilities::ConvertGuidToString(&SessionId);
    cout << "InitColumn" << endl;
    g_pySessionMap[session]->InitColumn(
        ColumnNumber,
        ColumnName,
        ColumnNameLength,
        DataType,
        ColumnSize,
        DecimalDigits,
        Nullable,
        PartitionByNumber,
        OrderByNumber);
    return SQL_SUCCESS;
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
    SQLGUID		 SessionId,
    SQLUSMALLINT TaskId,
    SQLUSMALLINT ParamNumber,
    SQLCHAR		 *ParamName,
    SQLSMALLINT	 ParamNameLength,
    SQLSMALLINT	 DataType,
    SQLULEN		 ParamSize,
    SQLSMALLINT	 DecimalDigits,
    SQLPOINTER	 ParamValue,
    SQLINTEGER	 StrLen_or_Ind,
    SQLSMALLINT	 InputOutputType)
{
    string session = Utilities::ConvertGuidToString(&SessionId);
    cout << "InitParam" << endl;
    g_pySessionMap[session]->InitParam(
        ParamNumber,
        ParamName,
        ParamNameLength,
        DataType,
        ParamSize,
        DecimalDigits,
        ParamValue,
        StrLen_or_Ind,
        InputOutputType);
    return SQL_SUCCESS;
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
    SQLGUID		 SessionId,
    SQLUSMALLINT TaskId,
    SQLULEN		 RowsNumber,
    SQLPOINTER	 *Data,
    SQLINTEGER	 **StrLen_or_Ind,
    SQLUSMALLINT *OutputSchemaColumnsNumber
)
{
    cout << "Execute" << endl;
    string session = Utilities::ConvertGuidToString(&SessionId);
    g_pySessionMap[session]->ExecuteWorkflow(RowsNumber,
                                Data,
                                StrLen_or_Ind,
                                OutputSchemaColumnsNumber);
    return SQL_SUCCESS;
}

SQLEXTENSION_INTERFACE
SQLRETURN GetResultColumn(
    SQLGUID		 SessionId,
    SQLUSMALLINT TaskId,
    SQLUSMALLINT ColumnNumber,
    SQLSMALLINT	 *DataType,
    SQLULEN		 *ColumnSize,
    SQLSMALLINT	 *DecimalDigits,
    SQLSMALLINT	 *Nullable
)
{
    cout << "GetResultColumn" << endl;
    string session = Utilities::ConvertGuidToString(&SessionId);
    g_pySessionMap[session]->GetResultColumn(
        ColumnNumber,
        DataType,
        ColumnSize,
        DecimalDigits,
        Nullable);
    return SQL_SUCCESS;
}

SQLEXTENSION_INTERFACE
SQLRETURN GetResults(
    SQLGUID		 SessionId,
    SQLUSMALLINT TaskId,
    SQLULEN		 *RowsNumber,
    SQLPOINTER	 **Data,
    SQLINTEGER	 ***StrLen_or_Ind
)
{
    cout << "GetResults" << endl;
    string session = Utilities::ConvertGuidToString(&SessionId);
    g_pySessionMap[session]->GetResults(RowsNumber,
        Data,
        StrLen_or_Ind);
    return SQL_SUCCESS;
}

SQLEXTENSION_INTERFACE
SQLRETURN GetOutputParam(
    SQLGUID		 SessionId,
    SQLUSMALLINT TaskId,
    SQLUSMALLINT ParamNumber,
    SQLPOINTER	 *ParamValue,
    SQLINTEGER	 *StrLen_or_Ind
)
{
    cout << "GetOutputParam" << endl;
    string session = Utilities::ConvertGuidToString(&SessionId);
    g_pySessionMap[session]->GetOutputParam(
        ParamNumber,
        ParamValue,
        StrLen_or_Ind);
    return SQL_SUCCESS;
}

SQLEXTENSION_INTERFACE
SQLRETURN CleanupSession(
    SQLGUID		 SessionId,
    SQLUSMALLINT TaskId
)
{
    cout << "CleanupSession" << endl;
    string session = Utilities::ConvertGuidToString(&SessionId);
    if (g_pySessionMap.count(session) > 0) {
        g_pySessionMap[session]->Cleanup();
        delete g_pySessionMap[session];
        g_pySessionMap.erase(session);
    }
    return SQL_SUCCESS;
}

SQLEXTENSION_INTERFACE
SQLRETURN Cleanup()
{
    cout << "Cleanup" << endl;

    if (g_pyDLL != nullptr) {
        FreeLibrary(g_pyDLL);
    }
    return SQL_SUCCESS;
}
