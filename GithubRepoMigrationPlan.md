# Plan to migrate from internal AzureDevOps repo to open-source github repo
## Introduction
For Data SQL Language Extension project, we have both the internal repo (this one) and an open-source github repo ([microsoft/sql-server-language-extensions: language-extensions-sqlserver (github.com)](https://github.com/microsoft/sql-server-language-extensions)) which we have few changes since the first release of each language extension. Therefore, this plan is to migrate the current updates from the internal AzureDevOps repo to the external open-source github repo.
## Expected outcome
After the migration, we should have all our language extensions implementations (under **language-extensions directory**) updated and reside in the open-source repo. Everything else (such as build pipelines, tests) should remain in the internal repo. The internal repo should have the open-source repo as a **submodule** or **subtree**. In the future, all the update of the language extensions should only be made to the github repo.
## Open-source github repo steps
1. Resolve the conflicts between the language extensions from internal and external repos and create a PR to merge the change from the internal repo to the external repo.
2. Add the **restore-packages.cmd** for each language extension to provide a default runtime for the customer. (Note: currently python `External-Boost` package which is an internal Nuget package to restore packages, we need to find a replacement for that)
## Introduction for git submodule and git subtree
**Git submodule** enables you to preserve one git repository as a sub directory of another. You don’t clone or integrate any of the actual code in your new repository when you use many submodules. The submodule corresponds to a specific commit on the child repository, so that pointer commits must be manually updated.
**Git Subtree** allows subprojects to be contained under a subfolder of the parent project, the same happens with the subproject. A subtree is a sub directory that may be committed to, branched, and merged in any form you choose doing your super project.
### Git subtree vs submodule
Both techniques allow you to link a specific version of an external component to the local repository and bundle them. Both tools keep tracking the external repository’s history, enabling you to check out previous commits.
|                | submodule| subtree                         |
|----------------|-------------------------------|-----------------------------|
|Development type|**component-based development**, where your main project depends on a fixed version of another component (repo).           |**system-based development**, where your repo contains everything at once, and you can modify any part.           |
|Repo size        |**Smaller** since they are just links to a single commit in a subproject          |**Larger** since they store the whole subproject, including its history         |
|Cloning repositories        |**Harder** since it requires downloading the submodules separately| **Easier** since it downloads subfolder automatically
|Update changes from sub repo      |**Easier** since it can reference to the new HEAD commit| **Harder** since it needs to pull all the changes.
## Recommendation: Git submodule
If we compare it with adding a subtree, adding a submodule is fairly straightforward. All of the hazards and flaws do not appear until the last moment, which can be annoying.
For our project, we do not need to update the pipeline and the language extensions all at once. The language extension itself can be treated as independent so that once it gets updated externally, we only need to update the reference to the HEAD commit. 
## Internal AzureDevOps steps
1. Delete everything under **language-extensions** directory
2. Create a new submodule using the git submodule command that saves the path and hyperlink references in a folder called .gitmodules.
3. Modify the current pipelines and tests to use language extensions from the submodule.
4. Review the changes and commit the changes to the repository