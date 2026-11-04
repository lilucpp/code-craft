#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

namespace cpp_concurrency {

/**
 * @brief Industrial-grade thread-safe queue using fine-grained head and tail mutexes.
 * 
 * Based on Chapter 6 of "C++ Concurrency in Action" (Anthony Williams).
 * 
 * Key Architecture:
 * 1. Dummy Node Invariant: The queue always contains at least one dummy node.
 * 2. Separate Locks: `head_mutex_` protects pops; `tail_mutex_` protects pushes.
 * 3. Zero Producer-Consumer Contention: A producer pushing to the tail never
 *    blocks a consumer popping from the head in a non-empty queue!
 */
template <typename T>
class ThreadSafeQueue {
private:
    struct Node {
        std::shared_ptr<T> data;
        std::unique_ptr<Node> next;
    };

    std::mutex head_mutex_;
    std::unique_ptr<Node> head_;

    std::mutex tail_mutex_;
    Node* tail_{nullptr};

    std::condition_variable data_cond_;

    Node* get_tail() {
        std::lock_guard<std::mutex> tail_lock(tail_mutex_);
        return tail_;
    }

    std::unique_ptr<Node> pop_head() {
        std::unique_ptr<Node> old_head = std::move(head_);
        head_ = std::move(old_head->next);
        return old_head;
    }

    std::unique_lock<std::mutex> wait_for_data() {
        std::unique_lock<std::mutex> head_lock(head_mutex_);
        data_cond_.wait(head_lock, [this] { return head_.get() != get_tail(); });
        return head_lock;
    }

    std::unique_ptr<Node> wait_pop_head() {
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        return pop_head();
    }

public:
    ThreadSafeQueue()
        : head_(std::make_unique<Node>()), tail_(head_.get()) {}

    ~ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    void push(T new_value) {
        auto new_data = std::make_shared<T>(std::move(new_value));
        auto p = std::make_unique<Node>();
        Node* const new_tail = p.get();

        {
            std::lock_guard<std::mutex> tail_lock(tail_mutex_);
            tail_->data = std::move(new_data);
            tail_->next = std::move(p);
            tail_ = new_tail;
        }

        data_cond_.notify_one();
    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_ptr<Node> const old_head = wait_pop_head();
        return old_head->data;
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> head_lock(head_mutex_);
        if (head_.get() == get_tail()) {
            return false;
        }
        value = std::move(*head_->data);
        pop_head();
        return true;
    }

    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> head_lock(head_mutex_);
        if (head_.get() == get_tail()) {
            return std::shared_ptr<T>();
        }
        std::unique_ptr<Node> const old_head = pop_head();
        return old_head->data;
    }

    [[nodiscard]] bool empty() {
        std::lock_guard<std::mutex> head_lock(head_mutex_);
        return head_.get() == get_tail();
    }
};

} // namespace cpp_concurrency
