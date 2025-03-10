#include "thread_pool.h"

namespace mpcs {

ThreadPool::ThreadPool(size_t num_threads)
    : running_(true), active_tasks_(0) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back(
            [this] { worker_thread(); }
        );
    }
}

ThreadPool::~ThreadPool() {
    stop();
}

ThreadPool::ThreadPool(ThreadPool&& other) noexcept
    : running_(other.running_.load()),
      workers_(std::move(other.workers_)),
      active_tasks_(other.active_tasks_.load()) {
    
    std::lock_guard<std::mutex> lock(other.queue_mutex_);
    tasks_ = std::move(other.tasks_);
    other.running_ = false;
}

ThreadPool& ThreadPool::operator=(ThreadPool&& other) noexcept {
    if (this != &other) {
        stop();
        
        std::lock_guard<std::mutex> lock_this(queue_mutex_);
        std::lock_guard<std::mutex> lock_other(other.queue_mutex_);
        
        running_ = other.running_.load();
        workers_ = std::move(other.workers_);
        tasks_ = std::move(other.tasks_);
        active_tasks_ = other.active_tasks_.load();
        
        other.running_ = false;
    }
    return *this;
}

void ThreadPool::worker_thread() {
    while (true) {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            condition_.wait(lock, [this] {
                return !running_ || !tasks_.empty();
            });
            
            if (!running_ && tasks_.empty()) {
                return;
            }
            
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        
        try {
            task();
        } catch (...) {
            // Log or handle exceptions
        }
    }
}

void ThreadPool::decrementActiveTask() {
    size_t remaining = --active_tasks_;
    if (remaining == 0) {
        all_done_condition_.notify_all();
    }
}

void ThreadPool::wait_all() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    all_done_condition_.wait(lock, [this] {
        return active_tasks_ == 0;
    });
}

void ThreadPool::stop() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        running_ = false;
    }
    
    condition_.notify_all();
    
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    workers_.clear();
}

size_t ThreadPool::size() const {
    return workers_.size();
}

size_t ThreadPool::pending_tasks() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return tasks_.size();
}

bool ThreadPool::is_running() const {
    return running_;
}

} // namespace mpcs