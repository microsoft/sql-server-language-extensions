# CMakeLists.txt

cmake_minimum_required (VERSION 3.5)

set (PROJECT_VERSION "1.0")

# this is what the final library is going to be named
project(Rextension VERSION ${PROJECT_VERSION} LANGUAGES CXX)

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
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION}
)

target_include_directories(Rextension
    PRIVATE ${REXTENSION_INCLUDE_DIR} ${R_INCLUDE_DIR}
)

set_property(TARGET Rextension PROPERTY INSTALL_RPATH ${REXTENSION_WORKING_DIR})

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    # G++
    target_compile_options(Rextension PRIVATE -Wall -Wextra -g -O2 -fPIC -Werror)
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed -Wl,--export-dynamic -fopenmp")
    find_library(PCRE_LIB pcre ${R_LIB_PATH})
    find_library(LZMA_LIB lzma ${R_LIB_PATH})
    find_library(BZ2_LIB bz2 ${R_LIB_PATH})
    find_library(Z_LIB z ${R_LIB_PATH})
    find_library(RT_LIB rt ${R_LIB_PATH})
    find_library(DL_LIB dl ${R_LIB_PATH})
    find_library(M_LIB m ${R_LIB_PATH})
    target_link_libraries(Rextension ${PCRE_LIB} ${LZMA_LIB} ${BZ2_LIB} ${Z_LIB}
        ${RT_LIB} ${DL_LIB} ${M_LIB}
    )
    target_compile_features(Rextension PRIVATE cxx_std_11)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    # MSVC
    add_definitions(-DWIN_EXPORT -D_WIN64 -D_WINDOWS)
    if (${CMAKE_CONFIGURATION} STREQUAL "Debug")
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

install(TARGETS Rextension DESTINATION ${REXTENSION_WORKING_DIR})
