cmake_minimum_required(VERSION 3.10)
project(mac_time_tracker)

add_compile_options(-std=c++11)

find_package(
    Boost 
    REQUIRED
    COMPONENTS 
    program_options
)

include_directories(
    include 
    ${Boost_INCLUDE_DIRS}
)

add_executable(
    mac_time_tracker
    src/mac_time_tracker.cpp
)