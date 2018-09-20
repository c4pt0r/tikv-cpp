
cmake_minimum_required(VERSION 3.1)

find_package(Boost COMPONENTS 
    thread
    system
REQUIRED)

add_library(tikv-cpp-api kv_client.cc pd_client.cc region_cache.cc utils.cc)
set_property(TARGET tikv-cpp-api PROPERTY CXX_STANDARD 11)
target_link_libraries(tikv-cpp-api kvproto ${Boost_LIBRARIES} ssl crypto protobuf grpc++)
target_include_directories(tikv-cpp-api
    PRIVATE 
        ../include
        ../3rd_party
)

