cmake_minimum_required(VERSION 3.5)
project(cpp_csp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)

IF(MSVC)
	set(LIB_FILES "")
ENDIF()

IF(NOT MSVC)
	set(LIB_FILES "pthread")
ENDIF()

include_directories(include)

add_subdirectory(src)