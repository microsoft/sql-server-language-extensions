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
#include "PythonSession.h"
#include "Logger.h"
#include "PythonExtensionUtils.h"

using namespace std;
namespace py = boost::python;

    // Data pertaining to a session
    //
string stdOutErr = "import sys\n\
class CatchOutErr:\n\
    def __init__(self):\n\
        self.value = ''\n\
    def write(self, txt):\n\
        self.value += txt\n\
catchOut = CatchOutErr()\n\
catchErr = CatchOutErr()\n\
sys.stdout = catchOut\n\
sys.stderr = catchErr\n";

// Constructor to initialize the members
//
PythonSession::PythonSession()
{}

// Destructor, releases global references
//
PythonSession::~PythonSession()
{}

// Init the session
//
void PythonSession::Init(
    _In_ const SQLGUID  *sessionId,
    _In_ SQLUSMALLINT   taskId,
    _In_ SQLUSMALLINT   numTasks,
    _In_ SQLCHAR        *script,
    _In_ SQLULEN        scriptLength,
    _In_ SQLUSMALLINT   inputSchemaColumnsNumber,
    _In_ SQLUSMALLINT   parametersNumber,
    _In_ SQLCHAR        *inputDataName,
    _In_ SQLUSMALLINT   inputDataNameLength,
    _In_ SQLCHAR        *outputDataName,
    _In_ SQLUSMALLINT   outputDataNameLength)
{

    cout << "PythonSession::Init" << endl;

    //Py_Initialize();
    cout << "PythonSession::Init modules " << endl;

    //m_builtinNamespace["__builtins__"] = py::import(PYTHON_BUILTINS);
    m_mainModule = py::import("__main__");
    m_mainNamespace = m_mainModule.attr("__dict__");

    py::exec(stdOutErr.c_str(), m_mainNamespace);

    m_sessionId = *sessionId;
    m_taskId = taskId;
    m_numTasks = numTasks;

    cout << "PythonSession::Init Script" << endl;
    // Initialize the script
    //
    m_scriptLength = scriptLength;
    m_script = new CHAR[scriptLength + 1];	// +1 to account for the null terminator
    memcpy(m_script, script, scriptLength + 1);

    // Initialize the InputDataSet
    //
    m_inputDataSetNameLength = inputDataNameLength;
    m_inputDataSetName = new CHAR[inputDataNameLength + 1];	// +1 to account for the null terminator
    memcpy(m_inputDataSetName, inputDataName, inputDataNameLength + 1);

    // Initialize the OutputDataSet
    //
    m_outputDataSetNameLength = outputDataNameLength;
    m_outputDataSetName = new CHAR[outputDataNameLength + 1];	// +1 to account for the null terminator
    memcpy(m_outputDataSetName, outputDataName, outputDataNameLength + 1);

    // Initialize the rest of the parameters like storing the schema
    //
    m_inputSchemaColumnsNumber = inputSchemaColumnsNumber;

    // Allocate space for the vectors so they can be populated later
    //
    m_inputDataTypes.resize(inputSchemaColumnsNumber);
    m_inputColumnNames.resize(inputSchemaColumnsNumber);
    m_inputColumnSizes.resize(inputSchemaColumnsNumber);
    m_inputNullColumns.resize(inputSchemaColumnsNumber);
    m_inputDecimalDigits.resize(inputSchemaColumnsNumber);

}

// Init the input column
//
void PythonSession::InitColumn(
    _In_ SQLUSMALLINT  ColumnNumber,
    _In_ const SQLCHAR *ColumnName,
    _In_ SQLSMALLINT   ColumnNameLength,
    _In_ SQLSMALLINT   DataType,
    _In_ SQLULEN	   ColumnSize,
    _In_ SQLSMALLINT   DecimalDigits,
    _In_ SQLSMALLINT   Nullable,
    _In_ SQLSMALLINT   PartitionByNumber,
    _In_ SQLSMALLINT   OrderByNumber)
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
    CHAR *name = new CHAR[ColumnNameLength + 1];	// +1 to account for the null terminator
    memcpy(name, ColumnName, ColumnNameLength + 1);

    m_inputColumnNames[ColumnNumber] = name;
    m_inputColumnSizes[ColumnNumber] = ColumnSize;
    m_inputDataTypes[ColumnNumber] = DataType;
    m_inputNullColumns[ColumnNumber] = Nullable;
    m_inputDecimalDigits[ColumnNumber] = DecimalDigits;
 }

// Init the input parameter
//
void PythonSession::InitParam(
    SQLUSMALLINT  ParamNumber,
    const SQLCHAR *ParamName,
    SQLSMALLINT	  ParamNameLength,
    SQLSMALLINT	  DataType,
    SQLULEN		  ArgSize,
    SQLSMALLINT	  DecimalDigits,
    SQLPOINTER	  ArgValue,
    SQLINTEGER	  StrLen_or_Ind,
    SQLSMALLINT	  InputOutputType)
{
    cout << "PythonSession::InitParam: " << ParamName << " with value: " << ArgValue << endl;
    if (ParamName == nullptr)
    {
        throw invalid_argument("Invalid input parameter name supplied");
    }

    string name(reinterpret_cast<const char*>((ParamName + 1)));

    if (InputOutputType == SQL_PARAM_INPUT) {
        PythonExtensionUtils::AddArgToNamespace(
            m_mainNamespace,
            name,
            DataType,
            ArgSize,
            DecimalDigits,
            ArgValue,
            StrLen_or_Ind);
    }
    //if (result != SQL_SUCCESS)
    //{
    //    // AddArg() will have outputted the correct error
    //    //
    //    throw runtime_error("Error adding script argument");
    //}
}

// Execute the workflow for the session
//
void PythonSession::ExecuteWorkflow(
    _In_ SQLULEN		RowsNumber,
    _In_opt_ SQLPOINTER *Data,
    _In_opt_ SQLINTEGER **StrLen_or_Ind,
    _Out_ SQLUSMALLINT	*OutputSchemaColumnsNumber)
{
    cout << "PythonSession::ExecuteWorkflow" << endl;

    *OutputSchemaColumnsNumber = 0;

    m_inputDataSet = py::dict();
    for (int i = 0; i < m_inputSchemaColumnsNumber; i++) {
        string name = m_inputColumnNames[i];

        SQLINTEGER *colNullMap = nullptr;
        SQLPOINTER colData = nullptr;

        if (StrLen_or_Ind != nullptr)
        {
            colNullMap = StrLen_or_Ind[i];
        }
        if (Data != nullptr)
        {
            colData = Data[i];
        }
        SQLSMALLINT dataType = m_inputDataTypes[i];
        PythonExtensionUtils::AddColumnToDictionary(&m_inputDataSet,
                                                    name,
                                                    RowsNumber,
                                                    colData,
                                                    dataType);
    }

    m_mainNamespace["InputDataSet_predict"] = m_inputDataSet;
    string createDictScript = string(m_inputDataSetName) + " = pandas.DataFrame(InputDataSet_predict)";

    py::exec("import pandas", m_mainNamespace);
    py::exec(createDictScript.c_str(), m_mainNamespace);


    try {
        py::exec(m_script, m_mainNamespace);

        string output = py::extract<string>(m_mainNamespace["catchOut"].attr("value"));
        string error  = py::extract<string>(m_mainNamespace["catchErr"].attr("value"));

        cout << output << endl;
        cerr << error << endl;
    }
    catch (invalid_argument) {
        throw;
    }
    catch (py::error_already_set&) {
        LOG_ERROR("Error running python");
        PyErr_Print();
    }
    catch (...) {
        LOG_ERROR("Unknown Error while trying to execute python script");
    }
}

// Get the metadata for the output column
//
void PythonSession::GetResultColumn(
    _In_ SQLUSMALLINT ColumnNumber,
    _Out_ SQLSMALLINT *DataType,
    _Out_ SQLULEN	  *ColumnSize,
    _Out_ SQLSMALLINT *DecimalDigits,
    _Out_ SQLSMALLINT *Nullable)
{
    cout << "PythonSession::GetResultColumn" << endl;
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

// Get the results
//
void PythonSession::GetResults(
    _Out_ SQLULEN		*RowsNumber,
    _Outptr_ SQLPOINTER **Data,
    _Outptr_ SQLINTEGER ***StrLen_or_Ind)
{
    cout << "PythonSession::GetResults" << endl;
}

// Get the the output parameter
//
void PythonSession::GetOutputParam(
    _In_ SQLUSMALLINT ParamNumber,
    _Out_ SQLPOINTER  *ParamValue,
    _Out_ SQLINTEGER  *StrLen_or_Ind)
{
    cout << "PythonSession::GetOutputParam" << endl;
}

// Cleanup session
//
void PythonSession::Cleanup()
{
    cout << "PythonSession::Cleanup" << endl;
}

void PythonSession::HandlePythonError(std::string expectedError, std::exception& exceptionToThrow)
{
    cout << "PythonSession::HandlePythonError" << endl;
    py::object errorObject = py::import("exceptions").attr(expectedError.c_str());
    PyObject *e, *v, *t;
    PyErr_Fetch(&e, &v, &t);

    // A NULL e means that there is not available Python
    // exception
    if (!e) return;

    // See if the exception was an AttributeError. If so,
    // throw a C++ version of that exception
    if (PyErr_GivenExceptionMatches(errorObject.ptr(), e))
    {
        // We construct objects now since we plan to keep
        // ownership of the references.
        py::object e_obj(py::handle<>(py::allow_null(e)));
        py::object v_obj(py::handle<>(py::allow_null(v)));
        py::object t_obj(py::handle<>(py::allow_null(t)));

        throw;
    }

    // We didn't do anything with the Python exception,
    // and we never took ownership of the refs, so it's
    // safe to simply pass them back to PyErr_Restore
    PyErr_Restore(e, v, t);

    // Rethrow the exception (or whatever...this
    // is just an example.)
    throw;
}

// Cleanup output buffers
//
void PythonSession::CleanupOutputDataBuffers()
{
    cout << "PythonSession::CleanupOutputDataBuffers" << endl;
}
