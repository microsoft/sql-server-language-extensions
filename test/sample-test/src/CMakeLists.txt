cmake_minimum_required (VERSION 3.5)

# this is what the final executable is going to be named
project(sample-test VERSION 8.0 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SAMPLETEST_CXXFLAGS}")

# this is not a standard include path so test projects need
# to add it explicitly
include_directories(
  "${GTEST_INCLUDE_DIR}"
)

# C++ unit tests code; only these files are compiled-in
set(SAMPLETEST_SOURCE_FILES
  main.cpp
  sanitytest.cpp
)

add_executable(sample-test
  ${SAMPLETEST_SOURCE_FILES}
)

set_property(TARGET sample-test PROPERTY INSTALL_RPATH ${SAMPLETEST_WORKING_DIR})

find_library(GTEST gtest ${GTEST_LIB_PATH})

target_link_libraries(sample-test
  ${GTEST}
)

install(TARGETS sample-test DESTINATION ${SAMPLETEST_WORKING_DIR})
