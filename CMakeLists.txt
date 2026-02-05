cmake_minimum_required(VERSION 3.10)
project(hakoniwa_remote_api CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
  /usr/local/hakoniwa/include/hakoniwa/pdu
)

if(APPLE)
  set(HAKONIWA_LIB_DIRS /usr/local/hakoniwa/lib)
else()
  set(HAKONIWA_LIB_DIRS /usr/local/hakoniwa/lib /usr/lib/x86_64-linux-gnu)
endif()

set(HAKONIWA_LINK_LIBS "")
foreach(lib_name shakoc hako assets conductor)
  find_library(HAKONIWA_${lib_name}_LIB NAMES ${lib_name} PATHS ${HAKONIWA_LIB_DIRS})
  if(HAKONIWA_${lib_name}_LIB)
    list(APPEND HAKONIWA_LINK_LIBS ${HAKONIWA_${lib_name}_LIB})
  else()
    list(APPEND HAKONIWA_LINK_LIBS ${lib_name})
  endif()
endforeach()

find_package(hakoniwa_pdu_endpoint CONFIG REQUIRED)
find_package(hakoniwa_pdu_rpc CONFIG REQUIRED)

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
    /usr/local/hakoniwa/include/hakoniwa
    messages/impl/types
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    hakoniwa_pdu_rpc
    ${HAKONIWA_LINK_LIBS}
)




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
    hakoniwa_pdu_endpoint
)
target_link_directories(server
  PRIVATE
    ${HAKONIWA_LIB_DIRS}
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
    hakoniwa_pdu_endpoint
)
target_link_directories(client
  PRIVATE
    ${HAKONIWA_LIB_DIRS}
)
