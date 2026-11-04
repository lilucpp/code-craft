# B-Tree 物理索引原理与组合索引列序法则

> **理论源泉**：Markus Winand《SQL Performance Explained》Ch. 1-2 & Ch. 4-5

---

## 1. B-Tree 索引的物理结构与检索开销

### 1.1 物理结构剖析
B-Tree（在现代数据库中普遍为 B+Tree）由两部分构成：
1. **上层分支节点（Search Tree）**：由根节点与分支节点构成多层有序目录，树高通常只有 3~4 层。每次查找只需 3~4 次 I/O 即可精确定位到目标叶子块；
2. **底层叶子节点（Leaf Nodes）**：存放有序的索引列值和对应数据行指针（ROWID 或主键 ID），**所有叶子节点之间通过双向链表互相串联**。

```
                     [ 根节点 Root ]
                     /             \
            [ 分支节点 A ]       [ 分支节点 B ]
              /        \            /        \
      [ 叶子 1 ] <---> [ 叶子 2 ] <---> [ 叶子 3 ] <---> [ 叶子 4 ]
         (双向链表：范围扫描只需在叶子层水平滑动，无需重新走上层树查找)
```

### 1.2 索引扫描的两大步
1. **INDEX RANGE SCAN（索引范围扫描）**：走 B-Tree 找到起始叶子节点，顺着双向链表遍历匹配项（极快）；
2. **TABLE ACCESS BY INDEX ROWID（回表随机读）**：拿叶子节点的指针去读取全量行数据（这是绝大多数慢查询的瓶颈所在！）。

---

## 2. 组合索引列顺序黄金法则 (Equality First, Range Second)

设计复合索引 `(Col_A, Col_B)` 时，**列的先后顺序决定了索引是“精准切片”还是“大范围扫描后过滤”**。

### 2.1 铁律：精确等值条件在前，范围条件在后
假设高频查询如下：
```sql
SELECT * FROM sales 
WHERE branch_id = 42 
  AND sale_date BETWEEN '2026-01-01' AND '2026-01-31';
```

#### ❌ 错误索引顺序：`INDEX (sale_date, branch_id)`
- B-Tree 首先按 `sale_date` 排序。
- 数据库找到了满足 `2026-01-01` 的起始点，但在整整 31 天的所有海量销售记录中，`branch_id` 是乱序分布的！
- 数据库必须扫描整整一个月内的**所有门店记录**，逐条检查 `branch_id == 42`，索引过滤效率极差。

####  正确索引顺序：`INDEX (branch_id, sale_date)`
- B-Tree 首先二分查找到 `branch_id = 42` 的独立子树区域；
- 在这个区域内部，数据严格按照 `sale_date` 物理连续存放；
- 数据库直接跳到 1 月 1 日，扫描到 1 月 31 日立刻停机，**只扫描该门店的数据，扫描行数减少 99%**！

---

## 3. 覆盖索引（Covering Index / Index-Only Scan）

### 3.1 消除回表（Zero-Random-I/O）
当一个查询所需要的所有列（`SELECT` 列、`WHERE` 过滤列、`ORDER BY` 排序列）**全部包含在同一个索引中**时，数据库引擎直接从 B-Tree 叶子节点提取结果，**完全不需要回表读取数据行**！

```sql
-- 查询高频排行榜：
SELECT user_id, score FROM leaderboards WHERE season_id = 2026 ORDER BY score DESC LIMIT 10;

-- 覆盖索引设计：
CREATE INDEX idx_season_score_user ON leaderboards (season_id, score DESC, user_id);
```
* **效果**：在 EXPLAIN 执行计划中显示 `Using index`。不仅免除了随机磁盘 I/O，而且叶子节点体积致密，能被 Buffer Pool 完全缓存。
