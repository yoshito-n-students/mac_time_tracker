cmake_minimum_required(VERSION 3.10)
project(mac_time_tracker)

add_compile_options(-std=c++11)

find_package(
    Boost REQUIRED
    COMPONENTS 
    program_options
)
find_package(
    GTest REQUIRED
)
include(GoogleTest)

include_directories(
    include 
    ${Boost_INCLUDE_DIRS}
)

##############
# Executables

add_executable(
    mac_time_tracker
    src/mac_time_tracker.cpp
)
target_link_libraries(
    mac_time_tracker
    ${Boost_LIBRARIES}
)

########
# Tests

enable_testing()

add_executable(
    address_test
    test/address_test.cpp
)
target_link_libraries(
    address_test
    GTest::GTest
)
gtest_discover_tests(
    address_test
)