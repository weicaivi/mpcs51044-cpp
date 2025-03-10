cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
project(Frame2)

include_directories(${CMAKE_SOURCE_DIR}/../../../fmt/include)

add_executable(frame2 frame2.cpp)

