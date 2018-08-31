
cmake_minimum_required(VERSION 3.1)

file(GLOB kvproto_SRC
    "../kvproto/*.h"
    "../kvproto/*.cc"
)

add_library(tikv-cpp-api ${kvproto_SRC} client.cc pd_client.cc)
set_property(TARGET tikv-cpp-api PROPERTY CXX_STANDARD 11)
target_link_libraries(tikv-cpp-api boost_system boost_thread-mt ssl crypto protobuf grpc++)
target_include_directories(tikv-cpp-api
    PRIVATE 
        ../include
)

