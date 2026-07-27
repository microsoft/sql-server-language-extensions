#!/bin/bash

# Keep one GCC 13 toolchain for every extension. R 4.5 uses C23 fixed-underlying-type
# enums in public headers, so the compiler must not vary between languages.
set -euo pipefail

apt-get -q -y install unixodbc-dev
apt-get update -y
apt-get install -y build-essential software-properties-common gcc-13 g++-13 cmake
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 70 \
	--slave /usr/bin/g++ g++ /usr/bin/g++-13

test "$(gcc -dumpfullversion -dumpversion | cut -d. -f1)" = "13"
test "$(g++ -dumpfullversion -dumpversion | cut -d. -f1)" = "13"
gcc -dumpfullversion -dumpversion
g++ -dumpfullversion -dumpversion

exit $?

