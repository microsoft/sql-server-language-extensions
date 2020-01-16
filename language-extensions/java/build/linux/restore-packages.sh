#!/bin/bash

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get --no-install-recommends -y install curl zip unzip apt-transport-https unixodbc-dev
apt-get install -y openjdk-8-jdk libc++1 multiarch-support libc6 libc++-dev libc++-helpers libc++-test libc++abi-dev libc++abi-test libc++abi1

exit $?
