#!/bin/bash

# Install unixodbc-dev for development headers
#
apt-get -q -y install unixodbc-dev

apt-get update -y
apt-get install build-essential software-properties-common -y
# Use GCC 11 (native on the Ubuntu 22.04 build image) so the extension links against
# GLIBCXX_3.4.29 / glibc 2.35 and stays loadable on RHEL9 (glibc 2.34). GCC 13 emitted
# GLIBCXX_3.4.32 and glibc 2.38 symbol references (_dl_find_object, arc4random,
# __isoc23_*) that RHEL9 cannot satisfy, so libPythonExtension.so failed to load there.
apt-get install gcc-11 g++-11 -y
apt-get install cmake -y
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 70 --slave /usr/bin/g++ g++ /usr/bin/g++-11

exit $?

