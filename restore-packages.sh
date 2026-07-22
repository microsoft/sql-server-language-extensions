#!/bin/bash

# Install unixodbc-dev for development headers
#
apt-get -q -y install unixodbc-dev

apt-get update -y
apt-get install build-essential software-properties-common -y
# Use one GCC 11 toolchain for every Linux language extension. DSLE is responsible
# for supplying compatible runtime headers and OS baselines for each language.
apt-get install gcc-11 g++-11 -y
apt-get install cmake -y
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 70 --slave /usr/bin/g++ g++ /usr/bin/g++-11

test "$(gcc -dumpfullversion -dumpversion | cut -d. -f1)" = "11"
test "$(g++ -dumpfullversion -dumpversion | cut -d. -f1)" = "11"
gcc --version | head -1
g++ --version | head -1

exit $?

