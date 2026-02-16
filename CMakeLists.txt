# ntdda - Discontinuous Deformation Analysis
# CMake build for libdda and unit tests
cmake_minimum_required(VERSION 3.10)
project(ntdda C)

set(CMAKE_C_STANDARD 99)

# Out-of-tree build recommended: mkdir build && cd build && cmake .. && make
option(BUILD_TESTS "Build unit tests" ON)
option(ENABLE_COVERAGE "Build with LLVM source-based coverage" OFF)

add_subdirectory(src)

if(BUILD_TESTS)
  add_subdirectory(tests/unit)
endif()
