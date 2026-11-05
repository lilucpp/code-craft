#pragma once

#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "filegate/types.hpp"

namespace filegate::concurrent {

// ============================================================================
// ConcurrentAuditQueue - 细粒度分离锁并发队列 (Anthony Williams 经典架构)
// 依据《C++ Concurrency in Action》(第2版) 第6.2节：
// 1. 头尾独立互斥锁 (head_mutex_ / tail_mutex_)，入队与出队完全并发互不阻塞；
// 2. 哨兵空节点 (Dummy Node) 消除空队列竞态；
// 3. alignas(64) 隔离头尾锁变量，消除多核 CPU L1 Cache 伪共享 (False Sharing)。
// ============================================================================
template <typename T>
class ConcurrentAuditQueue {
private:
    struct Node {
        std::shared_ptr<T> data;
        std::unique_ptr<Node> next;
    };

    // 头部区域 (alignas(64) 独占 Cache Line)
    alignas(64) std::mutex head_mutex_;
    std::unique_ptr<Node> head_;

    // 尾部区域 (alignas(64) 独占独立 Cache Line，防止伪共享)
    alignas(64) std::mutex tail_mutex_;
    Node* tail_;

    std::condition_variable cond_var_;
    std::atomic<bool> is_shutdown_{false};

    Node* GetTail() {
        std::lock_guard<std::mutex> tail_lock(tail_mutex_);
        return tail_;
    }

    std::unique_ptr<Node> PopHead() {
        std::lock_guard<std::mutex> head_lock(head_mutex_);
        if (head_.get() == GetTail()) {
            return nullptr;
        }
        std::unique_ptr<Node> old_head = std::move(head_);
        head_ = std::move(old_head->next);
        return old_head;
    }

public:
    ConcurrentAuditQueue() 
        : head_(std::make_unique<Node>()), tail_(head_.get()) {}

    ~ConcurrentAuditQueue() {
        Shutdown();
    }

    // 禁用拷贝与赋值
    ConcurrentAuditQueue(const ConcurrentAuditQueue&) = delete;
    ConcurrentAuditQueue& operator=(const ConcurrentAuditQueue&) = delete;

    // 入队操作 (只获取尾锁，与出队完全并发)
    void Push(T new_value) {
        auto new_data = std::make_shared<T>(std::move(new_value));
        auto p = std::make_unique<Node>();
        Node* const new_tail = p.get();

        {
            std::lock_guard<std::mutex> tail_lock(tail_mutex_);
            tail_->data = new_data;
            tail_->next = std::move(p);
            tail_ = new_tail;
        }
        cond_var_.notify_one();
    }

    // 阻塞式出队 (只获取头锁)
    std::shared_ptr<T> WaitAndPop() {
        std::unique_lock<std::mutex> head_lock(head_mutex_);
        cond_var_.wait(head_lock, [&] {
            return head_.get() != GetTail() || is_shutdown_.load();
        });

        if (head_.get() == GetTail()) {
            return nullptr; // 队列已关闭且为空
        }
        std::unique_ptr<Node> old_head = std::move(head_);
        head_ = std::move(old_head->next);
        return old_head->data;
    }

    // 非阻塞式尝试出队
    std::shared_ptr<T> TryPop() {
        std::unique_ptr<Node> old_head = PopHead();
        return old_head ? old_head->data : nullptr;
    }

    // 关闭队列并唤醒所有等待者
    void Shutdown() {
        is_shutdown_.store(true);
        cond_var_.notify_all();
    }
};

} // namespace filegate::concurrent
