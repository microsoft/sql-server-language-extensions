-- Step 1: Create sample data

CREATE DATABASE csharptest
GO
USE csharptest
GO

CREATE TABLE testdata (
    [id] int,
    [text] varchar(100),
)
GO

INSERT INTO testdata(id, "text") VALUES (4, 'This sentence contains C#')
INSERT INTO testdata(id, "text") VALUES (1, 'This sentence does not')
INSERT INTO testdata(id, "text") VALUES (3, 'I love c#!')
INSERT INTO testdata(id, "text") VALUES (2, NULL)
GO

-- Step 2: Create the main class
-- Step 3: Compile and create a .dll file

-- Step 4: Create external language

CREATE EXTERNAL LANGUAGE Dotnet
FROM
(CONTENT = N'<Data-SQL-Language-Extensions root path>\build-output\dotnet-core-CSharp-extension\windows\release\packages\dotnet-core-CSharp-lang-extension.zip', FILE_NAME = 'nativecsharpextension.dll' );
GO

-- Step 5: Create external libraries

CREATE EXTERNAL LIBRARY regex
FROM (CONTENT = '<Data-SQL-Language-Extensions root path>\language-extensions\dotnet-core-CSharp\sample\regex\pkg\obj\Debug\RegexSample.dll')
WITH (LANGUAGE = 'Dotnet');
GO

-- Step 6: Call the .NET Core C# class

declare @rowsCount int
declare @regexExpr varchar(200)
set @regexExpr = N'[Cc]#'

EXEC sp_execute_external_script
  @language = N'Dotnet'
, @script = N'regex;UserExecutor.CSharpRegexExecutor'
, @input_data_1 = N'SELECT * FROM testdata'
, @params = N'@regexExpr varchar(200) OUTPUT, @rowsCount int OUTPUT'
, @regexExpr =  @regexExpr OUTPUT
, @rowsCount = @rowsCount OUTPUT
with result sets ((id int, text varchar(100)));

select @rowsCount as rowsCount, @regexExpr as message
print @rowsCount
print @regexExpr

