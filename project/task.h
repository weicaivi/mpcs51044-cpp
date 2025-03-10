#ifndef TASK_H
#define TASK_H

#include <memory>
#include <future>
#include <functional>
#include <vector>
#include <type_traits>
#include "task_concept.h"

namespace mpcs {

// Task wrapper with generic result type
template<typename Result = void>
class Task {
private:
    std::unique_ptr<TaskConcept> impl_;
    std::shared_ptr<std::promise<Result>> promise_;
    int priority_ = 0;
    std::vector<Task<void>> dependencies_;

public:
    // Default constructor
    Task() = default;

    // Constructor from any callable that returns Result
    template<typename F, 
             typename = std::enable_if_t<std::is_convertible_v<std::invoke_result_t<F>, Result>>>
    explicit Task(F&& func) {
        auto promise_ptr = std::make_shared<std::promise<Result>>();
        
        auto wrapper = [f = std::forward<F>(func), p = promise_ptr]() {
            try {
                if constexpr (std::is_void_v<Result>) {
                    f();
                    p->set_value();
                } else {
                    p->set_value(f());
                }
            } catch (...) {
                p->set_exception(std::current_exception());
            }
        };
        
        impl_ = std::make_unique<TaskModel<std::function<void()>>>(std::move(wrapper));
        promise_ = std::move(promise_ptr);
    }
    
    // Copy constructor
    Task(const Task& other) 
        : impl_(other.impl_ ? other.impl_->clone() : nullptr),
          promise_(other.promise_),
          priority_(other.priority_),
          dependencies_(other.dependencies_) {}
    
    // Move constructor
    Task(Task&& other) noexcept = default;
    
    // Copy assignment
    Task& operator=(const Task& other) {
        if (this != &other) {
            impl_ = other.impl_ ? other.impl_->clone() : nullptr;
            promise_ = other.promise_;
            priority_ = other.priority_;
            dependencies_ = other.dependencies_;
        }
        return *this;
    }
    
    // Move assignment
    Task& operator=(Task&& other) noexcept = default;
    
    // Get future to access the result
    std::future<Result> get_future() const {
        if (!promise_) {
            throw std::runtime_error("Task has no associated promise");
        }
        return promise_->get_future();
    }
    
    // Priority operations with operator overloading
    Task& operator<<(int priority) {
        priority_ = priority;
        return *this;
    }
    
    bool operator<(const Task& other) const {
        return priority_ < other.priority_;
    }
    
    bool operator>(const Task& other) const {
        return priority_ > other.priority_;
    }
    
    // Dependency chaining with operator overloading
    Task& operator>>(Task<void>& dependency) {
        dependencies_.push_back(dependency);
        return *this;
    }
    
    // Execute the task with its dependencies
    void execute() {
        if (!impl_) return;
        
        // Execute all dependencies first
        for (auto& dep : dependencies_) {
            dep.execute();
        }
        impl_->execute();
    }
    
    // Check if the task is completed
    bool is_completed() const {
        return impl_ ? impl_->is_completed() : true;
    }
    
    // Get the task priority
    int priority() const {
        return priority_;
    }
};

// Helper function to create a task from a callable
template<typename F>
auto make_task(F&& func) {
    using result_type = std::invoke_result_t<F>;
    return Task<result_type>(std::forward<F>(func));
}

} // namespace mpcs

#endif // TASK_H