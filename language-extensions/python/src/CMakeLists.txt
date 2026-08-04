# CMakeLists.txt

cmake_minimum_required (VERSION 3.5)

set(PYTHONEXTENSION_VERSION_MAJOR "1")
set(PYTHONEXTENSION_VERSION_MINOR "2")
set(PYTHONEXTENSION_VERSION ${PYTHONEXTENSION_VERSION_MAJOR}.${PYTHONEXTENSION_VERSION_MINOR})
set(PYTHON_VERSION "3.10")
set(PYTHON_VERSION_NO_DOT "310")

# This is what the final library is going to be named
#
project(PythonExtension VERSION ${PYTHONEXTENSION_VERSION} LANGUAGES CXX)

# All string comparisons are CASE SENSITIVE in CMAKE. Make all strings lower before comparisons!
#
string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
string(TOLOWER ${PLATFORM} PLATFORM)

# To build shared libraries in Windows, we set CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS to TRUE.
# See https://cmake.org/cmake/help/v3.4/variable/CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS.html
# See https://blog.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
#
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

file(TO_CMAKE_PATH ${ENL_ROOT}/language-extensions/python PYTHONEXTENSION_HOME)
file(TO_CMAKE_PATH ${PYTHONEXTENSION_HOME}/src PYTHONEXTENSION_SRC_DIR)
file(TO_CMAKE_PATH ${PYTHONEXTENSION_HOME}/include PYTHONEXTENSION_INCLUDE_DIR)
file(TO_CMAKE_PATH ${ENL_ROOT}/extension-host/include EXTENSION_API_INCLUDE_DIR)
file(TO_CMAKE_PATH ${ENL_ROOT}/build-output/pythonextension/${PLATFORM} PYTHONEXTENSION_WORKING_DIR)

# C++ code; only these files are compiled-in
#
file(GLOB PYTHONEXTENSION_SOURCE_FILES ${PYTHONEXTENSION_SRC_DIR}/*.cpp ${PYTHONEXTENSION_SRC_DIR}/${PLATFORM}/*.cpp)

<<<<<<< HEAD
# RHEL 9 ABI floor shims. Required because the Ubuntu 24.04 toolchain references
# glibc symbols newer than 2.34. See
# language-extensions/common/linux/AbiFloorCompat_linux.cpp.
#
if (${PLATFORM} STREQUAL linux)
	list(APPEND PYTHONEXTENSION_SOURCE_FILES ${ENL_ROOT}/language-extensions/common/linux/AbiFloorCompat_linux.cpp)
endif()

# Python helpers are authored as real .py files under src/scripts so they can be linted and
# syntax-checked, then embedded verbatim into the binary as raw string literals. Nothing is
# shipped alongside the extension - see scripts/EmbeddedScripts.h.in.
#
set(NORMALIZE_ZIP_SCRIPT ${PYTHONEXTENSION_SRC_DIR}/scripts/normalize_zip.py)
file(READ ${NORMALIZE_ZIP_SCRIPT} NORMALIZE_ZIP_PY)
configure_file(
	${PYTHONEXTENSION_SRC_DIR}/scripts/EmbeddedScripts.h.in
	${CMAKE_CURRENT_BINARY_DIR}/EmbeddedScripts.h
	@ONLY)

# Re-run CMake when the script changes, otherwise the generated header would go stale.
#
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${NORMALIZE_ZIP_SCRIPT})

# Create the target library
#
add_library(PythonExtension SHARED
	${PYTHONEXTENSION_SOURCE_FILES}
)

set_target_properties(PythonExtension
	PROPERTIES
	VERSION ${PYTHONEXTENSION_VERSION}
	SOVERSION ${PYTHONEXTENSION_VERSION_MAJOR}
)

if (${PLATFORM} STREQUAL linux)
	# Python links libstdc++ dynamically, so RHEL 9's 3.4.29 copy will not
	# provide std::ios_base_library_init(). Ask the shared compat unit to
	# define it.
	#
	target_compile_definitions(PythonExtension PRIVATE ABI_FLOOR_PROVIDE_IOS_BASE_LIBRARY_INIT)

	# Python runtime version used in the linux environment differs from the one used in Windows at this time.
	# This declaration should no longer be needed once the Windows version is updated to 3.12.
	#
	set(PYTHON_VERSION "3.12")
	set(PYTHON_VERSION_NO_DOT "312")

	target_compile_options(PythonExtension PRIVATE -Wall -Wextra -g -O2 -fPIC -Werror -std=c++17 -Wno-unused-parameter -Wno-maybe-uninitialized -fshort-wchar)

	# libstdc++ is deliberately NOT linked statically. The embedded interpreter
	# imports numpy and pandas, whose C extensions link libstdc++.so.6, so a
	# static copy puts two C++ runtimes in one process and the test binary
	# aborts (SIGABRT) during GetOutputParam. RHEL 9 ships libstdc++ 3.4.29, so
	# depending on it is fine as long as nothing above that version is
	# referenced - the one offending symbol is defined locally in
	# language-extensions/common/linux/AbiFloorCompat_linux.cpp.
	#
	# NOTE: the Boost-must-not-be-a-runtime-dependency requirement is NOT enforced
	# by this line. It comes from -DBOOST_USE_STATIC_LIBS (see below) plus the
	# find_library calls, which run AFTER CMAKE_FIND_LIBRARY_SUFFIXES below is set
	# to prefer .so - so if a shared Boost were ever present alongside the static
	# one, the static-only invariant would break silently here. The ELF gate in the
	# superproject is what actually catches that (no libboost_* in DT_NEEDED).
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed -Wl,--export-dynamic -fopenmp")

	# Prefer shared libraries over static libraries
	set(CMAKE_FIND_LIBRARY_SUFFIXES .so .so.1.0 ${CMAKE_FIND_LIBRARY_SUFFIXES})
	
	set(USR_LIB_PATH /usr/local/lib)
	set(PYTHON_LIB_PATH ${PYTHONHOME}/lib/python${PYTHON_VERSION}/config-${PYTHON_VERSION}-x86_64-linux-gnu)

	# Look for shared Python library in system locations
	find_library(PYTHON_LIB python${PYTHON_VERSION} PATHS /usr/lib/x86_64-linux-gnu /usr/lib /lib/x86_64-linux-gnu /lib NO_DEFAULT_PATH)
	if(NOT PYTHON_LIB)
		# Try with explicit .so extension
		find_library(PYTHON_LIB libpython${PYTHON_VERSION}.so PATHS /usr/lib/x86_64-linux-gnu /usr/lib NO_DEFAULT_PATH)
	endif()
	if(NOT PYTHON_LIB)
		message(STATUS "Shared Python library not found in standard locations, trying system default search")
		find_library(PYTHON_LIB python${PYTHON_VERSION})
	endif()
	find_library(BOOST_PYTHON_LIB boost_python${PYTHON_VERSION_NO_DOT} ${BOOST_PYTHON_ROOT})
	find_library(BOOST_NUMPY_LIB boost_numpy${PYTHON_VERSION_NO_DOT} ${BOOST_PYTHON_ROOT})

	file(TO_CMAKE_PATH ${INCLUDE_ROOT}/python${PYTHON_VERSION} PYTHON_INCLUDE)
	file(TO_CMAKE_PATH ${INCLUDE_ROOT}/boost BOOST_INCLUDE)
	file(TO_CMAKE_PATH ${INCLUDE_ROOT}/boost/python BOOST_PYTHON_INCLUDE)

	set(ADDITIONAL_INCLUDES ${PYTHON_INCLUDE} ${BOOST_INCLUDE} ${BOOST_PYTHON_INCLUDE})

	target_link_libraries(PythonExtension 
		stdc++fs
	)
elseif(${PLATFORM} STREQUAL windows)
	add_definitions(-DWIN_EXPORT -D_WIN64 -D_WINDOWS)

	# Compile option /c to compile only
	#
	set(COMPILE_OPTIONS /c /std:c++17)

	find_library(PYTHON_LIB python${PYTHON_VERSION_NO_DOT} ${PYTHONHOME}/libs)

	# MDd is for debug DLL and MD is for release DLL
	#
	if (${CMAKE_BUILD_TYPE} STREQUAL debug)
		set(COMPILE_OPTIONS ${COMPILE_OPTIONS} /MDd)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
		set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG:FULL /OPT:REF /OPT:ICF")
		find_library(BOOST_PYTHON_LIB libboost_python${PYTHON_VERSION_NO_DOT}-vc143-mt-gd-x64-1_90 ${BOOST_PYTHON_ROOT})
		find_library(BOOST_NUMPY_LIB libboost_numpy${PYTHON_VERSION_NO_DOT}-vc143-mt-gd-x64-1_90 ${BOOST_PYTHON_ROOT})
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG:FULL /OPT:REF /OPT:ICF")
		set(COMPILE_OPTIONS ${COMPILE_OPTIONS} /MD)
		find_library(BOOST_PYTHON_LIB libboost_python${PYTHON_VERSION_NO_DOT}-vc143-mt-x64-1_90 ${BOOST_PYTHON_ROOT})
		find_library(BOOST_NUMPY_LIB libboost_numpy${PYTHON_VERSION_NO_DOT}-vc143-mt-x64-1_90 ${BOOST_PYTHON_ROOT})
	endif()

	target_compile_options(PythonExtension PRIVATE ${COMPILE_OPTIONS})

	# Set the DLLEXPORT variable to export symbols
	#
	target_compile_definitions(PythonExtension PRIVATE WIN_EXPORT)

	file(TO_CMAKE_PATH ${PYTHONHOME}/include PYTHON_INCLUDE)
	file(TO_CMAKE_PATH ${BOOST_ROOT} BOOST_INCLUDE)

	set(ADDITIONAL_INCLUDES ${PYTHON_INCLUDE} ${BOOST_INCLUDE})
endif()

add_definitions(-DBOOST_USE_STATIC_LIBS -DBOOST_PYTHON_STATIC_LIB -DBOOST_ALL_NO_LIB -DBOOST_NUMPY_STATIC_LIB)

if (${CMAKE_BUILD_TYPE} STREQUAL debug)
	add_definitions(-D_DEBUG)
else()
	add_definitions(-DNDEBUG)
endif()


target_link_libraries(PythonExtension 
	${PYTHON_LIB} 
	${BOOST_PYTHON_LIB} 
	${BOOST_NUMPY_LIB}
)

target_include_directories(PythonExtension
	PRIVATE ${EXTENSION_API_INCLUDE_DIR} ${PYTHONEXTENSION_INCLUDE_DIR} ${ADDITIONAL_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR}
)

# A syntax error in an embedded script would otherwise surface only at runtime, inside the
# satellite process that an uncaught exception terminates. Compile it at build time instead.
# The copy keeps __pycache__ out of the source tree.
#
find_program(PYTHON_SYNTAX_CHECKER NAMES python3 python)
if (PYTHON_SYNTAX_CHECKER)
	configure_file(${NORMALIZE_ZIP_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/normalize_zip.py COPYONLY)
	add_custom_command(TARGET PythonExtension PRE_BUILD
		COMMAND ${PYTHON_SYNTAX_CHECKER} -m py_compile ${CMAKE_CURRENT_BINARY_DIR}/normalize_zip.py
		COMMENT "Syntax-checking embedded Python: normalize_zip.py")
else()
	message(WARNING "No python interpreter found; embedded scripts will not be syntax-checked.")
endif()

install(TARGETS PythonExtension DESTINATION "${PYTHONEXTENSION_WORKING_DIR}" LIBRARY NAMELINK_ONLY)
