#!/bin/bash
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get --no-install-recommends -y install curl zip unzip 

apt-get update
apt-get install -y software-properties-common
add-apt-repository -y ppa:deadsnakes/ppa
apt-get update

apt-get install -y python3.7-dev libboost-all-dev python3-pip

/usr/bin/python3.7 -m pip install --upgrade pip==18.1
/usr/bin/python3.7 -m pip install numpy -t /usr/lib/python3.7/dist-packages
/usr/bin/python3.7 -m pip install pandas -t /usr/lib/python3.7/dist-packages

wget -O boost_1_69_0.tar.gz https://sourceforge.net/projects/boost/files/boost/1.69.0/boost_1_69_0.tar.gz/download
tar xzvf boost_1_69_0.tar.gz -C /usr/lib/
pushd /usr/lib/boost_1_69_0

# Build Python3.7 version of boost and boost python
#
./bootstrap.sh --without-icu --with-python=/usr/bin/python3.7 --with-python-version=3.7 --with-python-root=/usr/lib/python3.7

echo "using python : 3.7 : /usr/bin/python3.7 : /usr/include/python3.7m : /usr/lib ;" >> project-config.jam

# Change cxx flags to force boost to compile with -fPIC compilation, otherwise will fail linking when building libPythonExtension.so
#
sed -i 's/using gcc[^;]*;/using gcc : foo : g++ : <cxxflags>-fPIC ;/g' project-config.jam 

./b2 --clean
./b2 toolset=gcc variant=debug address-model=64 include=/usr/include/python3.7m/ link=static threading=multi -j12

cp -rf boost /usr/include/

popd

exit $?
