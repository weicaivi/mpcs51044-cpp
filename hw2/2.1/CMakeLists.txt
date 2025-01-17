cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
project(square)

include_directories(${CMAKE_SOURCE_DIR}/../../fmt/include)

add_executable(vector_squared vector_squared.cpp)