# 物理内存布局、缓存微架构与分级分配器规范

> **理论源泉**：Brendan Gregg《Systems Performance (2nd Ed)》Ch. 6-7、Anthony Williams《C++ Concurrency in Action (2nd Ed)》Ch. 8

---

## 1. 结构体对齐法则：消除隐式 Padding 空洞

### 1.1 硬件物理原理
CPU 访问内存总是以总线宽度（如 32 位或 64 位，4 字节或 8 字节）对齐寻址。如果一个 8 字节指针位于奇数地址（未对齐），现代 x86/ARM CPU 必须发起两次总线读取并执行拼接待价（甚至直接触发未对齐异常）。为此，编译器会自动在结构体成员之间插入无用字节（Padding）。

### 1.2 降序对齐黄金法则 (Field Reordering)
所有结构体成员**必须严格按照类型尺寸由大到小排序**：
`8 字节类型 (指针、uint64_t、double) -> 4 字节类型 (uint32_t、float) -> 2 字节类型 (uint16_t) -> 1 字节类型 (char、bool)`。

#### ❌ 脆弱反例（乱序导致浪费 50% 内存）
```cpp
struct BadNode {
    bool is_valid;       // 1 字节
    // [编译器填充 7 字节 Padding]
    uint64_t session_id; // 8 字节
    char status;         // 1 字节
    // [编译器填充 3 字节 Padding]
    uint32_t sequence;   // 4 字节
    void* buffer;        // 8 字节
};
// sizeof(BadNode) = 32 字节！有效数据仅 20 字节，37.5% 空间为纯垃圾填充！
// 一个 64B Cache Line 只能容纳 2 个对象。
```

####  降序重排优雅实践
```cpp
struct OptimizedNode {
    // 8 字节对齐区
    uint64_t session_id; // 8 字节
    void* buffer;        // 8 字节
    // 4 字节对齐区
    uint32_t sequence;   // 4 字节
    // 1 字节对齐区
    bool is_valid;       // 1 字节
    char status;         // 1 字节
    // [尾部仅需填充 2 字节以对齐 8 字节边界]
};
// sizeof(OptimizedNode) = 24 字节（体积缩减 25%）
// 如果连续存储，相同内存下能容纳更多对象，L1 Cache 命中率显著提升！
```

---

## 2. 伪共享防御（False Sharing Defense）

### 2.1 物理根因：Cache Line Bouncing（缓存行颠簸）
现代 CPU 的 L1/L2 缓存以 **64 字节（Cache Line）** 为最小同步粒度。
如果线程 A（运行在 Core 0）频繁写入变量 `X`，线程 B（运行在 Core 1）频繁写入变量 `Y`，而 `X` 和 `Y` 在内存中相邻且位于**同一个 64 字节 Cache Line 内**：
- Core 0 每次写 `X`，MESI 协议会导致 Core 1 上的整条 Cache Line 强制变为 Invalid（失效）；
- Core 1 接着写 `Y`，又强制 Core 0 上的 Cache Line 失效；
- 两个核心虽然修改不同的逻辑变量，却在 CPU 总线上陷入疯狂的缓存一致性风暴，导致并发吞吐暴跌！

### 2.2 防御规范
跨线程并发高频读写变量，必须显式隔离：

```cpp
#include <new>

// C++17 标准常量，硬件破坏性干扰尺寸（通常为 64 字节）
#ifdef __cpp_lib_hardware_interference_size
    using std::hardware_destructive_interference_size;
#else
    constexpr size_t hardware_destructive_interference_size = 64;
#endif

// 强制对齐到独立的 Cache Line，避免多核伪共享
struct alignas(hardware_destructive_interference_size) AlignedCounter {
    std::atomic<uint64_t> count{0};
};
```

---

## 3. 分级分配器架构规范 (Tiered Allocator Architecture)

严禁在高性能循环或请求处理路径中无节制调用原生 `malloc` / `new`（避免全局堆锁争用与严重碎片化）。

### 3.1 短生命周期（请求级/事务级）：单调线性分配器（Arena / Bump Allocator）
* **模式机制**：预先分配整块大内存，每次分配仅递增游标指针：
  ```cpp
  class Arena {
  public:
      explicit Arena(size_t capacity) : buffer_(new uint8_t[capacity]), capacity_(capacity) {}
      ~Arena() { delete[] buffer_; }

      void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
          size_t current = reinterpret_cast<uintptr_t>(buffer_ + offset_);
          size_t padding = (alignment - (current % alignment)) % alignment;
          if (offset_ + padding + size > capacity_) throw std::bad_alloc();
          offset_ += padding;
          void* ptr = buffer_ + offset_;
          offset_ += size;
          return ptr;
      }

      // 请求处理结束，纳秒级瞬间释放全部对象
      void reset() noexcept { offset_ = 0; }
  private:
      uint8_t* buffer_;
      size_t capacity_;
      size_t offset_{0};
  };
  ```
* **核心价值**：分配仅需 2 条汇编指令，整体销毁无需调用各个子对象的 `free`，内存局部性极佳。

### 3.2 中等生命周期（高频固定尺寸）：Slab / 侵入式对象池（Object Pool）
* **机制**：通过 Free-List 回收，将空闲链表指针直接复用在对象自身的内存块内（零额外内存开销），回收时仅放回空闲链表，绝不归还操作系统。
