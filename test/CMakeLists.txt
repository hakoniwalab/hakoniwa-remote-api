# Add test source file
add_executable(remote_api_test remote_api_test.cpp)

# Link the library to the test executable
target_link_libraries(remote_api_test hakoniwa_remote_api)

# Add test to CTest
add_test(NAME remote_api_test COMMAND remote_api_test)
