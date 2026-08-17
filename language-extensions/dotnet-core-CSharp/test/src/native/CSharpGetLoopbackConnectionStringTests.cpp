//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpGetLoopbackConnectionStringTests.cpp
//
// Purpose:
//  Tests the SDK loopback connection string builder.
//
//*********************************************************************
#include "CSharpExtensionApiTests.h"

using namespace std;

namespace ExtensionApiTest
{
    //----------------------------------------------------------------------------------------------
    // Name: GetConnectionStringBuilderOutputParamTest
    //
    // Description:
    //  Verify implied-auth and SQL-auth loopback connection string shapes, required
    //  environment variable failures, password validation, and database name fallbacks.
    //
    TEST_F(CSharpExtensionApiTests, GetConnectionStringBuilderOutputParamTest)
    {
        string userClassFullName =
            "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorConnectionStringBuilder";
        string scriptString = m_UserLibName + m_Separator + userClassFullName;

        InitializeSession(
            0,   // inputSchemaColumnsNumber
            9,   // parametersNumber
            scriptString);

        constexpr SQLULEN connectionStringParameterSize = 512;
        for (SQLUSMALLINT paramNumber = 0; paramNumber < 9; ++paramNumber)
        {
            InitStringParameter(
                paramNumber,
                "",
                connectionStringParameterSize,
                false, // isFixedType
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

        const vector<string> expectedParamValueStrings = {
            "Driver={ODBC Driver 18 for SQL Server};TrustServerCertificate=Yes;"
                "Server=localhost;Database=TestDb;Trusted_Connection=Yes;",
            "Driver={ODBC Driver 18 for SQL Server};Server=np:loopback-pipe;Database=TestDb;"
                "Encrypt=yes;TrustServerCertificate=Yes;ClientCertificate=sha1:0123456789ABCDEF;"
                "Connection Timeout=10;ConnectRetryCount=0;APP=CSharpExtension",
            "XDB satellite is missing required environment variable 'LoopbackConnectionPipe'. "
                "This variable is normally published by launchpad; "
                "its absence indicates a launchpad misconfiguration.",
            "XDB satellite is missing required environment variable 'ExtensibilityCertificateHash'. "
                "This variable is normally published by launchpad; "
                "its absence indicates a launchpad misconfiguration.",
            "Driver={ODBC Driver 18 for SQL Server};TrustServerCertificate=Yes;"
                "Server=localhost;Database=PhysicalDb;Trusted_Connection=Yes;",
            "Driver={ODBC Driver 18 for SQL Server};TrustServerCertificate=Yes;"
                "Server=localhost;Database=master;Trusted_Connection=Yes;",
            "Driver={ODBC Driver 18 for SQL Server};TrustServerCertificate=Yes;"
                "Server=localhost;Database=TestDb;UID=TestUser;PWD=TestPassword;",
            "Driver={ODBC Driver 18 for SQL Server};Server=localhost,1433;Database=TestDb;"
                "UID=TestUser;PWD=TestPassword;",
            "password" };

        vector<const char*> expectedParamValues;
        vector<SQLINTEGER> expectedStrLenOrInd;
        for (const string &expectedValue : expectedParamValueStrings)
        {
            expectedParamValues.push_back(expectedValue.c_str());
            expectedStrLenOrInd.push_back(static_cast<SQLINTEGER>(expectedValue.length()));
        }

        GetStringOutputParam(expectedParamValues, expectedStrLenOrInd);
    }
}