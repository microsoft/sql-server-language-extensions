# Regex sample

This sample shows you how to use SQL Server Language Extensions to create a Java program that receives two columns (ID and text) from SQL Server and a regular expression as an input parameter. The class returns two columns back to SQL Server (ID and text).

For a given text in the text column sent to the Java program, the code checks if the given regular expression is fulfilled, and if it is fulfilled, it returns that text together with the original ID.

This particular sample uses a regular expression that checks if a text contains the word "Java" or "java".

## Sample files

* RegexSample.Java - This is the class containing the Java code that will be invoked from SQL Server
* java-regex-linux.sql and java-regex-win.sql - SQL file containing T-SQL script for configuring and creating test data and calling the Java sample program.

## Prerequisites

+ SQL Server 2019 Database Engine instance with the extensibility framework and Java programming extension. [Installation guide](https://docs.microsoft.com/en-us/sql/language-extensions/install/install-sql-server-language-extensions-on-windows?view=sqlallproducts-allversions). 

+ SQL Server Management Studio or Azure Data Studio for executing T-SQL.

+ Java SE Development Kit (JDK) 8 or JRE 8 on Windows or Linux.

+ Command-line compilation using **javac** is sufficient for this sample.

### Compiling a class file

The class file can be created using the command-line compiler **javac**. For this sample, create a folder containing the RegexSample.java file.

To create the class files, navigate to the folder containing your java file and run this command:

```cmd
javac -cp <mssql-java-lang-extension.jar> *.java
```

### Creating a jar file

When using this sample and executing a Java code from SQL Server, we recommend packaging your class files into a jar file.

To create a jar from class files, navigate to the parent folder of the location that contains the class file and run this command:

```cmd
jar -cf <MyJar.jar> folder/*.class
```
