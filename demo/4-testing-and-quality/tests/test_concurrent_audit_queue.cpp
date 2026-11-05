#include "test_framework.hpp"
#include "filegate/concurrent_audit_queue.hpp"
#include <thread>
#include <vector>
#include <atomic>
#include <string>

using namespace filegate;

// ============================================================================
// 测试套件 4: ConcurrentAuditQueueTest (并发安全与细粒度锁压力测试)
// ============================================================================

TEST(ConcurrentQueueTest, MultiProducerMultiConsumerZeroLoss) {
    // 1. Arrange: 4个生产者 + 2个消费者
    concurrent::ConcurrentAuditQueue<int> queue;
    const int kProducers = 4;
    const int kItemsPerProducer = 1000;
    const int kTotalItems = kProducers * kItemsPerProducer;
    std::atomic<int> consumed_sum{0};
    std::atomic<int> consumed_count{0};

    // 2. Act: 启动消费者
    std::vector<std::thread> consumers;
    for (int c = 0; c < 2; ++c) {
        consumers.emplace_back([&] {
            while (true) {
                auto val = queue.WaitAndPop();
                if (!val) break; // 队列关闭且已排空，退出
                consumed_sum.fetch_add(*val, std::memory_order_relaxed);
                if (consumed_count.fetch_add(1, std::memory_order_relaxed) + 1 == kTotalItems) {
                    queue.Shutdown(); // 全部消费完毕，优雅关闭队列唤醒其他消费者
                }
            }
        });
    }

    // 启动生产者
    std::vector<std::thread> producers;
    for (int p = 0; p < kProducers; ++p) {
        producers.emplace_back([&queue, kItemsPerProducer] {
            for (int i = 0; i < kItemsPerProducer; ++i) {
                queue.Push(1); // 每个item值为1
            }
        });
    }

    for (auto& t : producers) t.join();
    queue.Shutdown(); // 确保唤醒可能在等待的消费者
    for (auto& t : consumers) t.join();

    // 3. Assert: 验证 4000 个 item 严格零丢失，累计和精确为 4000
    EXPECT_EQ(consumed_count.load(), kTotalItems);
    EXPECT_EQ(consumed_sum.load(), kTotalItems);
}

TEST(ConcurrentQueueTest, NonBlockingTryPopOnEmptyQueue) {
    // 1. Arrange: 空队列
    concurrent::ConcurrentAuditQueue<std::string> queue;

    // 2. Act
    auto item = queue.TryPop();

    // 3. Assert
    EXPECT_TRUE(item == nullptr);
}

TEST(ConcurrentQueueTest, GracefulShutdownWakesWaitingThreads) {
    // 1. Arrange: 阻塞等待的消费者
    concurrent::ConcurrentAuditQueue<std::string> queue;
    std::atomic<bool> worker_finished{false};

    std::thread worker([&] {
        auto item = queue.WaitAndPop(); // 队列为空，将在此阻塞
        EXPECT_TRUE(item == nullptr);   // Shutdown 唤醒后应返回 nullptr
        worker_finished.store(true);
    });

    // 等待线程进入等待状态
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    // 2. Act: 触发优雅关闭
    queue.Shutdown();
    worker.join();

    // 3. Assert
    EXPECT_TRUE(worker_finished.load());
}
