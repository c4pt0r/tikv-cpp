cmake_minimum_required(VERSION 3.1)

add_subdirectory(3rd_party/catch)

include_directories(../include)

add_executable(unit_tests 
    mocktikv.cc
    region_cache_test.cc
    mocktikv_test.cc
    region_test.cc
    pd_client_test.cc)

target_link_libraries(unit_tests
    tikv-cpp-api
    jsoncpp_lib_static
    Catch::Test
)

add_test(test_all unit_tests)
