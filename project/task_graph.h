#ifndef TASK_GRAPH_H
#define TASK_GRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <condition_variable>
#include "task.h"
#include "thread_pool.h"

namespace mpcs {

template<typename NodeValue = void>
class TaskGraph {
private:
    struct Node {
        Task<NodeValue> task;
        std::vector<size_t> dependencies;
        std::vector<size_t> dependents;
    };
    
    std::vector<Node> nodes_;
    std::unordered_map<std::string, size_t> node_names_;
    
public:
    // Add a named task
    template<typename F>
    void add_task(const std::string& name, F&& func) {
        nodes_.push_back({Task<NodeValue>(std::forward<F>(func)), {}, {}});
        node_names_[name] = nodes_.size() - 1;
    }
    
    // Add dependency between tasks
    void add_dependency(const std::string& dependent, const std::string& dependency) {
        auto dep_it = node_names_.find(dependency);
        auto dependent_it = node_names_.find(dependent);
        
        if (dep_it == node_names_.end() || dependent_it == node_names_.end()) {
            throw std::runtime_error("Task name not found in graph");
        }
        
        size_t dep_idx = dep_it->second;
        size_t dependent_idx = dependent_it->second;
        
        nodes_[dependent_idx].dependencies.push_back(dep_idx);
        nodes_[dep_idx].dependents.push_back(dependent_idx);
    }
    
    // Execute the graph on a thread pool
    void execute(ThreadPool& pool) {
        std::vector<std::future<NodeValue>> futures;
        std::unordered_set<size_t> completed;
        std::mutex completed_mutex;
        std::condition_variable completed_cv;
        
        // Find all nodes with no dependencies
        std::vector<size_t> ready_nodes;
        for (size_t i = 0; i < nodes_.size(); ++i) {
            if (nodes_[i].dependencies.empty()) {
                ready_nodes.push_back(i);
            }
        }
        
        // Function to check and update the graph when a task completes
        auto on_task_complete = [&](size_t node_idx) {
            std::unique_lock<std::mutex> lock(completed_mutex);
            completed.insert(node_idx);
            
            // Check for new ready nodes
            for (size_t dependent : nodes_[node_idx].dependents) {
                bool all_deps_done = true;
                for (size_t dep : nodes_[dependent].dependencies) {
                    if (completed.find(dep) == completed.end()) {
                        all_deps_done = false;
                        break;
                    }
                }
                
                if (all_deps_done) {
                    ready_nodes.push_back(dependent);
                    completed_cv.notify_all();
                }
            }
        };
        
        // Submit ready tasks and wait for more to become ready
        while (completed.size() < nodes_.size()) {
            // Submit all ready tasks
            {
                std::unique_lock<std::mutex> lock(completed_mutex);
                
                while (!ready_nodes.empty()) {
                    size_t node_idx = ready_nodes.back();
                    ready_nodes.pop_back();
                    
                    // Avoid submitting the same task twice
                    if (completed.find(node_idx) != completed.end()) {
                        continue;
                    }
                    
                    auto task_wrapper = [node_idx, &on_task_complete, this]() {
                        nodes_[node_idx].task.execute();
                        on_task_complete(node_idx);
                        if constexpr (!std::is_void_v<NodeValue>) {
                            return nodes_[node_idx].task.get_future().get();
                        }
                    };
                    
                    lock.unlock();
                    futures.push_back(pool.submit(task_wrapper));
                    lock.lock();
                }
                
                // If not all tasks are completed, wait for more tasks to be ready
                if (completed.size() < nodes_.size()) {
                    completed_cv.wait(lock, [&ready_nodes]() {
                        return !ready_nodes.empty();
                    });
                }
            }
        }
        
        // Wait for all futures
        for (auto& future : futures) {
            future.wait();
        }
    }
    
    // Check if the graph has cycles
    bool has_cycles() const {
        std::unordered_set<size_t> visited;
        std::unordered_set<size_t> rec_stack;
        
        for (size_t i = 0; i < nodes_.size(); ++i) {
            if (has_cycles_dfs(i, visited, rec_stack)) {
                return true;
            }
        }
        
        return false;
    }
    
private:
    bool has_cycles_dfs(size_t node, std::unordered_set<size_t>& visited, 
                       std::unordered_set<size_t>& rec_stack) const {
        
        if (rec_stack.find(node) != rec_stack.end()) {
            return true;
        }
        
        if (visited.find(node) != visited.end()) {
            return false;
        }
        
        visited.insert(node);
        rec_stack.insert(node);
        
        for (size_t dep : nodes_[node].dependencies) {
            if (has_cycles_dfs(dep, visited, rec_stack)) {
                return true;
            }
        }
        
        rec_stack.erase(node);
        return false;
    }
};

} // namespace mpcs

#endif // TASK_GRAPH_H