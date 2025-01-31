cmake_minimum_required(VERSION 3.5.0)
project(Lecture_3 VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR}/../../fmt/include)

add_executable(animal_game animal_game.cpp)
add_executable(animal_game_copy animal_game_copy.cpp)
add_executable(EmptyHello  EmptyHello.cpp)
add_executable(EmptyHelloFancy  EmptyHelloFancy.cpp)
add_executable(Nth_power Nth_power.cpp)
add_executable(Matrix Matrix.cpp)
add_executable(PSMatrix PSMatrix.cpp)
add_executable(OverloadMatrix OverloadMatrix.cpp)

