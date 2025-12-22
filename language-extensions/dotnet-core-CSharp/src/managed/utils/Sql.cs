//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: Sql.cs
//
// Purpose:
//  This is the the main include for SqlDataType and Sql return values
//
//*********************************************************************
using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class includes SqlDataType and Sql related values.
    /// </summary>
    public class Sql
    {
        public const short SQL_SUCCESS = 0;
        public const short SQL_ERROR = -1;
        public const short SQL_NULL_DATA = -1;
        public const short SQL_UNSIGNED_OFFSET = -22;
        public const short SQL_SIGNED_OFFSET = -20;

        public const short MinUtf8CharSize = 1;
        public const short MinUtf16CharSize = 2;
        public enum SqlDataType: short
        {
            DotNetBigInt = -5 + SQL_SIGNED_OFFSET, //SQL_C_SBIGINT + SQL_SIGNED_OFFSET
            DotNetUBigInt = -5 + SQL_UNSIGNED_OFFSET, //SQL_C_UBIGINT + SQL_UNSIGNED_OFFSET
            DotNetBit = -7, //SQL_C_BIT
            DotNetChar = 1, //SQL_C_CHAR
            DotNetWChar = -8, //SQL_C_WCHAR
            DotNetDate = 91, //SQL_C_TYPE_DATE
            DotNetDecimal = 3, //SQL_DECIMAL
            DotNetDouble = 8, //SQL_C_DOUBLE
            DotNetFloat = 6, //SQL_C_FLOAT
            DotNetInteger = 4 + SQL_SIGNED_OFFSET, //SQL_C_SLONG + SQL_SIGNED_OFFSET
            DotNetUInteger = 4 + SQL_UNSIGNED_OFFSET, //SQL_C_ULONG + SQL_UNSIGNED_OFFSET
            DotNetNumeric = 2, //SQL_NUMERIC
            DotNetNVarChar = -9, //SQL_WVARCHAR
            DotNetReal = 7, //SQL_REAL
            DotNetSmallInt = 5 + SQL_SIGNED_OFFSET, //SQL_C_SSHORT + SQL_SIGNED_OFFSET
            DotNetUSmallInt = 5 + SQL_UNSIGNED_OFFSET, //SQL_C_USHORT + SQL_UNSIGNED_OFFSET
            DotNetTimestamp = 93, //SQL_TYPE_TIMESTAMP
            DotNetTinyInt = -6 + SQL_SIGNED_OFFSET, //SQL_C_STINYINT + SQL_SIGNED_OFFSET
            DotNetUTinyInt = -6 + SQL_UNSIGNED_OFFSET, //SQL_C_UTINYINT + SQL_UNSIGNED_OFFSET
            DotNetVarBinary = -3, //SQL_VARBINARY
            DotNetVarChar = 12, //SQL_VARCHAR
        };

        /// <summary>
        /// This Dictionary maps C# type to SqlDataType.
        /// </summary>
        public readonly static Dictionary<Type, SqlDataType> DataTypeMap = new Dictionary<Type, SqlDataType>()
        {
            {typeof(int), SqlDataType.DotNetInteger},
            {typeof(uint), SqlDataType.DotNetUInteger},
            {typeof(long), SqlDataType.DotNetBigInt},
            {typeof(ulong), SqlDataType.DotNetUBigInt},
            {typeof(short), SqlDataType.DotNetSmallInt},
            {typeof(ushort), SqlDataType.DotNetUSmallInt},
            {typeof(sbyte), SqlDataType.DotNetTinyInt},
            {typeof(byte), SqlDataType.DotNetUTinyInt},
            {typeof(float), SqlDataType.DotNetReal},
            {typeof(double), SqlDataType.DotNetDouble},
            {typeof(bool), SqlDataType.DotNetBit},
            {typeof(string), SqlDataType.DotNetChar}
        };

        /// <summary>
        /// This Dictionary maps SqlDataType to size in bytes.
        /// </summary>
        public readonly static Dictionary<SqlDataType, short> DataTypeSize = new Dictionary<SqlDataType, short>()
        {
            {SqlDataType.DotNetInteger, sizeof(int)},
            {SqlDataType.DotNetUInteger, sizeof(uint)},
            {SqlDataType.DotNetBigInt, sizeof(long)},
            {SqlDataType.DotNetUBigInt, sizeof(ulong)},
            {SqlDataType.DotNetSmallInt, sizeof(short)},
            {SqlDataType.DotNetUSmallInt, sizeof(ushort)},
            {SqlDataType.DotNetTinyInt, sizeof(sbyte)},
            {SqlDataType.DotNetUTinyInt, sizeof(byte)},
            {SqlDataType.DotNetReal, sizeof(float)},
            {SqlDataType.DotNetFloat, sizeof(double)},
            {SqlDataType.DotNetDouble, sizeof(double)},
            {SqlDataType.DotNetBit, sizeof(bool)},
            {SqlDataType.DotNetChar, MinUtf8CharSize},
            {SqlDataType.DotNetWChar, MinUtf16CharSize}
        };

        /// <summary>
        /// This method converts datatype from short to enum SqlDataType.
        /// </summary>
        /// <param name="SQLDataType">
        /// datatype in short
        /// </param>
        /// <returns>
        /// enum SqlDataType
        /// </returns>
        public static SqlDataType ToManagedDataType(short SQLDataType)
        {
            if (!Enum.IsDefined(typeof(SqlDataType), SQLDataType))
            {
                throw new Exception("Data type not found.");
            }

            return (SqlDataType)SQLDataType;
        }

        /// <summary>
        /// This method converts datatype from enum SqlDataType to short.
        /// </summary>
        /// <param name="dataType">
        /// enum SqlDataType
        /// </param>
        /// <returns>
        /// datatype in short
        /// </returns>
        public static short ToSQLDataType(SqlDataType dataType)
        {
            return (short)dataType;
        }
    }
}
