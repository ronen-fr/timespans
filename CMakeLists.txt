cmake_minimum_required(VERSION 3.20)
project(timespan)

set(CMAKE_CXX_STANDARD 20)


find_package(benchmark REQUIRED)
find_package(fmt REQUIRED)

add_executable(timespan main.cpp)

target_link_libraries(timespan benchmark::benchmark fmt::fmt)
