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
apt-get --no-install-recommends -y install curl zip unzip apt-transport-https libstdc++6

# Add R CRAN repository.
#
apt-key adv --keyserver keyserver.ubuntu.com --recv-keys E298A3A825C0D65DFD57CBB651716619E084DAB9
add-apt-repository 'deb https://cloud.r-project.org/bin/linux/ubuntu xenial-cran35/'
apt-get update

# Install R runtime.
#
apt-get --no-install-recommends -y install r-base-core
check_exit_code "Success: Installed R runtime." "Error: Failed to install R runtime."

DEFAULT_R_HOME=/usr/lib/R

# Find R_HOME from user, or set to default for installing Rcpp and RInside.
# Error code 1 is generic bash error.
#
if [ -z "${R_HOME}" ]; then
	if [ -d "${DEFAULT_R_HOME}" ]; then
		R_HOME=${DEFAULT_R_HOME}
	else
		echo "R_HOME is empty"
		exit 1
	fi
fi
R_LIBRARY_PATH=${R_HOME}/library

# Install Rcpp.
#
${R_HOME}/bin/R -e "install.packages('Rcpp', lib = '${R_LIBRARY_PATH}', repos = 'https://ftp.osuosl.org/pub/cran/')"
${R_HOME}/bin/R -e "stopifnot(require(Rcpp))"
check_exit_code "Success: Installed Rcpp package." "Error: Failed to install Rcpp package."

# Install RInside.
#
${R_HOME}/bin/R -e "install.packages('RInside', lib = '${R_LIBRARY_PATH}', repos = 'https://ftp.osuosl.org/pub/cran/')"
${R_HOME}/bin/R -e "stopifnot(require(RInside))"
check_exit_code "Success: Installed RInside package." "Error: Failed to install RInside package."

exit $?
