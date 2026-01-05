cmake_minimum_required(VERSION 3.10)
project(hakoniwa_remote_api CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Add the source files to the library
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_library(${PROJECT_NAME} ${SOURCES})

# Tests
enable_testing()
add_subdirectory(test)
