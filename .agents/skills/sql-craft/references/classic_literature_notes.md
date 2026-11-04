# Markus Winand《SQL Performance Explained》全景核心观点精要

---

## 一、 索引解剖学 (Anatomy of an Index)

### 1. B-Tree 的双重力量 (The Two Powers of Indexing)
- **第一重力量：对数级树高扩展性（Logarithmic Scalability）**
  - B-Tree（实际为 B+Tree）高度极其矮胖，通常只有 3~4 层。
  - 1000 万条记录与 1 亿条记录，树高可能同样只有 4 层。二分查找到达叶子节点仅需 3~4 次 I/O，这是主键点查极速的物理根基。
- **第二重力量：叶子节点双向链表（Doubly Linked Leaf Nodes）**
  - 所有叶子节点按照键值顺序严格物理排列，且相邻节点间维护着双向指针。
  - **范围查询（Range Scan）只需在叶子层沿链表水平滑动，绝不需要重复从根节点遍历**。

### 2. 回表代价（The Table Access Overhead）
- 绝大多数慢查询的根源不是“索引查找慢”，而是**“索引命中后拿着 ROWID 回表读取数据行的随机 I/O 代价过高”**。
- **覆盖索引（Covering Index / Index-Only Scan）**：
  - 当查询所请求的全部列（`SELECT` 列、`WHERE` 列、`ORDER BY` 列）均在索引树中时，数据库直接从叶子节点返回数据，**彻底消除回表随机 I/O，性能可提升数十倍**。

---

## 二、 WHERE 子句与复合索引列顺序 (The WHERE Clause)

### 1. 击碎流传数十年的“区分度优先”谣言 (Selectivity Myth Debunked)
- ❌ **流传甚广的误区**：“在复合索引中，唯一值最多、区分度最高（Selectivity）的列必须排在最前面”。
-  **Markus 黄金法则**：
  **决定复合索引列顺序的绝不是区分度，而是查询的操作符类型！**
  **等值查找列（`=`）必须排在最前面，范围查找列（`BETWEEN, >, <`）必须排在最后面！**
- **物理底层原理**：
  B-Tree 一旦在某一列遭遇了范围扫描，后续所有列在物理叶子节点上的连续性就被彻底破坏，后续列无法再作为二分检索切片，只能退化为低效的逐行比对过滤。

### 2. 索引列纯洁性法则 (Purity of Indexed Columns)
- 严禁在 `WHERE` 条件的索引列上包裹任何函数、数学运算或隐式类型转换：
  - ❌ `WHERE YEAR(created_at) = 2026`（全表扫描）
  -  `WHERE created_at >= '2026-01-01' AND created_at < '2027-01-01'`（索引范围切片）
- **字符串与数字的隐式转换灾难**：
  若列为 `VARCHAR`，传入数字参数将隐式触发 `CAST(col AS UNSIGNED)`，强制全表扫描。

### 3. 模糊匹配法则 (Wildcard Search)
- `LIKE 'abc%'`：前缀固定，可利用首字母物理排序进行高效的 **Index Range Scan**；
- `LIKE '%abc'`：首字母未知，B-Tree 物理排序完全失效，必然触发全表扫描。

---

## 三、 连接操作物理机制 (The Join Operation)

| 连接算法 | 物理特征与内部机制 | 适用场景与索引配套 |
| :--- | :--- | :--- |
| **Nested Loops**<br/>(嵌套循环) | 外层小表驱动，内层大表逐行利用索引点查。响应速度极快。 | **OLTP 核心业务首选**。<br/>**外键列必须建索引**，否则触发内层全表扫描灾难。 |
| **Hash Join**<br/>(哈希连接) | 扫描小表在内存中建立 Hash 表，全表扫描大表探测。无需被驱动表有索引。 | **大表全量无序关联、数据仓库分析**。<br/>消耗大量 `work_mem`。 |
| **Sort Merge Join**<br/>(排序合并) | 双表均已在连接列上有序时，顺序双指针归并。 | 两表数据量巨大且已具备预排序索引时。 |

---

## 四、 排序与分组 (Sorting & Grouping)

### 1. 流水线排序（Pipelined Sort） vs 显式临时文件排序（Using filesort）
- 当 `ORDER BY` 的列顺序与 B-Tree 复合索引完全契合时，数据库无需对数据行做任何额外排序计算，直接顺着叶子节点链表流式输出，**首行响应时间（Time to First Row）几乎为 0**。
- 一旦列顺序不匹配，必须在内存的 Sort Buffer 中做快排；数据量超标时甚至分块刷入磁盘临时表，性能断崖暴跌。

### 2. 复合索引排序方向陷阱
- `ORDER BY a ASC, b DESC` 无法复用默认双向相同的 `INDEX (a, b)`。
- 若必须做混合排序，在 DDL 中显式指定物理存储方向：`CREATE INDEX idx_a_b ON t (a ASC, b DESC)`。

---

## 五、 分页的终极解决方案：Keyset Pagination (Seek Method)

### 1. `LIMIT OFFSET` 的致命缺陷
```sql
SELECT * FROM orders ORDER BY id ASC LIMIT 1000000, 20;
```
- 数据库必须沿着叶子链表读取并排序 1,000,020 条数据，回表 1,000,020 次，然后**把前 100 万条全部抛弃**！
- 内存、CPU 与磁盘 I/O 随页码深度呈完全线性消耗恶化。

### 2. Keyset Pagination 寻址分页方案
利用主键或有序游标直接定位 B-Tree 切片：
```sql
SELECT * FROM orders 
WHERE id > :last_seen_id 
ORDER BY id ASC 
LIMIT 20;
```
- **核心收益**：直接从根节点做 3 次 I/O 二分寻址，**翻到第 1 页与翻到第 1000 万页耗时完全恒定在 1~2 毫秒**。

---

## 六、 数据修改写放大 (Modifying Data)

1. **写放大（Write Amplification）**：
   - 表上每多一个二级索引，每次 `INSERT` / `DELETE` 就要额外增加一次独立的 B-Tree 物理写入；
   - 插入随机键值会导致叶子节点频繁 **页分裂（Page Split）**，产生大量磁盘碎片与未充分利用的半空页面。
2. **更新索引列的重代价**：
   - 更新非索引列只需原地覆盖；更新索引列等价于在 B-Tree 中“删除旧键 + 插入新键”，加剧页面争用。
