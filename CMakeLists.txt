cmake_minimum_required(VERSION 3.10)
project(hakoniwa_remote_api CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add submodule
# The second argument specifies the build directory for the submodule, preventing clutter in its source tree
add_subdirectory(hakoniwa-pdu-rpc build/hakoniwa-pdu-rpc)

# Add the source files to the library
file(GLOB_RECURSE SOURCES "src/**/*.cpp")
add_library(${PROJECT_NAME} ${SOURCES})

# Target-specific include directories
target_include_directories(${PROJECT_NAME}
  PUBLIC
    include
    /usr/local/hakoniwa/include
)

# Link against the submodule library
# This also brings in its public include directories automatically
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    hakoniwa_pdu_rpc
)


# Tests
enable_testing()
add_subdirectory(test)
