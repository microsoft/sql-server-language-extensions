#!/bin/bash

# Install unixodbc-dev for development headers
#
apt-get -q -y install unixodbc-dev

apt-get update -y
apt-get install build-essential software-properties-common cmake -y

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

# Step 3: Manually download prerequisites
echo "Downloading GMP, MPFR, and MPC..."
wget https://ftp.gnu.org/gnu/gmp/gmp-6.1.0.tar.bz2
wget https://ftp.gnu.org/gnu/mpfr/mpfr-3.1.4.tar.bz2
wget https://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz

# Step 4: Extract and move prerequisites
echo "Extracting and linking prerequisites..."
tar -xvjf gmp-6.1.0.tar.bz2
mv gmp-6.1.0 gmp

tar -xvjf mpfr-3.1.4.tar.bz2
mv mpfr-3.1.4 mpfr

tar -xvzf mpc-1.0.3.tar.gz
mv mpc-1.0.3 mpc

# Step 5: Create build directory
mkdir build && cd build

# Step 6: Configure with --disable-bootstrap
echo "Configuring GCC build..."
../configure --enable-languages=c,c++ --disable-multilib --disable-bootstrap

# Step 7: Build and install
echo "Building GCC (this may take a while)..."
make -j$(nproc)

echo "Installing GCC..."
make install

# Step 8: Verify installation
echo "Verifying GCC installation..."
gcc-7 --version
g++-7 --version
gcc --version

exit $?
