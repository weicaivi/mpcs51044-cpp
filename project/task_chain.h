#ifndef TASK_CHAIN_H
#define TASK_CHAIN_H

#include <tuple>
#include <type_traits>
#include <utility>
#include "task.h"

namespace mpcs {

namespace detail {

// Helper to execute tasks in sequence
template<typename T, typename Arg, typename... Rest>
auto chain_execute(T&& first, Arg&& arg, Rest&&... rest) {
    // Execute first task with the argument
    auto result = first(std::forward<Arg>(arg));
    
    if constexpr (sizeof...(Rest) == 0) {
        return result;
    } else {
        // Pass the result to the next task in the chain
        return chain_execute(std::forward<Rest>(rest)..., std::move(result));
    }
}

// Base case with just a single task
template<typename T>
auto chain_execute(T&& task) {
    return task();
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
            return std::apply([](auto&&... args) {
                return detail::chain_execute(std::forward<decltype(args)>(args)...);
            }, tasks_tuple);
        });
    }
}

// Specialized version for void result type
template<typename... Tasks>
Task<void> chain_tasks_void(Tasks&&... tasks) {
    if constexpr (sizeof...(Tasks) == 0) {
        return Task<void>([]() {});
    } else {
        return Task<void>([tasks_tuple = std::make_tuple(std::forward<Tasks>(tasks)...)]() {
            std::apply([](auto&&... args) {
                detail::chain_execute(std::forward<decltype(args)>(args)...);
            }, tasks_tuple);
        });
    }
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