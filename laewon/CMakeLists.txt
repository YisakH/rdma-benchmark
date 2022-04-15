cmake_minimum_required(VERSION 3.16)

project(RDMAtest)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

add_compile_options("-O2 -pg")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/tools)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/network)

add_executable (rdmaTest ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

target_link_libraries(rdmaTest PRIVATE tools network)
target_compile_features(rdmaTest PRIVATE cxx_std_17)
