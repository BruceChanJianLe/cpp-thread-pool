cmake_minimum_required(VERSION 2.8.0)

project(cpp_thread_pool)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads)

add_executable(
    thread_pool_without_return
    thread_pool_without_return.cpp
)
target_link_libraries(thread_pool_without_return ${CMAKE_THREAD_LIBS_INIT})