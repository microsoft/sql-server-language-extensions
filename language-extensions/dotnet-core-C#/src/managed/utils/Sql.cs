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
    /// This class includes SqlDataType and Sql related values
    /// </summary>
    public class Sql
    {
        public static readonly short SQL_SUCCESS = 0;
        public static readonly short SQL_ERROR = -1;
        public const short SQL_UNSIGNED_OFFSET = -22;
        public const short SQL_SIGNED_OFFSET = -20;
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
        /// This method converts datatype from short to enum SqlDataType
        /// </summary>
        /// <param name="satelliteDataType">
        /// datatype in short
        /// </param>
        /// <returns>
        /// enum SqlDataType
        /// </returns>
        public static SqlDataType ToDataType(short satelliteDataType)
        {
            if (!Enum.IsDefined(typeof(SqlDataType), satelliteDataType))
            {
                throw new Exception("Data type not found.");
            }

            return (SqlDataType)satelliteDataType;
        }
    }
}
