#!/bin/bash

# Keep one GCC 13 toolchain for every extension. R 4.5 uses C23 fixed-underlying-type
# enums in public headers, so the compiler must not vary between languages.
set -euo pipefail

# apt-get update must precede the first install. It did NOT before this change:
# `apt-get install unixodbc-dev` ran ahead of `apt-get update`, which was tolerable
# while a stale index only produced a warning. Under set -e it aborts the whole
# restore, so the update has been moved to the front. Do not reorder these back.
apt-get update -y
apt-get -q -y install unixodbc-dev
apt-get install -y build-essential software-properties-common gcc-13 g++-13 cmake
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 70 \
	--slave /usr/bin/g++ g++ /usr/bin/g++-13

# Print before asserting: under set -e a failing test aborts the script, so if the
# assertions came first there would be no record of which compiler was actually selected.
gcc -dumpfullversion -dumpversion
g++ -dumpfullversion -dumpversion
test "$(gcc -dumpfullversion -dumpversion | cut -d. -f1)" = "13"
test "$(g++ -dumpfullversion -dumpversion | cut -d. -f1)" = "13"

