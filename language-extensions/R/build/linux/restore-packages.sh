#!/bin/bash

export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get --no-install-recommends -y install curl zip unzip apt-transport-https

# Add Microsoft repository.
#
curl https://packages.microsoft.com/keys/microsoft.asc | apt-key add -
curl https://packages.microsoft.com/config/ubuntu/16.04/prod.list | tee /etc/apt/sources.list.d/msprod.list
apt-get update

# Install R runtime.
#
MS_ROPEN="microsoft-r-open"
MS_ROPEN_VERSION="3.5.2"
MS_ROPEN_MRO="${MS_ROPEN}-mro-${MS_ROPEN_VERSION}"

apt-get -q -y install "${MS_ROPEN_MRO}"

R_HOME=/opt/microsoft/ropen/3.5.2/lib64/R
R_LIBRARY_PATH=${R_HOME}/library

# Install Rcpp.
#
/opt/microsoft/ropen/3.5.2/lib64/R/bin/R -e "install.packages('https://cran.r-project.org/src/contrib/Rcpp_1.0.3.tar.gz', lib = '${R_LIBRARY_PATH}')"

# Install RInside.
#
/opt/microsoft/ropen/3.5.2/lib64/R/bin/R -e "install.packages('https://cran.r-project.org/src/contrib/RInside_0.2.15.tar.gz', lib = '${R_LIBRARY_PATH}')"

exit $?
