# CMakeList.txt : CMake project for AZLAutopatcher, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project ("AZLAutopatcher")

set(BUILD_SHARED_LIBS OFF)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

set(CMAKE_EXE_LINKER_FLAGS "-static")

# Add source to this project's executable.
add_executable (AZLAutopatcher "AZLAutopatcher.cpp" "AZLAutopatcher.h")

# TODO: Add tests and install targets if needed.
