cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
project(Hello_World)

include_directories(${CMAKE_SOURCE_DIR}/../../fmt/include)

add_executable(hellow_world hello_world.cpp)