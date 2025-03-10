cmake_minimum_required(VERSION 3.10)
project(Lecture_6 CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(counter_benchmark counter_benchmark.cpp)

find_package(Threads REQUIRED)
target_link_libraries(counter_benchmark PRIVATE Threads::Threads)