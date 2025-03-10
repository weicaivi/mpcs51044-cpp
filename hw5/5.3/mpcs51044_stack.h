#ifndef MPCS51044_STACK_H
#define MPCS51044_STACK_H

#include <mutex>
#include <stdexcept>
#include <initializer_list>
#include <memory>

namespace mpcs51044 {

template<typename T>
class ThreadSafeStack {
private:
    struct Node {
        std::shared_ptr<T> data;
        std::shared_ptr<Node> next;
        Node(const T& value) : data(std::make_shared<T>(value)) {}
    };

    std::shared_ptr<Node> head;
    mutable std::mutex mutex;
    std::size_t size_;

public:
    // Default constructor
    ThreadSafeStack() : head(nullptr), size_(0) {}

    // Copy constructor
    ThreadSafeStack(const ThreadSafeStack& other) {
        std::lock_guard<std::mutex> lock(other.mutex);
        head = other.head;
        size_ = other.size_;
    }

    // Initializer list constructor
    ThreadSafeStack(std::initializer_list<T> init) : head(nullptr), size_(0) {
        for (auto it = std::rbegin(init); it != std::rend(init); ++it) {
            push(*it);
        }
    }

    // Move constructor
    ThreadSafeStack(ThreadSafeStack&& other) noexcept {
        std::lock_guard<std::mutex> lock(other.mutex);
        head = std::move(other.head);
        size_ = other.size_;
        other.size_ = 0;
    }

    // Push operation
    void push(const T& value) {
        std::shared_ptr<Node> new_node = std::make_shared<Node>(value);
        std::lock_guard<std::mutex> lock(mutex);
        new_node->next = head;
        head = new_node;
        ++size_;
    }

    // Pop operation
    bool pop(T& value) {
        std::lock_guard<std::mutex> lock(mutex);
        if (!head) {
            return false;
        }
        value = *head->data;
        head = head->next;
        --size_;
        return true;
    }

    // Try peek at top element without removing it
    bool peek(T& value) const {
        std::lock_guard<std::mutex> lock(mutex);
        if (!head) {
            return false;
        }
        value = *head->data;
        return true;
    }

    // Check if stack is empty
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex);
        return !head;
    }

    // Get size of stack
    std::size_t size() const {
        std::lock_guard<std::mutex> lock(mutex);
        return size_;
    }

    // Clear the stack
    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        head = nullptr;
        size_ = 0;
    }

    // Assignment operator
    ThreadSafeStack& operator=(const ThreadSafeStack& other) {
        if (this != &other) {
            std::unique_lock<std::mutex> lock1(mutex, std::defer_lock);
            std::unique_lock<std::mutex> lock2(other.mutex, std::defer_lock);
            std::lock(lock1, lock2);  // Prevent deadlock
            head = other.head;
            size_ = other.size_;
        }
        return *this;
    }

    // Move assignment operator
    ThreadSafeStack& operator=(ThreadSafeStack&& other) noexcept {
        if (this != &other) {
            std::unique_lock<std::mutex> lock1(mutex, std::defer_lock);
            std::unique_lock<std::mutex> lock2(other.mutex, std::defer_lock);
            std::lock(lock1, lock2);
            head = std::move(other.head);
            size_ = other.size_;
            other.size_ = 0;
        }
        return *this;
    }

    // Swap operation
    void swap(ThreadSafeStack& other) noexcept {
        if (this != &other) {
            std::unique_lock<std::mutex> lock1(mutex, std::defer_lock);
            std::unique_lock<std::mutex> lock2(other.mutex, std::defer_lock);
            std::lock(lock1, lock2);
            std::swap(head, other.head);
            std::swap(size_, other.size_);
        }
    }
};

}

#endif