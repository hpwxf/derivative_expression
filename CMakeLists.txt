cmake_minimum_required(VERSION 3.16)
project(libKriging_parser)

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
# All include libs are included by default
conan_basic_setup()

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


add_subdirectory(src)

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