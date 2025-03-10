# Task Scheduler with Thread Pool

This project implements a thread pool and task scheduling system using modern C++ techniques.

- Templates and type erasure
- Move semantics and RAII
- Operator overloading
- Thread synchronization primitives
- Smart pointers
- CRTP (Curiously Recurring Template Pattern)
- Custom allocators

## Project Structure

- `task_concept.h` - Type-erased interface for tasks
- `task.h` - Task wrapper with type erasure and priority support
- `thread_pool.h/cpp` - Thread pool implementation for parallel task execution
- `task_scheduler.h/cpp` - High-level scheduler for managing tasks
- `task_graph.h` - Dependency graph for ordered task execution
- `task_chain.h` - Tools for chaining tasks together
- `task_allocator.h` - Custom memory allocator for tasks
- `main.cpp` - Demo application showing various features

## Features

1. **Type-erased Tasks**: Store any callable in a type-safe manner
2. **Priority-based Execution**: Assign priorities to tasks
3. **Task Chaining**: Create pipelines of dependent tasks
4. **Task Graph**: Define complex task dependencies
5. **Custom Task Execution**: Extend task execution with CRTP
6. **Repeating Tasks**: Schedule tasks to execute periodically
7. **Memory Management**: Efficient allocation with custom allocator
8. **Thread Safety**: All operations are thread-safe

## Building and Running

### Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 10+, or MSVC 19.28+)

### Compilation

```bash
# Using make
make

# Manual compilation
g++ -std=c++20 -Wall -Wextra -pedantic -pthread main.cpp thread_pool.cpp task_scheduler.cpp -o task_scheduler
```

### Running

```bash
# Using make
make run

# Manual execution
./task_scheduler
```

## Usage Examples

### Basic Task Submission

```cpp
ThreadPool pool(4);  // Create a thread pool with 4 threads

// Submit a task and get a future
auto future = pool.submit([]() {
    return 42;
});

// Get the result
int result = future.get();  // result = 42
```

### Task with Priority

```cpp
TaskScheduler scheduler(4);

// Schedule a high-priority task
auto high_priority = scheduler.schedule_with_priority([]() {
    return "Important task";
}, 10);

// Schedule a low-priority task
auto low_priority = scheduler.schedule_with_priority([]() {
    return "Less important task";
}, 1);

// High priority task likely executes first
```

### Task Chaining

```cpp
// Create a chain of tasks where each task's result feeds into the next
auto task_chain = chain_tasks<int>(
    []() { return 1; },                 // First task returns 1
    [](int i) { return i + 2; },        // Add 2 to previous result
    [](int i) { return i * 3; }         // Multiply previous result by 3
);

auto future = scheduler.schedule(std::move(task_chain));
int result = future.get();  // result = (1 + 2) * 3 = 9
```

### Task Graph with Dependencies

```cpp
TaskGraph<void> graph;

// Add tasks
graph.add_task("task1", []() { std::cout << "Task 1\n"; });
graph.add_task("task2", []() { std::cout << "Task 2\n"; });
graph.add_task("task3", []() { std::cout << "Task 3\n"; });

// Add dependencies: task3 depends on both task1 and task2
graph.add_dependency("task3", "task1");
graph.add_dependency("task3", "task2");

// Execute graph - task1 and task2 can run in parallel, task3 waits for both
scheduler.schedule_graph(graph);
```
