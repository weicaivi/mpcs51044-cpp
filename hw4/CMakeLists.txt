cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Matrix)

include_directories(${CMAKE_SOURCE_DIR}/../../fmt/include)

add_executable(Matrix Matrix.cpp)
add_executable(PSMatrix PSMatrix.cpp)
add_executable(OverloadMatrix OverloadMatrix.cpp)

add_custom_target(run_all
    COMMAND Matrix
    COMMAND PSMatrix
    COMMAND OverloadMatrix
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS Matrix PSMatrix OverloadMatrix
)