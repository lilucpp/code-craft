# 物理数据库设计反模式、数据完整性与索引霰弹枪防御

> **理论源泉**：Bill Karwin《SQL 反模式》第二部分（物理设计反模式）与第四部分（应用开发反模式）

---

## 1. 取整错误反模式（Rounding Errors）：金额严禁浮点数

### 1.1 灾难根因：IEEE 754 浮点二进制精度丢失
`FLOAT` 和 `DOUBLE` 是基于二进制科学计数法的近似值表示。在二进制中，绝大多数十进制小数（如 `0.1`、`0.01`）是无限循环小数，无法精确表示：
```sql
-- ❌ 危险操作：
CREATE TABLE accounts (
    id INT PRIMARY KEY,
    balance FLOAT -- 累加多次后出现 100.00000214 甚至丢钱！
);
```

### 1.2 强制规范：定点数 `DECIMAL / NUMERIC`
金融、支付、利息计算必须使用定点数类型：
```sql
--  精确定点数，18 位整数有效位，4 位小数位：
CREATE TABLE accounts (
    id INT PRIMARY KEY,
    balance DECIMAL(18, 4) NOT NULL DEFAULT 0.0000
);
```

---

## 2. 索引霰弹枪反模式（Index Shotgun）：写放大治理

### 2.1 坏味道：无脑给所有列分别建单列索引
开发人员为了让所有查询都有可能走索引，给每个字段都建一个单独的索引：
`CREATE INDEX idx_a ON t(a);` `CREATE INDEX idx_b ON t(b);` `CREATE INDEX idx_c ON t(c);`

* **致命代价（写放大 Write Amplification）**：
  1. 插入一行数据不仅要写主表（聚簇索引），还要同时写 5~10 个辅助 B-Tree；
  2. B-Tree 频繁发生页分裂（Page Split）和随机 I/O，写入吞吐暴跌 80% 以上；
  3. 复合条件查询（如 `WHERE a = ? AND b = ?`）无法利用单列索引的联合优势，往往只命中一个索引甚至退化。

### 2.2 规范：高信噪比复合索引优先
- 分析系统中前 20% 的高频 SQL 查询，针对性设计最少、覆盖度最高的复合索引；
- 避免冗余前缀索引：如果已存在 `INDEX (a, b)`，则不需要再单独建 `INDEX (a)`。

---

## 3. 外键约束与无钥匙入口反模式（Keyless Entry）

### 3.1 辩证看待外键约束
* **常见误区**：“互联网大厂都禁止加外键，所以任何系统都不应该加外键”。
* **现实权衡**：
  - 如果应用层没有极其严密的分布式 Saga 事务框架，弃用外键约束会导致数据库中充斥着大量的**孤儿记录（Orphan Rows）**与不一致数据；
  - **规范**：在单机关系型数据库或中核心业务系统（订单-明细、用户-主档）中，必须保留外键完整性约束；在高并发分库分表分布式场景下，方可将完整性校验上移至领域服务层。

---

## 4. 对未知的恐惧（Fear of the Unknown）：NULL 的三值逻辑陷阱

### 4.1 SQL 的三值逻辑（Three-Valued Logic）
在 SQL 中，任何与 `NULL` 的比较（`NULL = 1`、`NULL != 1` 甚至是 `NULL = NULL`）的结果都不是 `TRUE` 也不是 `FALSE`，而是 **`UNKNOWN`**。

### 4.2 经典致命踩坑：`NOT IN` 遇到 `NULL`
```sql
-- 假设子查询返回 (1, 2, NULL)
SELECT * FROM users WHERE user_id NOT IN (SELECT blocked_id FROM blacklist);
```
* **灾难结果**：由于 `x != NULL` 永远求值为 `UNKNOWN`，整个表达式恒为 `UNKNOWN`，**该查询将返回 0 条记录（整表消失）**！
* **防御规范**：
  - 外键与关键枚举列强制显式声明 `NOT NULL DEFAULT ...`；
  - 检查子查询存在性时，优先使用 `NOT EXISTS` 代替 `NOT IN`。
