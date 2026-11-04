# Mutexes, Locks, and Deadlock Defense

This reference covers mutual exclusion, lock management strategies, deadlock avoidance algorithms, and lazy initialization patterns from Chapter 3 of *C++ Concurrency in Action*.

---

## 1. Mutex Types and RAII Lock Wrappers

| Mutex / Lock Type | Header | Purpose & Usage Guidelines |
|---|---|---|
| `std::mutex` | `<mutex>` | Standard non-recursive exclusive lock. Default choice. |
| `std::recursive_mutex` | `<mutex>` | Reentrant lock. Avoid: indicates poor design or design smell. |
| `std::timed_mutex` | `<mutex>` | Adds `try_lock_for()` and `try_lock_until()`. |
| `std::shared_mutex` | `<shared_mutex>` | Reader-Writer lock (C++17). Multiple concurrent readers, single exclusive writer. |
| `std::lock_guard<Mutex>` | `<mutex>` | Minimal RAII scope guard. Acquires on construction, releases on destruction. |
| `std::unique_lock<Mutex>` | `<mutex>` | Flexible RAII wrapper. Supports deferred locking, manual `unlock()`, move semantics, and `condition_variable`. |
| `std::scoped_lock<Mutexes...>` | `<mutex>` | **C++17 Recommended**: Variadic deadlock-free multi-mutex RAII wrapper. |
| `std::shared_lock<Mutex>` | `<shared_mutex>` | RAII wrapper acquiring shared (reader) ownership on `std::shared_mutex`. |

---

## 2. Deadlock: Causes and Defense Rules

A deadlock occurs when two or more threads cannot proceed because each is waiting for a lock held by the other.

### Anthony Williams' 4 Deadlock Defense Guidelines

#### 1. Avoid Nested Locks
Do not acquire a lock if you already hold one. If multiple locks are truly required, acquire them simultaneously using `std::scoped_lock` or `std::lock`.

#### 2. Avoid Calling User-Supplied Code While Holding a Lock
Never invoke function pointers, virtual methods, or callback functors while holding a mutex. You cannot know if the callback attempts to acquire other locks or re-enter the current subsystem.

#### 3. Acquire Locks in a Fixed Global Order
If two objects of the same class need to be locked simultaneously (e.g., swapping two containers or transferring funds between accounts), locking order based on argument order leads to deadlocks (`A.swap(B)` vs `B.swap(A)`).

```cpp
// Deadlock-free multi-lock swap in C++17
void swap_accounts(Account& lhs, Account& rhs) {
    if (&lhs == &rhs) return; // Prevent self-locking deadlock!

    // std::scoped_lock uses a deadlock-avoidance algorithm (equivalent to std::lock)
    std::scoped_lock guard(lhs.mutex_, rhs.mutex_);
    std::swap(lhs.balance_, rhs.balance_);
}
```

#### 4. Use a Lock Hierarchy (`hierarchical_mutex`)
Assign an explicit integer hierarchy value to every mutex. A thread holding a mutex at level $N$ is only permitted to acquire mutexes at level $< N$. If it attempts to acquire a mutex at level $\ge N$, an exception or assertion is immediately thrown at runtime.

---

## 3. Read-Write Locking (`std::shared_mutex`)

Use `std::shared_mutex` for data structures with **high read-to-write ratios** (e.g., DNS caches, routing tables, configuration maps):

```cpp
#include <map>
#include <shared_mutex>
#include <string>

class ThreadSafeDNSCache {
public:
    std::string find_entry(const std::string& domain) const {
        // Shared lock: multiple threads can read concurrently!
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        auto it = entries_.find(domain);
        return it != entries_.end() ? it->second : "";
    }

    void update_entry(const std::string& domain, const std::string& ip) {
        // Exclusive lock: prevents all readers and other writers
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        entries_[domain] = ip;
    }

private:
    mutable std::shared_mutex mutex_;
    std::map<std::string, std::string> entries_;
};
```

---

## 4. Thread-Safe Lazy Initialization (`std::call_once`)

### The Double-Checked Locking Anti-Pattern
In pre-C++11 code, developers wrote Double-Checked Locking (DCLP) using raw pointers and flags:
```cpp
// BROKEN / UNDEFINED BEHAVIOR:
if (!resource_ptr) {
    std::lock_guard<std::mutex> lk(m);
    if (!resource_ptr) {
        resource_ptr = new Resource(); // CPU/Compiler reorders write vs init!
    }
}
```
Due to out-of-order execution, thread B might observe a non-null `resource_ptr` pointing to raw, uninitialized memory!

### Modern C++ Solution 1: Function-Local Static (Meyers' Singleton)
Guaranteed thread-safe by the C++11 standard (§6.7 [stmt.dcl]):
```cpp
Resource& get_resource() {
    static Resource instance; // Thread-safe magic static
    return instance;
}
```

### Modern C++ Solution 2: `std::call_once` & `std::once_flag`
Ideal for member variables and lazy connection establishment:
```cpp
class DatabaseClient {
public:
    void send_query(std::string_view sql) {
        // Guarantees init_connection runs exactly once, even under concurrent access
        std::call_once(connection_flag_, &DatabaseClient::init_connection, this);
        // Execute query...
    }

private:
    void init_connection();
    std::once_flag connection_flag_;
};
```
