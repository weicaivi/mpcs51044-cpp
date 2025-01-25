cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
project(NthPower)

include_directories(${CMAKE_SOURCE_DIR}/../../fmt/include)

add_executable(Nth_power Nth_power.cpp)