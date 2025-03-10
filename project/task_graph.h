#ifndef TASK_GRAPH_H
#define TASK_GRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <condition_variable>
#include <iostream>
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
    
    // Execute the graph on a thread pool - simplified version
    void execute(ThreadPool& pool) {
        std::cout << "Starting task graph execution with " << nodes_.size() << " nodes" << std::endl;
        
        // Track completed nodes
        std::vector<bool> completed(nodes_.size(), false);
        std::mutex mutex;
        std::condition_variable cv;
        
        // Create a shared_ptr to hold futures (allows sharing between lambdas)
        auto futures_ptr = std::make_shared<std::vector<std::future<void>>>();
        
        // Submit all tasks that can start immediately (no dependencies)
        for (size_t i = 0; i < nodes_.size(); ++i) {
            if (nodes_[i].dependencies.empty()) {
                std::cout << "Submitting initial node " << i << std::endl;
                
                futures_ptr->push_back(pool.submit([this, i, &completed, &mutex, &cv, &pool, futures_ptr]() {
                    // Execute this task
                    nodes_[i].task.execute();
                    
                    // Mark as completed
                    {
                        std::lock_guard<std::mutex> lock(mutex);
                        completed[i] = true;
                        std::cout << "Node " << i << " completed" << std::endl;
                    }
                    cv.notify_all();
                    
                    // Check for dependent tasks that can now run
                    process_dependents(i, completed, mutex, cv, pool, futures_ptr);
                }));
            }
        }
        
        // Wait until all nodes are completed
        {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [&completed]() {
                for (bool is_completed : completed) {
                    if (!is_completed) return false;
                }
                return true;
            });
        }
        
        // Wait for all futures to complete
        for (auto& future : *futures_ptr) {
            future.wait();
        }
        
        std::cout << "All graph tasks completed" << std::endl;
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
    // Helper to process dependent tasks after a task completes
    void process_dependents(
        size_t completed_idx, 
        std::vector<bool>& completed, 
        std::mutex& mutex,
        std::condition_variable& cv,
        ThreadPool& pool, 
        std::shared_ptr<std::vector<std::future<void>>> futures_ptr) {
        
        // Get all dependent tasks
        const auto& dependents = nodes_[completed_idx].dependents;
        
        for (size_t dependent_idx : dependents) {
            bool can_execute = false;
            
            // Check if all dependencies are completed
            {
                std::lock_guard<std::mutex> lock(mutex);
                
                can_execute = true;
                for (size_t dep_idx : nodes_[dependent_idx].dependencies) {
                    if (!completed[dep_idx]) {
                        can_execute = false;
                        break;
                    }
                }
                
                // If already completed or can't execute yet, skip
                if (completed[dependent_idx] || !can_execute) {
                    continue;
                }
            }
            
            // Submit the task if it can execute
            if (can_execute) {
                std::cout << "Submitting dependent node " << dependent_idx << std::endl;
                
                futures_ptr->push_back(pool.submit([this, dependent_idx, &completed, &mutex, &cv, &pool, futures_ptr]() {
                    // Execute the task
                    nodes_[dependent_idx].task.execute();
                    
                    // Mark as completed
                    {
                        std::lock_guard<std::mutex> lock(mutex);
                        completed[dependent_idx] = true;
                        std::cout << "Node " << dependent_idx << " completed" << std::endl;
                    }
                    cv.notify_all();
                    
                    // Process its dependents
                    process_dependents(dependent_idx, completed, mutex, cv, pool, futures_ptr);
                }));
            }
        }
    }
    
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