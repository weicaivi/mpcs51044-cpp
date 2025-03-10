#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <future>
#include "thread_pool.h"
#include "task_graph.h"
#include "task.h"

namespace mpcs {

class TaskScheduler {
private:
    ThreadPool pool_;
    std::unordered_map<std::string, std::vector<std::string>> task_groups_;
    std::unordered_map<std::string, std::vector<std::future<void>>> group_futures_;
    
public:
    // Constructor with specified number of threads
    explicit TaskScheduler(size_t num_threads = std::thread::hardware_concurrency());
    
    // Schedule a single task
    template<typename F>
    auto schedule(F&& func) {
        return pool_.submit(std::forward<F>(func));
    }
    
    // Schedule a task with priority
    template<typename F>
    auto schedule_with_priority(F&& func, int priority) {
        using result_type = std::invoke_result_t<F>;
        Task<result_type> task(std::forward<F>(func));
        task << priority;
        return pool_.submit(std::move(task));
    }
    
    // Schedule a task with timeout
    template<typename F, typename Rep, typename Period>
    auto schedule_with_timeout(F&& func, const std::chrono::duration<Rep, Period>& timeout) {
        auto future = pool_.submit(std::forward<F>(func));
        
        // Create a separate function to wait with timeout
        auto wait_with_timeout = [future = std::move(future), timeout]() mutable {
            if (future.wait_for(timeout) == std::future_status::timeout) {
                // Handle timeout - could throw exception or return error status
                throw std::runtime_error("Task timed out");
            }
            return future.get();
        };
        
        // Return a future for the timeout-wrapped task
        return pool_.submit(wait_with_timeout);
    }
    
    // Schedule a group of tasks
    template<typename F>
    void schedule_group(const std::string& group_name, std::vector<F> functions) {
        task_groups_[group_name] = {};
        group_futures_[group_name] = {};
        
        for (size_t i = 0; i < functions.size(); ++i) {
            std::string task_name = group_name + "_" + std::to_string(i);
            task_groups_[group_name].push_back(task_name);
            
            auto future = pool_.submit([f = std::move(functions[i])]() { f(); });
            group_futures_[group_name].push_back(std::move(future));
        }
    }
    
    // Wait for all tasks in a group to complete
    void wait_for_group(const std::string& group_name);
    
    // Schedule a task graph
    template<typename NodeValue = void>
    void schedule_graph(TaskGraph<NodeValue>& graph) {
        // Check for cycles before executing
        if (graph.has_cycles()) {
            throw std::runtime_error("Cannot execute graph with cycles");
        }
        
        graph.execute(pool_);
    }
    
    // Schedule a repeating task with fixed delay
    template<typename F>
    std::future<void> schedule_repeating(F&& func, std::chrono::milliseconds interval, 
                                         size_t repetitions = 0) {
        std::shared_ptr<std::atomic<bool>> stop = std::make_shared<std::atomic<bool>>(false);
        
        auto repeating_task = [f = std::forward<F>(func), interval, repetitions, stop]() {
            size_t count = 0;
            while (!stop->load() && (repetitions == 0 || count < repetitions)) {
                f();
                count++;
                
                if (repetitions == 0 || count < repetitions) {
                    std::this_thread::sleep_for(interval);
                    if (stop->load()) break;
                }
            }
        };
        
        auto future = pool_.submit(repeating_task);
        
        // Return a future that allows stopping the repeating task
        return std::async(std::launch::deferred, [future = std::move(future), stop]() mutable {
            stop->store(true);
            future.wait();
        });
    }
    
    // Wait for all tasks to complete
    void wait_all();
    
    // Shutdown the scheduler
    void shutdown();
    
    // Get pool statistics
    size_t thread_count() const;
    size_t pending_tasks() const;
};

} // namespace mpcs

#endif // TASK_SCHEDULER_H