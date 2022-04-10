cmake_minimum_required(VERSION 3.11)

project(
    SocksManyToMany
    VERSION 0.1
    DESCRIPTION "다대다 통신 프로젝트"
    LANGUAGES CXX
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_BUILD_TYPE Debug)
find_package(Threads REQUIRED)

add_executable(program main.cpp myTcp.cpp)

#add_subdirectory(lib)
#target_link_libraries(program PRIVATE Threads::Threads)
target_link_libraries(program PRIVATE pthread)
target_include_directories(program PUBLIC includes)