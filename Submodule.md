# Migration from internal AzureDevOps repo to open-source github repo
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

## Development Step

### Open Source Repository
If you'd like to contribute to the source code, the process remains unchanged. You can directly make your code changes to the open source repo as before.

To test the build pipelines for changes in the open source repo, modify the `.gitsubmodule` file in this repo to target your private branch and update the `sql-server-language-extensions` file to contain your most recent commit of your branch.

Use the following commands:

```bash
cd sql-server-language-extensions
git checkout <your_branch>
git pull
cd ..
git add sql-server-language-extensions
git commit -m "Update submodule"
git push
```

Run a PR pipeline to test your change and ensure it won't break the pipeline. Once confirmed, complete the PR for the open source repo.

### This Repository

To clone the repo with submodule, use:

``` bash
git clone --recurse-submodules <this repo link>
```

If you already have the repo locally, use:

``` bash
git submodule init
git submodule update
```
After the PR for the open source repo is complete, update this repo to target the most recent commit of the open source repo using the following commands:

``` bash
cd sql-server-language-extensions
git checkout main
git pull
cd ..
git add sql-server-language-extensions
git commit -m "Update submodule"
git push
```

The final step is to update the version of data-sql-language-extensions in the downstream repositories, specifically DsMainDev. The objective of this procedure is to ensure that each time a change is implemented here, the DsMainDev component is updated concurrently. This allows us to identify any potential regression in the testshelltests at the earliest opportunity, enabling swift action to rectify any issues.