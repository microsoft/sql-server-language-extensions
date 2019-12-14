#!/bin/bash

export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get --no-install-recommends -y install curl zip unzip apt-transport-https

# Add Microsoft repository
#
curl https://packages.microsoft.com/keys/microsoft.asc | apt-key add -
curl https://packages.microsoft.com/config/ubuntu/16.04/prod.list | tee /etc/apt/sources.list.d/msprod.list
apt-get update

# Install R runtime
#
MS_ROPEN="microsoft-r-open"
MS_ROPEN_VERSION="3.5.2"
MS_ROPEN_MRO="${MS_ROPEN}-mro-${MS_ROPEN_VERSION}"

apt-get -q -y install "${MS_ROPEN_MRO}"

exit $?
