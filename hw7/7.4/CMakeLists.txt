cmake_minimum_required(VERSION 3.10)
project(Lecture_7 CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)

add_executable(promise promise.cpp)

target_link_libraries(promise PRIVATE Threads::Threads)