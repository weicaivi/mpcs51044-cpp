#include "DistributedCounter1.h"
#include "DistributedCounter2.h"
#include "DistributedCounter3.h"
#include <thread>
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <iomanip>

template<typename Counter>
double runTest(size_t threadCount, size_t reps) {
    Counter counter;
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t s = 0; s < threadCount; s++) {
        threads.emplace_back([&counter, reps]() {
            for (size_t i = 0; i < reps; i++) {
                counter++;
            }
        });
    }
    
    for (auto& thr : threads) {
        thr.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    auto finalCount = counter.get();
    if (finalCount != threadCount * reps) {
        std::cerr << "Error: Count mismatch! Expected " << threadCount * reps 
                  << " but got " << finalCount << std::endl;
    }
    
    return duration;
}

template<typename Counter>
void benchmarkCounter(const std::string& name, const std::vector<size_t>& threadCounts, 
                     size_t repsPerThread, size_t trials) {
    std::cout << "\nBenchmarking " << name << "\n";
    std::cout << std::setw(10) << "Threads" << std::setw(15) << "Time (ms)" 
              << std::setw(15) << "Ops/ms" << std::endl;
    std::cout << std::string(40, '-') << std::endl;
    
    for (size_t threads : threadCounts) {
        double totalTime = 0;
        
        for (size_t t = 0; t < trials; t++) {
            totalTime += runTest<Counter>(threads, repsPerThread);
        }
        
        double avgTime = totalTime / trials;
        double opsPerMs = (threads * repsPerThread) / avgTime;
        
        std::cout << std::setw(10) << threads 
                  << std::setw(15) << std::fixed << std::setprecision(2) << avgTime
                  << std::setw(15) << std::fixed << std::setprecision(2) << opsPerMs 
                  << std::endl;
    }
}

int main() {
    std::vector<size_t> threadCounts = {1, 2, 4, 8, 16};
    size_t repsPerThread = 10'000'000;  // 10M operations per thread
    size_t trials = 3;
    
    std::cout << "Running benchmarks with " << repsPerThread 
              << " increments per thread, " << trials << " trials each\n";
    
    benchmarkCounter<mpcs::DistributedCounter1>("Single Counter", threadCounts, repsPerThread, trials);
    benchmarkCounter<mpcs::DistributedCounter2>("Bucketed Counter", threadCounts, repsPerThread, trials);
    benchmarkCounter<mpcs::DistributedCounter3>("Padded Counter", threadCounts, repsPerThread, trials);
    
    return 0;
}