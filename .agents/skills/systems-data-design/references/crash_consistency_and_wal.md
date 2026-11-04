# 进程内持久化、崩溃一致性与预写日志 (WAL)

> **理论源泉**：OSDI 2014 顶会论文《All File Systems Are Not Created Equal: On the Complexity of Crafting Crash-Consistent Applications》- Pillai 等人；《OSTEP》Ch. 39 & 42-44

---

## 1. 真实世界的崩溃陷阱 (Crash-Consistency Vulnerabilities)

在 POSIX 文件系统上，绝大多数初学者对持久化的假设是完全错误的：
1. **`write()` 并不保证落盘**：`write()` 只是将数据拷贝进操作系统的 Page Cache（脏页），系统掉电后数据立刻丢失；
2. **原地覆写（In-Place Overwrite）是致命的**：如果覆写已有文件时机器断电，文件内容会处于“部分新数据 + 部分旧数据”的破损状态；
3. **只调 `rename()` 是不够的**：OSDI 14 论文实证，如果重命名前没有对新文件内容执行 `fdatasync()`，或者重命名后**没有对父目录执行 `fsync()`**，在 ext4（延迟分配）、btrfs 崩溃重启后，新文件可能被截断为 0 字节，旧文件也被彻底删除，造成永久数据丢失！

---

## 2. 崩溃安全原子替换标准（Atomic File Replacement）

当需要更新整个配置文件、状态快照或数据文件时，必须严格执行 **Crash-Safe 四步法**：

```
[步骤 1: 写入临时文件]
open("target.tmp", O_WRONLY | O_CREAT | O_TRUNC)
write(fd, data, size)

[步骤 2: 强制数据与必要元数据落盘]
fdatasync(fd)
close(fd)

[步骤 3: 利用 POSIX 目录项原子性更名]
rename("target.tmp", "target")

[步骤 4: 关键！强制父目录项元数据刷盘！]
dir_fd = open(parent_dir_path, O_RDONLY | O_DIRECTORY)
fsync(dir_fd)
close(dir_fd)
```

### 为什么必须第 4 步（父目录 fsync）？
`rename` 虽然在操作系统内存中是原子的，但如果父目录的目录项修改还停留在 Page Cache 中，断电重启后文件系统可能回滚目录项，导致目标文件在目录树中“消失”。

---

## 3. 预写式日志（WAL - Write-Ahead Logging）协议规范

对于高频事务性增量修改（如嵌入式 KV 引擎、本地订单暂存），全量重写代价太高，必须采用 WAL：

```
                        ┌───────────────────────────────┐
                        │      追加写入 WAL 日志流      │
                        │ (顺序 I/O + Checksum + LSN)   │
                        └───────────────┬───────────────┘
                                        │
                                        ▼
                         应用到内存索引 (MemTable/B-Tree)
                                        │
                                        ▼
                                向业务返回成功
                                        │ (后台周期性 / 阈值触发)
                                        ▼
                        生成全量快照 (Checkpoint) 并截断旧 WAL
```

### 3.1 防断裂页与半写入校验（Torn Write Defense）
磁盘的物理原子写入扇区通常为 512B 或 4KB。如果写入 8KB 的日志时系统断电，可能会写入前半截、后半截丢失（Torn Write）。
* **规范**：
  - 每个 WAL 记录必须包含头部 Magic Number、单调递增的序列号（LSN）、有效载荷长度与 **CRC32 Checksum**；
  - 重启重放时，一旦计算 Checksum 失败，立即停止重放并自动截断未完成的尾部脏数据，确保已提交事务绝对安全。
