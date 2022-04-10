cmake_minimum_required(VERSION 3.16)

set(CMAKE_BUILD_TYPE Debug)

project(benchmark)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/tools)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/network)

add_executable (benchmark ${CMAKE_CURRENT_SOURCE_DIR}/benchmark.cpp)

target_link_libraries(benchmark PRIVATE tools network)
target_include_directories(benchmark PUBLIC ${CMAKE_SOURCE_DIR}/includes)
target_compile_features(benchmark PRIVATE cxx_std_17)
