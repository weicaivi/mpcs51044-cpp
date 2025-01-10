cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
project(PersonalizedHello)

include_directories(${CMAKE_SOURCE_DIR}/../../../fmt/include)

add_executable(personalized_hello personalized_hello.cpp)
