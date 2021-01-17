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

add_executable(
    csv_test
    test/csv_test.cpp
)
target_link_libraries(
    csv_test
    GTest::GTest
)
gtest_discover_tests(
    csv_test
)

add_executable(
    set_test
    test/set_test.cpp
)
target_link_libraries(
    set_test
    GTest::GTest
)
gtest_discover_tests(
    set_test
)

add_executable(
    time_test
    test/time_test.cpp
)
target_link_libraries(
    time_test
    GTest::GTest
)
gtest_discover_tests(
    time_test
)