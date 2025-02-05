#ifndef MPCS51044_STACK_H
#define MPCS51044_STACK_H

#include <mutex>
#include <stack>
#include <stdexcept>
#include <initializer_list>

namespace mpcs51044 {

template<typename T>
class stack {
private:
    std::stack<T> data;
    mutable std::mutex m;

public:
    // Default constructor
    stack() = default;

    // Initializer list constructor
    stack(std::initializer_list<T> init) : data(init) {}

    // Copy constructor
    stack(const stack& other) {
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data;
    }

    // Copy assignment operator
    stack& operator=(const stack& other) {
        if (this != &other) {
            std::scoped_lock lock(m, other.m);  // Lock both mutexes
            data = other.data;
        }
        return *this;
    }

    // Destructor
    ~stack() = default;

    void push(const T& value) {
        std::lock_guard<std::mutex> lock(m);
        data.push(value);
    }

    T pop() {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) {
            throw std::runtime_error("Stack is empty");
        }
        T value = data.top();
        data.pop();
        return value;
    }
};

}

#endif