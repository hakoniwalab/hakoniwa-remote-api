# Add test source file
add_executable(dummy_test dummy_test.cpp)

# Link the library to the test executable
target_link_libraries(dummy_test hakoniwa_remote_api)

# Add test to CTest
add_test(NAME dummy_test COMMAND dummy_test)
