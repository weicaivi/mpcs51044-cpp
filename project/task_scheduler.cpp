#include "task_scheduler.h"

namespace mpcs {

TaskScheduler::TaskScheduler(size_t num_threads)
    : pool_(num_threads) {
}

void TaskScheduler::wait_for_group(const std::string& group_name) {
    auto it = group_futures_.find(group_name);
    if (it == group_futures_.end()) {
        throw std::runtime_error("Group not found: " + group_name);
    }
    
    for (auto& future : it->second) {
        future.wait();
    }
}

void TaskScheduler::wait_all() {
    pool_.wait_all();
}

void TaskScheduler::shutdown() {
    pool_.stop();
}

size_t TaskScheduler::thread_count() const {
    return pool_.size();
}

size_t TaskScheduler::pending_tasks() const {
    return pool_.pending_tasks();
}

} // namespace mpcs