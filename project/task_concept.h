#ifndef TASK_CONCEPT_H
#define TASK_CONCEPT_H

#include <functional>
#include <future>
#include <memory>

namespace mpcs {

// Type-erased task concept
class TaskConcept {
public:
    virtual ~TaskConcept() = default;
    virtual void execute() = 0;
    virtual bool is_completed() const = 0;
    virtual std::unique_ptr<TaskConcept> clone() const = 0;
};

// Concrete implementation for any callable
template<typename F>
class TaskModel : public TaskConcept {
private:
    F func_;
    bool completed_ = false;
public:
    explicit TaskModel(F&& func) : func_(std::forward<F>(func)) {}
    
    void execute() override {
        func_();
        completed_ = true;
    }
    
    bool is_completed() const override {
        return completed_;
    }
    
    std::unique_ptr<TaskConcept> clone() const override {
        return std::make_unique<TaskModel<F>>(F(func_));
    }
};

} // namespace mpcs

#endif // TASK_CONCEPT_H