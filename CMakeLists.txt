cmake_minimum_required(VERSION 3.10)
project(hakoniwa_remote_api CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add submodule
# The second argument specifies the build directory for the submodule, preventing clutter in its source tree
add_subdirectory(hakoniwa-pdu-rpc build/hakoniwa-pdu-rpc)

# Add the source files to the library

add_library(${PROJECT_NAME}
  src/server/server_core.cpp
  src/server/concrete_service_handler.cpp
  src/client/client_core.cpp
)

# Target-specific include directories
target_include_directories(${PROJECT_NAME}
  PUBLIC
    include
    /usr/local/hakoniwa/include
    messages/impl/types
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


include(FetchContent)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json)
add_executable(server 
  src/main/server.cpp
)
target_include_directories(server
  PUBLIC
    include
    /usr/local/hakoniwa/include
    messages/impl/types
)
target_link_libraries(server
  PRIVATE
    ${PROJECT_NAME}
    nlohmann_json::nlohmann_json
    hakoniwa_pdu_rpc
    shakoc
)
target_link_directories(server
  PRIVATE
    /usr/local/hakoniwa/lib
)

add_executable(client 
  src/main/client.cpp
)
target_include_directories(client
  PUBLIC
    include
    /usr/local/hakoniwa/include
    messages/impl/types
)
target_link_libraries(client
  PRIVATE
    ${PROJECT_NAME}
    nlohmann_json::nlohmann_json
    hakoniwa_pdu_rpc
    shakoc
)
target_link_directories(client
  PRIVATE
    /usr/local/hakoniwa/lib
)