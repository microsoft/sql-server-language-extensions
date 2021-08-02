//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpGetResultColumnTests.cpp
//
// Purpose:
//  Tests the CSharp Extension's implementation of the external language GetResultColumn API.
//
//*********************************************************************
#include "CSharpExtensionApiTests.h"

using namespace std;

namespace ExtensionApiTest
{
    //----------------------------------------------------------------------------------------------
    // Name: GetIntegerResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with default script expecting an OutputDataSet of Integer columns.
    //
    TEST_F(CSharpExtensionApiTests, GetIntegerResultColumnsTest)
    {
        InitializeSession(
            (*m_integerInfo).GetColumnsNumber(),
            0, // parametersNumber
            m_scriptString);

        InitializeColumns<SQLINTEGER, SQL_C_SLONG>(m_integerInfo.get());

        Execute<SQLINTEGER, SQL_C_SLONG>(
            ColumnInfo<SQLINTEGER>::sm_rowsNumber,
            (*m_integerInfo).m_dataSet.data(),
            (*m_integerInfo).m_strLen_or_Ind.data(),
            (*m_integerInfo).m_columnNames);

        GetResultColumn(
            0,                  // columnNumber
            SQL_C_SLONG,        // dataType
            m_IntSize,          // columnSize
            0,                  // decimalDigits
            SQL_NO_NULLS);      // nullable

        GetResultColumn(
            1,                  // columnNumber
            SQL_C_SLONG,        // dataType
            m_IntSize,          // columnSize
            0,                  // decimalDigits
            SQL_NULLABLE);      // nullable

        GetResultColumn(
            2,                  // columnNumber
            SQL_C_SLONG,        // dataType
            m_IntSize,          // columnSize
            0,                  // decimalDigits
            SQL_NULLABLE,       // nullable
            SQL_ERROR);         // Invalid Column Number

        GetResultColumn(
            -1,                 // columnNumber
            SQL_C_SLONG,        // dataType
            m_IntSize,          // columnSize
            0,                  // decimalDigits
            SQL_NULLABLE,       // nullable
            SQL_ERROR);         // Negative Column Number
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetBooleanResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with default script expecting an OutputDataSet of Boolean columns.
    //
    TEST_F(CSharpExtensionApiTests, GetBooleanResultColumnsTest)
    {
        InitializeSession(
            (*m_booleanInfo).GetColumnsNumber(),
            0, // parametersNumber
            m_scriptString);

        InitializeColumns<SQLCHAR, SQL_C_BIT>(m_booleanInfo.get());

        Execute<SQLCHAR, SQL_C_BIT>(
            ColumnInfo<SQLCHAR>::sm_rowsNumber,
            (*m_booleanInfo).m_dataSet.data(),
            (*m_booleanInfo).m_strLen_or_Ind.data(),
            (*m_booleanInfo).m_columnNames);

        GetResultColumn(
            0,             // columnNumber
            SQL_C_BIT,     // dataType
            m_BooleanSize, // columnSize
            0,             // decimalDigits
            SQL_NO_NULLS); // nullable

        GetResultColumn(
            1,             // columnNumber
            SQL_C_BIT,     // dataType
            m_BooleanSize, // columnSize
            0,             // decimalDigits
            SQL_NULLABLE); // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetBigIntResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with default script expecting an OutputDataSet of BigInt columns.
    //
    TEST_F(CSharpExtensionApiTests, GetBigIntResultColumnsTest)
    {
        InitializeSession(
            (*m_bigIntInfo).GetColumnsNumber(),
            0, // parametersNumber
            m_scriptString);

        InitializeColumns<SQLBIGINT, SQL_C_SBIGINT>(m_bigIntInfo.get());

        Execute<SQLBIGINT, SQL_C_SBIGINT>(
            ColumnInfo<SQLBIGINT>::sm_rowsNumber,
            (*m_bigIntInfo).m_dataSet.data(),
            (*m_bigIntInfo).m_strLen_or_Ind.data(),
            (*m_bigIntInfo).m_columnNames);

        GetResultColumn(
            0,             // columnNumber
            SQL_C_SBIGINT, // dataType
            m_BigIntSize,  // columnSize
            0,             // decimalDigits
            SQL_NO_NULLS); // nullable

        GetResultColumn(
            1,             // columnNumber
            SQL_C_SBIGINT, // dataType
            m_BigIntSize,  // columnSize
            0,             // decimalDigits
            SQL_NULLABLE); // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetSmallIntResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with default script expecting an OutputDataSet of SmallInt columns.
    //
    TEST_F(CSharpExtensionApiTests, GetSmallIntResultColumnsTest)
    {
        InitializeSession(
            (*m_smallIntInfo).GetColumnsNumber(),
            0, // parametersNumber
            m_scriptString);

        InitializeColumns<SQLSMALLINT, SQL_C_SSHORT>(m_smallIntInfo.get());

        Execute<SQLSMALLINT, SQL_C_SSHORT>(
            ColumnInfo<SQLSMALLINT>::sm_rowsNumber,
            (*m_smallIntInfo).m_dataSet.data(),
            (*m_smallIntInfo).m_strLen_or_Ind.data(),
            (*m_smallIntInfo).m_columnNames);

        GetResultColumn(
            0,              // columnNumber
            SQL_C_SSHORT,   // dataType
            m_SmallIntSize, // columnSize
            0,              // decimalDigits
            SQL_NO_NULLS);  // nullable

        GetResultColumn(
            1,              // columnNumber
            SQL_C_SSHORT,   // dataType
            m_SmallIntSize, // columnSize
            0,              // decimalDigits
            SQL_NULLABLE);  // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetTinyIntResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with default script expecting an OutputDataSet of TinyInt columns.
    //
    TEST_F(CSharpExtensionApiTests, GetTinyIntResultColumnsTest)
    {
        InitializeSession(
            (*m_tinyIntInfo).GetColumnsNumber(),
            0, // parametersNumber
            m_scriptString);

        InitializeColumns<SQLCHAR, SQL_C_UTINYINT>(m_tinyIntInfo.get());

        Execute<SQLCHAR, SQL_C_UTINYINT>(
            ColumnInfo<SQLCHAR>::sm_rowsNumber,
            (*m_tinyIntInfo).m_dataSet.data(),
            (*m_tinyIntInfo).m_strLen_or_Ind.data(),
            (*m_tinyIntInfo).m_columnNames);

        GetResultColumn(
            0,              // columnNumber
            SQL_C_UTINYINT, // dataType
            m_TinyIntSize,  // columnSize
            0,              // decimalDigits
            SQL_NO_NULLS);  // nullable

        GetResultColumn(
            1,              // columnNumber
            SQL_C_UTINYINT, // dataType
            m_TinyIntSize,  // columnSize
            0,              // decimalDigits
            SQL_NULLABLE);  // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetRealResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with default script expecting an OutputDataSet of Real columns.
    //
    TEST_F(CSharpExtensionApiTests, GetRealResultColumnsTest)
    {
        InitializeSession(
            (*m_realInfo).GetColumnsNumber(),
            0, // parametersNumber
            m_scriptString);

        InitializeColumns<SQLREAL, SQL_C_FLOAT>(m_realInfo.get());

        Execute<SQLREAL, SQL_C_FLOAT>(
            ColumnInfo<SQLREAL>::sm_rowsNumber,
            (*m_realInfo).m_dataSet.data(),
            (*m_realInfo).m_strLen_or_Ind.data(),
            (*m_realInfo).m_columnNames);

        GetResultColumn(
            0,             // columnNumber
            SQL_C_FLOAT,   // dataType
            m_RealSize,    // columnSize
            0,             // decimalDigits
            SQL_NO_NULLS); // nullable

        GetResultColumn(
            1,             // columnNumber
            SQL_C_FLOAT,   // dataType
            m_RealSize,    // columnSize
            0,             // decimalDigits
            SQL_NULLABLE); // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetDoubleResultColumnsTest
    //
    // Description:
    //  Test GetResultColumn with default script expecting an OutputDataSet of Double columns.
    //
    TEST_F(CSharpExtensionApiTests, GetDoubleResultColumnsTest)
    {
        InitializeSession(
            (*m_doubleInfo).GetColumnsNumber(),
            0, // parametersNumber
            m_scriptString);

        InitializeColumns<SQLDOUBLE, SQL_C_DOUBLE>(m_doubleInfo.get());

        Execute<SQLDOUBLE, SQL_C_DOUBLE>(
            ColumnInfo<SQLDOUBLE>::sm_rowsNumber,
            (*m_doubleInfo).m_dataSet.data(),
            (*m_doubleInfo).m_strLen_or_Ind.data(),
            (*m_doubleInfo).m_columnNames);

        GetResultColumn(
            0,             // columnNumber
            SQL_C_DOUBLE,  // dataType
            m_DoubleSize,  // columnSize
            0,             // decimalDigits
            SQL_NO_NULLS); // nullable

        GetResultColumn(
            1,             // columnNumber
            SQL_C_DOUBLE,  // dataType
            m_DoubleSize,  // columnSize
            0,             // decimalDigits
            SQL_NULLABLE); // nullable
    }

    //----------------------------------------------------------------------------------------------
    // Name: GetResultColumn
    //
    // Description:
    //  Test GetResultColumn to verify the expected result column information is obtained.
    //
    void CSharpExtensionApiTests::GetResultColumn(
        SQLUSMALLINT columnNumber,
        SQLSMALLINT  expectedDataType,
        SQLULEN      expectedColumnSize,
        SQLSMALLINT  expectedDecimalDigits,
        SQLSMALLINT  expectedNullable,
        SQLRETURN    SQLResult)
    {
        SQLSMALLINT dataType = 0;
        SQLULEN columnSize = 0;
        SQLSMALLINT decimalDigits = 0;
        SQLSMALLINT nullable = 0;

        SQLRETURN result = (sm_getResultColumnFuncPtr)(
            *m_sessionId,
            m_taskId,
            columnNumber,
            &dataType,
            &columnSize,
            &decimalDigits,
            &nullable);
        EXPECT_EQ(result, SQLResult);

        if(SQLResult == SQL_SUCCESS)
        {
            EXPECT_EQ(dataType, expectedDataType);
            EXPECT_EQ(columnSize, expectedColumnSize);
            EXPECT_EQ(decimalDigits, expectedDecimalDigits);
            EXPECT_EQ(nullable, expectedNullable);
        }
    }
}
