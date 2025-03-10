cmake_minimum_required(VERSION 3.10)
project(Lecture_8 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-std=c++20)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-std=c++20)
endif()


find_package(Threads REQUIRED)

add_executable(my_promise my_promise.cpp)
target_link_libraries(my_promise PRIVATE Threads::Threads)

find_library(ATOMIC_LIBRARY NAMES atomic atomic.so.1 libatomic.so.1)
if(ATOMIC_LIBRARY)
    target_link_libraries(my_promise PRIVATE ${ATOMIC_LIBRARY})
endif()