
# Language extensions for Microsoft SQL Server 2019 (preview)

Welcome to the language extensions for SQL Server project!

In SQL Server 2019 (preview), we have added support for language extensibility, which means that you can now execute external languages like Java from SQL Server.

We hope you enjoy using Microsoft SQL Server and the language extension feature.

-SQL Server Team

## Announcements

* In SQL Server 2019 CTP 3.0, we are open sourcing the SDK for our Java language extension: **Microsoft Extensibility SDK for Java for Microsoft SQL Server​**

## Things to Know

* The Java components in this project are built on Java 8
* The SDK is also installed as part of SQL Server 2019 preview on both Windows as Linux
  * In CTP 3.0, default installation path on Windows: [instance installation home directory]\MSSQL\Binn\mssql-java-lang-extension.jar
  * In CTP 3.0, default installation path on Linux: /opt/mssql/lib/mssql-java-lang-extension.jar 

## Documentation

You can read more about how to use language extensions in SQL Server in the Microsoft [documentation](https://docs.microsoft.com/en-us/sql/language-extensions/language-extensions-overview?view=sqlallproducts-allversions).

## Get Started

This [tutorial](https://docs.microsoft.com/en-us/sql/language-extensions/tutorials/search-for-string-using-regular-expressions-in-java?view=sqlallproducts-allversions) will walk you through an end to end sample using the java language extension for SQL Server. Sample code files for the tutorial can also be found under samples in this repository.

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
