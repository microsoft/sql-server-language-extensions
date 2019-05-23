-- TSQL script for the Java regex sample on Windows

EXEC sp_configure 'external scripts enabled', 1
RECONFIGURE WITH OVERRIDE
GO

CREATE DATABASE javatest
GO
USE javatest
GO

CREATE TABLE testdata (
    id int NOT NULL,
    "text" nvarchar(100) NOT NULL
)
GO

-- Insert data into test table
INSERT INTO testdata(id, "text") VALUES (1, 'This sentence contains java')
INSERT INTO testdata(id, "text") VALUES (2, 'This sentence does not')
INSERT INTO testdata(id, "text") VALUES (3, 'I love Java!')
GO

--CTP 3.0: Make sure you create a .zip file from the javaextension.dll file saved here: [SQL Server install path]\MSSQL\Binn\javaextension.dll
-- You might need to set read permisisons for SQLRUserGroup and ALL APPLICATION PACKAGES, depending on where you save the .zip file
CREATE EXTERNAL LANGUAGE Java
FROM
(CONTENT = N'[Path to .zip file]\javaextension.zip', FILE_NAME = 'javaextension.dll')
GO

--The extension jar is also saved under a predefined path as part of the sql server installation
--Win path: <SQL Server installation directory>\MSSQL\Binn\mssql-java-lang-extension.jar
CREATE EXTERNAL LIBRARY sdk
FROM (CONTENT = '<SQL Server installation directory>\MSSQL\Binn\mssql-java-lang-extension.jar')
WITH (LANGUAGE = 'Java');

--Create external library for the sample jar (including the RegexSample class you compiled)
CREATE EXTERNAL LIBRARY regex
FROM (CONTENT = '<path>/regex.jar')
WITH (LANGUAGE = 'Java');
GO

CREATE OR ALTER PROCEDURE [dbo].[java_regex] @expr nvarchar(200), @query nvarchar(400)
AS
BEGIN
--Call the Java program by giving the package.className in @script
--The method invoked in the Java code is always the "execute" method
EXEC sp_execute_external_script
  @language = N'Java'
, @script = N'pkg.RegexSample'
, @input_data_1 = @query
, @params = N'@regexExpr nvarchar(200)'
, @regexExpr = @expr
with result sets ((ID int, text nvarchar(100)));
END
GO

--Now execute the above stored procedure and provide the regular expression and an input query
EXECUTE [dbo].[java_regex] N'[Jj]ava', N'SELECT id, text FROM testdata'
GO
