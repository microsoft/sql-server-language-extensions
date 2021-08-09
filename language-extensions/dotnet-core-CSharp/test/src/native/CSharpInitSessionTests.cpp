//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpInitSessionTests.cpp
//
// Purpose:
//  Test the .NET Core CSharp extension initialization and
//  session initialization using the Extension API
//
//*********************************************************************
#include "CSharpExtensionApiTests.h"

using namespace std;

namespace ExtensionApiTest
{
    //----------------------------------------------------------------------------------------------
    // Name: InitValidSessionTest
    //
    // Description:
    //  Test InitSession() API with valid values.
    //
    TEST_F(CSharpExtensionApiTests, InitValidSessionTest)
    {
        SQLRETURN result = (*sm_initSessionFuncPtr)(
            *m_sessionId,
            m_taskId,
            m_numTasks,
            m_script,
            m_scriptString.length(),
            m_inputSchemaColumnsNumber,
            m_parametersNumber,
            m_inputDataName,
            m_inputDataNameString.length(),
            m_outputDataName,
            m_outputDataNameString.length());

        EXPECT_EQ(result, SQL_SUCCESS);
    }
}
