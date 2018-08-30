cmake_minimum_required(VERSION 3.1)

project(tikv-cpp-api)

include_directories(SYSTEM)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")

add_subdirectory(src)
add_subdirectory(test)
