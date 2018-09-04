cmake_minimum_required(VERSION 3.1)

project(tikv-cpp-api)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


include_directories(SYSTEM)
include_directories(./3rd_party)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(Boost REQUIRED COMPONENTS thread)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

add_subdirectory(3rd_party/cpp-btree)
add_subdirectory(kvproto)
add_subdirectory(src)

enable_testing()
add_subdirectory(test)
