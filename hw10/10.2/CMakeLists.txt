cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
project(Hurricane)

include_directories(${CMAKE_SOURCE_DIR}/../../fmt/include)

add_executable(Hurricane Hurricane.cpp)

configure_file(${CMAKE_SOURCE_DIR}/hurdat_atlantic_1851-2011.txt
               ${CMAKE_BINARY_DIR}/hurdat_atlantic_1851-2011.txt COPYONLY)