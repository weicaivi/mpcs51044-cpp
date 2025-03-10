cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
project(Thread_Counter)

include_directories(${CMAKE_SOURCE_DIR}/../../fmt/include)

add_executable(thread_counter thread_counter.cpp)

find_package(Threads REQUIRED)
target_link_libraries(thread_counter PRIVATE Threads::Threads)
