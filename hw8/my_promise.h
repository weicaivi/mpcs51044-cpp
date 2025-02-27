#ifndef MY_PROMISE_H
#define MY_PROMISE_H

#include <optional>
#include <variant>
#include <memory>
#include <exception>
#include <stdexcept>
#include <atomic>
#include <functional>

namespace mpcs {

// Helper template for std::visit with lambdas
template<class... Ts> 
struct overloaded : Ts... { 
    using Ts::operator()...; 
};
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template<class T> class MyPromise;

// Using variant to hold either a value or an exception
template<class T>
struct SharedState {
    // use std::variant to store either a value of type T or an exception_ptr
    using ValueVariant = std::variant<std::monostate, T, std::exception_ptr>;
    
    std::atomic<bool> ready{false};
    std::atomic<bool> consumer_waiting{false};
    
    ValueVariant value;
    
    // Atomic notify/wait mechanisms
    std::atomic_flag notifier = ATOMIC_FLAG_INIT;
    
    void notify() {
        ready.store(true, std::memory_order_release);
        notifier.test_and_set(std::memory_order_release);
        notifier.notify_one();
    }
    
    void wait() {
        consumer_waiting.store(true, std::memory_order_release);
        while (!ready.load(std::memory_order_acquire)) {
            notifier.wait(false, std::memory_order_acquire);
        }
    }
};

template<typename T>
class MyFuture {
public:
    MyFuture(const MyFuture&) = delete;
    MyFuture(MyFuture&&) = default;
    
    T get() {
        sharedState->wait();
        
        return std::visit(overloaded {
            [](std::monostate&) -> T {
                throw std::runtime_error{"Future accessed but no value set"};
            },
            [](T& value) -> T {
                return std::move(value);
            },
            [](std::exception_ptr& exc) -> T {
                std::rethrow_exception(exc);
            }
        }, sharedState->value);
    }
    
    bool is_ready() const {
        return sharedState->ready.load(std::memory_order_acquire);
    }

private:
    friend class MyPromise<T>;
    explicit MyFuture(std::shared_ptr<SharedState<T>>& state) : sharedState(state) {}
    std::shared_ptr<SharedState<T>> sharedState;
};

template<typename T>
class MyPromise {
public:
    MyPromise() : sharedState{std::make_shared<SharedState<T>>()} {}
    
    void set_value(T value) {
        if (sharedState->ready.load(std::memory_order_acquire)) {
            throw std::runtime_error{"Promise value already set"};
        }
        
        sharedState->value = std::move(value);
        sharedState->notify();
    }
    
    void set_exception(std::exception_ptr exc) {
        if (sharedState->ready.load(std::memory_order_acquire)) {
            throw std::runtime_error{"Promise value already set"};
        }
        
        sharedState->value = exc;
        sharedState->notify();
    }
    
    MyFuture<T> get_future() {
        return MyFuture<T>{sharedState};
    }
    
    bool has_consumer() const {
        return sharedState->consumer_waiting.load(std::memory_order_acquire);
    }

private:
    std::shared_ptr<SharedState<T>> sharedState;
};

}

#endif