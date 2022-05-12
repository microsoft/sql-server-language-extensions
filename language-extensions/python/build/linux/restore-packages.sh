#!/bin/bash
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get --no-install-recommends -y install curl zip unzip 

apt-get update
apt-get install -y software-properties-common
add-apt-repository -y ppa:deadsnakes/ppa
apt-get update

DEFAULT_PYTHONHOME=/usr
BOOST_VERSION=1.79.0
BOOST_VERSION_IN_UNDERSCORE=1_79_0
PYTHON_VERSION=3.10
NUMPY_VERSION=1.22.3
PANDAS_VERSION=1.4.2

apt-get install -y python${PYTHON_VERSION}-dev libboost-all-dev python${PYTHON_VERSION}-distutils
curl -sS https://bootstrap.pypa.io/get-pip.py | python${PYTHON_VERSION}

# Find PYTHONHOME from user, or set to default for tests.
# Error code 1 is generic bash error.
# 
if [ -z "${PYTHONHOME}" ]; then
	if [ -x "${DEFAULT_PYTHONHOME}" ]; then 
		PYTHONHOME=${DEFAULT_PYTHONHOME}
	else
		echo "PYTHONHOME is empty"
		exit 1
	fi
fi

echo "Python home is ${PYTHONHOME}"

# Lock versions of numpy and pandas to versions compatible for the defined python version
#
${PYTHONHOME}/bin/python${PYTHON_VERSION} -m pip install --force-reinstall numpy==${NUMPY_VERSION} -t ${PYTHONHOME}/lib/python${PYTHON_VERSION}/dist-packages
${PYTHONHOME}/bin/python${PYTHON_VERSION} -m pip install --force-reinstall pandas==${PANDAS_VERSION} -t ${PYTHONHOME}/lib/python${PYTHON_VERSION}/dist-packages

# Download and install boost, then navigate to boost root directory
#
wget -O boost_${BOOST_VERSION_IN_UNDERSCORE}.tar.gz https://sourceforge.net/projects/boost/files/boost/${BOOST_VERSION}/boost_${BOOST_VERSION_IN_UNDERSCORE}.tar.gz/download
tar xzvf boost_${BOOST_VERSION_IN_UNDERSCORE}.tar.gz -C /usr/lib/
pushd /usr/lib/boost_${BOOST_VERSION_IN_UNDERSCORE}

# Build defined python version of boost and boost python
#
./bootstrap.sh --without-icu --with-python=${PYTHONHOME}/bin/python${PYTHON_VERSION} --with-python-version=${PYTHON_VERSION} --with-python-root=${PYTHONHOME}/lib/python${PYTHON_VERSION}

echo "using python : ${PYTHON_VERSION} : ${PYTHONHOME}/bin/python${PYTHON_VERSION} : ${PYTHONHOME}/include/python${PYTHON_VERSION} : ${PYTHONHOME}/lib ;" >> project-config.jam

# Change cxx flags to force boost to compile with -fPIC compilation, otherwise will fail linking when building libPythonExtension.so
#
sed -i 's/using gcc[^;]*;/using gcc : foo : g++ : <cxxflags>-fPIC ;/g' project-config.jam 

./b2 --clean
./b2 toolset=gcc variant=debug address-model=64 include=${PYTHONHOME}/include/python${PYTHON_VERSION}/ link=static threading=multi -j12

cp -rf boost /usr/include/

popd

exit $?
