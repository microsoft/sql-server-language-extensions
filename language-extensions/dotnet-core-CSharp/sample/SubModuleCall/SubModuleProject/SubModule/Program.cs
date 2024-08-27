using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
namespace UserExecutor
{
    // Lets assume that a DLL is created using this kind of code in another project. 
    // The DLL is moved into the folder where access is granted. 
    // You need to grant access to the DLL's parent folder before you execute these commands. 
    // To grant access to the folder, run the following commands in Windows command prompt:
    // Note: The folder name may change in your environment. So please specify the right folder name.
    // Command1: 
    //          icacls "C:\Program Files\Microsoft SQL Server\MSSQL16.SQLSERVER2022\MSSQL\ExternalLibraries\6\65537\1" /grant "SQLRUsergroupSQLSERVER2022":(OI)(CI)RX /T  
    // Change the server name according to your naming convension .
    // Command2:
    //          icacls "C:\Program Files\Microsoft SQL Server\MSSQL16.SQLSERVER2022\MSSQL\ExternalLibraries\6\65537\1"  /grant *S-1-15-2-1:(OI)(CI)RX /T
    // After executing above commands, SQL server will have access to local folder which has been specified in the commands. 
    // All these commands are available in  OneTimeSetupCommands.cmd file in this solution as well
    //
    public class SubModule
    {
        public void printConsole()
        {
            Console.WriteLine("This is hello world from adhoc DLL file");
        }
    }
}
