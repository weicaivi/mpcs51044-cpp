#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <memory>
#include <atomic>
#include "task.h"

namespace mpcs {

class ThreadPool {
private:
    std::atomic<bool> running_;
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<size_t> active_tasks_;
    std::condition_variable all_done_condition_;
    
public:
    // Constructor with number of threads
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
    
    // Destructor to join threads
    ~ThreadPool();
    
    // No copying
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    
    // Moving is allowed
    ThreadPool(ThreadPool&&) noexcept;
    ThreadPool& operator=(ThreadPool&&) noexcept;
    
    // Submit a function with arguments
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using result_type = std::invoke_result_t<F, Args...>;
        
        auto task = std::make_shared<std::packaged_task<result_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<result_type> result = task->get_future();
        
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (!running_) {
                throw std::runtime_error("Cannot submit to a stopped thread pool");
            }
            
            active_tasks_++;
            tasks_.emplace([task, this]() {
                (*task)();
                decrementActiveTask();
            });
        }
        
        condition_.notify_one();
        return result;
    }
    
    // Submit a Task object
    template<typename Result>
    std::future<Result> submit(Task<Result> task) {
        auto future = task.get_future();
        
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (!running_) {
                throw std::runtime_error("Cannot submit to a stopped thread pool");
            }
            
            active_tasks_++;
            tasks_.emplace([t = std::move(task), this]() mutable {
                t.execute();
                decrementActiveTask();
            });
        }
        
        condition_.notify_one();
        return future;
    }
    
    // Operator overloading for task submission
    template<typename Result>
    ThreadPool& operator<<(Task<Result> task) {
        submit(std::move(task));
        return *this;
    }
    
    // Wait for all tasks to complete
    void wait_all();
    
    // Stop the thread pool
    void stop();
    
    // Get the number of threads
    size_t size() const;
    
    // Get the number of pending tasks
    size_t pending_tasks() const;
    
    // Check if the pool is running
    bool is_running() const;

private:
    void worker_thread();
    void decrementActiveTask();
};

} // namespace mpcs

#endif // THREAD_POOL_H