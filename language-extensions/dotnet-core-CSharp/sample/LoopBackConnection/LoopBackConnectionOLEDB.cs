//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: LoopBackConnectionOLEDB.cs
//
// Purpose:
//  Connect to SQL server using the loopback connection in a Language extension environment.
//
//*********************************************************************
using Microsoft.Data.Analysis;
using Microsoft.Data.SqlClient;
using Microsoft.SqlServer.CSharpExtension.SDK;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;
using System.Data.OleDb;

namespace UserExecutor
{
    /// <summary>
    /// This class extends the AbstractSqlServerExtensionExecutor.
    /// This class can be used to execute custom SQL queries within the Language Extension environment.
    /// </summary>
    public class LoopBackConnectionOLEDB : AbstractSqlServerExtensionExecutor
    {
        /// <summary>
        /// This method overrides the Execute method from AbstractSqlServerExtensionExecutor.
        /// </summary>
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            // Ole DB implementation
            // Connection string to the server.. 
            // This is a standard connection string. 
            // Example: 
            //string connectionstring = "Data Source=<ServerName>;User Id=<<UserID>>;Password=<<Credentials>>;Initial Catalog=<<Database>>;Trusted_Connection=True;Encrypt=False;";
            string connectionstring = sqlParams["@connectionString"];

            // Create empty output DataFrame with One column
            //
            DataFrame output = new DataFrame(new StringDataFrameColumn("text", 0));
            using (OleDbConnection connection = new OleDbConnection(connectionstring))
            {
                connection.Open();

                // The SQL command that you need to execute on the SQL server.
                // This can be passed as a parameter as well into this method if you want to make it more dynamic.
                String sql = sqlParams["@query"];
                using (OleDbCommand command = new OleDbCommand(sql, connection))
                {
                    using (OleDbDataReader reader = command.ExecuteReader())
                    {
                        while (reader.Read())
                        {
                            String outstring = "{0} {1}", reader.GetString(0), reader.GetString(1);
                            Console.WriteLine(outstring);
                            output.append(outstring);
                        }
                    }
                }
            }

            // Modify the parameters
            //
            sqlParams["@rowsCount"] = output.Rows.Count;
            sqlParams["@Status"] = "Success!";

            return output;
        }
    }
}
