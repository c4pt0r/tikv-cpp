cmake_minimum_required(VERSION 3.1)

add_subdirectory(3rd_party/catch)
include_directories(../include)

add_executable(unit_tests 
    mocktikv.cc
    region_cache_test.cc
    pd_client_test.cc)

target_link_libraries(unit_tests
    tikv-cpp-api
    Catch::Test
)

add_test(test_all unit_tests)
