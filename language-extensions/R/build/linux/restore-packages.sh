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
LSB_RELEASE=$(lsb_release -cs)
apt-key adv --keyserver keyserver.ubuntu.com --recv-keys E298A3A825C0D65DFD57CBB651716619E084DAB9
add-apt-repository "deb https://cloud.r-project.org/bin/linux/ubuntu ${LSB_RELEASE}-cran40/"
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

# Remove codetools from default library so it doesn't interfere with Rcpp installation below.
#
${R_HOME}/bin/R -e "remove.packages('codetools', lib = '${R_LIBRARY_PATH}')"

# Install Rcpp.
#
${R_HOME}/bin/R -e "install.packages('https://cran.r-project.org/src/contrib/Archive/Rcpp/Rcpp_1.0.6.tar.gz', lib = '${R_LIBRARY_PATH}', repos = NULL, type='source')"
${R_HOME}/bin/R -e "stopifnot(require(Rcpp))"
check_exit_code "Success: Installed Rcpp package." "Error: Failed to install Rcpp package."

# Install RInside.
#
${R_HOME}/bin/R -e "install.packages('https://cran.r-project.org/src/contrib/Archive/RInside/RInside_0.2.15.tar.gz', lib = '${R_LIBRARY_PATH}', repos = NULL, , type='source')"
${R_HOME}/bin/R -e "stopifnot(require(RInside))"
check_exit_code "Success: Installed RInside package." "Error: Failed to install RInside package."

exit $?
