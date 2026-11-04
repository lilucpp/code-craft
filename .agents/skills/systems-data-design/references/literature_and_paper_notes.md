# 系统级数据设计经典著作与奠基论文论点速查

---

## 1. 《C++ Concurrency in Action (2nd Ed)》- Anthony Williams
* **Ch. 5 (内存模型与原子操作)**：
  - 核心定义：对象的内存位置（Memory Location）、修改顺序（Modification Order）。
  - 同步原语：`memory_order_seq_cst`（顺序一致）vs `acquire-release`（同步依赖传递）vs `relaxed`（单变量原子无序）。
* **Ch. 6 & 7 (并发与无锁数据结构)**：
  - 无锁定义：至少有一个线程能持续向前推进（Lock-Free）；所有线程都能在有限步内完成（Wait-Free）。
  - 内存回收难题：ABA 问题、Hazard Pointers 危险指针、Epoch-based 回收机制。
* **Ch. 8 (多线程性能设计)**：
  - 数据竞争因子：Cache Line Bouncing（缓存行颠簸）、False Sharing（伪共享）、数据临近性与局部性。

---

## 2. 《Systems Performance (2nd Ed)》- Brendan Gregg
* **Ch. 6 (CPUs)**：
  - 硬件缓存层级：L1 Data/Instruction (32KB, ~1ns)、L2 (256KB~1MB, ~4ns)、L3 LLC (Shared, ~10-20ns)、RAM (~60-100ns)。
  - 硬件预取器（Stream/Stride Prefetchers）：连续顺序内存访问可触发线性硬件预取，吞吐量比随机跳跃高 5~10 倍。
* **Ch. 7 (Memory)**：
  - 虚拟内存映射、Page Cache 脏页生命周期、`dirty_writeback_centisecs`。
  - Slab 与伙伴分配器（Buddy Allocator）。

---

## 3. 《Operating Systems: Three Easy Pieces (OSTEP)》- Remzi & Andrea Arpaci-Dusseau
* **Part II Concurrency**：
  - Ch. 28 锁的硬件支撑：`test-and-set`, `compare-and-swap`, `fetch-and-add`。
  - Ch. 32 并发缺陷分类：死锁的四个必要条件（互斥、持有并等待、非抢占、循环等待）；非死锁 Bug（原子性破坏、时序错误）。
  - Ch. 33 事件驱动并发：单线程 Event Loop，无锁无竞争，状态机模型。
* **Part III Persistence**：
  - Ch. 39 文件接口：`fsync()` 将脏页与元数据刷盘；`rename()` 的 POSIX 原子性保证。
  - Ch. 42 崩溃一致性：FSCK 离线扫描的局限性；Journaling（日志化 / WAL）的 Checkpoint 与崩溃重放机制。
  - Ch. 44 数据完整性：扇区损坏与 Checksum 校验。

---

## 4. OSDI 2014 顶会论文《All File Systems Are Not Created Equal...》
* **核心贡献**：首次系统性实测了现代 Linux 文件系统（ext2, ext3, ext4, btrfs, xfs）在应用崩溃时的原子性与保序性行为。
* **关键结论**：
  - 文件系统在断电时并不会保证多个系统调用的时序一致性（即使是连续两次追加写）；
  - `rename()` 本身不足以守护数据一致性，必须配合 `fdatasync()` 与父目录的 `fsync()`，否则在延迟分配（Delayed Allocation）下会导致零填充（Zeroed File）；
  - 详细披露了 Git、LevelDB、SQLite、ZooKeeper 在异常断电时的 60 个崩溃漏洞与修复方案。
