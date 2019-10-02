# CMakeLists.txt

cmake_minimum_required (VERSION 3.5)

set(REXTENSION_VERSION_MAJOR "1")
set(REXTENSION_VERSION_MINOR "0")
set(REXTENSION_VERSION ${REXTENSION_VERSION_MAJOR}.${REXTENSION_VERSION_MINOR})

# this is what the final library is going to be named
project(Rextension VERSION ${REXTENSION_VERSION} LANGUAGES CXX)

# To build shared libraries in Windows, we set CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS to TRUE.
# See https://cmake.org/cmake/help/v3.4/variable/CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS.html
# See https://blog.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# C++ code; only these files are compiled-in
file(TO_CMAKE_PATH ${REXTENSION_SRC_DIR} REXTENSION_SRC_CMAKE_DIR)
set(REXTENSION_SOURCE_FILES
    ${REXTENSION_SRC_CMAKE_DIR}/Rextension.cpp
)

# Create the target library
add_library(Rextension SHARED
    ${REXTENSION_SOURCE_FILES}
)

set_target_properties(Rextension
    PROPERTIES
    VERSION ${REXTENSION_VERSION}
    SOVERSION ${REXTENSION_VERSION_MAJOR}
)

target_include_directories(Rextension
    PRIVATE ${REXTENSION_INCLUDE_DIR} ${R_INCLUDE_DIR}
)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    # G++
    target_compile_options(Rextension PRIVATE -Wall -Wextra -g -O2 -fPIC -Werror)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed -Wl,--export-dynamic -fopenmp")
    set(USR_LIB_PATH "/usr/local/lib")
    find_library(PCRE_LIB pcre ${USR_LIB_PATH})
    find_library(LZMA_LIB lzma ${USR_LIB_PATH})
    find_library(BZ2_LIB bz2 ${USR_LIB_PATH})
    find_library(Z_LIB z ${USR_LIB_PATH})
    find_library(RT_LIB rt ${USR_LIB_PATH})
    find_library(DL_LIB dl ${USR_LIB_PATH})
    find_library(M_LIB m ${USR_LIB_PATH})
    target_link_libraries(Rextension ${PCRE_LIB} ${LZMA_LIB} ${BZ2_LIB} ${Z_LIB}
        ${RT_LIB} ${DL_LIB} ${M_LIB}
    )
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    # MSVC
    add_definitions(-DWIN_EXPORT -D_WIN64 -D_WINDOWS)
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_definitions(-DDEBUG)
    endif()
    target_compile_options(Rextension PRIVATE /EHsc /MTd /W2 /c -O3 -Wall -pedantic _CRT_SECURE_NO_WARNINGS)
    # Set the DLLEXPORT variable to export symbols
    target_compile_definitions(Rextension PRIVATE WIN_EXPORT)
endif()

find_library(R_LIB R ${R_LIB_DIR} )
target_link_libraries(Rextension
  ${R_LIB}
)

install(TARGETS Rextension DESTINATION "${REXTENSION_WORKING_DIR}" LIBRARY NAMELINK_ONLY)
