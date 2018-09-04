cmake_minimum_required(VERSION 3.1)

project(kvproto CXX)
add_definitions(-std=c++11)
set(CMAKE_CXX_FLAGS "-g -O2")

file(GLOB kvproto_SRC
    "*.h"
    "*.cc"
)

add_library(kvproto ${kvproto_SRC})

