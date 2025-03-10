cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
project(PascalsTriangle)

include_directories(${CMAKE_SOURCE_DIR}/../../fmt/include)

add_executable(pascals_triangle pascals_triangle.cpp)