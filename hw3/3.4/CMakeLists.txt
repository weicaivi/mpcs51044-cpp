cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
project(animal_game)

include_directories(${CMAKE_SOURCE_DIR}/../../fmt/include)

add_executable(animal_game animal_game.cpp)