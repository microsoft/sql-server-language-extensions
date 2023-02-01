//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpExtensionApiTests.cpp
//
// Purpose:
//  Defines the base test fixture members and tests .NET Core CSharpExtension's implementation of
//  the external language initialization and cleanup APIs.
//
//*********************************************************************
#include "CSharpExtensionApiTests.h"

using namespace std;
namespace fs = experimental::filesystem;

namespace ExtensionApiTest
{
    // Initialize all the static members
    //
    string CSharpExtensionApiTests::sm_extensionPath;
    string CSharpExtensionApiTests::sm_libPath;
    HINSTANCE CSharpExtensionApiTests::sm_libHandle;
    FN_init *CSharpExtensionApiTests::sm_initFuncPtr = nullptr;
    FN_initSession *CSharpExtensionApiTests::sm_initSessionFuncPtr = nullptr;
    FN_initColumn *CSharpExtensionApiTests::sm_initColumnFuncPtr = nullptr;
    FN_initParam *CSharpExtensionApiTests::sm_initParamFuncPtr = nullptr;
    FN_execute *CSharpExtensionApiTests::sm_executeFuncPtr = nullptr;
    FN_getResultColumn *CSharpExtensionApiTests::sm_getResultColumnFuncPtr = nullptr;
    FN_getResults *CSharpExtensionApiTests::sm_getResultsFuncPtr = nullptr;
    FN_getOutputParam *CSharpExtensionApiTests::sm_getOutputParamFuncPtr = nullptr;
    FN_cleanupSession *CSharpExtensionApiTests::sm_cleanupSessionFuncPtr = nullptr;
    FN_cleanup *CSharpExtensionApiTests::sm_cleanupFuncPtr = nullptr;

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::SetUpTestCase
    //
    // Description:
    //  Per-test-suite set-up. Called before the first test in every test suite.
    //  But we want to execute this only once in the entire test run before the first test case.
    //  since .NET can be initialized only once.
    //
    void CSharpExtensionApiTests::SetUpTestCase()
    {
        SetUpPath();
        GetHandles();
        DoInit();
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::TearDownTestCase
    //
    // Description:
    //  Per-test-suite tear-down. Called after the last test in every test suite.
    //  But we want to execute this only once in the entire test run after the last test case
    //  since .NET can be torn down only once.
    //
    void CSharpExtensionApiTests::TearDownTestCase()
    {
        DoCleanup();
    }

    // Name: CSharpExtensionApiTest::SetUp
    //
    // Description:
    //  Code here will be called immediately after the constructor (right
    //  before each test).
    //
    void CSharpExtensionApiTests::SetUp()
    {
        SetupVariables();
    }

    // Name: CSharpExtensionApiTest::TearDown
    //
    // Description:
    //  Code here will be called immediately after each test
    //  (right before the destructor).
    //
    void CSharpExtensionApiTests::TearDown()
    {
        SessionCleanup();
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::SetUpPath
    //
    // Description:
    //  Sets the extension path variables.
    //
    void CSharpExtensionApiTests::SetUpPath()
    {
        char path[MAX_PATH+1] = {0};
        GetModuleFileName(NULL, path, MAX_PATH);
        fs::path exePath = path;
        fs::path buildOutputPath = exePath.parent_path().parent_path().parent_path().parent_path();
    #if defined(_DEBUG)
        sm_extensionPath = (buildOutputPath / "dotnet-core-CSharp-extension/windows/debug").string();
    #else
        sm_extensionPath = (buildOutputPath / "dotnet-core-CSharp-extension/windows/release").string();
    #endif
        sm_libPath = exePath.parent_path().string();
    }

    // Name: CSharpExtensionApiTest::SetupVariables
    //
    // Description:
    //  Sets up default, valid variables for use in tests
    //
    void CSharpExtensionApiTests::SetupVariables()
    {
        shared_ptr<SQLGUID> sessionId(new SQLGUID());
        m_sessionId = move(sessionId);
        m_taskId = 0;
        m_numTasks = 1;
        m_inputSchemaColumnsNumber = 1;
        m_parametersNumber = 0;
        m_scriptString = m_UserLibName + m_Separator + m_UserClassFullName;
        m_script = static_cast<SQLCHAR *>(static_cast<void *>(const_cast<char *>(m_scriptString.c_str())));
        m_scriptLength = m_scriptString.length();

        m_inputDataNameString = "InputDataSet";
        m_inputDataName = static_cast<SQLCHAR *>(
            static_cast<void *>(const_cast<char *>(m_inputDataNameString.c_str())));
        m_inputDataNameLength = m_inputDataNameString.length();

        m_outputDataNameString = "OutputDataSet";
        m_outputDataName = static_cast<SQLCHAR *>(
            static_cast<void *>(const_cast<char *>(m_outputDataNameString.c_str())));
        m_outputDataNameLength = m_outputDataNameString.length();

        m_emptyIntegerInfo = make_unique<ColumnInfo<SQLINTEGER>>(
            "IntegerColumn1",
            vector<SQLINTEGER>{ },
            vector<SQLINTEGER>(0, m_IntSize),
            "IntegerColumn2",
            vector<SQLINTEGER>{ },
            vector<SQLINTEGER>(0, m_IntSize),
            vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NO_NULLS });

        m_integerInfo = make_unique<ColumnInfo<SQLINTEGER>>(
            "IntegerColumn1",
            vector<SQLINTEGER>{ 1, 2, 3, 4, 5 },
            vector<SQLINTEGER>(ColumnInfo<SQLINTEGER>::sm_rowsNumber, m_IntSize),
            "IntegerColumn2",
            vector<SQLINTEGER>{ m_MaxInt, m_MinInt, 0, 0, -1 },
            vector<SQLINTEGER>{ m_IntSize, m_IntSize, SQL_NULL_DATA,
                SQL_NULL_DATA, m_IntSize },
            vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });

        m_booleanInfo = make_unique<ColumnInfo<SQLCHAR>>(
            "BooleanColumn1",
            vector<SQLCHAR>{ '1', '0', '1', 0, 1 },
            vector<SQLINTEGER>(ColumnInfo<SQLCHAR>::sm_rowsNumber, m_BooleanSize),
            "BooleanColumn2",
            vector<SQLCHAR>{ '\0', '2', '1', '0', '\0' },
            vector<SQLINTEGER>{ SQL_NULL_DATA, m_BooleanSize, m_BooleanSize,
                m_BooleanSize, SQL_NULL_DATA },
            vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });

        m_realInfo = make_unique<ColumnInfo<SQLREAL>>(
            "RealColumn1",
            vector<SQLREAL>{ 0.34, 1.33, 83.98, 72.45, 68e10 },
            vector<SQLINTEGER>(ColumnInfo<SQLREAL>::sm_rowsNumber, m_RealSize),
            "RealColumn2",
            vector<SQLREAL>{ m_MaxReal, NAN, m_MinReal, NAN, 0 },
            vector<SQLINTEGER>{ m_RealSize, SQL_NULL_DATA, m_RealSize,
                SQL_NULL_DATA, m_RealSize },
            vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });

        m_doubleInfo = make_unique<ColumnInfo<SQLDOUBLE>>(
            "DoubleColumn1",
            vector<SQLDOUBLE>{ -1.79e301, 1.33, 83.98, 72.45, 1.79e30 },
            vector<SQLINTEGER>(ColumnInfo<SQLDOUBLE>::sm_rowsNumber, m_DoubleSize),
            "DoubleColumn2",
            vector<SQLDOUBLE>{ NAN, m_MaxDouble, NAN, m_MinDouble, NAN },
            vector<SQLINTEGER>{ SQL_NULL_DATA, m_DoubleSize, SQL_NULL_DATA,
                m_DoubleSize, SQL_NULL_DATA },
            vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });

        m_bigIntInfo = make_unique<ColumnInfo<SQLBIGINT>>(
            "BigIntColumn1",
            vector<SQLBIGINT>{ m_MaxBigInt, 1, 88883939, m_MinBigInt, -622280108 },
            vector<SQLINTEGER>(ColumnInfo<SQLBIGINT>::sm_rowsNumber, m_BigIntSize),
            "BigIntColumn2",
            vector<SQLBIGINT>{0, 0, 0, 12341512213, -12341512213 },
            vector<SQLINTEGER>{ SQL_NULL_DATA, SQL_NULL_DATA,
            SQL_NULL_DATA, m_BigIntSize, m_BigIntSize },
            vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });

        m_smallIntInfo = make_unique<ColumnInfo<SQLSMALLINT>>(
            "SmallIntColumn1",
            vector<SQLSMALLINT>{ m_MaxSmallInt, 33, 9811, m_MinSmallInt, 6810 },
            vector<SQLINTEGER>(ColumnInfo<SQLSMALLINT>::sm_rowsNumber, m_SmallIntSize),
            "SmallIntColumn2",
            vector<SQLSMALLINT>{ -1, 0, 0, -725, 3'276 },
            vector<SQLINTEGER>{ m_SmallIntSize, SQL_NULL_DATA,
            SQL_NULL_DATA, m_SmallIntSize,m_SmallIntSize },
            vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });

        m_tinyIntInfo = make_unique<ColumnInfo<SQLCHAR>>(
            "TinyIntColumn1",
            vector<SQLCHAR>{ 34, 133, 98, 72, 10 },
            vector<SQLINTEGER>(ColumnInfo<SQLCHAR>::sm_rowsNumber, m_TinyIntSize),
            "TinyIntColumn2",
            vector<SQLCHAR>{ m_MaxTinyInt, m_MinTinyInt, 1, 0, 128 },
            vector<SQLINTEGER>{ m_TinyIntSize, SQL_NULL_DATA,
                SQL_NULL_DATA, SQL_NULL_DATA, m_TinyIntSize },
            vector<SQLSMALLINT>{ SQL_NO_NULLS, SQL_NULLABLE });
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::GetHandles
    //
    // Description:
    //  Loads library and gets handles to different functions
    //  Testing if CSharpExtension is successfully loaded dynamically
    //
    void CSharpExtensionApiTests::GetHandles()
    {
        sm_libHandle = LoadLibrary((sm_extensionPath+"\\nativecsharpextension.dll").c_str());
        EXPECT_TRUE(sm_libHandle != nullptr);

        sm_initFuncPtr = reinterpret_cast<FN_init*>(GetProcAddress(sm_libHandle, "Init"));
        EXPECT_TRUE(sm_initFuncPtr != nullptr);

        sm_initSessionFuncPtr = reinterpret_cast<FN_initSession*>(GetProcAddress(sm_libHandle, "InitSession"));
        EXPECT_TRUE(sm_initSessionFuncPtr != nullptr);

        sm_initColumnFuncPtr = reinterpret_cast<FN_initColumn*>(GetProcAddress(sm_libHandle, "InitColumn"));
        EXPECT_TRUE(sm_initColumnFuncPtr != nullptr);

        sm_initParamFuncPtr = reinterpret_cast<FN_initParam*>(GetProcAddress(sm_libHandle, "InitParam"));
        EXPECT_TRUE(sm_initParamFuncPtr != nullptr);

        sm_executeFuncPtr = reinterpret_cast<FN_execute*>(GetProcAddress(sm_libHandle, "Execute"));
        EXPECT_TRUE(sm_executeFuncPtr != nullptr);

        sm_getResultColumnFuncPtr = reinterpret_cast<FN_getResultColumn*>(GetProcAddress(sm_libHandle, "GetResultColumn"));
        EXPECT_TRUE(sm_getResultColumnFuncPtr != nullptr);

        sm_getResultsFuncPtr = reinterpret_cast<FN_getResults*>(GetProcAddress(sm_libHandle, "GetResults"));
        EXPECT_TRUE(sm_getResultsFuncPtr != nullptr);

        sm_getOutputParamFuncPtr = reinterpret_cast<FN_getOutputParam*>(GetProcAddress(sm_libHandle, "GetOutputParam"));
        EXPECT_TRUE(sm_getOutputParamFuncPtr != nullptr);

        sm_cleanupSessionFuncPtr = reinterpret_cast<FN_cleanupSession*>(GetProcAddress(sm_libHandle, "CleanupSession"));
        EXPECT_TRUE(sm_cleanupSessionFuncPtr != nullptr);

        sm_cleanupFuncPtr = reinterpret_cast<FN_cleanup*>(GetProcAddress(sm_libHandle, "Cleanup"));
        EXPECT_TRUE(sm_cleanupFuncPtr != nullptr);
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::DoInit
    //
    // Description:
    //  Does Init where embedded .NET is initialized - can be called only once in the test suite.
    //  Testing if Init api is implemented correctly.
    //
    void CSharpExtensionApiTests::DoInit()
    {
        SQLCHAR *unsignedExtensionPath = static_cast<SQLCHAR *>(
            static_cast<void *>(const_cast<char *>(sm_extensionPath.c_str())));

        SQLCHAR *unsignedLibPath = static_cast<SQLCHAR *>(
            static_cast<void *>(const_cast<char *>(sm_libPath.c_str())));

        SQLRETURN result = (*sm_initFuncPtr)(
            nullptr,                   // Extension Params
            0,                         // Extension Params Length
            unsignedExtensionPath,     // Extension Path
            sm_extensionPath.length(), // Extension Path Length
            unsignedLibPath,           // Public Library Path
            sm_libPath.length(),       // Public Library Path Length
            nullptr,                   // Private Library Path
            0                          // Private Library Path Length
        );

        EXPECT_EQ(result, SQL_SUCCESS);
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::InitializeSession
    //
    // Description:
    //  Initializes a valid, default session for later tests
    //
    void CSharpExtensionApiTests::InitializeSession(
        SQLUSMALLINT inputSchemaColumnsNumber,
        SQLUSMALLINT parametersNumber,
        string       scriptString)
    {
        SQLCHAR *script = static_cast<SQLCHAR*>(
            static_cast<void*>(const_cast<char*>(scriptString.c_str())));

        SQLRETURN result = (*sm_initSessionFuncPtr)(
            *m_sessionId,
            m_taskId,
            m_numTasks,
            script,
            scriptString.length(),
            inputSchemaColumnsNumber,
            parametersNumber,
            m_inputDataName,
            m_inputDataNameString.length(),
            m_outputDataName,
            m_outputDataNameString.length());

        EXPECT_EQ(result, SQL_SUCCESS);
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::InitializeColumn
    //
    // Description:
    //  Call InitColumn for the given columnNumber, columnName, dataType and columnSize.
    //
    void CSharpExtensionApiTests::InitializeColumn(
        SQLSMALLINT columnNumber,
        string      columnNameString,
        SQLSMALLINT dataType,
        SQLULEN     columnSize,
        SQLSMALLINT nullable,
        SQLSMALLINT partitionByNumber)
    {
        SQLCHAR *columnName = static_cast<SQLCHAR *>(
            static_cast<void *>(const_cast<char *>(columnNameString.c_str()))
            );

        SQLRETURN result = (*sm_initColumnFuncPtr)(
            *m_sessionId,
            m_taskId,
            columnNumber,
            columnName,
            columnNameString.length(),
            dataType,
            columnSize,
            0,                 // decimalDigits
            nullable,          // nullable
            partitionByNumber, // partitionByNumber
            -1);               // orderByNumber
        EXPECT_EQ(result, SQL_SUCCESS);
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::SessionCleanup
    //
    // Description:
    //  Cleans up a valid, default session for later tests
    //
    void CSharpExtensionApiTests::SessionCleanup()
    {
        SQLRETURN result = (*sm_cleanupSessionFuncPtr)(*m_sessionId, m_taskId);
        EXPECT_EQ(result, SQL_SUCCESS);
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::DoCleanup
    //
    // Description:
    // Calls Cleanup on the .NET Core CSharpExtension.
    // Testing if Cleanup is implemented correctly.
    //
    void CSharpExtensionApiTests::DoCleanup()
    {
        SQLRETURN result = (*sm_cleanupFuncPtr)();
        EXPECT_EQ(result, SQL_SUCCESS);
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::InitializeColumns
    //
    // Description:
    //  Template function to call InitializeColumn for all columns.
    //
    template<class SQLType, SQLSMALLINT dataType>
    void CSharpExtensionApiTests::InitializeColumns(ColumnInfo<SQLType> *columnInfo)
    {
        SQLUSMALLINT inputSchemaColumnsNumber = columnInfo->GetColumnsNumber();
        for (SQLUSMALLINT columnNumber = 0; columnNumber < inputSchemaColumnsNumber; ++columnNumber)
        {
            InitializeColumn(columnNumber,
                columnInfo->m_columnNames[columnNumber],
                dataType,
                sizeof(SQLType),
                columnInfo->m_nullable[columnNumber],
                columnInfo->m_partitionByIndexes[columnNumber]);
        }
    }

    //----------------------------------------------------------------------------------------------
    // Name: ColumnInfo::ColumnInfo
    //
    // Description:
    // Template constructor for the type information.
    // Useful for ColumnInfo of integer, basic numeric, logical and date(time) types.
    //
    template<class SQLType>
    ColumnInfo<SQLType>::ColumnInfo(
        string column1Name, vector<SQLType> column1, vector<SQLINTEGER> col1StrLenOrInd,
        string column2Name, vector<SQLType> column2, vector<SQLINTEGER> col2StrLenOrInd,
        vector<SQLSMALLINT> nullable,
        vector<SQLSMALLINT> partitionByIndexes)
    {
        m_columnNames = { column1Name, column2Name };
        m_column1 = column1;
        m_column2 = column2;
        m_dataSet = { m_column1.data(), m_column2.data()};
        m_col1StrLenOrInd = col1StrLenOrInd;
        if (m_col1StrLenOrInd.empty())
        {
            m_strLen_or_Ind.push_back(nullptr);
        }
        else
        {
            m_strLen_or_Ind.push_back(m_col1StrLenOrInd.data());
        }

        m_col2StrLenOrInd = col2StrLenOrInd;
        if (m_col2StrLenOrInd.empty())
        {
            m_strLen_or_Ind.push_back(nullptr);
        }
        else
        {
            m_strLen_or_Ind.push_back(m_col2StrLenOrInd.data());
        }

        m_nullable = nullable;
        m_partitionByIndexes = partitionByIndexes;
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::GenerateContiguousData
    //
    // Description:
    //  Fill a contiguous array columnData with members from the given columnVector
    //  having lengths defined in strLenOrInd, unless it is SQL_NULL_DATA.
    //
    template<class SQLType>
    vector<SQLType> CSharpExtensionApiTests::GenerateContiguousData(
        vector<const SQLType*> columnVector,
        SQLINTEGER             *strLenOrInd)
    {
        vector<SQLType> retVal;

        for (SQLULEN index = 0; index < columnVector.size(); ++index)
        {
            if (strLenOrInd[index] != SQL_NULL_DATA)
            {
                SQLINTEGER strLen = strLenOrInd[index] / sizeof(SQLType);
                vector<SQLType> data(columnVector[index], columnVector[index] + strLen);
                retVal.insert(retVal.end(), data.begin(), data.end());
            }
        }

        return retVal;
    }

    // Template instantiation
    //
    template ColumnInfo<SQL_NUMERIC_STRUCT>::ColumnInfo(
        string column1Name,
        vector<SQL_NUMERIC_STRUCT> column1,
        vector<SQLINTEGER> col1StrLenOrInd,
        string column2Name,
        vector<SQL_NUMERIC_STRUCT> column2,
        vector<SQLINTEGER> col2StrLenOrInd,
        vector<SQLSMALLINT> nullable,
        vector<SQLSMALLINT> partitionByIndexes);

    template void CSharpExtensionApiTests::InitializeColumns<SQLINTEGER, SQL_C_SLONG>(
        ColumnInfo<SQLINTEGER> *ColumnInfo);
    template void CSharpExtensionApiTests::InitializeColumns<SQLCHAR, SQL_C_BIT>(
        ColumnInfo<SQLCHAR> *ColumnInfo);
    template void CSharpExtensionApiTests::InitializeColumns<SQLREAL, SQL_C_FLOAT>(
        ColumnInfo<SQLREAL> *ColumnInfo);
    template void CSharpExtensionApiTests::InitializeColumns<SQLDOUBLE, SQL_C_DOUBLE>(
        ColumnInfo<SQLDOUBLE> *ColumnInfo);
    template void CSharpExtensionApiTests::InitializeColumns<SQLBIGINT, SQL_C_SBIGINT>(
        ColumnInfo<SQLBIGINT> *ColumnInfo);
    template void CSharpExtensionApiTests::InitializeColumns<SQLSMALLINT, SQL_C_SSHORT>(
        ColumnInfo<SQLSMALLINT> *ColumnInfo);
    template void CSharpExtensionApiTests::InitializeColumns<SQLCHAR, SQL_C_UTINYINT>(
        ColumnInfo<SQLCHAR> *ColumnInfo);

    template vector<char> CSharpExtensionApiTests::GenerateContiguousData(
        vector<const char*> columnVector,
        SQLINTEGER          *strLenOrInd);
}
