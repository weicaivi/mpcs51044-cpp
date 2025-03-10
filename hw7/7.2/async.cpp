#include <future>
#include <thread>
#include <type_traits>
#include <functional>
#include <iostream>
#include <chrono>

// Define launch policies similar to std::launch
enum class launch {
    async = 1,
    deferred = 2,
    any = async | deferred
};

template<typename Func, typename... Args>
auto my_async(launch policy, Func&& f, Args&&... args) {
    // Get the return type of the function when called with these arguments
    using result_type = std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>;
    
    // Create a packaged task that will store our function and its future result
    auto task_function = [f = std::decay_t<Func>(std::forward<Func>(f)), 
                         ...args = std::decay_t<Args>(std::forward<Args>(args))]() mutable {
        return f(args...);
    };
    
    std::packaged_task<result_type()> task(std::move(task_function));
    
    // Get the future before we potentially move the task to a new thread
    std::future<result_type> future = task.get_future();
    
    // Handle different launch policies
    if (policy == launch::deferred) {
        std::cout << "Executing deferred task..." << std::endl;
        task();
    }
    else if (policy == launch::async || policy == launch::any) {
        // Create a new thread but don't detach it immediately
        std::thread t(std::move(task));
        t.detach();
    }
    
    return future;
}

// Overload without launch policy - defaults to launch::any
template<typename Func, typename... Args>
auto my_async(Func&& f, Args&&... args) {
    return my_async(launch::any, std::forward<Func>(f), 
                   std::forward<Args>(args)...);
}

int compute_value() {
    std::cout << "Computing value in thread " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 42;
}

int main() {
    // Launch async task
    auto future1 = my_async(launch::async, compute_value);
    
    // Launch deferred task
    auto future2 = my_async(launch::deferred, compute_value);
    
    // Launch default policy task
    auto future3 = my_async(compute_value);

    std::cout << "Result 1: " << future1.get() << std::endl;
    std::cout << "Result 2: " << future2.get() << std::endl;
    std::cout << "Result 3: " << future3.get() << std::endl;
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return 0;
}