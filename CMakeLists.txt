cmake_minimum_required(VERSION 3.9)
project(cpp_csp)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake ${CMAKE_MODULE_PATH})

option(ENABLE_DOC "Generates the documentation target" OFF)
option(ENABLE_COVERAGE "Generates the coverage build" OFF)
option(ENABLE_TESTING "Turns on testing" OFF)
option(ENABLE_BENCHMARKING "Turns on benchmarking" OFF)

set(CMAKE_CXX_STANDARD 17)
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

if (ENABLE_BENCHMARKING)
    set(BENCHMARK_ENABLE_TESTING OFF)
    set(BENCHMARK_ENABLE_INSTALL OFF)
    set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
    set(BENCHMARK_ENABLE_ASSEMBLY_TESTS OFF)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
    add_subdirectory(extern/benchmark)
    include_directories(extern/benchmark/include)
    add_subdirectory(benchmarks)
endif()

option(BUILD_DOC "Build Documentation" OFF)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target(doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
else(DOXYGEN_FOUND)
    message("Doxygen needs to be installed to generate the doxygen documentation")
endif(DOXYGEN_FOUND)

add_subdirectory(demos)