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

# Step 1: Install dependencies
echo "Installing build dependencies..."
apt-get update -y
apt-get install -y libgmp-dev libmpfr-dev libmpc-dev flex bison wget

# Step 2: Download GCC 7 source
echo "Downloading GCC 7.5.0 source..."
wget http://ftp.gnu.org/gnu/gcc/gcc-7.5.0/gcc-7.5.0.tar.gz
tar -xvzf gcc-7.5.0.tar.gz
cd gcc-7.5.0

# Step 3: Download prerequisites
echo "Downloading prerequisites..."
./contrib/download_prerequisites

# Step 4: Create build directory
mkdir build && cd build

# Step 5: Configure with --disable-bootstrap
echo "Configuring GCC build..."
../configure --enable-languages=c,c++ --disable-multilib --disable-bootstrap

# Step 6: Build and install
echo "Building GCC (this may take a while)..."
make -j$(nproc)

echo "Installing GCC..."
make install

# Step 7: Verify installation
echo "Verifying GCC installation..."
gcc-7 --version
g++-7 --version
gcc --version

exit $?
