# 并发状态流转、内存序与单向所有权模型

> **理论源泉**：Anthony Williams《C++ Concurrency in Action (2nd Ed)》Ch. 5-7、《OSTEP》Ch. 28-33

---

## 1. 严格的有向无环所有权（DAG Ownership Topology）

### 1.1 黄金法则
在进程内对象模型中，**所有权必须形成严格的有向无环图（DAG），严禁网状持有或循环持有**。

### 1.2 严控 `shared_ptr` 的泛滥
* **原子引用计数的性能杀手**：
  `std::shared_ptr` 的每一次拷贝与销毁都会执行硬件级原子增减指令（如 x86 上的 `lock xadd`）。在高并发多核环境下，这会造成 CPU 缓存总线争用（Bus Contention）与明显的性能退化。
* **职责模糊**：
  滥用 `shared_ptr` 意味着“没有任何模块知道这个对象何时被释放”，极易因循环引用造成内存泄漏。
* **标准所有权分工**：
  1. **独占持有者（Owner）**：使用 `std::unique_ptr` 或直接在栈/容器中持有值；
  2. **短周期借用者（Borrower / Observer）**：只接收裸指针 `T*`、引用 `const T&` 或轻量视图 `std::span` / `std::string_view`；
  3. **生命周期绝不超越所有者**。

---

## 2. C++ 内存序（Memory Order）模型与无锁流转

绝大多数无锁数据结构的性能提升，来自于**放弃全系统强一致（`seq_cst`），转为使用成对的 `acquire` 和 `release` 内存序**。

### 2.1 内存序核心等级
1. **`memory_order_relaxed`**：仅保证当前单个原子变量自身的操作具有原子性，不保证任何跨变量的内存读写重排限制（适合无依赖的简单计数器）。
2. **`memory_order_release`（写屏障语义）**：确保当前线程中位于该原子写之前的**所有内存读写操作**，绝不能被重排到该写操作之后。
3. **`memory_order_acquire`（读屏障语义）**：确保当前线程中位于该原子读之后的**所有内存读写操作**，绝不能被重排到该读操作之前。
4. **`memory_order_seq_cst`（顺序一致性，默认值）**：全系统严格单调时序，开销最大。

### 2.2 Acquire-Release 经典配对（无锁状态发布）
```cpp
#include <atomic>
#include <string>

struct Payload {
    std::string message;
    int code;
};

Payload* g_payload = nullptr;
std::atomic<bool> g_ready{false};

// 生产者线程
void producer() {
    Payload* p = new Payload{"Data ready", 200}; // 普通内存写入
    g_payload = p;
    // release 确保在此之前的 g_payload 赋值对后续的 acquire 读者立即可见
    g_ready.store(true, std::memory_order_release);
}

// 消费者线程
void consumer() {
    // acquire 确保读到 true 后，后续读取 g_payload 绝不会读到旧数据或空指针
    while (!g_ready.load(std::memory_order_acquire)) {
        // 自旋等待或让出 CPU
    }
    // 安全访问 g_payload，无需互斥锁！
    process(g_payload->message, g_payload->code);
}
```

---

## 3. 并发架构范式：无共享（Thread-Confined）与 SPSC 环形队列

### 3.1 “不要通过共享内存来通信，而要通过通信来共享内存”
* 复杂的共享状态加锁（`std::mutex`、`shared_mutex`）往往是死锁、优先级反转和性能瓶颈的根源；
* **Reactor / Thread-per-Core 模式**（如 Nginx、Seastar、Redis）：每个线程拥有完全私有的状态机，零锁争用。

### 3.2 SPSC（单生产单消费）无锁环形队列 (RingBuffer)
* 生产者只写 `tail_`，消费者只读 `head_`；
* 通过环形数组和模运算（或容量为 $2^n$ 时用位与 `index & (size - 1)`）循环推进；
* `tail_` 与 `head_` 必须通过 `alignas(64)` 隔离，彻底消除伪共享。
