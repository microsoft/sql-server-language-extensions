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
    namespace
    {
        constexpr const char *DriverClause =
            "Driver={ODBC Driver 18 for SQL Server};";
        constexpr const char *OnPremPrefix =
            "Driver={ODBC Driver 18 for SQL Server};Server=localhost;Database=TestDb;";
        constexpr const char *XdbImpliedPrefix =
            "Driver={ODBC Driver 18 for SQL Server};Server=np:loopback-pipe;Database=TestDb;";
        constexpr const char *XdbCertificateOptions =
            "Encrypt=yes;TrustServerCertificate=Yes;ClientCertificate=sha1:01 23 45 67 89 AB CD EF;"
            "Connection Timeout=10;ConnectRetryCount=0;";

        constexpr const char *ParameterNames[] =
        {
            "@onPremImpliedAuth",
            "@xdbImpliedAuth",
            "@missingXdbPipeError",
            "@missingXdbCertificateError",
            "@physicalDbFallback",
            "@masterDbFallback",
            "@onPremSqlAuth",
            "@xdbSqlAuth",
            "@missingPasswordParamName",
            "@missingUserNameParamName",
            "@emptyUserNameParamName",
            "@escapedDbName",
            "@escapedCredentials",
            "@escapedPhysicalDb",
            "@customApplicationName",
            "@unsupportedWcowError",
            "@invalidXdbMarkerError",
            "@invalidXdbEndpointError",
        };

        constexpr SQLUSMALLINT ParameterCount =
            sizeof(ParameterNames) / sizeof(ParameterNames[0]);
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetConnectionStringBuilderOutputParamTest
    //
    // Description:
    //  Verify implied-auth and SQL-auth loopback connection string shapes, required
    //  environment variable failures, input validation, and database name fallbacks.
    //
    TEST_F(CSharpExtensionApiTests, GetConnectionStringBuilderOutputParamTest)
    {
        string userClassFullName =
            "Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutorConnectionStringBuilder";
        string scriptString = m_UserLibName + m_Separator + userClassFullName;

        InitializeSession(
            0,   // inputSchemaColumnsNumber
            ParameterCount,
            scriptString);

        constexpr SQLULEN connectionStringParameterSize = 512;
        for (SQLUSMALLINT paramNumber = 0; paramNumber < ParameterCount; ++paramNumber)
        {
            InitStringParameter(
                paramNumber,
                "" /*paramValue*/,
                connectionStringParameterSize,
                false /*isFixedType*/,
                SQL_PARAM_INPUT_OUTPUT,
                SQL_SUCCESS,
                ParameterNames[paramNumber]);
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
            string(OnPremPrefix) + "TrustServerCertificate=Yes;Trusted_Connection=Yes;",
            string(XdbImpliedPrefix) + XdbCertificateOptions + "APP=CSharpExtension;",
            "XDB satellite is missing required environment variable 'LoopbackConnectionPipe'. "
                "This variable is normally published by launchpad; "
                "its absence indicates a launchpad misconfiguration.",
            "XDB satellite is missing required environment variable 'ExtensibilityCertificateHash'. "
                "This variable is normally published by launchpad; "
                "its absence indicates a launchpad misconfiguration.",
            string(DriverClause) + "Server=localhost;Database=PhysicalDb;"
                "TrustServerCertificate=Yes;Trusted_Connection=Yes;",
            string(DriverClause) + "Server=localhost;Database=master;"
                "TrustServerCertificate=Yes;Trusted_Connection=Yes;",
            string(OnPremPrefix) + "TrustServerCertificate=Yes;"
                "UID=TestUser;PWD=TestPassword;",
            string(DriverClause) + "Server=localhost,1433;Database=TestDb;"
                "UID=TestUser;PWD=TestPassword;",
            "password",
            "userName",
            "userName",
            string(DriverClause) + "Server=localhost;Database=\"TestDb;Encrypt=no\";"
                "TrustServerCertificate=Yes;Trusted_Connection=Yes;",
            string(OnPremPrefix) + "TrustServerCertificate=Yes;"
                "UID=\"TestUser=Injected\";PWD=\"{Test;Password}\";",
            string(DriverClause) + "Server=localhost;Database=\"PhysicalDb;UID=Injected\";"
                "TrustServerCertificate=Yes;Trusted_Connection=Yes;",
            string(XdbImpliedPrefix) + XdbCertificateOptions
                + "APP=\"OtherExtension;UID=Injected\";",
            "SQL Managed Instance (WCOW) is not supported by ODBC loopback connections.",
            "Environment variable 'IS_XDB' must be either absent or set to 'TRUE'.",
            "Environment variable 'SqlTdsLoopbackConnectionEndpoint' contains a character "
                "that is not allowed in an ODBC connection string.",
        };

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
