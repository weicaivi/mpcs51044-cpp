cmake_minimum_required(VERSION 3.10)
project(Task_Scheduler CXX)

# Set C++20 standard without using the newer toolchains feature
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler-specific flags directly
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-Wall -Wextra -pthread)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -pthread)
elseif(MSVC)
    add_compile_options(/W4)
endif()

# Find the threads package
find_package(Threads REQUIRED)

# Source files
set(SOURCES
    main.cpp
    thread_pool.cpp
    task_scheduler.cpp
)

# Header files
set(HEADERS
    task_concept.h
    task.h
    thread_pool.h
    task_graph.h
    task_chain.h
    task_scheduler.h
    task_allocator.h
)

# Create executable
add_executable(main ${SOURCES})
target_link_libraries(main PRIVATE Threads::Threads)

# Add atomic library if available (for older GCC that needs it linked explicitly)
find_library(ATOMIC_LIBRARY NAMES atomic atomic.so.1 libatomic.so.1)
if(ATOMIC_LIBRARY)
    target_link_libraries(main PRIVATE ${ATOMIC_LIBRARY})
endif()

# Add filesystem library if using older compilers that need it linked explicitly
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(main PRIVATE stdc++fs)
endif()