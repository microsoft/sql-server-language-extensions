## Using the ​Microsoft Extensibility SDK for Java for Microsoft SQL Server​

### Things to Know

* The SDK is built on Java 8
* The SDK is also installed as part of SQL Server 2019 preview on both Windows as Linux
  * In CTP 3.0, default installation path on Windows: [instance installation home directory]\MSSQL\Binn\mssql-java-lang-extension.jar
  * In CTP 3.0, default installation path on Linux: /opt/mssql/lib/mssql-java-lang-extension.jar 

### SDK Overview

The Microsoft Extensibility SDK for Java will help you to implement a Java program that can be called from SQL Server 2019 (preview). The SDK is an interface for the Java language extension and is used to exchange data with SQL Server and to execute Java code from SQL Server.

+ Download the [Microsoft Extensibility SDK for Java for Microsoft SQL Server](http://aka.ms/mssql-java-lang-extension).
Very soon, the SDK will be available under releases in this repository.

Read more about the [​Microsoft Extensibility SDK for Java for Microsoft SQL Server​](https://docs.microsoft.com/en-us/sql/language-extensions/how-to/extensibility-sdk-java-sql-server?view=sqlallproducts-allversions)

#### Implementation requirements

The SDK interface defines a set of requirements that need to be fulfilled for SQL Server to communicate with the Java runtime. To use the SDK, you need to follow some implementation rules in your main class. SQL Server can then execute a specific method in the Java class and exchange data using the Java language extension.

For an example of how you can use the SDK, see [Tutorial: Search for a string using regular expressions (regex) in Java](../tutorials/search-for-string-using-regular-expressions-in-java.md).

### SDK CLasses

The SDK consists of three Java classes. [Source code](https://github.com/microsoft/sql-server-language-extensions/tree/master/language-extensions/java/sdk/src/main/java/com/microsoft/sqlserver/javalangextension)

Two abstract classes that define the interface the Java extension uses to exchange data with SQL Server:

- **AbstractSqlServerExtensionExecutor**
- **AbstractSqlServerExtensionDataset**

The third class is a helper class, which contains an implementation of a data set object. It is an optional class you can use, which makes it easier to get started. You can also use your own implementation of such a class instead.

- **PrimitiveDataset**


#### AbstractSqlServerExtensionExecutor

The abstract class `AbstractSqlServerExtensionExecutor` contains the interface used to execute Java code by the Java language extension for SQL Server.

Your main Java class needs to inherit from this class. Inheriting from this class means that there are certain methods in the class you need to implement in your own class.

#### AbstractSqlServerExtensionDataset

The abstract class `AbstractSqlServerExtensionDataset` contains the interface for handling input and output data used by the Java extension.

#### PrimitiveDataset

The class `PrimitiveDataset` is an implementation of `AbstractSqlServerExtensionDataset` that stores simple types as primitives arrays.

It is provided in the SDK simply as an optional helper class. If you don't use this class, you need to implement your own class that inherits from `AbstractSqlServerExtensionDataset`. 

### Sample

This [tutorial](https://docs.microsoft.com/en-us/sql/language-extensions/tutorials/search-for-string-using-regular-expressions-in-java?view=sqlallproducts-allversions) will walk you through an end to end sample using the java language extension for SQL Server. Sample code files for the tutorial can also be found under samples in this repository.