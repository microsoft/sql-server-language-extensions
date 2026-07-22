#!/bin/bash

# Install unixodbc-dev for development headers
#
apt-get -q -y install unixodbc-dev

apt-get update -y
apt-get install build-essential software-properties-common -y
# Keep one GCC 13 toolchain for every extension. R 4.5 uses C23 fixed-underlying-type
# enums in public headers, which GCC 11 cannot parse when compiling R packages.
apt-get install gcc-13 g++-13 -y
apt-get install cmake -y
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 70 --slave /usr/bin/g++ g++ /usr/bin/g++-13

exit $?

