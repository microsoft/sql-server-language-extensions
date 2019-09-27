//*********************************************************************
//                 Copyright (C) Microsoft Corporation.
//
// @File: JavaExtensionUtils.cpp
// @Owner: brnieb
//
// Purpose:
//  Platform independent utility functions used throughout the
//  extension
//
//*********************************************************************
#ifdef _WIN64
#include <windows.h>
#endif
#include <boost/python.hpp>
#include <exception>
#include <wchar.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <list>
#include <string>
#include <assert.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#ifndef _WIN64
#include <sal_def.h>
#include <xplat_sal.h>
#endif
#include "PythonExtensionUtils.h"
#include "Logger.h"

using namespace std;

//----------------------------------------------------------------------------
// Name: PythonExtensionUtils::GetDLLPath
//
// Description:
//  Tries to construct a python path from the PYTHONHOME enviroment variable
//
// Returns:
//  String to the full path to the python library if valid, else the filename of
//  the python library.
//
string PythonExtensionUtils::GetDLLPath()
{
    string pythonDLLPath = "";

    // Get the enviroment variable JRE_HOME
    //
    char * pythonHomeValue;
    SIZE_T len;
    errno_t err;

#if defined ( _MSC_VER )
    err = _dupenv_s(&pythonHomeValue, &len, "PYTHONHOME");
#else
    pythonHomeValue = getenv("PYTHONHOME");
#endif

    LOG(pythonHomeValue);

    pythonDLLPath = string(pythonHomeValue) + "/python37.dll";

    return pythonDLLPath;
}


BOOL PythonExtensionUtils::AddArgToNamespace(
    boost::python::object nameSpace,
    std::string name,
    SQLSMALLINT	  DataType,
    SQLULEN		  ArgSize,
    SQLSMALLINT	  DecimalDigits,
    SQLPOINTER	  ArgValue,
    SQLINTEGER	  StrLen_or_Ind)
{
    switch (DataType)
    {
    case SQL_C_SLONG:
    {
        nameSpace[name] = *reinterpret_cast<SLONG *>(ArgValue);
        break;
    }
    case SQL_C_BIT:
    {
        nameSpace[name] = *reinterpret_cast<SQLCHAR *>(ArgValue);
        break;
    }
    case SQL_C_FLOAT:
    {
        float num = *reinterpret_cast<SQLREAL *>(ArgValue);
        nameSpace[name] = num;
        break;
    }
    case SQL_C_DOUBLE:
    {
        double num = *reinterpret_cast<SQLDOUBLE *>(ArgValue);
        nameSpace[name] = num;
        break;
    }
    case SQL_C_SBIGINT:
    {
        nameSpace[name] = *reinterpret_cast<SQLBIGINT *>(ArgValue);
        break;
    }
    case SQL_C_UTINYINT:
    {
        nameSpace[name] = *reinterpret_cast<SQLCHAR *>(ArgValue);
        break;
    }
    case SQL_C_SSHORT:
    {
        nameSpace[name] = *reinterpret_cast<SQLSMALLINT *>(ArgValue);
        break;
    }
    case SQL_C_CHAR:
    {
        string value(reinterpret_cast<CHAR *>(ArgValue));
        INT strlen = StrLen_or_Ind / sizeof(CHAR);
        value.resize(strlen);
        nameSpace[name] = value;
        break;
    }
    case SQL_C_WCHAR:
    {
        wstring value(reinterpret_cast<WCHAR *>(ArgValue));
        INT strlen = StrLen_or_Ind / sizeof(WCHAR);
        value.resize(strlen);
        nameSpace[name] = value;
        break;
    }
    case SQL_C_BINARY:
    {
        SQLCHAR * value(reinterpret_cast<SQLCHAR *>(ArgValue));
        nameSpace[name] = value;
        break;
    }
    case SQL_C_GUID:
    case SQL_C_TYPE_DATE:
    case SQL_C_NUMERIC:
    case SQL_C_TYPE_TIMESTAMP:
    default:
        throw invalid_argument("Unsupported input parameter type");
    }
    return TRUE;
}


BOOL PythonExtensionUtils::AddColumnToDictionary(
    _Inout_ boost::python::dict    *pDataSet,
    _In_ std::string            Name,
    _In_ SQLULEN                RowsNumber,
    _In_ SQLPOINTER             Data,
    _In_  SQLSMALLINT           DataType
    )
{

    cout << "Adding column " << Name << " with " << RowsNumber << " rows" << endl;
    boost::python::list dataList = boost::python::list();
    boost::python::dict DataSet = *pDataSet;

    switch (DataType)
    {
    case SQL_C_SLONG:
    {
        for (int j = 0; j < RowsNumber; j++) {
            SLONG t = reinterpret_cast<SLONG *>(Data)[j];
            cout << "input data: " << t << endl;
            dataList.append(t);
        }
        DataSet[Name] = dataList;
        break;
    }
    case SQL_C_BIT:
    {
        for (int j = 0; j < RowsNumber; j++) {
            SQLCHAR t = reinterpret_cast<SQLCHAR *>(Data)[j];
            cout << "input data: " << t << endl;
            dataList.append(t);
        }
        DataSet[Name] = dataList;
        break;
    }
    case SQL_C_FLOAT:
    {
        for (int j = 0; j < RowsNumber; j++) {
            SQLREAL t = reinterpret_cast<SQLREAL *>(Data)[j];
            cout << "input data: " << t << endl;
            dataList.append(t);
        }
        DataSet[Name] = dataList;
        break;
    }
    case SQL_C_DOUBLE:
    {
        for (int j = 0; j < RowsNumber; j++) {
            SQLDOUBLE t = reinterpret_cast<SQLDOUBLE *>(Data)[j];
            cout << "input data: " << t << endl;
            dataList.append(t);
        }
        DataSet[Name] = dataList;
        break;
    }
    case SQL_C_SBIGINT:
    {
        for (int j = 0; j < RowsNumber; j++) {
            SQLBIGINT t = reinterpret_cast<SQLBIGINT *>(Data)[j];
            cout << "input data: " << t << endl;
            dataList.append(t);
        }
        DataSet[Name] = dataList;
        break;
    }
    case SQL_C_UTINYINT:
    {
        for (int j = 0; j < RowsNumber; j++) {
            SQLCHAR t = reinterpret_cast<SQLCHAR *>(Data)[j];
            cout << "input data: " << t << endl;
            dataList.append(t);
        }
        DataSet[Name] = dataList;
        break;
    }
    case SQL_C_SSHORT:
    {
        for (int j = 0; j < RowsNumber; j++) {
            SQLSMALLINT t = reinterpret_cast<SQLSMALLINT *>(Data)[j];
            cout << "input data: " << t << endl;
            dataList.append(t);
        }
        DataSet[Name] = dataList;
        break;
    }
    case SQL_C_CHAR:
    {
        //string value(reinterpret_cast<CHAR *>(ArgValue));
        //INT strlen = StrLen_or_Ind / sizeof(CHAR);
        //value.resize(strlen);
        //nameSpace[Name] = value;
        break;
    }
    case SQL_C_WCHAR:
    {
        for (int j = 0; j < RowsNumber; j++) {
            wstring value(reinterpret_cast<WCHAR **>(Data)[j]);
            INT strlen = StrLen_or_Ind / sizeof(WCHAR);
            value.resize(strlen);

            wcout << "input data: " << value << endl;
            dataList.append(value);
        }
        DataSet[Name] = dataList;
        break;
    }
    case SQL_C_BINARY:
    {
        for (int j = 0; j < RowsNumber; j++) {
            SQLCHAR t = reinterpret_cast<SQLCHAR *>(Data)[j];
            cout << "input data: " << t << endl;
            dataList.append(t);
        }
        DataSet[Name] = dataList;
        break;
    }
    case SQL_C_GUID:
    case SQL_C_TYPE_DATE:
    case SQL_C_NUMERIC:
    case SQL_C_TYPE_TIMESTAMP:
    default:
        throw invalid_argument("Unsupported input parameter type");
    }
    return TRUE;
}