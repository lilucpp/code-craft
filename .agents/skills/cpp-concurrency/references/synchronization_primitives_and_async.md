# Synchronization Primitives, Condition Variables, and Async Tasks

This reference covers condition variables, futures, promises, packaged tasks, and message-passing synchronization patterns based on Chapter 4 of *C++ Concurrency in Action*.

---

## 1. `std::condition_variable` and Spurious Wakeups

A condition variable enables threads to wait for a shared condition to become true without CPU-intensive spin-polling.

### The Spurious Wakeup Hazard
Operating systems may wake waiting threads even when **no** thread called `notify_one()` or `notify_all()` (due to kernel signal interrupts or OS scheduling quirks). Furthermore, between the moment a thread is notified and the moment it re-acquires the mutex, another thread may have already consumed the data!

### The Golden Rule: Always Use a Predicate
Never call `wait()` without a predicate loop:

```cpp
#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class SimpleThreadSafeQueue {
public:
    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(value));
        }
        cv_.notify_one(); // Wake one waiting consumer
    }

    T wait_and_pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        // MANDATORY: cv.wait with predicate loop
        // Equivalent to: while(queue_.empty()) cv_.wait(lock);
        cv_.wait(lock, [this] { return !queue_.empty(); });

        T result = std::move(queue_.front());
        queue_.pop();
        return result;
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<T> queue_;
};
```

---

## 2. One-Off Events: Futures and Promises

While condition variables are ideal for recurring events (like continuous queue processing), one-off events (like asynchronous computations or network responses) are cleanly represented by **Futures** (`std::future<T>`) and **Promises** (`std::promise<T>`).

### 1. `std::promise` & `std::future` Pipeline
A `promise` is the write-end; a `future` is the read-end. The promise moves data or exceptions into the shared state:

```cpp
#include <future>
#include <iostream>
#include <thread>

void compute_data(std::promise<int> result_promise) {
    try {
        int calculation = 42; // Heavy work
        result_promise.set_value(calculation);
    } catch (...) {
        result_promise.set_exception(std::current_exception()); // Transmit exception
    }
}

void consumer() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();

    std::jthread worker(compute_data, std::move(prom));

    // Blocks until promise sets value or exception
    int value = fut.get(); 
    std::cout << "Result: " << value << '\n';
}
```

### 2. `std::packaged_task`
Wraps any callable target (function, lambda, functor) so that it can be invoked asynchronously, automatically delivering its return value or exception to a `std::future`:

```cpp
std::packaged_task<double(int, double)> task([](int a, double b) {
    return a * b;
});

std::future<double> result = task.get_future();
std::thread t(std::move(task), 10, 3.14);
t.detach();

double val = result.get(); // 31.4
```

---

## 3. High-Level Concurrency with `std::async`

`std::async` runs a function asynchronously and returns a `std::future` representing the outcome.

### Launch Policies
- `std::launch::async`: Guarantees execution on a separate asynchronous thread.
- `std::launch::deferred`: Defers execution until `future.get()` or `future.wait()` is called (runs synchronously on calling thread).
- Default (`std::launch::async | std::launch::deferred`): The runtime chooses based on system load. **Caution**: If system is overloaded, it may defer indefinitely until `.get()`!

```cpp
// Always specify std::launch::async if concurrency is strictly required:
auto fut = std::async(std::launch::async, [] {
    return download_data();
});
```

### The Destructor Trap of `std::async`
> [!WARNING]
> If the `std::future` returned by `std::async(std::launch::async, ...)` is **not** stored in a variable, its temporary destructor **blocks until the task finishes**!
> ```cpp
> std::async(std::launch::async, task1); // BLOCKS synchronously here!
> std::async(std::launch::async, task2); // Only starts after task1 finishes!
> ```
> Always bind the return value: `auto f1 = std::async(...); auto f2 = std::async(...);`

---

## 4. `std::shared_future` (One-to-Many Synchronization)

A standard `std::future` is move-only; only a single thread can call `.get()`.
When multiple threads need to wait for the same event, convert the future to a `std::shared_future` using `fut.share()`:

```cpp
std::promise<void> start_signal;
std::shared_future<void> shared_fut = start_signal.get_future().share();

for (int i = 0; i < 10; ++i) {
    std::jthread([shared_fut] {
        shared_fut.wait(); // All 10 worker threads wait simultaneously!
        do_work();
    });
}

start_signal.set_value(); // Releases all workers at once!
```
