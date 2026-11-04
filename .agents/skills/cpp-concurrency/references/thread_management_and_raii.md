# Thread Management and RAII Lifecycle

This reference details thread creation, ownership transfer, parameter passing pitfalls, and RAII destruction guarantees based on Chapters 1 and 2 of *C++ Concurrency in Action* (Anthony Williams).

---

## 1. The Core Hazard of `std::thread`

In standard C++11/14/17, `std::thread` does **not** join automatically upon destruction.
If a `std::thread` object goes out of scope while still in a joinable state (`t.joinable() == true`), its destructor invokes **`std::terminate()`**, immediately crashing the entire process!

### Dangerous Legacy Pattern
```cpp
void risky_worker() {
    int local_state = 100;
    std::thread t([&local_state] {
        do_something(local_state); // DANGER: local_state may be destroyed before thread finishes!
    });
    do_something_else(); // If this throws an exception, t.~thread() runs while joinable -> std::terminate()!
    t.join();
}
```

---

## 2. RAII Thread Management

### Modern C++20 Solution: `std::jthread`
C++20 introduced `std::jthread`, which:
1. Automatically calls `join()` in its destructor if joinable.
2. Supports cooperative cancellation tokens (`std::stop_token`).

```cpp
#include <thread>
#include <chrono>

void modern_task(std::stop_token stoken) {
    while (!stoken.stop_requested()) {
        // Do chunk of work
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void modern_usage() {
    std::jthread worker(modern_task);
    // worker automatically signals stop and joins when going out of scope!
}
```

### Pre-C++20 Idiom: `joining_thread` Wrapper
In C++11/14/17 environments without C++20 support, wrap `std::thread` in an RAII container:

```cpp
#include <thread>
#include <utility>

class joining_thread {
public:
    joining_thread() noexcept = default;

    template <typename Callable, typename... Args>
    explicit joining_thread(Callable&& func, Args&&... args)
        : t_(std::forward<Callable>(func), std::forward<Args>(args)...) {}

    explicit joining_thread(std::thread t) noexcept : t_(std::move(t)) {}

    ~joining_thread() {
        if (joinable()) {
            join();
        }
    }

    joining_thread(joining_thread&& other) noexcept : t_(std::move(other.t_)) {}

    joining_thread& operator=(joining_thread&& other) noexcept {
        if (joinable()) {
            join();
        }
        t_ = std::move(other.t_);
        return *this;
    }

    joining_thread(const joining_thread&) = delete;
    joining_thread& operator=(const joining_thread&) = delete;

    void swap(joining_thread& other) noexcept { t_.swap(other.t_); }
    [[nodiscard]] bool joinable() const noexcept { return t_.joinable(); }
    void join() { t_.join(); }
    void detach() { t_.detach(); }
    [[nodiscard]] std::thread::id get_id() const noexcept { return t_.get_id(); }

private:
    std::thread t_;
};
```

---

## 3. Passing Arguments to Thread Functions

### 1. By-Value Decay Default
By default, arguments to `std::thread` are copied into internal storage as rvalues.
To pass a modifiable reference to the thread function, wrap the argument with `std::ref`:

```cpp
void update_data(int id, Widget& w);

void launch() {
    Widget w;
    // std::thread t(update_data, 1, w); // COMPILE ERROR: w decayed to value!
    std::jthread t(update_data, 1, std::ref(w)); // CORRECT: passes reference
}
```

### 2. Dangling Reference Pitfall (Stack Lifetime)
Never pass a pointer or reference to a local variable if the thread might outlive the calling function scope (especially if `detach()` is used):

```cpp
void oops() {
    char buffer[1024];
    sprintf(buffer, "%i", 42);
    // DANGER: buffer points to local array. By the time the thread converts it to std::string,
    // oops() may have already returned!
    std::thread t(process_string, buffer); 
    t.detach();
}

// FIX: Explicitly construct std::string before passing to avoid race on buffer
void fixed() {
    char buffer[1024];
    sprintf(buffer, "%i", 42);
    std::thread t(process_string, std::string(buffer)); // Copied safely
    t.detach();
}
```

---

## 4. Hardware Concurrency & Thread Sizing

Never hardcode the number of worker threads. Use `std::thread::hardware_concurrency()` to query available CPU cores:

```cpp
unsigned long const hardware_threads = std::thread::hardware_concurrency();
// hardware_concurrency() may return 0 if the query fails; provide a safe fallback
unsigned long const num_threads = hardware_threads != 0 ? hardware_threads : 2;
```

- **CPU-bound tasks**: Ideal thread count is equal to `hardware_concurrency` (avoid thread oversubscription and context-switch churn).
- **I/O-bound tasks**: Sizing can scale higher based on expected I/O wait latency ratios.
