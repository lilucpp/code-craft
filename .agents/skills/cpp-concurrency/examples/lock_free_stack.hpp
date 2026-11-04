#pragma once

#include <atomic>
#include <memory>
#include <optional>

namespace cpp_concurrency {

/**
 * @brief Lock-free concurrent stack with split reference counting.
 * 
 * Based on Chapter 7 of "C++ Concurrency in Action" (Anthony Williams).
 * 
 * Features:
 * 1. True lock-free execution (uses atomic compare_exchange_weak).
 * 2. Solves the Memory Reclamation Problem: internal count + external count
 *    ensures that a node is freed ONLY when no concurrent reader holds a reference.
 * 3. Exception-safe design: data is stored via std::shared_ptr to avoid throwing in pop().
 * 
 * Note on 64-bit platforms:
 * CountedNodePtr is 16 bytes; 128-bit atomics require linking with `-latomic` on GCC/Clang.
 */
template <typename T>
class LockFreeStack {
private:
    struct Node;

    struct CountedNodePtr {
        int external_count{0};
        Node* ptr{nullptr};
    };

    struct Node {
        std::shared_ptr<T> data;
        std::atomic<int> internal_count{0};
        CountedNodePtr next;

        explicit Node(T const& data_)
            : data(std::make_shared<T>(data_)) {}
    };

    std::atomic<CountedNodePtr> head_;

    void increase_head_count(CountedNodePtr& old_counter) {
        CountedNodePtr new_counter;
        do {
            new_counter = old_counter;
            ++new_counter.external_count;
        } while (!head_.compare_exchange_strong(old_counter, new_counter,
                                                std::memory_order_acquire,
                                                std::memory_order_relaxed));
        old_counter.external_count = new_counter.external_count;
    }

public:
    LockFreeStack() {
        head_.store(CountedNodePtr{0, nullptr}, std::memory_order_relaxed);
    }

    ~LockFreeStack() {
        while (pop()) {}
    }

    LockFreeStack(const LockFreeStack&) = delete;
    LockFreeStack& operator=(const LockFreeStack&) = delete;

    void push(T const& data) {
        CountedNodePtr new_node;
        new_node.ptr = new Node(data);
        new_node.external_count = 1;
        new_node.ptr->next = head_.load(std::memory_order_relaxed);

        while (!head_.compare_exchange_weak(new_node.ptr->next, new_node,
                                            std::memory_order_release,
                                            std::memory_order_relaxed));
    }

    std::shared_ptr<T> pop() {
        CountedNodePtr old_head = head_.load(std::memory_order_relaxed);
        for (;;) {
            increase_head_count(old_head);
            Node* const ptr = old_head.ptr;
            if (!ptr) {
                return std::shared_ptr<T>();
            }

            if (head_.compare_exchange_strong(old_head, ptr->next,
                                              std::memory_order_relaxed)) {
                std::shared_ptr<T> res;
                res.swap(ptr->data);

                int const count_increase = old_head.external_count - 2;
                if (ptr->internal_count.fetch_add(count_increase,
                                                  std::memory_order_release) == -count_increase) {
                    delete ptr;
                }
                return res;
            } else if (ptr->internal_count.fetch_sub(1, std::memory_order_relaxed) == 1) {
                ptr->internal_count.load(std::memory_order_acquire);
                delete ptr;
            }
        }
    }
};

} // namespace cpp_concurrency
