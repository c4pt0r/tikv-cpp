cmake_minimum_required(VERSION 3.1)

project(tikv-cpp-api)

include_directories(SYSTEM)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(Boost REQUIRED COMPONENTS thread)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

add_subdirectory(3rd_party/cpp-btree)
add_subdirectory(src)

enable_testing()
add_subdirectory(test)
