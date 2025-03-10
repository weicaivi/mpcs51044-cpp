cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(ThreadSafeStack)

add_executable(mpcs51044_stack mpcs51044_stack.cpp)

find_package(Threads REQUIRED)
target_link_libraries(mpcs51044_stack PRIVATE Threads::Threads)