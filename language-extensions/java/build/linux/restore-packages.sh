#!/bin/bash

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

export DEBIAN_FRONTEND=noninteractive
apt-get update

apt-get --no-install-recommends -y install curl zip unzip apt-transport-https

# Need bionic (Ubuntu 18.04 repo) for multiarch-support package.
#
add-apt-repository 'deb http://mirrors.kernel.org/ubuntu bionic main universe'

apt-get install -y openjdk-17-jdk libc++1 multiarch-support libc6 libc++-dev libc++-helpers libc++abi-dev libc++abi1

exit $?
