cmake_minimum_required(VERSION 3.1)

enable_testing()

add_subdirectory(3rd_party/catch)
include_directories(../include)

add_executable(unit_tests kvtest.cc)

target_link_libraries(unit_tests
    tikv-cpp-api
    Catch::Test
)

add_test(test_all unit_tests)
