#!/bin/bash

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

export DEBIAN_FRONTEND=noninteractive
apt-get update

apt-get --no-install-recommends -y install curl zip unzip apt-transport-https

# Need bionic (Ubuntu 18.04 repo) for multiarch-support package.
#
# add-apt-repository 'deb http://mirrors.kernel.org/ubuntu bionic main universe'

apt-get install -y openjdk-17-jdk libc++1 libc6 libc++-dev libc++abi-dev libc++abi1

# Install multiarch-support package for cmake
#
wget http://archive.ubuntu.com/ubuntu/pool/main/g/glibc/multiarch-support_2.27-3ubuntu1.6_amd64.deb
sudo apt-get install ./multiarch-support_2.27-3ubuntu1.6_amd64.deb

exit $?
