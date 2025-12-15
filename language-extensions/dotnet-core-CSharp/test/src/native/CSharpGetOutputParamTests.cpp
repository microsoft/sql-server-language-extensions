//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpGetOutputParamTests.cpp
//
// Purpose:
//  Tests the CSharp Extension's implementation of the external language GetOutputParam API.
//
//*********************************************************************
#include "CSharpExtensionApiTests.h"

using namespace std;

namespace ExtensionApiTest
{
    //----------------------------------------------------------------------------------------------
    // Name: GetIntegerOutputParamTest
    //
    // Description:
    // Test multiple Int values
    //
    TEST_F(CSharpExtensionApiTests, GetIntegerOutputParamTest)
    {
        int paramsNumber = 5;
        SQLINTEGER p0 = m_MaxInt;
        SQLINTEGER p1 = m_MinInt;
        SQLINTEGER p2 = 4;
        SQLINTEGER p3 = 0;

        string userClassFullName = "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorIntParam";
        string scriptString = m_UserLibName + m_Separator + userClassFullName;

        InitializeSession(
            0, 		       // inputSchemaColumnsNumber
            paramsNumber,  // parametersNumber
            scriptString); // scriptString

        for(int i = 0; i < paramsNumber; ++i)
        {
            InitParam<SQLINTEGER, SQL_C_SLONG>(
                i,                       // paramNumber
                0,                       // paramValue
                false,                   // isNull
                SQL_PARAM_INPUT_OUTPUT); // inputOutputType
        }

        SQLUSMALLINT outputSchemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            0,       // rowsNumber
            nullptr, // dataSet
            nullptr, // strLen_or_Ind
            &outputSchemaColumnsNumber);
        ASSERT_EQ(result, SQL_SUCCESS);

        EXPECT_EQ(outputSchemaColumnsNumber, 0);

        vector<SQLINTEGER*> paramValues = { &p0, &p1, &p2, &p3, nullptr};
        vector<SQLINTEGER> strLenOrIndValues = {
            m_IntSize,
            m_IntSize,
            m_IntSize,
            m_IntSize,
            SQL_NULL_DATA};

        // Verify that the parameters we get back are what we expect
        //
        GetOutputParam<SQLINTEGER>(
            paramValues,
            strLenOrIndValues);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetBitOutputParamTest
    //
    // Description:
    // Test multiple Bit values
    //
    TEST_F(CSharpExtensionApiTests, GetBitOutputParamTest)
    {
        int paramsNumber = 5;
        SQLCHAR p0 = '\x1';
        SQLCHAR p1 = '\0';
        SQLCHAR p2 = '\x1';
        SQLCHAR p3 = '\0';

        string userClassFullName = "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorBitParam";
        string scriptString = m_UserLibName + m_Separator + userClassFullName;

        InitializeSession(
            0, 		       // inputSchemaColumnsNumber
            paramsNumber,  // parametersNumber
            scriptString); // scriptString

        for(int i = 0; i < paramsNumber; ++i)
        {
            InitParam<SQLCHAR, SQL_C_BIT>(
                i,                       // paramNumber
                0,                       // paramValue
                false,                   // isNull
                SQL_PARAM_INPUT_OUTPUT); // inputOutputType
        }

        SQLUSMALLINT outputSchemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            0,       // rowsNumber
            nullptr, // dataSet
            nullptr, // strLen_or_Ind
            &outputSchemaColumnsNumber);
        ASSERT_EQ(result, SQL_SUCCESS);

        EXPECT_EQ(outputSchemaColumnsNumber, 0);

        vector<SQLCHAR*> paramValues = { &p0, &p1, &p2, &p3, nullptr};
        vector<SQLINTEGER> strLenOrIndValues = {
            m_BooleanSize,
            m_BooleanSize,
            m_BooleanSize,
            m_BooleanSize,
            SQL_NULL_DATA};

        // Verify that the parameters we get back are what we expect
        //
        GetOutputParam<SQLCHAR>(
            paramValues,
            strLenOrIndValues);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetRealOutputParamTest
    //
    // Description:
    // Test multiple Real values
    //
    TEST_F(CSharpExtensionApiTests, GetRealOutputParamTest)
    {
        int paramsNumber = 5;
        SQLREAL p0 = m_MaxReal;
        SQLREAL p1 = m_MinReal;
        SQLREAL p2 = 2.3e4;
        SQLREAL p3 = 0;

        string userClassFullName = "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorRealParam";
        string scriptString = m_UserLibName + m_Separator + userClassFullName;

        InitializeSession(
            0, 		       // inputSchemaColumnsNumber
            paramsNumber,  // parametersNumber
            scriptString); // scriptString

        for(int i = 0; i < paramsNumber; ++i)
        {
            InitParam<SQLREAL, SQL_C_FLOAT>(
                i,                       // paramNumber
                0,                       // paramValue
                false,                   // isNull
                SQL_PARAM_INPUT_OUTPUT); // inputOutputType
        }

        SQLUSMALLINT outputSchemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            0,       // rowsNumber
            nullptr, // dataSet
            nullptr, // strLen_or_Ind
            &outputSchemaColumnsNumber);
        ASSERT_EQ(result, SQL_SUCCESS);

        EXPECT_EQ(outputSchemaColumnsNumber, 0);

        vector<SQLREAL*> paramValues = { &p0, &p1, &p2, &p3, nullptr};
        vector<SQLINTEGER> strLenOrIndValues = {
            m_RealSize,
            m_RealSize,
            m_RealSize,
            m_RealSize,
            SQL_NULL_DATA};

        // Verify that the parameters we get back are what we expect
        //
        GetOutputParam<SQLREAL>(
            paramValues,
            strLenOrIndValues);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetDoubleOutputParamTest
    //
    // Description:
    // Test multiple Double values
    //
    TEST_F(CSharpExtensionApiTests, GetDoubleOutputParamTest)
    {
        int paramsNumber = 5;
        SQLDOUBLE p0 = m_MaxDouble;
        SQLDOUBLE p1 = m_MinDouble;
        SQLDOUBLE p2 = 1.45e38;
        SQLDOUBLE p3 = 0;

        string userClassFullName = "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorDoubleParam";
        string scriptString = m_UserLibName + m_Separator + userClassFullName;

        InitializeSession(
            0, 		       // inputSchemaColumnsNumber
            paramsNumber,  // parametersNumber
            scriptString); // scriptString

        for(int i = 0; i < paramsNumber; ++i)
        {
            InitParam<SQLDOUBLE, SQL_C_DOUBLE>(
                i,                       // paramNumber
                0,                       // paramValue
                false,                   // isNull
                SQL_PARAM_INPUT_OUTPUT); // inputOutputType
        }

        SQLUSMALLINT outputSchemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            0,       // rowsNumber
            nullptr, // dataSet
            nullptr, // strLen_or_Ind
            &outputSchemaColumnsNumber);
        ASSERT_EQ(result, SQL_SUCCESS);

        EXPECT_EQ(outputSchemaColumnsNumber, 0);

        vector<SQLDOUBLE*> paramValues = { &p0, &p1, &p2, &p3, nullptr};
        vector<SQLINTEGER> strLenOrIndValues = {
            m_DoubleSize,
            m_DoubleSize,
            m_DoubleSize,
            m_DoubleSize,
            SQL_NULL_DATA};

        // Verify that the parameters we get back are what we expect
        //
        GetOutputParam<SQLDOUBLE>(
            paramValues,
            strLenOrIndValues);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetBigIntOutputParamTest
    //
    // Description:
    // Test multiple Big Int values
    //
    TEST_F(CSharpExtensionApiTests, GetBigIntOutputParamTest)
    {
        int paramsNumber = 5;
        SQLBIGINT p0 = m_MaxBigInt;
        SQLBIGINT p1 = m_MinBigInt;
        SQLBIGINT p2 = 9372036854775;
        SQLBIGINT p3 = 0;

        string userClassFullName = "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorBigIntParam";
        string scriptString = m_UserLibName + m_Separator + userClassFullName;

        InitializeSession(
            0, 		       // inputSchemaColumnsNumber
            paramsNumber,  // parametersNumber
            scriptString); // scriptString

        for(int i = 0; i < paramsNumber; ++i)
        {
            InitParam<SQLBIGINT, SQL_C_SBIGINT>(
                i,                       // paramNumber
                0,                       // paramValue
                false,                   // isNull
                SQL_PARAM_INPUT_OUTPUT); // inputOutputType
        }

        SQLUSMALLINT outputSchemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            0,       // rowsNumber
            nullptr, // dataSet
            nullptr, // strLen_or_Ind
            &outputSchemaColumnsNumber);
        ASSERT_EQ(result, SQL_SUCCESS);

        EXPECT_EQ(outputSchemaColumnsNumber, 0);

        vector<SQLBIGINT*> paramValues = { &p0, &p1, &p2, &p3, nullptr};
        vector<SQLINTEGER> strLenOrIndValues = {
            m_BigIntSize,
            m_BigIntSize,
            m_BigIntSize,
            m_BigIntSize,
            SQL_NULL_DATA};

        // Verify that the parameters we get back are what we expect
        //
        GetOutputParam<SQLBIGINT>(
            paramValues,
            strLenOrIndValues);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetSmallIntOutputParamTest
    //
    // Description:
    // Test multiple Small Int values
    //
    TEST_F(CSharpExtensionApiTests, GetSmallIntOutputParamTest)
    {
        int paramsNumber = 5;
        SQLSMALLINT p0 = m_MaxSmallInt;
        SQLSMALLINT p1 = m_MinSmallInt;
        SQLSMALLINT p2 = 3007;
        SQLSMALLINT p3 = 0;

        string userClassFullName = "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorSmallIntParam";
        string scriptString = m_UserLibName + m_Separator + userClassFullName;

        InitializeSession(
            0, 		       // inputSchemaColumnsNumber
            paramsNumber,  // parametersNumber
            scriptString); // scriptString

        for(int i = 0; i < paramsNumber; ++i)
        {
            InitParam<SQLSMALLINT, SQL_C_SSHORT>(
                i,                       // paramNumber
                0,                       // paramValue
                false,                   // isNull
                SQL_PARAM_INPUT_OUTPUT); // inputOutputType
        }

        SQLUSMALLINT outputSchemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            0,       // rowsNumber
            nullptr, // dataSet
            nullptr, // strLen_or_Ind
            &outputSchemaColumnsNumber);
        ASSERT_EQ(result, SQL_SUCCESS);

        EXPECT_EQ(outputSchemaColumnsNumber, 0);

        vector<SQLSMALLINT*> paramValues = { &p0, &p1, &p2, &p3, nullptr};
        vector<SQLINTEGER> strLenOrIndValues = {
            m_SmallIntSize,
            m_SmallIntSize,
            m_SmallIntSize,
            m_SmallIntSize,
            SQL_NULL_DATA};

        // Verify that the parameters we get back are what we expect
        //
        GetOutputParam<SQLSMALLINT>(
            paramValues,
            strLenOrIndValues);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetTinyIntOutputParamTest
    //
    // Description:
    // Test multiple Tiny Int values
    //
    TEST_F(CSharpExtensionApiTests, GetTinyIntOutputParamTest)
    {
        int paramsNumber = 5;
        SQLCHAR p0 = m_MaxTinyInt;
        SQLCHAR p1 = m_MinTinyInt;
        SQLCHAR p2 = 123;
        SQLCHAR p3 = -1;

        string userClassFullName = "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorTinyIntParam";
        string scriptString = m_UserLibName + m_Separator + userClassFullName;

        InitializeSession(
            0, 		       // inputSchemaColumnsNumber
            paramsNumber,  // parametersNumber
            scriptString); // scriptString

        for(int i = 0; i < paramsNumber; ++i)
        {
            InitParam<SQLCHAR, SQL_C_UTINYINT>(
                i,                       // paramNumber
                0,                       // paramValue
                false,                   // isNull
                SQL_PARAM_INPUT_OUTPUT); // inputOutputType
        }

        SQLUSMALLINT outputSchemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            0,       // rowsNumber
            nullptr, // dataSet
            nullptr, // strLen_or_Ind
            &outputSchemaColumnsNumber);
        ASSERT_EQ(result, SQL_SUCCESS);

        EXPECT_EQ(outputSchemaColumnsNumber, 0);

        vector<SQLCHAR*> paramValues = { &p0, &p1, &p2, &p3, nullptr};
        vector<SQLINTEGER> strLenOrIndValues = {
            m_TinyIntSize,
            m_TinyIntSize,
            m_TinyIntSize,
            m_TinyIntSize,
            SQL_NULL_DATA};

        // Verify that the parameters we get back are what we expect
        //
        GetOutputParam<SQLCHAR>(
            paramValues,
            strLenOrIndValues);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetStringOutputParamTest
    //
    // Description:
    // Test multiple string values
    //
    TEST_F(CSharpExtensionApiTests, GetStringOutputParamTest)
    {
        string userClassFullName = "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorStringParam";
        string scriptString = m_UserLibName + m_Separator + userClassFullName;

        // Initialize with a Session that executes the above script
        // that sets output parameters.
        //
        InitializeSession(
            0,   // parametersNumber
            6,   // inputSchemaColumnsNumber
            scriptString);

        // Note: The behavior of fixed and varying character types is same when it comes to output
        // parameters. So it doesn't matter if we initialize these output parameters as fixed type.
        //
        vector<bool> isFixedType = { true, false, true, false, true, true };
        vector<SQLULEN> paramSizes = { 5, 6, 10, 5, 5, 5 };

        for(size_t paramNumber=0; paramNumber < paramSizes.size(); ++paramNumber)
        {
            InitStringParameter(
                paramNumber,
                "",             // paramValue
                paramSizes[paramNumber],
                isFixedType[paramNumber],
                SQL_PARAM_INPUT_OUTPUT);
        }

        SQLUSMALLINT outputSchemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            0,       // rowsNumber
            nullptr, // dataSet
            nullptr, // strLen_or_Ind
            &outputSchemaColumnsNumber);
        ASSERT_EQ(result, SQL_SUCCESS);

        EXPECT_EQ(outputSchemaColumnsNumber, 0);

        const vector<string> ExpectedParamValueStrings = {
            // Test simple CHAR(5) value with exact string length as the type allows i.e. here 5.
            //
            "HELLO",
            // Test VARCHAR(6) value with string length more than the type allows - expected truncation.
            //
            "C#Exte",
            // Test a 0 length string
            //
            "" ,
            // Test CHAR(10) value with string length less than the type allows.
            //
            "WORLD"};

        vector<const char*> expectedParamValues = {
            ExpectedParamValueStrings[0].c_str(),
            ExpectedParamValueStrings[1].c_str(),
            ExpectedParamValueStrings[2].c_str(),
            ExpectedParamValueStrings[3].c_str(),

            // Test None returned in a VARCHAR(5) parameter.
            //
            nullptr,

            // Test None CHAR(5) value.
            //
            nullptr };

        vector<SQLINTEGER> expectedStrLenOrInd = {
            static_cast<SQLINTEGER>(ExpectedParamValueStrings[0].length()),
            static_cast<SQLINTEGER>(ExpectedParamValueStrings[1].length()),
            static_cast<SQLINTEGER>(ExpectedParamValueStrings[2].length()),
            static_cast<SQLINTEGER>(ExpectedParamValueStrings[3].length()),
            SQL_NULL_DATA,
            SQL_NULL_DATA };

        GetStringOutputParam(
            expectedParamValues,
            expectedStrLenOrInd);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetWStringOutputParamTest
    //
    // Description:
    // Test multiple Unicode string values
    //
    TEST_F(CSharpExtensionApiTests, GetWStringOutputParamTest)
    {
        string userClassFullName = "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorWStringParam";
        string scriptString = m_UserLibName + m_Separator + userClassFullName;

        // Initialize with a Session that executes the above script
        // that sets output parameters.
        //
        InitializeSession(
            0,   // parametersNumber
            6,   // inputSchemaColumnsNumber
            scriptString);

        // Note: The behavior of fixed and varying character types is same when it comes to output
        // parameters. So it doesn't matter if we initialize these output parameters as fixed type.
        //
        vector<bool> isFixedType = { true, false, true, false, true, true };
        vector<SQLINTEGER> paramSizes = { 5, 6, 10, 5, 5, 5 };

        for(size_t paramNumber=0; paramNumber < paramSizes.size(); ++paramNumber)
        {
            InitWStringParameter(
                paramNumber,
                L"",            // paramValue
                paramSizes[paramNumber],
                isFixedType[paramNumber],
                SQL_PARAM_INPUT_OUTPUT);
        }

        SQLUSMALLINT outputSchemaColumnsNumber = 0;
        SQLRETURN result = (*sm_executeFuncPtr)(
            *m_sessionId,
            m_taskId,
            0,       // rowsNumber
            nullptr, // dataSet
            nullptr, // strLen_or_Ind
            &outputSchemaColumnsNumber);
        ASSERT_EQ(result, SQL_SUCCESS);

        EXPECT_EQ(outputSchemaColumnsNumber, 0);

        const vector<wstring> ExpectedParamValueStrings = {
            // Test simple NCHAR(5) value with exact string length as the type allows i.e. here 5.
            //
            L"HELLO",
            // Test NVARCHAR(6) value with string length more than the type allows - expected truncation.
            //
            L"C#Exte",
            // Test a 0 length string
            //
            L"" ,
            // Test NCHAR(10) value with string length less than the type allows.
            //
            L"WORLD"};

        vector<const wchar_t*> expectedParamValues = {
            ExpectedParamValueStrings[0].c_str(),
            ExpectedParamValueStrings[1].c_str(),
            ExpectedParamValueStrings[2].c_str(),
            ExpectedParamValueStrings[3].c_str(),

            // Test None returned in a NVARCHAR(5) parameter.
            //
            nullptr,

            // Test None NCHAR(5) value.
            //
            nullptr };

        // strLenOrInd is in bytes for NCHAR/NVARCHAR, so multiply by sizeof(wchar_t)
        //
        vector<SQLINTEGER> expectedStrLenOrInd = {
            static_cast<SQLINTEGER>(ExpectedParamValueStrings[0].length() * sizeof(wchar_t)),
            static_cast<SQLINTEGER>(ExpectedParamValueStrings[1].length() * sizeof(wchar_t)),
            static_cast<SQLINTEGER>(ExpectedParamValueStrings[2].length() * sizeof(wchar_t)),
            static_cast<SQLINTEGER>(ExpectedParamValueStrings[3].length() * sizeof(wchar_t)),
            SQL_NULL_DATA,
            SQL_NULL_DATA };

        GetWStringOutputParam(
            expectedParamValues,
            expectedStrLenOrInd);
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::GetOutputParam
    //
    // Description:
    //  Templatized function to test output param value and strLenOrInd is as expected.
    //
    template<class SQLType>
    void CSharpExtensionApiTests::GetOutputParam(
        vector<SQLType*>   expectedParamValueVector,
        vector<SQLINTEGER> expectedStrLenOrIndVector)
    {
        ASSERT_EQ(expectedParamValueVector.size(), expectedStrLenOrIndVector.size());

        for(size_t i = 0; i < expectedParamValueVector.size(); ++i)
        {
            SQLType *expectedParamValue = expectedParamValueVector[i];
            SQLINTEGER expectedStrLenOrInd = expectedStrLenOrIndVector[i];

            SQLPOINTER paramValue = nullptr;
            SQLINTEGER strLen_or_Ind = 0;

            SQLRETURN result = SQL_ERROR;
            result = (*sm_getOutputParamFuncPtr)(
                    *m_sessionId,
                    m_taskId,
                    i,  // paramNumber
                    &paramValue,
                    &strLen_or_Ind);
            ASSERT_EQ(result, SQL_SUCCESS);

            EXPECT_EQ(strLen_or_Ind, expectedStrLenOrInd);

            if (expectedParamValue != nullptr)
            {
                EXPECT_NE(paramValue, nullptr);

                EXPECT_EQ(*(static_cast<SQLType*>(paramValue)), *expectedParamValue);
            }
            else
            {
                EXPECT_EQ(paramValue, nullptr);
            }
        }
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::GetStringOutputParam
    //
    // Description:
    // Get and test string output param value and strLenOrInd is as expected.
    //
    void CSharpExtensionApiTests::GetStringOutputParam(
        vector<const char*> expectedParamValues,
        vector<SQLINTEGER>  expectedStrLenOrInd)
    {
        ASSERT_EQ(expectedParamValues.size(), expectedStrLenOrInd.size());

        for (size_t paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
        {
            SQLPOINTER paramValue = nullptr;
            SQLINTEGER strLen_or_Ind = 0;
            SQLRETURN result = SQL_ERROR;

            result = (*sm_getOutputParamFuncPtr)(
                *m_sessionId,
                m_taskId,
                paramNumber,
                &paramValue,
                &strLen_or_Ind);
            ASSERT_EQ(result, SQL_SUCCESS);

            EXPECT_EQ(strLen_or_Ind, expectedStrLenOrInd[paramNumber]);

            if (expectedParamValues[paramNumber] != nullptr)
            {
                EXPECT_NE(paramValue, nullptr);

                string paramValueString(static_cast<char*>(paramValue),
                    strLen_or_Ind);
                string expectedParamValueString(expectedParamValues[paramNumber],
                    expectedStrLenOrInd[paramNumber]);

                EXPECT_EQ(paramValueString, expectedParamValueString);
            }
            else
            {
                EXPECT_EQ(paramValue, nullptr);
            }
        }
    }

    //----------------------------------------------------------------------------------------------
    // Name: CSharpExtensionApiTest::GetWStringOutputParam
    //
    // Description:
    // Get and test Unicode string output param value and strLenOrInd is as expected.
    //
    void CSharpExtensionApiTests::GetWStringOutputParam(
        vector<const wchar_t*> expectedParamValues,
        vector<SQLINTEGER>     expectedStrLenOrInd)
    {
        ASSERT_EQ(expectedParamValues.size(), expectedStrLenOrInd.size());

        for (size_t paramNumber = 0; paramNumber < expectedParamValues.size(); ++paramNumber)
        {
            SQLPOINTER paramValue = nullptr;
            SQLINTEGER strLen_or_Ind = 0;
            SQLRETURN result = SQL_ERROR;

            result = (*sm_getOutputParamFuncPtr)(
                *m_sessionId,
                m_taskId,
                paramNumber,
                &paramValue,
                &strLen_or_Ind);
            ASSERT_EQ(result, SQL_SUCCESS);

            EXPECT_EQ(strLen_or_Ind, expectedStrLenOrInd[paramNumber]);

            if (expectedParamValues[paramNumber] != nullptr)
            {
                EXPECT_NE(paramValue, nullptr);

                // strLen_or_Ind is in bytes, divide by sizeof(wchar_t) to get character count
                //
                SQLINTEGER charCount = strLen_or_Ind / sizeof(wchar_t);
                wstring paramValueString(static_cast<wchar_t*>(paramValue), charCount);

                SQLINTEGER expectedCharCount = expectedStrLenOrInd[paramNumber] / sizeof(wchar_t);
                wstring expectedParamValueString(expectedParamValues[paramNumber], expectedCharCount);

                EXPECT_EQ(paramValueString, expectedParamValueString);
            }
            else
            {
                EXPECT_EQ(paramValue, nullptr);
            }
        }
    }
}
