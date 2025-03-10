#ifndef TASK_ALLOCATOR_H
#define TASK_ALLOCATOR_H

#include <vector>
#include <memory>
#include <mutex>

namespace mpcs {

template<typename T>
class TaskAllocator {
private:
    static constexpr size_t block_size = 4096;
    std::vector<std::unique_ptr<char[]>> blocks_;
    std::vector<char*> free_list_;
    std::mutex allocator_mutex_;
    
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    
    // Default constructor
    TaskAllocator() = default;
    
    // Copy constructor
    TaskAllocator(const TaskAllocator&) noexcept {}
    
    // Rebind constructor
    template<typename U>
    TaskAllocator(const TaskAllocator<U>&) noexcept {}
    
    // Assignment operator - no-op
    TaskAllocator& operator=(const TaskAllocator&) = default;
    
    // Allocate memory for n objects of type T
    pointer allocate(size_type n) {
        if (n != 1) {
            return static_cast<pointer>(::operator new(n * sizeof(T)));
        }
        
        std::lock_guard<std::mutex> lock(allocator_mutex_);
        if (free_list_.empty()) {
            // Allocate a new block
            auto block = std::make_unique<char[]>(block_size);
            char* start = block.get();
            
            // Divide the block into chunks and add to free list
            size_t num_chunks = block_size / sizeof(T);
            for (size_t i = 0; i < num_chunks; ++i) {
                free_list_.push_back(start + i * sizeof(T));
            }
            
            blocks_.push_back(std::move(block));
        }
        
        pointer result = reinterpret_cast<pointer>(free_list_.back());
        free_list_.pop_back();
        return result;
    }
    
    // Deallocate memory pointed to by p
    void deallocate(pointer p, size_type n) noexcept {
        if (n != 1) {
            ::operator delete(p);
            return;
        }
        
        std::lock_guard<std::mutex> lock(allocator_mutex_);
        free_list_.push_back(reinterpret_cast<char*>(p));
    }
    
    // Construct an object at the given address
    template<typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new((void*)p) U(std::forward<Args>(args)...);
    }
    
    // Destroy an object at the given address
    template<typename U>
    void destroy(U* p) {
        p->~U();
    }
    
    // Maximum size that can be allocated
    size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }
    
    // Rebind to a different type
    template<typename U>
    struct rebind {
        using other = TaskAllocator<U>;
    };
    
    // Equality operator
    template<typename U>
    bool operator==(const TaskAllocator<U>&) const noexcept {
        return true;
    }
    
    // Inequality operator
    template<typename U>
    bool operator!=(const TaskAllocator<U>&) const noexcept {
        return false;
    }
};

} // namespace mpcs

#endif // TASK_ALLOCATOR_H