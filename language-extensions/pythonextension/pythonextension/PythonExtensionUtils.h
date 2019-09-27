#pragma once

class PythonExtensionUtils
{
public:
    static std::string GetDLLPath();

    static BOOL AddArgToNamespace(
        _In_ boost::python::object nameSpace,
        _In_ std::string name,
        _In_ SQLSMALLINT DataType,
        _In_ SQLULEN ArgSize,
        _In_ SQLSMALLINT DecimalDigits,
        _In_ SQLPOINTER ArgValue,
        _In_ SQLINTEGER StrLen_or_Ind);

    static BOOL AddColumnToDictionary(
        _In_ boost::python::dict    *pDataSet,
        _In_ std::string            Name,
        _In_ SQLULEN                RowsNumber,
        _In_ SQLPOINTER             Data,
        _In_ SQLSMALLINT            DataType);

    //static bool ConvertSQLToPythonType(_In_ SQLSMALLINT DataType);

private:

};
