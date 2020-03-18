#!/bin/bash

function check_exit_code {
	EXIT_CODE=$?
	if [ ${EXIT_CODE} -eq 0 ]; then
		echo $1
	else
		echo $2
		exit ${EXIT_CODE}
	fi
}

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
check_exit_code "Success: Installed R runtime." "Error: Failed to install R runtime."

R_HOME=/opt/microsoft/ropen/3.5.2/lib64/R
R_LIBRARY_PATH=${R_HOME}/library

# Install Rcpp.
#
${R_HOME}/bin/R -e "install.packages('https://cran.r-project.org/src/contrib/Rcpp_1.0.3.tar.gz', lib = '${R_LIBRARY_PATH}')"
${R_HOME}/bin/R -e "stopifnot(require(Rcpp))"
check_exit_code "Success: Installed Rcpp package." "Error: Failed to install Rcpp package."

# Install RInside.
#
${R_HOME}/bin/R -e "install.packages('https://cran.r-project.org/src/contrib/RInside_0.2.16.tar.gz', lib = '${R_LIBRARY_PATH}')"
${R_HOME}/bin/R -e "stopifnot(require(RInside))"
check_exit_code "Success: Installed RInside package." "Error: Failed to install RInside package."

exit $?
