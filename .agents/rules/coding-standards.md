# 编码规范与设计底线准则 (Clean Coding Standards)

> 常驻高信噪比底线准则：凡可由工具链强检的格式、复杂度、红线 API 与依赖边界，一律交由确定性门禁拦截；本文件仅保留静态工具无法替代的架构哲学与工程铁律。

---

## 1. 架构与设计原则

- **单一职责 (SRP) 与 KISS**：业务调度、领域模型与底层 I/O / 协议解析严格分离。
- **机制与策略分离 & 数据驱动**：核心逻辑保持纯粹无状态；多分支条件优先用分派表或策略接口取代多层 `if-else / switch`。
- **演进式设计与 YAGNI**：拒绝“夸夸其谈未来性”（事不过三，三则重构）；**组合优于继承**（继承深度 $\le 3$ 层）；遵守**迪米特法则**（避免 `a.getB().getC().action()` 长链）。
- **契约式设计 (DbC) & 依赖无环 (ADP)**：入口前置校验、出口状态承诺；模块依赖必须为 DAG（物理边界由 CMake `PUBLIC`/`PRIVATE` 与 `check_include_dag.py` 强制校验）。

---

## 2. 函数、语义与表达规范（事前生成心智 + 事后 Lint 兜底）

1. **函数形状（一次写对）**：单函数控制在 **20~30 行**（`.clang-tidy` 硬阻断上限 60 行）、入参 **$\le 3$ 个**（超限封装为参数对象/结构体）、卫语句优先平铺（嵌套 **$\le 3$ 层**、认知复杂度 $\le 15$），消除裸魔法数，单参构造强制 `explicit`。
2. **反序列化、算术与系统调用边界**：
   - 二进制/持久化校验和必须同时覆盖 **Header 元数据（版本/`count`/flags）与 Payload**；
   - 不可信 `count` 调用 `reserve(count)` 前必校验物理上界 `count <= remaining_bytes / min_record_bytes`（防 OOM 放大）；
   - 区间越界校验统一用减法 **`offset > size || len > size - offset`**（严禁 `offset + len > size` 加法回绕）；
   - 调用 `std::filesystem::exists(p, ec)` 后必须立即检查 `ec`（区分文件不存在与 `EACCES`/`ENOTDIR`）。
3. **查询与修改分离 (CQS) & 移除控制标记**：Query 不改状态，Modifier 不返回复杂状态；禁用布尔控制标记在循环/分支跳转。
4. **注释作为除臭剂 & 数据封装**：需要注释解释“做什么”时先提炼自解释函数，注释仅记录**“为什么（业务约束/底层陷阱）”**；禁止暴露类裸字段，关联业务量封装为不可变值对象。
5. **I/O 流分离与缄默原则**：`stdout` 仅承载机器消费数据，诊断日志一律写 `stderr`；成功且无输出数据时保持缄默。

---

## 3. 系统级数据布局与存储底线

- **内存与缓存**：结构体成员按 **8 $\to$ 4 $\to$ 2 $\to$ 1 字节** 降序排布消除 Padding；跨核高频写变量使用 `alignas(64)` 隔离伪共享。
- **低阻力正确路径（Paved Road）**：新 C++17 模块优先复用 `StrongType`、`Result<T, E>`、`ScopeGuard`、`ChannelHandle`（Typestate）与 `CrashSafeWriter`（`Write-Temp -> fdatasync -> Rename -> Dir fsync`）。
- **SQL 存储底线**：金额强用 `DECIMAL(18, 4)` 禁 `FLOAT/DOUBLE`；多值建关联表、深层树用闭包表；复合索引遵循“等值在前、范围在后”；大表分页采用 Keyset 寻址。

---

## 4. 机器硬门禁与实证交付底线

- **C++17 交付前必过门禁**（默认严格阻断；老项目摸底须由用户明确指示后才追加 `--warn-only`）：
  1. `.agents/skills/cpp-code-quality/scripts/run_check.sh --mode=diff`（覆盖 `clang-format` + `clang-tidy` + `cppcheck` + `lint_redlines.py`）；
  2. 多模块工程执行 `.agents/skills/code-design/scripts/check_include_dag.py` 校验无环与边界；
  3. 测试套件开启 `ASan + UBSan`（并发模块额外过 `TSan`）。
- **快速失败与实证验证**：严禁静默吞异常；遇歧义或并发/底层机制不确定时，编写最小 Demo 实际运行验证。
- **小步微调与提交确认**：坚持最小可编译变更集；严禁私自 `git commit`，待用户明确确认后提交。
