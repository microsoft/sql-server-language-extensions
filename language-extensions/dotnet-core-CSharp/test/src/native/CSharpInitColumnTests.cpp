//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpInitColumnTests.cpp
//
// Purpose:
//  Test the .NET Core CSharp extension columns using the Extension API
//
//*********************************************************************
#include "CSharpExtensionApiTests.h"

using namespace std;

namespace ExtensionApiTest
{
    //----------------------------------------------------------------------------------------------
    // Name: InitColumnTest
    //
    // Description:
    //  Test InitColumn() API with valid values
    //
    TEST_F(CSharpExtensionApiTests, InitColumnTest)
    {
        InitializeSession(1); // inputSchemaColumnsNumber
        SQLCHAR * columnName = static_cast<SQLCHAR *>(
            static_cast<void *>(const_cast<char *>("Column1")));
        SQLSMALLINT columnNameLength = 7;

        SQLRETURN result = (*sm_initColumnFuncPtr)(
            *m_sessionId,     // Session ID
            m_taskId,         // Task ID
            0,                // Column Number
            columnName,       // Column Name
            columnNameLength, // Column Name Length
            SQL_C_SLONG,      // Data Type
            m_IntSize,        // Column Size
            0,                // Decimal Digits
            1,                // Nullable
            -1,               // PartitionByNumber
            -1                // OrderByNumber
        );

        EXPECT_EQ(result, SQL_SUCCESS);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InitInvalidColumnTest
    //
    // Description:
    //  Test InitColumn() API with null column name
    //
    TEST_F(CSharpExtensionApiTests, InitInvalidColumnTest)
    {
        InitializeSession(1); // inputSchemaColumnsNumber

        SQLRETURN result = (*sm_initColumnFuncPtr)(
            *m_sessionId, // Session ID
            m_taskId,     // Task ID
            0,            // Column Number
            nullptr,      // Column Name
            0,            // Column Name Length
            SQL_C_SLONG,  // Data Type
            m_IntSize,    // Column Size
            0,            // Decimal Digits
            1,            // Nullable
            -1,           // PartitionByNumber
            -1            // OrderByNumber
        );

        EXPECT_EQ(result, SQL_ERROR);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InitInvalidColumnNumberTest
    //
    // Description:
    //  Test InitColumn() API with bad column numbers (too big)
    //
    TEST_F(CSharpExtensionApiTests, InitInvalidColumnNumberTest)
    {
        InitializeSession(1); // inputSchemaColumnsNumber
        SQLCHAR * columnName = static_cast<SQLCHAR *>(
            static_cast<void *>(const_cast<char *>("Column1")));
        SQLSMALLINT columnNameLength = 7;

        SQLRETURN result = (*sm_initColumnFuncPtr)(
            *m_sessionId,     // Session ID
            m_taskId,         // Task ID
            2,                // Column number greater than initialized columns
            columnName,       // Column Name
            columnNameLength, // Column Name Length
            SQL_C_SLONG,      // Data Type
            m_IntSize,        // Column Size
            0,                // Decimal Digits
            1,                // Nullable
            -1,               // PartitionByNumber
            -1                // OrderByNumber
        );

        EXPECT_EQ(result, SQL_ERROR);
    }

    //----------------------------------------------------------------------------------------------
    // Name: InitNegativeColumnNumberTest
    //
    // Description:
    //  Test InitColumn() API with bad column numbers (less than 0)
    //
    TEST_F(CSharpExtensionApiTests, InitNegativeColumnNumberTest)
    {
        InitializeSession(1); // inputSchemaColumnsNumber
        SQLCHAR * columnName = static_cast<SQLCHAR *>(
            static_cast<void *>(const_cast<char *>("Column1")));
        SQLSMALLINT columnNameLength = 7;

        SQLRETURN result = (*sm_initColumnFuncPtr)(
            *m_sessionId,     // Session ID
            m_taskId,         // Task ID
            -1,               // Negative Column number
            columnName,       // Column Name
            columnNameLength, // Column Name Length
            SQL_C_SLONG,      // Data Type
            m_IntSize,        // Column Size
            0,                // Decimal Digits
            1,                // Nullable
            -1,               // PartitionByNumber
            -1                // OrderByNumber
        );

        EXPECT_EQ(result, SQL_ERROR);
    }
}
