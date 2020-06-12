enable_testing()

# Build a list of test binaries
add_custom_target(all_test_binaries COMMENT "all unit tests.")

add_executable(trivial test_trivial.cpp)
target_link_libraries(trivial LINK_PUBLIC parser)
add_dependencies(all_test_binaries trivial)

add_executable(parse test_parse.cpp)
target_link_libraries(parse LINK_PUBLIC parser)
add_dependencies(all_test_binaries parse)

ParseAndAddCatchTests(trivial)
ParseAndAddCatchTests(parse)