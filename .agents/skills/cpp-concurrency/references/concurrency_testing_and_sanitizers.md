# Concurrency Testing, Debugging, and ThreadSanitizer (TSAN)

This reference covers bug classification, automated sanitizer tooling, and stress-testing strategies based on Chapter 11 of *C++ Concurrency in Action*.

---

## 1. Concurrency Bug Taxonomy

| Bug Class | Root Cause | Typical Symptom |
|---|---|---|
| **Deadlock** | Circular wait on locks or resources. | Process hangs indefinitely; 0% CPU utilization. |
| **Data Race** | Unsynchronized concurrent read/write to same address. | Heisenbugs, memory corruption, corrupted invariants. |
| **Livelock** | Threads actively reacting to each other without progress. | Process hangs; 100% CPU utilization across cores. |
| **Spurious / Lost Wakeup** | Missing predicate loop on condition variable. | Threads sleep forever even after notify; premature execution. |
| **ABA Problem** | Memory address reused while pointer is held in lock-free CAS. | Silent data corruption, dangling pointer dereferences. |

---

## 2. Locating Bugs with ThreadSanitizer (TSAN)

ThreadSanitizer (TSAN) is the industry-standard tool built into Clang and GCC for detecting data races and deadlocks at runtime.

### Enabling TSAN in GCC/Clang
Compile and link with `-fsanitize=thread -g -O1`:

```bash
g++ -std=c++20 -fsanitize=thread -g -O1 test_concurrency.cpp -o test_concurrency
./test_concurrency
```

### Interpreting TSAN Output
When a data race occurs, TSAN prints:
1. The memory location where the race occurred.
2. The stack trace of the write operation.
3. The previous conflicting read or write operation stack trace.
4. The thread IDs involved.

> [!TIP]
> TSAN adds ~2x to 5x CPU overhead and ~5x to 10x memory overhead. Always build a dedicated TSAN CI pipeline to catch race regressions automatically on pull requests.

---

## 3. Designing Unit Tests for Concurrent Code

Unit testing multithreaded code is notoriously challenging because races depend on unpredictable OS thread scheduling interleavings.

### 1. The Multi-Threaded Stress Loop
Run thousands of iterations with randomized or alternating thread interleavings:

```cpp
#include <cassert>
#include <thread>
#include <vector>

void test_concurrent_push_pop() {
    ThreadSafeQueue<int> queue;
    constexpr int items_per_thread = 10000;
    constexpr int num_producers = 4;
    constexpr int num_consumers = 4;

    std::vector<std::jthread> producers;
    for (int p = 0; p < num_producers; ++p) {
        producers.emplace_back([&queue, p] {
            for (int i = 0; i < items_per_thread; ++i) {
                queue.push(p * items_per_thread + i);
            }
        });
    }

    std::atomic<int> total_consumed{0};
    std::vector<std::jthread> consumers;
    for (int c = 0; c < num_consumers; ++c) {
        consumers.emplace_back([&queue, &total_consumed] {
            for (int i = 0; i < items_per_thread; ++i) {
                int val = queue.wait_and_pop();
                total_consumed.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    producers.clear(); // Joins all producers
    consumers.clear(); // Joins all consumers

    assert(total_consumed.load() == num_producers * items_per_thread);
}
```

### 2. Injecting Artificial Interleaving Delays
In test builds, inject small random yields (`std::this_thread::yield()`) between critical steps (e.g., between loading head and performing CAS) to dramatically increase the probability of exposing rare race windows during testing.
