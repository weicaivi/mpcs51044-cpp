#ifndef TASK_CHAIN_H
#define TASK_CHAIN_H

#include <tuple>
#include <type_traits>
#include <utility>
#include "task.h"

namespace mpcs {

namespace detail {

// Helper for the base case with a single task
template<typename T>
auto chain_execute(T&& task) {
    return task();
}

// Helper to chain two tasks - execute the first, then pass its result to the second
template<typename T, typename U>
auto chain_execute(T&& first, U&& second) {
    auto first_result = first();
    return second(first_result);
}

// Helper to chain multiple tasks sequentially
template<typename T, typename U, typename... Rest>
auto chain_execute(T&& first, U&& second, Rest&&... rest) {
    // Execute first task, pass its result to second task
    auto intermediate_result = chain_execute(std::forward<T>(first), std::forward<U>(second));
    
    // Continue chaining with the intermediate result
    if constexpr (sizeof...(Rest) == 0) {
        return intermediate_result;
    } else {
        // Create a lambda to hold the intermediate result
        auto next_task = [result = std::move(intermediate_result)]() {
            return result;
        };
        
        // Continue chaining
        return chain_execute(next_task, std::forward<Rest>(rest)...);
    }
}

} // namespace detail

// Chain tasks together to execute in sequence
template<typename Result, typename... Tasks>
Task<Result> chain_tasks(Tasks&&... tasks) {
    if constexpr (sizeof...(Tasks) == 0) {
        return Task<Result>([]() -> Result { 
            if constexpr (std::is_default_constructible_v<Result>) {
                return Result{};
            } else {
                throw std::runtime_error("Cannot create empty task chain with non-default constructible result type");
            }
        });
    } else {
        return Task<Result>([tasks_tuple = std::make_tuple(std::forward<Tasks>(tasks)...)]() -> Result {
            return std::apply([](auto&&... tasks) {
                return detail::chain_execute(std::forward<decltype(tasks)>(tasks)...);
            }, tasks_tuple);
        });
    }
}

// Helper for void tasks
template<typename... Tasks>
Task<void> chain_void_tasks(Tasks&&... tasks) {
    return Task<void>([tasks_tuple = std::make_tuple(std::forward<Tasks>(tasks)...)]() {
        std::apply([](auto&&... tasks) {
            if constexpr (sizeof...(tasks) > 0) {
                (tasks(), ...); // Execute all tasks in sequence using fold expression
            }
        }, tasks_tuple);
    });
}

// CRTP base class for task executor customization
template<typename Derived>
class TaskExecutor {
public:
    template<typename TaskType>
    void execute_task(TaskType& task) {
        static_cast<Derived*>(this)->before_execution();
        task.execute();
        static_cast<Derived*>(this)->after_execution();
    }
    
    // Default implementations
    void before_execution() {}
    void after_execution() {}
};

// Task iterator for lazy evaluation
template<typename TaskContainer>
class TaskIterator {
private:
    TaskContainer& container_;
    size_t index_ = 0;
    
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = decltype(std::declval<TaskContainer>()[0].get_future());
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;
    
    explicit TaskIterator(TaskContainer& container, size_t start_index = 0) 
        : container_(container), index_(start_index) {}
    
    auto operator*() {
        return container_[index_].get_future();
    }
    
    TaskIterator& operator++() {
        ++index_;
        return *this;
    }
    
    TaskIterator operator++(int) {
        TaskIterator tmp(*this);
        ++(*this);
        return tmp;
    }
    
    bool operator==(const TaskIterator& other) const {
        return &container_ == &other.container_ && index_ == other.index_;
    }
    
    bool operator!=(const TaskIterator& other) const {
        return !(*this == other);
    }
};

} // namespace mpcs

#endif // TASK_CHAIN_H