//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: LoopBackConnectionADONET.cs
//
// Purpose:
//  Sample code for loopback connection using ADO.NET driver with comes with the SQLCLIENT implementation. 
//  This will help in running any custom query against the same SQL server engine where the Language extension is running.
//
//*********************************************************************
using Microsoft.Data.Analysis;
using Microsoft.Data.SqlClient;
using Microsoft.SqlServer.CSharpExtension.SDK;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;


namespace UserExecutor
{
    /// <summary>
    /// This class extends the AbstractSqlServerExtensionExecutor and uses
    /// a regular expression that checks if a text contains the word "C#" or "c#"
    /// </summary>
    public class LoopBackConnectionADONET: AbstractSqlServerExtensionExecutor
    {
        /// <summary>
        /// This method overrides the Execute method from AbstractSqlServerExtensionExecutor.
        /// </summary>
        
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            // Connection string to the server.. 
            // This is a standard connection string. 
            // Example: 
            // string connectionstring = "Data Source=SQLSERVER2022;User Id=user1;Password=xxxx;Initial Catalog=DBNAME;Trusted_Connection=True;Encrypt=False;";
            string connectionstring = "Data Source=<ServerName>;User Id=<<UserID>>;Password=<<Credentials>>;Initial Catalog=<<Database>>;Trusted_Connection=True;Encrypt=False;";
            
            using (SqlConnection connection = new SqlConnection(connectionstring))
            {
                connection.Open();
                // The SQL command that you need to execute on the SQL server.
                // This can be passed as a parameter as well into this method if you want to make it more dynamic.
                String sql = "SELECT field1, field2 FROM table1";

                using (SqlCommand command = new SqlCommand(sql, connection))
                {
                    using (SqlDataReader reader = command.ExecuteReader())
                    {
                        while (reader.Read())
                        {
                            Console.WriteLine("{0} {1}", reader.GetString(0), reader.GetString(1));
                        }
                    }
                }
            }

            return null;
        }

      
    }
}