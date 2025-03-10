#include <iostream>
#include <thread>
#include <mutex>
#include <sstream>
#include <map>

std::mutex console_mutex;

std::map<std::thread::id, int> thread_numbers;
std::mutex map_mutex;

void count_to_100() {
    auto thread_id = std::this_thread::get_id();
    
    int thread_num;
    {
        std::lock_guard<std::mutex> lock(map_mutex);
        if (thread_numbers.empty()) {
            thread_num = 1;
        } else {
            thread_num = thread_numbers.rbegin()->second + 1;
        }
        thread_numbers[thread_id] = thread_num;
    }
    
    for (int i = 1; i <= 100; i++) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cout << "Thread " << thread_num << " has been called " 
                  << i << " times" << std::endl;
        std::cout.flush();
    }
}

int main() {
    {
        std::jthread t1(count_to_100);
        std::jthread t2(count_to_100);
        std::jthread t3(count_to_100);
        
    } 
    
    return 0;
}