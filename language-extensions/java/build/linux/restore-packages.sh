#!/bin/bash

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

export DEBIAN_FRONTEND=noninteractive
apt-get update

apt-get --no-install-recommends -y install curl zip unzip apt-transport-https

apt-get install -y openjdk-17-jdk libc++1 libc6 libc++-dev libc++abi-dev libc++abi1

exit $?
