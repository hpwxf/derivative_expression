enable_testing()

# Build a list of test binaries
add_custom_target(all_test_binaries COMMENT "all unit tests.")

add_executable(test_echo test_echo.cpp)
target_link_libraries(test_echo LINK_PUBLIC parser)
add_dependencies(all_test_binaries test_echo)

ParseAndAddCatchTests(test_echo)