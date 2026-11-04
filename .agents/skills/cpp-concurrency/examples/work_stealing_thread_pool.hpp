#pragma once

#include <atomic>
#include <concepts>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace cpp_concurrency {

/**
 * @brief Thread-safe double-ended queue supporting work-stealing.
 * 
 * The owning thread pushes and pops from the front (LIFO cache-hot).
 * Stealing threads pop from the back (FIFO minimizing contention).
 */
class WorkStealingQueue {
private:
    using Task = std::function<void()>;
    mutable std::mutex mutex_;
    std::deque<Task> deque_;

public:
    WorkStealingQueue() = default;
    WorkStealingQueue(const WorkStealingQueue&) = delete;
    WorkStealingQueue& operator=(const WorkStealingQueue&) = delete;

    void push(Task data) {
        std::lock_guard<std::mutex> lock(mutex_);
        deque_.push_front(std::move(data));
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return deque_.empty();
    }

    bool try_pop(Task& res) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (deque_.empty()) {
            return false;
        }
        res = std::move(deque_.front());
        deque_.pop_front();
        return true;
    }

    bool try_steal(Task& res) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (deque_.empty()) {
            return false;
        }
        res = std::move(deque_.back());
        deque_.pop_back();
        return true;
    }
};

/**
 * @brief Scalable Work-Stealing Thread Pool.
 * 
 * Based on Chapter 9 of "C++ Concurrency in Action" (Anthony Williams).
 */
class WorkStealingThreadPool {
private:
    using Task = std::function<void()>;

    std::atomic<bool> done_{false};
    std::vector<std::unique_ptr<WorkStealingQueue>> queues_;
    std::vector<std::jthread> threads_;

    static inline thread_local WorkStealingQueue* local_work_queue_{nullptr};
    static inline thread_local std::size_t my_index_{0};

    void worker_thread(std::size_t my_index) {
        my_index_ = my_index;
        local_work_queue_ = queues_[my_index].get();

        while (!done_.load(std::memory_order_relaxed)) {
            run_pending_task();
        }
    }

    bool pop_task_from_local_queue(Task& task) {
        return local_work_queue_ && local_work_queue_->try_pop(task);
    }

    bool pop_task_from_other_thread_queue(Task& task) {
        const std::size_t count = queues_.size();
        for (std::size_t i = 0; i < count; ++i) {
            const std::size_t index = (my_index_ + i + 1) % count;
            if (queues_[index]->try_steal(task)) {
                return true;
            }
        }
        return false;
    }

public:
    explicit WorkStealingThreadPool(std::size_t thread_count = 0) {
        if (thread_count == 0) {
            thread_count = std::thread::hardware_concurrency();
            if (thread_count == 0) thread_count = 2;
        }

        queues_.reserve(thread_count);
        for (std::size_t i = 0; i < thread_count; ++i) {
            queues_.push_back(std::make_unique<WorkStealingQueue>());
        }

        threads_.reserve(thread_count);
        for (std::size_t i = 0; i < thread_count; ++i) {
            threads_.emplace_back(&WorkStealingThreadPool::worker_thread, this, i);
        }
    }

    ~WorkStealingThreadPool() {
        done_.store(true, std::memory_order_relaxed);
    }

    void run_pending_task() {
        Task task;
        if (pop_task_from_local_queue(task) || pop_task_from_other_thread_queue(task)) {
            task();
        } else {
            std::this_thread::yield();
        }
    }

    template <typename FunctionType, typename... Args>
    auto submit(FunctionType&& f, Args&&... args)
        -> std::future<std::invoke_result_t<FunctionType, Args...>> {
        using result_type = std::invoke_result_t<FunctionType, Args...>;

        auto bound_task = std::bind(std::forward<FunctionType>(f), std::forward<Args>(args)...);
        auto task = std::make_shared<std::packaged_task<result_type()>>(std::move(bound_task));
        std::future<result_type> res = task->get_future();

        if (local_work_queue_) {
            local_work_queue_->push([task]() { (*task)(); });
        } else {
            // External caller pushes to queue 0
            queues_[0]->push([task]() { (*task)(); });
        }

        return res;
    }
};

} // namespace cpp_concurrency
