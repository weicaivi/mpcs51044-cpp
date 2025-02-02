cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER g++-10)
project(Matrix)

include_directories(${CMAKE_SOURCE_DIR}/../../fmt/include)

add_executable(Matrix Matrix.cpp)
add_executable(PSMatrix PSMatrix.cpp)
add_executable(OverloadMatrix OverloadMatrix.cpp)