// Type-erased task concept
class TaskConcept {
public:
    virtual ~TaskConcept() = default;
    virtual void execute() = 0;
    virtual bool is_completed() const = 0;
    virtual std::unique_ptr<TaskConcept> clone() const = 0;
};

// Concrete implementation for any callable
template<typename F>
class TaskModel : public TaskConcept {
private:
    F func_;
    bool completed_ = false;
public:
    explicit TaskModel(F&& func) : func_(std::forward<F>(func)) {}
    
    void execute() override {
        func_();
        completed_ = true;
    }
    
    bool is_completed() const override {
        return completed_;
    }
    
    std::unique_ptr<TaskConcept> clone() const override {
        return std::make_unique<TaskModel<F>>(func_);
    }
};

// Task wrapper with generic result type
template<typename Result = void>
class Task {
private:
    std::unique_ptr<TaskConcept> impl_;
    std::shared_ptr<std::promise<Result>> promise_;
    int priority_ = 0;
    std::vector<Task<void>> dependencies_;

public:
    // Constructor from any callable that returns Result
    template<typename F, 
             typename = std::enable_if_t<std::is_convertible_v<std::invoke_result_t<F>, Result>>>
    explicit Task(F&& func) 
        : impl_(std::make_unique<TaskModel<std::function<void()>>>(
            [f = std::forward<F>(func), p = std::make_shared<std::promise<Result>>()](){ 
                try {
                    if constexpr (std::is_void_v<Result>) {
                        f();
                        p->set_value();
                    } else {
                        p->set_value(f());
                    }
                } catch (...) {
                    p->set_exception(std::current_exception());
                }
            })),
          promise_(p) {}
    
    // Copy/move constructors and assignment
    Task(const Task& other) : impl_(other.impl_->clone()), promise_(other.promise_) {}
    Task(Task&& other) noexcept = default;
    Task& operator=(const Task& other);
    Task& operator=(Task&& other) noexcept = default;
    
    // Get future to access the result
    std::future<Result> get_future() const {
        return promise_->get_future();
    }
    
    // Priority operations with operator overloading
    Task& operator<<(int priority) {
        priority_ = priority;
        return *this;
    }
    
    bool operator<(const Task& other) const {
        return priority_ < other.priority_;
    }
    
    bool operator>(const Task& other) const {
        return priority_ > other.priority_;
    }
    
    // Dependency chaining with operator overloading
    Task& operator>>(Task<void>& dependency) {
        dependencies_.push_back(dependency);
        return *this;
    }
    
    // Other methods
    void execute() {
        for (auto& dep : dependencies_) {
            dep.execute();
        }
        impl_->execute();
    }
    
    bool is_completed() const {
        return impl_->is_completed();
    }
};

class ThreadPool {
private:
    bool running_ = false;
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    
public:
    // Constructor with number of threads
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
    
    // Destructor to join threads
    ~ThreadPool();
    
    // No copying
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    
    // Moving is allowed
    ThreadPool(ThreadPool&&) noexcept = default;
    ThreadPool& operator=(ThreadPool&&) noexcept = default;
    
    // Submit task with result
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) {
        using result_type = std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<result_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<result_type> result = task->get_future();
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (!running_) {
                throw std::runtime_error("Cannot submit to a stopped thread pool");
            }
            
            tasks_.emplace([task]() { (*task)(); });
        }
        condition_.notify_one();
        return result;
    }
    
    // Submit Task object
    template<typename Result>
    std::future<Result> submit(Task<Result> task) {
        auto future = task.get_future();
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (!running_) {
                throw std::runtime_error("Cannot submit to a stopped thread pool");
            }
            
            tasks_.emplace([t = std::move(task)]() { t.execute(); });
        }
        condition_.notify_one();
        return future;
    }
    
    // Operator overloading for task submission
    template<typename Result>
    ThreadPool& operator<<(Task<Result> task) {
        submit(std::move(task));
        return *this;
    }
    
    // Wait for all tasks to complete
    void wait_all();
    
    // Stop the thread pool
    void stop();
};

template<typename NodeValue = void>
class TaskGraph {
private:
    struct Node {
        Task<NodeValue> task;
        std::vector<size_t> dependencies;
        std::vector<size_t> dependents;
    };
    
    std::vector<Node> nodes_;
    std::unordered_map<std::string, size_t> node_names_;
    
public:
    // Add a named task
    template<typename F>
    void add_task(const std::string& name, F&& func) {
        nodes_.push_back({Task<NodeValue>(std::forward<F>(func)), {}, {}});
        node_names_[name] = nodes_.size() - 1;
    }
    
    // Add dependency between tasks
    void add_dependency(const std::string& dependent, const std::string& dependency) {
        size_t dep_idx = node_names_.at(dependency);
        size_t dependent_idx = node_names_.at(dependent);
        
        nodes_[dependent_idx].dependencies.push_back(dep_idx);
        nodes_[dep_idx].dependents.push_back(dependent_idx);
    }
    
    // Execute the graph on a thread pool
    void execute(ThreadPool& pool) {
        std::vector<std::future<NodeValue>> futures;
        std::unordered_set<size_t> completed;
        std::mutex completed_mutex;
        std::condition_variable completed_cv;
        
        // Find all nodes with no dependencies
        std::vector<size_t> ready_nodes;
        for (size_t i = 0; i < nodes_.size(); ++i) {
            if (nodes_[i].dependencies.empty()) {
                ready_nodes.push_back(i);
            }
        }
        
        // Function to check and update the graph when a task completes
        auto on_task_complete = [&](size_t node_idx) {
            std::unique_lock<std::mutex> lock(completed_mutex);
            completed.insert(node_idx);
            
            // Check for new ready nodes
            for (size_t dependent : nodes_[node_idx].dependents) {
                bool all_deps_done = true;
                for (size_t dep : nodes_[dependent].dependencies) {
                    if (completed.find(dep) == completed.end()) {
                        all_deps_done = false;
                        break;
                    }
                }
                
                if (all_deps_done) {
                    ready_nodes.push_back(dependent);
                }
            }
            
            lock.unlock();
            completed_cv.notify_all();
        };
        
        // Submit initial tasks
        while (!ready_nodes.empty() || completed.size() < nodes_.size()) {
            // Submit all ready tasks
            for (size_t node_idx : ready_nodes) {
                auto task_wrapper = [node_idx, &on_task_complete, this]() {
                    nodes_[node_idx].task.execute();
                    on_task_complete(node_idx);
                    if constexpr (!std::is_void_v<NodeValue>) {
                        return nodes_[node_idx].task.get_future().get();
                    }
                };
                
                futures.push_back(pool.submit(task_wrapper));
            }
            
            ready_nodes.clear();
            
            // Wait for more tasks to be ready
            std::unique_lock<std::mutex> lock(completed_mutex);
            completed_cv.wait(lock, [&]() {
                return !ready_nodes.empty() || completed.size() == nodes_.size();
            });
        }
        
        // Wait for all futures
        for (auto& future : futures) {
            future.wait();
        }
    }
};

template<typename T>
class TaskAllocator {
private:
    static constexpr size_t block_size = 4096;
    std::vector<std::unique_ptr<char[]>> blocks_;
    std::vector<char*> free_list_;
    std::mutex allocator_mutex_;
    
public:
    using value_type = T;
    
    TaskAllocator() = default;
    
    template<typename U>
    TaskAllocator(const TaskAllocator<U>&) noexcept {}
    
    T* allocate(size_t n) {
        if (n != 1) {
            return static_cast<T*>(::operator new(n * sizeof(T)));
        }
        
        std::lock_guard<std::mutex> lock(allocator_mutex_);
        if (free_list_.empty()) {
            // Allocate a new block
            auto block = std::make_unique<char[]>(block_size);
            char* start = block.get();
            
            // Divide the block into chunks and add to free list
            for (size_t i = 0; i < block_size; i += sizeof(T)) {
                free_list_.push_back(start + i);
            }
            
            blocks_.push_back(std::move(block));
        }
        
        T* result = reinterpret_cast<T*>(free_list_.back());
        free_list_.pop_back();
        return result;
    }
    
    void deallocate(T* p, size_t n) noexcept {
        if (n != 1) {
            ::operator delete(p);
            return;
        }
        
        std::lock_guard<std::mutex> lock(allocator_mutex_);
        free_list_.push_back(reinterpret_cast<char*>(p));
    }
    
    // Required for allocator compatibility
    template<typename U>
    struct rebind {
        using other = TaskAllocator<U>;
    };
};

class TaskScheduler {
private:
    ThreadPool pool_;
    std::unordered_map<std::string, std::vector<std::string>> task_groups_;
    
public:
    explicit TaskScheduler(size_t num_threads = std::thread::hardware_concurrency())
        : pool_(num_threads) {}
    
    // Schedule a single task
    template<typename F>
    auto schedule(F&& func) {
        using result_type = std::invoke_result_t<F>;
        Task<result_type> task(std::forward<F>(func));
        return pool_.submit(std::move(task));
    }
    
    // Schedule a task with priority
    template<typename F>
    auto schedule_with_priority(F&& func, int priority) {
        using result_type = std::invoke_result_t<F>;
        Task<result_type> task(std::forward<F>(func));
        task << priority;
        return pool_.submit(std::move(task));
    }
    
    // Schedule a group of tasks
    template<typename F>
    void schedule_group(const std::string& group_name, std::vector<F> functions) {
        task_groups_[group_name] = {};
        
        for (size_t i = 0; i < functions.size(); ++i) {
            std::string task_name = group_name + "_" + std::to_string(i);
            task_groups_[group_name].push_back(task_name);
            pool_ << Task<void>([f = std::move(functions[i])]() { f(); });
        }
    }
    
    // Wait for all tasks in a group to complete
    void wait_for_group(const std::string& group_name);
    
    // Schedule a task graph
    template<typename NodeValue = void>
    void schedule_graph(TaskGraph<NodeValue>& graph) {
        graph.execute(pool_);
    }
    
    // Wait for all tasks to complete
    void wait_all() {
        pool_.wait_all();
    }
    
    // Shutdown the scheduler
    void shutdown() {
        pool_.stop();
    }
};