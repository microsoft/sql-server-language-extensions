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
	target_compile_options(PythonExtension PRIVATE -Wall -Wextra -g -O2 -fPIC -Werror -std=c++17 -Wno-unused-parameter -fshort-wchar)

	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed -Wl,--export-dynamic -fopenmp")

	set(USR_LIB_PATH /usr/local/lib)
	set(PYTHON_LIB_PATH ${PYTHONHOME}/lib/python${PYTHON_VERSION}/config-${PYTHON_VERSION}-x86_64-linux-gnu)

	find_library(PYTHON_LIB python${PYTHON_VERSION} ${PYTHON_LIB_PATH})
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
		find_library(BOOST_PYTHON_LIB libboost_python${PYTHON_VERSION_NO_DOT}-vc140-mt-gd-x64-1_69 ${BOOST_PYTHON_ROOT})
		find_library(BOOST_NUMPY_LIB libboost_numpy${PYTHON_VERSION_NO_DOT}-vc140-mt-gd-x64-1_69 ${BOOST_PYTHON_ROOT})
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG:FULL /OPT:REF /OPT:ICF")
		set(COMPILE_OPTIONS ${COMPILE_OPTIONS} /MD)
		find_library(BOOST_PYTHON_LIB libboost_python${PYTHON_VERSION_NO_DOT}-vc140-mt-x64-1_69 ${BOOST_PYTHON_ROOT})
		find_library(BOOST_NUMPY_LIB libboost_numpy${PYTHON_VERSION_NO_DOT}-vc140-mt-x64-1_69 ${BOOST_PYTHON_ROOT})
	endif()

	target_compile_options(PythonExtension PRIVATE ${COMPILE_OPTIONS})

	# Set the DLLEXPORT variable to export symbols
	#
	target_compile_definitions(PythonExtension PRIVATE WIN_EXPORT)

	file(TO_CMAKE_PATH ${PYTHONHOME}/include PYTHON_INCLUDE)
	file(TO_CMAKE_PATH ${BOOST_ROOT}/include BOOST_INCLUDE)

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
	PRIVATE ${EXTENSION_API_INCLUDE_DIR} ${PYTHONEXTENSION_INCLUDE_DIR} ${ADDITIONAL_INCLUDES} 
)

install(TARGETS PythonExtension DESTINATION "${PYTHONEXTENSION_WORKING_DIR}" LIBRARY NAMELINK_ONLY)
