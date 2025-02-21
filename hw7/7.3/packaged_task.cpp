#include <future>
#include <memory>
#include <utility>
#include <type_traits>
#include <iostream>
#include <thread>
#include <chrono>

template<typename> class my_packaged_task; // Primary template declaration

template<typename R, typename... Args>
class my_packaged_task<R(Args...)> {
private:
    // Promise to store the result
    std::promise<R> promise;
    // Function to be called
    std::function<R(Args...)> func;
    // Flag to check if task is valid
    bool valid = false;

public:
    // Default constructor
    my_packaged_task() noexcept : valid(false) {}

    // Constructor from callable
    template<typename F>
    explicit my_packaged_task(F&& f) 
        : func(std::forward<F>(f))
        , valid(true) {}

    // Move constructor
    my_packaged_task(my_packaged_task&& other) noexcept
        : promise(std::move(other.promise))
        , func(std::move(other.func))
        , valid(other.valid) {
        other.valid = false;
    }

    // Delete copy constructor and assignment
    my_packaged_task(const my_packaged_task&) = delete;
    my_packaged_task& operator=(const my_packaged_task&) = delete;

    // Move assignment
    my_packaged_task& operator=(my_packaged_task&& other) noexcept {
        if (this != &other) {
            promise = std::move(other.promise);
            func = std::move(other.func);
            valid = other.valid;
            other.valid = false;
        }
        return *this;
    }

    // Check if task is valid
    bool valid_task() const noexcept {
        return valid && func;
    }

    // Get future
    std::future<R> get_future() {
        if (!valid) {
            throw std::future_error(std::future_errc::no_state);
        }
        return promise.get_future();
    }

    // Reset the task
    void reset() {
        if (!valid) {
            throw std::future_error(std::future_errc::no_state);
        }
        my_packaged_task tmp(std::move(func));
        *this = std::move(tmp);
    }

    // Call operator
    void operator()(Args... args) {
        if (!valid) {
            throw std::future_error(std::future_errc::no_state);
        }
        try {
            if constexpr (std::is_void_v<R>) {
                func(std::forward<Args>(args)...);
                promise.set_value();
            } else {
                promise.set_value(func(std::forward<Args>(args)...));
            }
        }
        catch (...) {
            promise.set_exception(std::current_exception());
        }
    }
};

int compute_value(int input) {
    std::cout << "Computing value in thread " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return input * 2;
}

void print_value(int x) {
    std::cout << "Value is: " << x << std::endl;
}

int main() {
    // Example 1: Basic usage with return value
    my_packaged_task<int(int)> task1(compute_value);
    std::future<int> fut1 = task1.get_future();
    
    // Run in separate thread
    std::thread t1(std::move(task1), 21);
    int result1 = fut1.get();
    std::cout << "Result 1: " << result1 << std::endl;
    t1.join();

    // Example 2: Void return type
    my_packaged_task<void(int)> task2(print_value);
    std::future<void> fut2 = task2.get_future();
    
    task2(42);  // Execute directly
    fut2.get(); // Wait for completion

    // Example 3: Lambda function
    my_packaged_task<double(double)> task3(
        [](double x) { return x * 3.14; }
    );
    std::future<double> fut3 = task3.get_future();
    
    task3(2.0);
    std::cout << "Result 3: " << fut3.get() << std::endl;

    return 0;
}