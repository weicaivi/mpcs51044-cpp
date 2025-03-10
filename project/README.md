# Task Scheduler with Thread Pool

This project Task Scheduler  implements a thread pool and task scheduling system using modern C++ techniques. It provides a flexible framework for parallel execution of tasks with features such as:

- Thread pool for concurrent task execution
- Type-erased task containers that can hold any callable
- Task prioritization with customizable scheduling
- Task dependency graphs for complex workflows
- Task chaining for composable operations
- Thread-safe execution with proper synchronization
- Custom memory allocation for performance optimization

## Components

### Core Components

- **TaskConcept/TaskModel**: Implements type erasure to store and execute any callable
- **Task**: Generic task wrapper supporting priorities and dependencies
- **MPCSThreadPool**: Thread pool implementation for parallel task execution
- **TaskScheduler**: High-level interface for scheduling and managing tasks
- **TaskGraph**: Dependency-based execution of task networks
- **TaskChain**: Sequential execution of interdependent tasks
- **TaskAllocator**: Custom memory allocation for task objects

## Building and Running

### Prerequisites

- C++20 compatible compiler
- CMake 3.10 or higher
- Threads library

### Build

```bash
mkdir build
cd build
cmake ..
make
```

### Running

```bash
./main
```

## Demonstrations

The project includes several demonstrations of the task scheduling system:

1. **Basic Task Submission**: Simple task execution with the thread pool
2. **Task Priorities**: Prioritized execution of tasks
3. **Task Chaining**: Sequential execution where results flow from one task to the next
4. **Task Graph with Dependencies**: Complex dependency-based execution of task networks
5. **Custom Task Executor**: Extending task execution with CRTP
6. **Repeating Tasks**: Schedule tasks to execute periodically
7. **Task Allocator**: Efficient memory management for tasks
8. **Performance Benchmarks**: Thread count performance comparison

## Implementation Details

### Type Erasure Pattern

The type erasure pattern is used to store any callable in a type-safe manner:

```cpp
class TaskConcept {
public:
    virtual ~TaskConcept() = default;
    virtual void execute() = 0;
    virtual bool is_completed() const = 0;
    virtual std::unique_ptr<TaskConcept> clone() const = 0;
};

template<typename F>
class TaskModel : public TaskConcept {
private:
    std::shared_ptr<F> func_;
    bool completed_ = false;
public:
    // Implementation details...
};
```

### Thread Synchronization

Thread safety is achieved through careful use of mutexes, condition variables, and atomic operations:

```cpp
std::mutex queue_mutex_;
std::condition_variable condition_;
std::atomic<size_t> active_tasks_;
```

### Move Semantics

Move semantics are used extensively for efficient transfer of resources:

```cpp
tasks_.emplace([task = std::move(task), this]() mutable {
    task.execute();
    decrementActiveTask();
});
```

### Template Programming

Templates enable generic programming while preserving type safety:

```cpp
template<typename Result, typename... Tasks>
Task<Result> chain_tasks(Tasks&&... tasks) {
    // Implementation details...
}
```

## Usage Examples

### Basic Task Submission

```cpp
ThreadPool pool(4);
auto future = pool.submit([]() {
    return 42;
});
int result = future.get();  // result = 42
```

### Task with Priority

```cpp
TaskScheduler scheduler(4);
auto high_priority = scheduler.schedule_with_priority([]() {
    return "Important task";
}, 10);
```

### Task Chaining

```cpp
auto task_chain = chain_tasks<int>(
    []() { return 1; },                 // First task returns 1
    [](int i) { return i + 2; },        // Add 2 to previous result
    [](int i) { return i * 3; }         // Multiply previous result by 3
);
```

### Task Graph

```cpp
TaskGraph<void> graph;
graph.add_task("task1", []() { std::cout << "Task 1\n"; });
graph.add_task("task2", []() { std::cout << "Task 2\n"; });
graph.add_dependency("task2", "task1");  // task2 depends on task1
scheduler.schedule_graph(graph);
```