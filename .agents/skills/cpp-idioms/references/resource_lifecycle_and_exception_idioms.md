# Resource Lifecycle and Exception Safety Idioms

This reference details the idioms governing safe object construction, assignment, destruction, and scope management from *More C++ Idioms*.

---

## 1. Copy-and-Swap Idiom

### Intent & Problem
Writing exception-safe assignment operators (`operator=`) manually is notoriously error-prone:
1. Self-assignment checks (`if (this != &rhs)`) are needed.
2. If allocation throws during reallocation, the destination object can be left in an invalid or partially destructed state (violating the Strong Exception Safety guarantee).
3. Code is duplicated between copy constructors, destructors, and assignment operators.

### Modern C++ Solution
Reuse the copy constructor and a non-throwing `swap` function:

```cpp
#include <algorithm>
#include <cstddef>
#include <utility>

class DynamicBuffer {
public:
    explicit DynamicBuffer(std::size_t size)
        : size_(size), data_(size > 0 ? new int[size]() : nullptr) {}

    ~DynamicBuffer() {
        delete[] data_;
    }

    // Copy Constructor
    DynamicBuffer(const DynamicBuffer& other)
        : size_(other.size_), data_(other.size_ > 0 ? new int[other.size_] : nullptr) {
        if (data_) {
            std::copy(other.data_, other.data_ + size_, data_);
        }
    }

    // Move Constructor
    DynamicBuffer(DynamicBuffer&& other) noexcept
        : size_(std::exchange(other.size_, 0)), data_(std::exchange(other.data_, nullptr)) {}

    // Unified Assignment Operator (takes by value!)
    // Handles BOTH copy-assignment and move-assignment automatically!
    DynamicBuffer& operator=(DynamicBuffer other) noexcept {
        swap(*this, other);
        return *this;
    }

    // Non-throwing hidden friend swap
    friend void swap(DynamicBuffer& first, DynamicBuffer& second) noexcept {
        using std::swap;
        swap(first.size_, second.size_);
        swap(first.data_, second.data_);
    }

private:
    std::size_t size_{0};
    int* data_{nullptr};
};
```

### Key Advantages
1. **Strong Exception Safety**: If memory allocation throws, it throws during argument copy construction before `operator=` body begins. The destination object remains untouched.
2. **Zero Code Duplication**: Destroys old resources naturally when temporary parameter `other` exits scope.
3. **Implicit Move Optimization**: Passing an rvalue moves directly into parameter `other` without separate `operator=(DynamicBuffer&&)` boilerplate.

---

## 2. Construct On First Use Idiom (Meyers' Singleton)

### Intent & Problem
In C++, the relative order of dynamic initialization of global/static variables across different translation units (.cpp files) is **undefined** (known as the **Static Initialization Order Fiasco**, or SIOF). If global object `A` in `a.cpp` references global object `B` in `b.cpp` during construction, and `B` hasn't been initialized yet, undefined behavior or crashes occur.

### Modern Solution
Wrap the static object in a function returning a reference. In C++11 and later, function-local static initialization is guaranteed by the language standard to be **thread-safe** (Magic Statics):

```cpp
class LoggerService {
public:
    static LoggerService& instance() {
        // Initialized on first call; completely thread-safe in C++11+
        static LoggerService global_instance;
        return global_instance;
    }

    void log(std::string_view message);

private:
    LoggerService() = default;
    ~LoggerService() = default;
    LoggerService(const LoggerService&) = delete;
    LoggerService& operator=(const LoggerService&) = delete;
};
```

---

## 3. Scope Guard Idiom

### Intent
Ensure arbitrary cleanup logic (releasing locks, closing file descriptors, rolling back database transactions, resetting flags) executes upon leaving the current lexical scope, whether by normal return or by exception.

```cpp
#include <concepts>
#include <utility>

template <typename Func>
    requires std::invocable<Func>
class ScopeGuard {
public:
    explicit ScopeGuard(Func cleanup) : cleanup_(std::move(cleanup)), active_(true) {}

    ~ScopeGuard() {
        if (active_) {
            cleanup_();
        }
    }

    // Dismiss guard when transaction commits successfully
    void dismiss() noexcept {
        active_ = false;
    }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&& other) noexcept
        : cleanup_(std::move(other.cleanup_)), active_(std::exchange(other.active_, false)) {}

private:
    Func cleanup_;
    bool active_;
};

// Deduction guide
template <typename Func>
ScopeGuard(Func) -> ScopeGuard<Func>;
```

### Usage
```cpp
void perform_database_update() {
    start_transaction();
    ScopeGuard rollback_on_fail([]() {
        rollback_transaction();
    });

    step1();
    step2(); // If step2 throws an exception, rollback_transaction() runs automatically!

    commit_transaction();
    rollback_on_fail.dismiss(); // Disarmed: do not rollback
}
```

---

## 4. Checked Delete Idiom

### Intent & Problem
In C++, `delete ptr;` on a pointer to an **incomplete type** (a forward-declared class `class Incomplete; Incomplete* ptr;`) compiles with a compiler warning, but its destructor is **not called**, leading to silent resource leaks.

### Idiom Solution
Enforce compile-time verification that the type is fully defined:

```cpp
template <typename T>
inline void checked_delete(T* ptr) {
    // Evaluating sizeof requires complete type definition.
    // Incomplete type causes compile-time error: invalid application of 'sizeof' to incomplete type
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void)sizeof(type_must_be_complete);
    delete ptr;
}
```
*(In modern C++: `static_assert(sizeof(T) > 0, "Type must be complete before deletion");`)*
