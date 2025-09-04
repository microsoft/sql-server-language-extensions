#!/bin/bash

# Install unixodbc-dev for development headers
#
apt-get -q -y install unixodbc-dev

apt-get update -y
apt-get install build-essential software-properties-common -y
add-apt-repository ppa:ubuntu-toolchain-r/test -y 
apt-get update -y
apt-get install cmake -y


# Install GCC-7
#
set -e

# Update and install dependencies
apt-get update -y
apt install -y \
  libgmp-dev \
  libmpfr-dev \
  libmpc-dev \
  flex \
  bison \
  wget \
  curl \
  ca-certificates \
  git

# Download GCC 7.5.0 source
cd /usr/local/src
wget http://ftp.gnu.org/gnu/gcc/gcc-7.5.0/gcc-7.5.0.tar.gz
tar -xvzf gcc-7.5.0.tar.gz
cd gcc-7.5.0

# Download prerequisites
./contrib/download_prerequisites

# Create build directory
mkdir build && cd build

# Configure build
../configure --enable-languages=c,c++ --disable-multilib

# Compile and install
make -j$(nproc)
make install

# Set GCC 7 as default (optional)
update-alternatives --install /usr/bin/gcc gcc /usr/local/bin/gcc 100
update-alternatives --install /usr/bin/g++ g++ /usr/local/bin/g++ 100

echo "✅ GCC 7 installation complete."
gcc --version

exit $?

