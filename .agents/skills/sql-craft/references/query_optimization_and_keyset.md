# 查询优化、索引失效真凶与 Keyset 寻址分页

> **理论源泉**：Markus Winand《SQL Performance Explained》Ch. 2-3 & Ch. 6-8

---

## 1. Keyset 寻址分页法（Seek Method 彻底替代 OFFSET）

### 1.1 `LIMIT OFFSET` 的深分页性能雪崩
```sql
-- ❌ 翻到第 50,000 页时：
SELECT * FROM orders ORDER BY id ASC LIMIT 1000000, 20;
```
* **底层原理**：数据库并不能直接跳到第 1,000,000 条记录！
* **致命开销**：数据库必须顺着 B-Tree 叶子节点读取 1,000,020 条数据，回表 1,000,020 次，把前 100 万条全部计算排序后**统统扔掉**，只返回最后 20 条。随着页码增加，耗时呈线性断崖式恶化。

### 1.2 Keyset Pagination（Seek Method）
在客户端保留上一页最后一条记录的键值（如主键 `id`），下一页直接基于该键值做 B-Tree 二分寻址：

```sql
--  优雅高效的 Keyset 分页：
SELECT * FROM orders 
WHERE id > :last_seen_id 
ORDER BY id ASC 
LIMIT 20;
```
* **性能对比**：利用主键索引直接进行点查定位（耗时 $\approx 1$ 毫秒），无论翻到第 1 页还是第 1000 万页，**耗时恒定为 1~2 毫秒**！
* **复合排序列 Keyset 模板**：
  若按 `(created_at, id)` 排序分页：
  ```sql
  SELECT * FROM orders 
  WHERE (created_at = :last_created_at AND id > :last_id)
     OR (created_at > :last_created_at)
  ORDER BY created_at ASC, id ASC 
  LIMIT 20;
  ```

---

## 2. 索引失效的六大罪魁祸首与规避

在 `WHERE` 条件中，B-Tree 依赖的是**索引列的原始有序值**。一旦对列施加变换，有序性立刻被打破，索引失效转为全表扫描。

### 2.1 罪魁祸首 1：对索引列使用函数或表达式
```sql
-- ❌ 导致索引失效（全表扫描）：
SELECT * FROM users WHERE YEAR(created_at) = 2026;
SELECT * FROM accounts WHERE balance - 100 > 5000;

--  保持列的原始性，将计算移至常量端：
SELECT * FROM users WHERE created_at >= '2026-01-01' AND created_at < '2027-01-01';
SELECT * FROM accounts WHERE balance > 5100;
```

### 2.2 罪魁祸首 2：隐式类型转换（Implicit Type Casting）
当列类型与查询参数类型不一致时，数据库会隐式在列上包裹转换函数：
```sql
-- 假设 phone 字段为 VARCHAR(32)
-- ❌ 传入数字触发隐式 CAST(phone AS UNSIGNED) = 13800138000，全表扫描！
SELECT * FROM users WHERE phone = 13800138000;

--  确保参数类型严格匹配字符串：
SELECT * FROM users WHERE phone = '13800138000';
```

### 2.3 罪魁祸首 3：左模糊匹配 `LIKE '%abc'`
* B-Tree 只能按照前缀字符进行首字母排序。
* `LIKE 'abc%'` 可以走索引范围扫描（Index Range Scan）；
* `LIKE '%abc'` 无法确定起始字符，只能全表扫描。
* **解法**：对高频模糊搜索，引入全文检索（Full-Text Search / ES）或倒排索引。

### 2.4 罪魁祸首 4：否定操作符（`!=`、`NOT IN`）
否定操作通常需要遍历绝大多数数据行，优化器通常认为全表扫描代价低于大量索引回表。
