#include <iostream>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <exception>
#include <future>

template<typename T>
class my_promise;

template<typename T>
class my_future {
private:
    // Shared state between promise and future
    struct shared_state {
        std::mutex mtx;
        std::condition_variable cv;
        T value;
        std::exception_ptr exception;
        bool ready = false;
        bool has_exception = false;
    };
    
    // Use shared_ptr since state is shared between promise and future
    std::shared_ptr<shared_state> state;

    // Only promise can create a future
    friend class my_promise<T>;
    explicit my_future(std::shared_ptr<shared_state> s) : state(s) {}

public:
    // Get the value - blocks until ready
    T get() {
        if (!state) {
            throw std::future_error(std::future_errc::no_state);
        }

        std::unique_lock<std::mutex> lock(state->mtx);
        state->cv.wait(lock, [this] { return state->ready; });

        if (state->has_exception) {
            std::rethrow_exception(state->exception);
        }

        return std::move(state->value);
    }

    // Check if the future has a valid state
    bool valid() const noexcept {
        return state != nullptr;
    }
};

template<typename T>
class my_promise {
private:
    std::shared_ptr<typename my_future<T>::shared_state> state;

public:
    my_promise() 
        : state(std::make_shared<typename my_future<T>::shared_state>()) {}

    // Move constructor
    my_promise(my_promise&& other) noexcept 
        : state(std::exchange(other.state, nullptr)) {}

    // Delete copy constructor and assignment
    my_promise(const my_promise&) = delete;
    my_promise& operator=(const my_promise&) = delete;

    // Move assignment
    my_promise& operator=(my_promise&& other) noexcept {
        state = std::exchange(other.state, nullptr);
        return *this;
    }

    // Get the future
    my_future<T> get_future() {
        if (!state) {
            throw std::future_error(std::future_errc::no_state);
        }
        return my_future<T>(state);
    }

    // Set the value
    void set_value(const T& value) {
        if (!state) {
            throw std::future_error(std::future_errc::no_state);
        }

        std::lock_guard<std::mutex> lock(state->mtx);
        if (state->ready) {
            throw std::future_error(std::future_errc::promise_already_satisfied);
        }

        state->value = value;
        state->ready = true;
        state->cv.notify_one();
    }

    // Set the value (move version)
    void set_value(T&& value) {
        if (!state) {
            throw std::future_error(std::future_errc::no_state);
        }

        std::lock_guard<std::mutex> lock(state->mtx);
        if (state->ready) {
            throw std::future_error(std::future_errc::promise_already_satisfied);
        }

        state->value = std::move(value);
        state->ready = true;
        state->cv.notify_one();
    }

    // Set an exception
    void set_exception(std::exception_ptr p) {
        if (!state) {
            throw std::future_error(std::future_errc::no_state);
        }

        std::lock_guard<std::mutex> lock(state->mtx);
        if (state->ready) {
            throw std::future_error(std::future_errc::promise_already_satisfied);
        }

        state->exception = p;
        state->has_exception = true;
        state->ready = true;
        state->cv.notify_one();
    }
};

int main() {
    // Test 1: Basic value transfer
    {
        my_promise<int> prom;
        my_future<int> fut = prom.get_future();

        std::thread t([p = std::move(prom)]() mutable {
            std::cout << "Thread: Computing value...\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "Thread: Setting value to 42\n";
            p.set_value(42);
        });

        std::cout << "Main: Waiting for value...\n";
        int value = fut.get();
        std::cout << "Main: Received value: " << value << "\n";
        t.join();
        std::cout << "\n";
    }

    // Test 2: Exception handling
    {
        my_promise<int> prom;
        my_future<int> fut = prom.get_future();

        std::thread t([p = std::move(prom)]() mutable {
            std::cout << "Thread: Going to throw an exception\n";
            try {
                throw std::runtime_error("Test exception");
            } catch (...) {
                p.set_exception(std::current_exception());
            }
        });

        std::cout << "Main: Waiting for value (expecting exception)...\n";
        try {
            fut.get();
        } catch (const std::runtime_error& e) {
            std::cout << "Main: Caught expected exception: " << e.what() << "\n";
        }
        t.join();
        std::cout << "\n";
    }

    // Test 3: Move semantics
    {
        my_promise<std::string> prom1;
        my_promise<std::string> prom2 = std::move(prom1);
        my_future<std::string> fut = prom2.get_future();

        std::thread t([p = std::move(prom2)]() mutable {
            std::cout << "Thread: Setting string value\n";
            p.set_value("Hello from moved promise!");
        });

        std::cout << "Main: Waiting for string...\n";
        std::string str = fut.get();
        std::cout << "Main: Received: " << str << "\n";
        t.join();
        std::cout << "\n";
    }

    return 0;
}