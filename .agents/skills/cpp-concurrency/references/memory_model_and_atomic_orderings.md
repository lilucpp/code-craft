# C++ Memory Model and Atomic Orderings

This reference details the low-level C++ memory model, atomic operations, and memory ordering guarantees based on Chapter 5 of *C++ Concurrency in Action*.

---

## 1. Memory Locations & Data Race Definition

### Memory Location
A memory location is either:
- An object of scalar type (integer, pointer, float), or
- A contiguous sequence of bit fields.

### Formal Data Race Definition
A **data race** occurs when:
1. Two or more concurrent threads access the **same memory location**, and
2. At least one access is a **write (modification)**, and
3. The threads do **not** use any mutual exclusion or synchronization to serialize the accesses.

> [!CAUTION]
> In C++, any data race results in **Undefined Behavior (UB)**. It is never "just a stale read"—the compiler and CPU are legally allowed to corrupt memory, eliminate checks, or execute unintended code paths!

---

## 2. The Six `std::memory_order` Models

```cpp
enum class memory_order : /* unspecified */ {
    relaxed,
    consume,
    acquire,
    release,
    acq_rel,
    seq_cst
};
```

| Memory Order | Operations Allowed | Key Semantics & Guarantees |
|---|---|---|
| `memory_order_seq_cst` | Load, Store, RMW | **Sequentially Consistent (Default)**: Enforces a single, globally agreed-upon total order across all threads. |
| `memory_order_relaxed` | Load, Store, RMW | **Relaxed**: Guarantees atomicity of the single variable, but **zero** cross-variable ordering or synchronization guarantees. |
| `memory_order_acquire` | Load | **Acquire**: No subsequent reads or writes in the current thread can be reordered *before* this load. |
| `memory_order_release` | Store | **Release**: No prior reads or writes in the current thread can be reordered *after* this store. |
| `memory_order_acq_rel` | Read-Modify-Write | **Acquire-Release**: Combines both acquire and release on a single operation (e.g., `fetch_add`, `compare_exchange`). |
| `memory_order_consume` | Load | **Consume**: Tracks data-dependencies. *(Discouraged in modern C++; compilers treat it as `acquire`)*. |

---

## 3. The Acquire-Release Synchronization Pattern

The most common, high-performance lock-free idiom is the **Acquire-Release Pair**:
- Thread A writes payload data into regular memory, then issues an atomic store with `memory_order_release` on a flag.
- Thread B loops on the flag with `memory_order_acquire`.
- Once Thread B sees the flag set, **all writes executed by Thread A before the release store are guaranteed visible to Thread B**!

```cpp
#include <atomic>
#include <string>
#include <thread>
#include <cassert>

std::string shared_payload; // Non-atomic data
std::atomic<bool> is_ready{false};

void producer() {
    shared_payload = "High performance payload data"; // 1. Write non-atomic data
    is_ready.store(true, std::memory_order_release);   // 2. Release fence: pushes 1 before 2
}

void consumer() {
    // 3. Acquire fence: pulls all writes that happened before the release
    while (!is_ready.load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    // 4. Guaranteed SAFE: no data race, shared_payload is 100% visible!
    assert(shared_payload == "High performance payload data");
}
```

---

## 4. `compare_exchange_weak` vs `compare_exchange_strong`

Atomic Compare-And-Swap (CAS) is the fundamental primitive for all lock-free algorithms:

```cpp
bool compare_exchange_weak(T& expected, T desired, 
                           memory_order success, memory_order failure) noexcept;
bool compare_exchange_strong(T& expected, T desired, 
                             memory_order success, memory_order failure) noexcept;
```

### When to Use Which
- **`compare_exchange_weak`**: May fail spuriously on architectures like ARM and PowerPC (due to cache line invalidation or interrupt contexts). **Must always be used inside a loop**. In a retry loop, `weak` generates faster machine instructions than `strong`.
- **`compare_exchange_strong`**: Guarantees failure only if `actual != expected`. Use when CAS is executed outside a loop (single attempt).

```cpp
// Canonical Lock-Free CAS Retry Loop:
T old_val = head_.load(std::memory_order_relaxed);
do {
    new_node->next = old_val;
} while (!head_.compare_exchange_weak(old_val, new_node,
                                      std::memory_order_release,
                                      std::memory_order_relaxed));
```

---

## 5. Anthony Williams' Pragmatic Memory Model Rules

1. **Start with `std::memory_order_seq_cst`**:
   During architecture and prototyping, always use default sequential consistency. Proving correctness of relaxed orderings without formal TLA+ or litmus verification is extremely prone to subtle bugs.
2. **Never use `relaxed` for flags or synchronization**:
   `memory_order_relaxed` only protects the single variable itself from tearing. It does **not** prevent the CPU or compiler from reordering surrounding reads and writes!
3. **Use Acquire-Release for one-way handoffs**:
   Acquire-Release is the optimal balance between performance and verifiable safety on modern out-of-order architectures (ARM64, x86-64).
