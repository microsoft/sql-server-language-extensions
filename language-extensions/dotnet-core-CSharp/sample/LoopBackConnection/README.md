# Loopback Connection
 This folder contains an implementation which will help in doing a loopback connection to SQL server. This will help in running any custom query against the SQL server through SQL Server Language Extension.
 ## Prerequisites
 * [LoopBackConnectionADONET.cs](LoopBackConnectionADONET.cs). This contains the ADO.NET driver implementation sample for Loopback connection.
 * [LoopBackCOnnectionOLEDB.cs](LoopBackConnectionOLEDB.cs). This contains the OLEDB driver implementation sample for Loopback Connection. 

## Implementation Steps

1) Download/clone the this folder. 
2) Depending on what driver you would like to use, edit that particual component. 
3) Follow the standard method to outlined in documentation to moved the language Extension DLL code into respective folders.
4) Execute the code through a stored procedure call as outlined in the regex sample. 
5) Once your run it you should be able to see the console output in SSMS. 