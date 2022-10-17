//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpExtensionApiTests.h
//
// Purpose:
//  Specifies some typedefinitions to be used in the test.
//  And declares the base test fixture class .NET Core CSharpExtensionApiTests.
//
//*********************************************************************
#pragma once
#include "Common.h"
#include <unordered_map>

typedef SQLRETURN FN_init(
    SQLCHAR *, // extensionParams
    SQLULEN,   // extensionParamsLength
    SQLCHAR *, // extensionPath
    SQLULEN,   // extensionPathLength
    SQLCHAR *, // publicLibraryPath
    SQLULEN,   // publicLibraryPathLength
    SQLCHAR *, // privateLibraryPath
    SQLULEN);  // privateLibraryPathLength

typedef SQLRETURN FN_initSession(
    SQLGUID,       // sessionId
    SQLUSMALLINT,  // taskId
    SQLUSMALLINT,  // numTasks
    SQLCHAR *,     // script
    SQLULEN,       // scriptLength
    SQLUSMALLINT,  // inputSchemaColumnsNumber
    SQLUSMALLINT,  // parametersNumber
    SQLCHAR*,      // inputDataName
    SQLUSMALLINT,  // inputDataNameLength
    SQLCHAR*,      // outputDataName
    SQLUSMALLINT); // outputDataNameLength

typedef SQLRETURN FN_initColumn(
    SQLGUID,      // sessionId
    SQLUSMALLINT, // taskId
    SQLUSMALLINT, // columnNumber
    SQLCHAR *,    // columnName
    SQLSMALLINT,  // columnNameLenth
    SQLSMALLINT,  // dataType
    SQLULEN,      // columnSize,
    SQLSMALLINT,  // decimalDigits
    SQLSMALLINT,  // nullable
    SQLSMALLINT,  // partitionByNumber
    SQLSMALLINT); // orderByNumber

typedef SQLRETURN FN_initParam(
    SQLGUID,      // sessionId
    SQLUSMALLINT, // taskId
    SQLUSMALLINT, // paramNumber
    SQLCHAR*,     // paramName,
    SQLSMALLINT,  // paramNameLength
    SQLSMALLINT,  // dataType
    SQLULEN,      // argSize
    SQLSMALLINT,  // decimalDigits
    SQLPOINTER,   // paramValue
    SQLINTEGER,   // strLen_or_Ind
    SQLSMALLINT); // inputOutputType

typedef SQLRETURN FN_execute(
    SQLGUID,        // sessionId
    SQLUSMALLINT,   // taskId
    SQLULEN,        // rowsNumber
    SQLPOINTER *,   // data
    SQLINTEGER **,  // strLenOrInd
    SQLUSMALLINT *);// outputSchemaColumnsNumber

typedef SQLRETURN FN_getResultColumn(
    SQLGUID,        // sessionId
    SQLUSMALLINT,   // taskId
    SQLUSMALLINT,   // columnNumber
    SQLSMALLINT *,  // dataType,
    SQLULEN *,      // columnSize,
    SQLSMALLINT *,  // decimalDigits,
    SQLSMALLINT *); // nullable

typedef SQLRETURN FN_getResults(
    SQLGUID,         // sessionId
    SQLUSMALLINT,    // taskId
    SQLULEN *,       // rowsNumber
    SQLPOINTER **,   // data
    SQLINTEGER ***); // strLen_or_Ind

typedef SQLRETURN FN_getOutputParam(
    SQLGUID,       // sessionId
    SQLUSMALLINT,  // taskId
    SQLUSMALLINT,  // paramNumber
    SQLPOINTER *,  // paramValue
    SQLINTEGER *); // strLen_or_Ind

typedef SQLRETURN FN_cleanupSession(
    SQLGUID,       // sessionId
    SQLUSMALLINT); // taskId

typedef SQLRETURN FN_cleanup();

namespace ExtensionApiTest
{
    // Forward declaration
    //
    template<class SQLType>
    class ColumnInfo;

    class CSharpExtensionApiTests : public ::testing::Test
    {
    protected:

        // Per-test-suite set-up.
        // Called before the first test in this test suite.
        //
        static void SetUpTestCase();

        // Per-test-suite tear-down.
        // Called after the last test in this test suite.
        //
        static void TearDownTestCase();

        // Code here will be called immediately after the constructor (right
        // before each test).
        //
        virtual void SetUp() override;

        // Code here will be called immediately after each test (right
        // before the destructor).
        //
        virtual void TearDown() override;

        // Set up default, valid variables for use in tests.
        //
        virtual void SetupVariables();

        // Load library and get handles to different functions
        // Testing if CSharpExtension is successfully loaded dynamically
        //
        static void GetHandles();

        // Do Init where .NET runtime is initialized - can be called only once in the validate suite.
        // Testing if Init is implemented correctly.
        //
        static void DoInit();

        // Get length of a wstring
        //
        SQLINTEGER GetWStringLength(const wchar_t *str);

        // Cleanup a valid session.
        //
        void SessionCleanup();

        // Set the extension path variables.
        //
        static void SetUpPath();

        // Call Cleanup on the CSharpExtension.
        // Testing if Cleanup is implemented correctly.
        //
        static void DoCleanup();

        // Initialize a valid session.
        //
        void InitializeSession(
            SQLUSMALLINT inputSchemaColumnsNumber = 0,
            SQLUSMALLINT parametersNumber = 0,
            std::string  scriptString = "");

        // Initialize a valid column
        //
        void InitializeColumn(
            SQLSMALLINT columnNumber,
            string      columnNameString,
            SQLSMALLINT dataType,
            SQLULEN     columnSize,
            SQLSMALLINT nullable = SQL_NULLABLE,
            SQLSMALLINT partitionByNumber = -1
        );

        // Template to initialize columns
        //
        template<class SQLType, SQLSMALLINT dataType>
        void InitializeColumns(ColumnInfo<SQLType> *ColumnInfo);

        // Template to init and test all input parameter numeric data types
        //
        template<class SQLType, SQLSMALLINT dataType>
        void InitParam(
            int         paramNumber,
            SQLType     paramValue,
            bool        isNull = false,
            SQLSMALLINT inputOutputType = SQL_PARAM_INPUT_OUTPUT,
            SQLRETURN   SQLResult = SQL_SUCCESS);

        // Init and test a string parameter
        //
        void InitStringParameter(
            int         paramNumber,
            const char  *paramValue,
            SQLULEN     paramSize,
            bool        isFixedType,
            SQLSMALLINT inputOutputType = SQL_PARAM_INPUT_OUTPUT,
            SQLRETURN   SQLResult = SQL_SUCCESS);

        // Init and test a string parameter
        //
        void InitWStringParameter(
            int           paramNumber,
            const wchar_t *paramValue,
            SQLINTEGER    paramSize,
            bool          isFixedType,
            SQLSMALLINT   inputOutputType = SQL_PARAM_INPUT_OUTPUT,
            SQLRETURN     SQLResult = SQL_SUCCESS);

        // Test GetResultColumn to verify the expected result column information.
        //
        void GetResultColumn(
            SQLUSMALLINT columnNumber,
            SQLSMALLINT  expectedDataType,
            SQLULEN      expectedColumnSize,
            SQLSMALLINT  expectedDecimalDigits,
            SQLSMALLINT  expectedNullable,
            SQLRETURN    SQLResult = SQL_SUCCESS);

        // Get max length of all strings from strLenOrInd.
        //
        SQLINTEGER GetMaxLength(SQLINTEGER *strLenOrInd, SQLULEN rowsNumber);

        // Template function to Test Execute with script that contains user executor class full name
        //
        template<class SQLType, SQLSMALLINT dataType>
        void Execute(
            SQLULEN                  rowsNumber,
            void                     **dataSet,
            SQLINTEGER               **strLen_or_Ind,
            std::vector<std::string> columnNames,
            SQLRETURN                SQLResult = SQL_SUCCESS);

        // Fill a contiguous array columnData with members from the given columnVector
        // having lengths defined in strLenOrInd, unless it is SQL_NULL_DATA.
        //
        template<class SQLType>
        std::vector<SQLType> GenerateContiguousData(
            std::vector<const SQLType*> columnVector,
            SQLINTEGER                  *strLenOrInd);

        // Templatized function to test output param value and strLenOrInd are as expected.
        //
        template<class SQLType>
        void GetOutputParam(
            std::vector<SQLType*>   expectedParamValueVector,
            std::vector<SQLINTEGER> expectedStrLenOrIndVector);

        // Test GetResults to verify the expected results are obtained.
        //
        template<class InputSQLType, class OutputSQLType, SQLSMALLINT OutputDataType>
        void GetResults(
            SQLULEN                  expectedRowsNumber,
            SQLPOINTER               *expectedData,
            SQLINTEGER               **expectedStrLen_or_Ind,
            std::vector<std::string> columnNames,
            std::vector<SQLSMALLINT> nullables);

        // Templatized function to compare the given column data
        // & nullMap with rowsNumber for equality.
        //
        template<class InputSQLType, class OutputSQLType, SQLSMALLINT outputDataType>
        void CheckColumnDataEquality(
            SQLULEN        rowsNumber,
            InputSQLType   *expectedColumnData,
            OutputSQLType  *columnData,
            SQLINTEGER     *expectedColumnStrLenOrInd,
            SQLINTEGER     *columnStrLenOrInd,
            SQLSMALLINT    nullable);

        // Test output string param value and strLenOrInd is as expected.
        //
        void GetStringOutputParam(
            std::vector<const char*> expectedParamValueVector,
            std::vector<SQLINTEGER>  expectedStrLenOrIndVector);

        // Test GetResults to verify the expected string results are obtained.
        //
        void GetStringResults(
            SQLULEN                  expectedRowsNumber,
            SQLPOINTER               *expectedData,
            SQLINTEGER               **expectedStrLen_or_Ind,
            std::vector<std::string> columnNames);

        // Compare the given string column data and nullMap for equality.
        //
        void CheckStringDataEquality(
            SQLULEN    rowsNumber,
            char       *expectedColumnData,
            char       *columnData,
            SQLINTEGER *expectedColumnStrLenOrInd,
            SQLINTEGER *columnStrLenOrInd);

        // Objects declared here can be used by all tests in the test suite.
        //
        std::shared_ptr<SQLGUID> m_sessionId;
        SQLUSMALLINT m_taskId;
        SQLUSMALLINT m_numTasks;

        SQLCHAR *m_script = nullptr;
        std::string m_scriptString;
        SQLSMALLINT m_scriptLength;

        SQLCHAR *m_columnName = nullptr;
        std::string m_columnNameString;

        SQLCHAR *m_inputDataName = nullptr;
        std::string m_inputDataNameString;
        SQLSMALLINT m_inputDataNameLength;

        SQLCHAR *m_outputDataName = nullptr;
        std::string m_outputDataNameString;
        SQLSMALLINT m_outputDataNameLength;

        SQLUSMALLINT m_inputSchemaColumnsNumber;
        SQLUSMALLINT m_parametersNumber;

        std::unique_ptr<ColumnInfo<SQLINTEGER>> m_emptyIntegerInfo = nullptr;
        std::unique_ptr<ColumnInfo<SQLINTEGER>> m_integerInfo = nullptr;
        std::unique_ptr<ColumnInfo<SQLCHAR>> m_booleanInfo = nullptr;
        std::unique_ptr<ColumnInfo<SQLREAL>> m_realInfo = nullptr;
        std::unique_ptr<ColumnInfo<SQLDOUBLE>> m_doubleInfo = nullptr;
        std::unique_ptr<ColumnInfo<SQLBIGINT>> m_bigIntInfo = nullptr;
        std::unique_ptr<ColumnInfo<SQLSMALLINT>> m_smallIntInfo = nullptr;
        std::unique_ptr<ColumnInfo<SQLCHAR>> m_tinyIntInfo = nullptr;
        std::unique_ptr<ColumnInfo<SQLCHAR>> m_charInfo = nullptr;

        // User library name and class full name
        // The name of the library is same as the dll file name.
        //
        const std::string m_UserLibName = "Microsoft.SqlServer.CSharpExtensionTest.dll";;
        const std::string m_UserClassFullName = "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutor";
        const std::string m_Separator = ";";

        // Size of each datatype
        //
        const SQLINTEGER m_IntSize = sizeof(SQLINTEGER);
        const SQLINTEGER m_BooleanSize = sizeof(SQLCHAR);
        const SQLINTEGER m_RealSize = sizeof(SQLREAL);
        const SQLINTEGER m_DoubleSize = sizeof(SQLDOUBLE);
        const SQLINTEGER m_BigIntSize = sizeof(SQLBIGINT);
        const SQLINTEGER m_SmallIntSize = sizeof(SQLSMALLINT);
        const SQLINTEGER m_TinyIntSize = sizeof(SQLCHAR);
        const SQLINTEGER m_CharSize = sizeof(SQLCHAR);

        // A value of 2'147'483'648
        //
        const SQLINTEGER m_MaxInt = (std::numeric_limits<SQLINTEGER>::max)();

        // A value of -2'147'483'648
        //
        const SQLINTEGER m_MinInt = (std::numeric_limits<SQLINTEGER>::min)();

        // A value of 9'223'372'036'854'775'807LL
        //
        const SQLBIGINT m_MaxBigInt = (std::numeric_limits<SQLBIGINT>::max)();

        // A value of -9'223'372'036'854'775'808LL
        //
        const SQLBIGINT m_MinBigInt = (std::numeric_limits<SQLBIGINT>::min)();

        // A value of 32'767
        //
        const SQLSMALLINT m_MaxSmallInt = (std::numeric_limits<SQLSMALLINT>::max)();

        // A value of -32'768
        //
        const SQLSMALLINT m_MinSmallInt = (std::numeric_limits<SQLSMALLINT>::min)();

        // A value of 255
        //
        const SQLCHAR m_MaxTinyInt = (std::numeric_limits<SQLCHAR>::max)();

        // A value of 0
        //
        const SQLCHAR m_MinTinyInt = (std::numeric_limits<SQLCHAR>::min)();

        // For floating types, not using numeric_limits because they can't be
        // used for equality comparisons.
        //
        const SQLREAL m_MaxReal = 3.4e38F;
        const SQLREAL m_MinReal = -3.4e38F;
        const SQLDOUBLE m_MaxDouble = 1.79e308;
        const SQLDOUBLE m_MinDouble = -1.79e308;

        // Path of .NET Core CSharp Extension
        //
        static std::string sm_extensionPath;

        // Path of .NET Core CSharp Extension library
        //
        static std::string sm_libPath;

        // Pointer handle to the library nativecsharpextension
        //
        static HINSTANCE sm_libHandle;

        // Pointer to the Init function
        //
        static FN_init *sm_initFuncPtr;

        // Pointer to the InitSession function
        //
        static FN_initSession *sm_initSessionFuncPtr;

        // Pointer to the InitColumn function
        //
        static FN_initColumn *sm_initColumnFuncPtr;

        // Pointer to the InitParam function
        //
        static FN_initParam *sm_initParamFuncPtr;

        // Pointer to the Execute function
        //
        static FN_execute *sm_executeFuncPtr;

        // Pointer to the GetResultColumn function
        //
        static FN_getResultColumn *sm_getResultColumnFuncPtr;

        // Pointer to the GetResults function
        //
        static FN_getResults *sm_getResultsFuncPtr;

        // Pointer to the GetOutputParam function
        //
        static FN_getOutputParam *sm_getOutputParamFuncPtr;

        // Pointer to the CleanupSession function
        //
        static FN_cleanupSession *sm_cleanupSessionFuncPtr;

        // Pointer to the Cleanup function
        //
        static FN_cleanup *sm_cleanupFuncPtr;
    };

    // ColumnInfo template class to store information
    // about integer, basic numeric, logical and date(time) columns.
    // This assumes two columns and five rows.
    //
    template<class SQLType>
    class ColumnInfo
    {
    public:
        ColumnInfo(
            std::string column1Name, std::vector<SQLType> column1,
            std::vector<SQLINTEGER> col1StrLenOrInd,
            std::string column2Name, std::vector<SQLType> column2,
            std::vector<SQLINTEGER> col2StrLenOrInd,
            std::vector<SQLSMALLINT> nullable,
            std::vector<SQLSMALLINT> partitionByIndexes = {-1, -1});

        SQLUSMALLINT GetColumnsNumber() const
        {
            return m_columnNames.size();
        }

        static const SQLULEN sm_rowsNumber = 5;
        std::vector<std::string> m_columnNames;
        std::vector<SQLType> m_column1;
        std::vector<SQLType> m_column2;
        std::vector<void*> m_dataSet;
        std::vector<SQLINTEGER> m_col1StrLenOrInd;
        std::vector<SQLINTEGER> m_col2StrLenOrInd;
        std::vector<SQLINTEGER*> m_strLen_or_Ind;
        std::vector<SQLSMALLINT> m_nullable;
        std::vector<SQLSMALLINT> m_partitionByIndexes;
    };
}
