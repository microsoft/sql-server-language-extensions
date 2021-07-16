//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpInitTests.cpp
//
// Purpose:
//  Test the .NET Core CSharp extension initialization and
//  session initialization using the Extension API
//
//*************************************************************************************************
#include "CSharpExtensionApiTests.h"

using namespace std;

namespace ExtensionApiTest
{
    // Name: InitSessionTest
    //
    // Description:
    //  Test Init() API and InitSession() API with valid values.
    //
    TEST_F(CSharpExtensionApiTests, InitSessionTest)
    {
        SQLCHAR *extensionParams = nullptr;
        SQLULEN extensionParamsLength = 0;
        string publicLibraryPath = sm_extensionPath;
        SQLULEN publicLibraryPathLength = publicLibraryPath.length();
        SQLCHAR *privateLibraryPath = nullptr;
        SQLULEN privateLibraryPathLength = 0;

        SQLCHAR *unsignedExtensionPath = static_cast<SQLCHAR *>(
            static_cast<void *>(const_cast<char *>(sm_extensionPath.c_str())));
        SQLCHAR *unsignedPublicLibraryPath = static_cast<SQLCHAR *>(
            static_cast<void *>(const_cast<char *>(publicLibraryPath.c_str())));

        SQLRETURN result = (*sm_initFuncPtr)(
            extensionParams,
            extensionParamsLength,
            unsignedExtensionPath,
            m_extensionPathLength,
            unsignedPublicLibraryPath,
            publicLibraryPathLength,
            privateLibraryPath,
            privateLibraryPathLength
        );

        EXPECT_EQ(result, SQL_SUCCESS);

        result = (*sm_initSessionFuncPtr)(
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
