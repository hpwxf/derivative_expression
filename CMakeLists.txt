cmake_minimum_required(VERSION 3.16)
project(libKriging_parser)

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
# All include libs are included by default
conan_basic_setup()

set(LIBKRIGING_PARSER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(LIBKRIGING_PARSER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

# ENABLE_STATIC_ANALYSIS default is AUTO
string(TOUPPER "${ENABLE_STATIC_ANALYSIS}" ENABLE_STATIC_ANALYSIS)
if (NOT DEFINED ENABLE_STATIC_ANALYSIS OR ENABLE_STATIC_ANALYSIS STREQUAL "")
    set(ENABLE_STATIC_ANALYSIS "AUTO" CACHE STRING
            "Enable static analysis; choose between ON, OFF and AUTO (if available and Debug mode)"
            FORCE)
else ()
    string(REGEX MATCH "^(ON|OFF|AUTO)$" VALID_STATIC_ANALYSIS "${ENABLE_STATIC_ANALYSIS}")
    if (VALID_STATIC_ANALYSIS STREQUAL "") # /!\ IF(VALID_STATIC_ANALYSIS) is false when ENABLE_STATIC_ANALYSIS is OFF
        logFatalError("Invalid ENABLE_STATIC_ANALYSIS option '${ENABLE_STATIC_ANALYSIS}'; choose between ON, OFF and AUTO.")
    endif ()
endif ()

# search for clang-tidy (while be used while adding library or executable)
find_program(CLANG_TIDY clang-tidy)
if (CLANG_TIDY)
    if (ENABLE_STATIC_ANALYSIS STREQUAL "ON")
        set(CXX_CLANG_TIDY ${CLANG_TIDY})
        message(STATUS "Static analysis requested and enabled while compiling.")
    elseif (ENABLE_STATIC_ANALYSIS STREQUAL "OFF")
        message(STATUS "Static analysis available but disabled as requested.")
    elseif (ENABLE_STATIC_ANALYSIS STREQUAL "AUTO")
        if (CMAKE_BUILD_TYPE MATCHES Debug)
            set(CXX_CLANG_TIDY ${CLANG_TIDY})
            message(STATUS "Static analysis using clang-tidy is enabled while compiling.")
        else ()
            message(STATUS "Static analysis available but disabled in ${CMAKE_BUILD_TYPE} mode.")
        endif ()
    else ()
        message(FATAL_ERROR "INTERNAL ERROR: value '${ENABLE_STATIC_ANALYSIS}' not managed")
    endif ()
else ()
    if (ENABLE_STATIC_ANALYSIS STREQUAL "ON")
        message(FATAL_ERROR "Static analysis requested but not available.")
    elseif (ENABLE_STATIC_ANALYSIS STREQUAL "OFF" OR ENABLE_STATIC_ANALYSIS STREQUAL "AUTO")
        message(STATUS "Static analysis not available.")
    else ()
        message(FATAL_ERROR "INTERNAL ERROR: value '${ENABLE_STATIC_ANALYSIS}' not managed")
    endif ()
endif ()

#-------------------------------------------------------------------------------------------

# Add new build types
# --coverage option is used to compile and link code instrumented for coverage analysis.
# The option is a synonym for -fprofile-arcs -ftest-coverage (when compiling) and -lgcov (when linking).
set(CMAKE_CXX_FLAGS_COVERAGE
        "-g -O0 --coverage"
        CACHE STRING "Flags used by the C++ compiler during coverage builds."
        FORCE)
set(CMAKE_C_FLAGS_COVERAGE
        "-g -O0 --coverage"
        CACHE STRING "Flags used by the C compiler during coverage builds."
        FORCE)
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
        "--coverage"
        CACHE STRING "Flags used for linking binaries during coverage builds."
        FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
        "--coverage"
        CACHE STRING "Flags used by the shared libraries linker during coverage builds."
        FORCE)
mark_as_advanced(
        CMAKE_CXX_FLAGS_COVERAGE
        CMAKE_C_FLAGS_COVERAGE
        CMAKE_EXE_LINKER_FLAGS_COVERAGE
        CMAKE_SHARED_LINKER_FLAGS_COVERAGE)

#-------------------------------------------------------------------------------------------

add_subdirectory(src)

#------------------------------------------------------

# unit tests coverage

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Coverage")
    find_program(LCOV lcov)
    if (NOT LCOV)
        message(FATAL_ERROR "lcov not found, cannot perform coverage.")
    endif ()

    # coveralls.io does not support striped paths
    #find_program (SED NAMES sed)
    #if (NOT SED)
    #    logFatalError("Unable to find sed")
    #else()
    #    # message(STATUS "sed found at ${SED}")
    #endif (NOT SED)

    # Don't forget '' around each pattern
    set(LCOV_EXCLUDE_PATTERN "'${LIBKRIGING_PARSER_SOURCE_DIR}/dependencies/*'")

    add_custom_target(coverage
            # Cleanup previously generated profiling data
            COMMAND ${LCOV} --base-directory ${LIBKRIGING_PARSER_SOURCE_DIR} --directory ${LIBKRIGING_PARSER_BINARY_DIR} --zerocounters
            # Initialize profiling data with zero coverage for every instrumented line of the project
            # This way the percentage of total lines covered will always be correct, even when not all source code files were loaded during the test(s)
            COMMAND ${LCOV} --base-directory ${LIBKRIGING_PARSER_SOURCE_DIR} --directory ${LIBKRIGING_PARSER_BINARY_DIR} --capture --initial --output-file coverage_base.info
            # Run tests
            COMMAND ${CMAKE_CTEST_COMMAND} -j ${PROCESSOR_COUNT}
            # Collect data from executions
            COMMAND ${LCOV} --base-directory ${LIBKRIGING_PARSER_SOURCE_DIR} --directory ${LIBKRIGING_PARSER_BINARY_DIR} --capture --output-file coverage_ctest.info
            # Combine base and ctest results
            COMMAND ${LCOV} --add-tracefile coverage_base.info --add-tracefile coverage_ctest.info --output-file coverage_full.info
            # Extract only project data (--no-capture or --remove options may be used to select collected data)
            COMMAND ${LCOV} --remove coverage_full.info ${LCOV_EXCLUDE_PATTERN} --output-file coverage_filtered.info
            COMMAND ${LCOV} --extract coverage_filtered.info '${LIBKRIGING_PARSER_SOURCE_DIR}/*' --output-file coverage.info
            # coveralls.io does not support striped paths
            #COMMAND ${SED} -i.bak 's|SF:${LIBKRIGING_PARSER_SOURCE_DIR}/|SF:|g' coverage.info
            DEPENDS all_test_binaries
            COMMENT "Running test coverage."
            WORKING_DIRECTORY "${LIBKRIGING_PARSER_BINARY_DIR}"
            )

    find_program(GENHTML genhtml)
    if (NOT GENHTML)
        message(WARNING "genhtml not found, cannot perform report-coverage.")
    else ()
        add_custom_target(coverage-report
                COMMAND ${CMAKE_COMMAND} -E remove_directory "${LIBKRIGING_PARSER_BINARY_DIR}/coverage"
                COMMAND ${CMAKE_COMMAND} -E make_directory "${LIBKRIGING_PARSER_BINARY_DIR}/coverage"
                COMMAND ${GENHTML} -o coverage -t "${CMAKE_PROJECT_NAME} test coverage" --ignore-errors source --legend --num-spaces 4 coverage.info
                COMMAND ${LCOV} --list coverage.info
                DEPENDS coverage
                COMMENT "Building coverage html report."
                WORKING_DIRECTORY "${LIBKRIGING_PARSER_BINARY_DIR}"
                )
    endif ()
else ()
    add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E echo ""
            COMMAND ${CMAKE_COMMAND} -E echo "*** Use CMAKE_BUILD_TYPE=Coverage option in cmake configuration to enable code coverage ***"
            COMMAND ${CMAKE_COMMAND} -E echo ""
            COMMENT "Inform about not available code coverage."
            )
    add_custom_target(coverage-report DEPENDS coverage)
endif ()

#-------------------------------------------------------------------------------------------

set(CATCH_MODULE_PATH "${CONAN_LIB_DIRS_CATCH2}/cmake/Catch2")
include("${CATCH_MODULE_PATH}/ParseAndAddCatchTests.cmake")

enable_testing()
add_subdirectory(tests)

add_executable(main main.cpp)
target_link_libraries(main parser)

add_executable(algebra algebra.cpp)


if (CXX_CLANG_TIDY)
    set_target_properties(parser
            PROPERTIES
            CXX_CLANG_TIDY ${CXX_CLANG_TIDY})
    set_target_properties(algebra
            PROPERTIES
            CXX_CLANG_TIDY ${CXX_CLANG_TIDY})
endif ()