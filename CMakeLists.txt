cmake_minimum_required(VERSION 3.9)
project(cpp_csp)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake ${CMAKE_MODULE_PATH})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../lib")

option(ENABLE_DOC "Generates the documentation target" OFF)
option(ENABLE_COVERAGE "Generates the coverage build" OFF)
option(ENABLE_TESTING "Turns on testing" OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include_directories(include)

# Enable code testing with -DENABLE_TESTING=1
if (ENABLE_TESTING OR ENABLE_COVERAGE)
    enable_testing()
    # Prevent overriding the parent project's compiler/linker
    # settings on WindowsS
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(extern/googletest)
    include_directories(extern/googletest/googletest/include extern/googletest/googlemock/include)
    add_subdirectory(test)
endif()

add_subdirectory(demos)