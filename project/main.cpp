#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <atomic>

#include "task_concept.h"
#include "task.h"
#include "thread_pool.h"
#include "task_scheduler.h"
#include "task_graph.h"
#include "task_chain.h"
#include "task_allocator.h"

using namespace mpcs;
using namespace std::chrono_literals;
namespace fs = std::filesystem;

// Custom task executor that logs execution
class LoggingTaskExecutor : public TaskExecutor<LoggingTaskExecutor> {
private:
    std::string name_;
    
public:
    explicit LoggingTaskExecutor(std::string name) : name_(std::move(name)) {}
    
    void before_execution() {
        std::cout << "[" << name_ << "] Starting task execution\n";
    }
    
    void after_execution() {
        std::cout << "[" << name_ << "] Task execution completed\n";
    }
};

// Example file processing functions
std::vector<std::string> read_file(const std::string& filename) {
    std::vector<std::string> lines;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    
    std::cout << "Read " << lines.size() << " lines from " << filename << std::endl;
    return lines;
}

std::vector<std::string> process_lines(const std::vector<std::string>& lines) {
    std::vector<std::string> processed;
    processed.reserve(lines.size());
    
    for (const auto& line : lines) {
        // Simulate processing by converting to uppercase
        std::string upper_line;
        upper_line.reserve(line.size());
        
        for (char c : line) {
            upper_line.push_back(std::toupper(c));
        }
        processed.push_back(upper_line);
    }
    
    std::cout << "Processed " << processed.size() << " lines" << std::endl;
    return processed;
}

void write_file(const std::string& filename, const std::vector<std::string>& lines) {
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }
    
    for (const auto& line : lines) {
        file << line << "\n";
    }
    
    std::cout << "Wrote " << lines.size() << " lines to " << filename << std::endl;
}

// Benchmark function to measure task performance
template<typename Func, typename... Args>
auto benchmark(const std::string& name, Func&& func, Args&&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    auto result = func(std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Benchmark [" << name << "]: " << duration << "ms" << std::endl;
    
    return result;
}

// Demo 1: Basic task submission and execution
void demo_basic_tasks() {
    std::cout << "\n--- Demo 1: Basic task submission and execution ---\n";
    
    mpcs::ThreadPool pool(4);
    
    // Submit a simple task
    auto future1 = pool.submit([]() {
        std::cout << "Hello from thread " << std::this_thread::get_id() << std::endl;
        return 42;
    });
    
    // Submit a task with arguments
    auto future2 = pool.submit([](int a, int b) {
        std::cout << "Computing " << a << " + " << b << " in thread " 
                  << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(100ms);
        return a + b;
    }, 10, 20);
    
    // Wait for results
    std::cout << "Result 1: " << future1.get() << std::endl;
    std::cout << "Result 2: " << future2.get() << std::endl;
}

// Demo 2: Task with priorities
void demo_task_priorities() {
    std::cout << "\n--- Demo 2: Task with priorities ---\n";
    
    mpcs::TaskScheduler scheduler(2);
    
    // Schedule tasks with different priorities
    auto low_priority = scheduler.schedule_with_priority([]() {
        std::cout << "Low priority task executed" << std::endl;
        std::this_thread::sleep_for(100ms);
        return "Low priority completed";
    }, 1);
    
    auto high_priority = scheduler.schedule_with_priority([]() {
        std::cout << "High priority task executed" << std::endl;
        std::this_thread::sleep_for(100ms);
        return "High priority completed";
    }, 10);
    
    auto medium_priority = scheduler.schedule_with_priority([]() {
        std::cout << "Medium priority task executed" << std::endl;
        std::this_thread::sleep_for(100ms);
        return "Medium priority completed";
    }, 5);
    
    // Wait for all tasks to complete
    std::cout << high_priority.get() << std::endl;
    std::cout << medium_priority.get() << std::endl;
    std::cout << low_priority.get() << std::endl;
}

// Demo 3: Task chaining
void demo_task_chaining() {
    std::cout << "\n--- Demo 3: Task chaining ---\n";
    
    TaskScheduler scheduler(4);
    
    // Create a chain of tasks for file processing
    auto task_chain = chain_tasks<std::vector<std::string>>(
        []() { 
            std::cout << "Step 1: Reading file\n"; 
            return read_file("input.txt"); 
        },
        [](std::vector<std::string> lines) { 
            std::cout << "Step 2: Processing lines\n"; 
            return process_lines(lines); 
        },
        [](std::vector<std::string> processed) { 
            std::cout << "Step 3: Writing file\n"; 
            write_file("output.txt", processed);
            return processed;
        }
    );
    
    // Execute the chain
    auto future = scheduler.schedule(std::move(task_chain));
    
    try {
        auto result = future.get();
        std::cout << "Task chain completed successfully with " << result.size() << " lines processed\n";
    } catch (const std::exception& e) {
        std::cout << "Task chain failed: " << e.what() << std::endl;
        // Create a dummy input file for demo purposes
        std::ofstream input("input.txt");
        input << "This is a test line\n";
        input << "Another line for processing\n";
        input.close();
        std::cout << "Created a dummy input.txt file. Try running the demo again.\n";
    }
}

// Demo 4: Task graph with dependencies
void demo_task_graph() {
    std::cout << "\n--- Demo 4: Task graph with dependencies ---\n";
    
    mpcs::TaskScheduler scheduler(4);
    mpcs::TaskGraph<int> graph;
    
    // Add tasks to the graph
    graph.add_task("load_config", []() {
        std::cout << "Loading configuration..." << std::endl;
        std::this_thread::sleep_for(100ms);
        return 1;
    });
    
    graph.add_task("load_data_1", []() {
        std::cout << "Loading data set 1..." << std::endl;
        std::this_thread::sleep_for(200ms);
        return 2;
    });
    
    graph.add_task("load_data_2", []() {
        std::cout << "Loading data set 2..." << std::endl;
        std::this_thread::sleep_for(150ms);
        return 3;
    });
    
    graph.add_task("process_data", []() {
        std::cout << "Processing all data..." << std::endl;
        std::this_thread::sleep_for(300ms);
        return 4;
    });
    
    graph.add_task("generate_report", []() {
        std::cout << "Generating report..." << std::endl;
        std::this_thread::sleep_for(200ms);
        return 5;
    });
    
    // Add dependencies
    graph.add_dependency("load_data_1", "load_config");
    graph.add_dependency("load_data_2", "load_config");
    graph.add_dependency("process_data", "load_data_1");
    graph.add_dependency("process_data", "load_data_2");
    graph.add_dependency("generate_report", "process_data");
    
    // Execute the graph
    std::cout << "Executing task graph..." << std::endl;
    scheduler.schedule_graph(graph);
    std::cout << "Task graph execution completed" << std::endl;
}

// Demo 5: Custom task executor with CRTP
void demo_custom_executor() {
    std::cout << "\n--- Demo 5: Custom task executor with CRTP ---\n";
    
    LoggingTaskExecutor executor("MainExecutor");
    
    mpcs::Task<int> task([]() {
        std::cout << "Executing task in thread " << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(100ms);
        return 42;
    });
    
    executor.execute_task(task);
    
    std::cout << "Task result: " << task.get_future().get() << std::endl;
}

// Demo 6: Repeating tasks
void demo_repeating_tasks() {
    std::cout << "\n--- Demo 6: Repeating tasks ---\n";
    
    mpcs::TaskScheduler scheduler(2);
    std::atomic<int> counter = 0;
    
    auto stop_token = scheduler.schedule_repeating([&counter]() {
        int current = ++counter;
        std::cout << "Repeating task execution #" << current << std::endl;
        std::this_thread::sleep_for(200ms);
    }, 500ms, 5);
    
    std::cout << "Waiting for repeating task to finish..." << std::endl;
    stop_token.wait();
    std::cout << "Repeating task completed " << counter << " times" << std::endl;
}

// Demo 7: Task allocator
void demo_task_allocator() {
    std::cout << "\n--- Demo 7: Task allocator ---\n";
    
    // Create a vector that uses our custom allocator
    std::vector<int, mpcs::TaskAllocator<int>> vec;
    
    // Add some elements
    std::cout << "Adding elements using TaskAllocator" << std::endl;
    for (int i = 0; i < 1000; ++i) {
        vec.push_back(i);
    }
    
    std::cout << "Vector size: " << vec.size() << std::endl;
    std::cout << "First few elements: ";
    for (size_t i = 0; i < std::min(static_cast<size_t>(10), vec.size()); ++i) {
        std::cout << vec[i] << " ";
    }
    std::cout << std::endl;
}

// Demo 8: Benchmark with different thread counts
void demo_benchmark() {
    std::cout << "\n--- Demo 8: Benchmark with different thread counts ---\n";
    
    const int num_tasks = 1000;
    const int work_size = 10000;
    
    auto benchmark_threads = [num_tasks, work_size](size_t num_threads) {
        mpcs::TaskScheduler scheduler(num_threads);
        std::vector<std::future<int>> results;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Submit a large number of compute-intensive tasks
        for (int i = 0; i < num_tasks; ++i) {
            results.push_back(scheduler.schedule([i, work_size]() {
                int sum = 0;
                for (int j = 0; j < work_size; ++j) {
                    sum += (i * j) % 997; // Some arbitrary computation
                }
                return sum;
            }));
        }
        
        // Wait for all tasks to complete
        for (auto& result : results) {
            result.wait();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return duration;
    };
    
    // Test with different thread counts
    std::vector<size_t> thread_counts = {1, 2, 4, 8, 16};
    
    for (size_t num_threads : thread_counts) {
        auto duration = benchmark_threads(num_threads);
        std::cout << "Threads: " << num_threads 
                  << ", Tasks: " << num_tasks 
                  << ", Time: " << duration << "ms" << std::endl;
    }
}

int main() {
    // std::cout << "=== Task Scheduler with Thread Pool Demo ===" << std::endl;
    // std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << " threads" << std::endl;
    
    // try {
    //     demo_basic_tasks();
    //     demo_task_priorities();
    //     demo_task_chaining();
    //     demo_task_graph();
    //     demo_custom_executor();
    //     demo_repeating_tasks();
    //     demo_task_allocator();
    //     demo_benchmark();
        
    //     std::cout << "\nAll demos completed successfully!" << std::endl;
    // } catch (const std::exception& e) {
    //     std::cerr << "Error: " << e.what() << std::endl;
    //     return 1;
    // }
    std::cout << "Program started!" << std::endl;
    std::cout << "Program finished!" << std::endl;
    
    return 0;
}