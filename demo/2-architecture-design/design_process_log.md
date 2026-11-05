# FileGate 系统架构与详细设计过程记录与推导演进日志 (Design Process Log)

> **记录目的**：真实记录从阶段一需求规格说明书（SRS）向阶段二系统架构与详细设计（Detailed Design）推导的完整决策链路、碰到的技术挑战、方案微调过程以及评估迭代依据。

---

## 1. 阶段一需求输入确认与职责解构

### 1.1 需求关键输入回顾
- **系统核心场景**：内部员工提交外发申请 $\to$ DLP 敏感规则自动检测推荐密级 $\to$ 部门主管在线审批 $\to$ 批准后生成受控访问 URL 与提货码 $\to$ 外部访问注入防伪盲水印 $\to$ 全量行为审计追踪；
- **核心非功能指标**：检索延迟 $\le 50\text{ms}$、掉电零损坏崩溃一致性、10 分钟内完成涉密追踪溯源。

### 1.2 意图到架构的职责拆解 (Intent Decomposition)
我们将整体复杂业务拆解为五个核心架构关注点，并映射到对应专业技能：
1. **中微观拓扑与依赖治理** $\to$ **`code-design`**
2. **底层物理内存、伪共享与掉电保护** $\to$ **`systems-data-design`**
3. **关系数据模型与 B-Tree 物理优化** $\to$ **`sql-craft`**
4. **命令行工具与管道过滤器机制** $\to$ **`unix-philosophy`**
5. **PC 桌面管理控制台美学与交互** $\to$ **`antqt`**

---

## 2. 详细设计推导与微调过程 (Step-by-Step Evolution)

### 步骤一：包拓扑划分与循环依赖解环（ADP 治理）
- **初始构想**：将所有存储与审计放在同一模块中。
- **发现问题**：审计模块依赖存储将日志落盘，而底层存储发生错误时也需要调用审计模块记录错误日志，导致 `filegate_storage <---> filegate_audit` 形成双向循环依赖（违反 ADP 原则）。
- **微调决策**：
  运用 **`code-design`** 的依赖倒置（DIP）与接口隔离（ISP）：
  1. 将基础强类型与错误码彻底抽离至最底层的叶子包 `filegate_types`；
  2. 让 `filegate_storage` 只提供通用的纯虚存储接口与物理文件原子操作，不直接感知上层业务审计；
  3. 让 `filegate_audit` 依赖 `filegate_storage`，打破双向耦合环路，使依赖拓扑收敛为单向严格的 DAG 图。

### 步骤二：接口契约化与开闭原则（DbC 与 OCP 落地）
- **设计推导**：
  文件水印嵌入支持显式文字水印、不可感知频域 DCT 盲水印、二维码身份戳等多种方式。为了未来无缝扩展其他加密或打码格式（如视频水印、Office 宏水印）：
  - 提取统一纯虚接口 `IWatermarkEngine`；
  - 严格定义契约：调用前必须担保输入文件真实存在且具备正向字节数（Precondition），执行后担保目标文件完整落地（Postcondition），全流程维持源文件只读不变式（Class Invariant）。

### 步骤三：物理内存排布优化（从 24 字节浪费到 0 浪费）
- **初始构想**：按照业务字段的直觉顺序排布 `ShareRequestMeta` 结构体：
  `security_level(1B) -> request_id(8B) -> status(1B) -> applicant_id(8B) -> dlp_score(2B)...`
- **量化发现**：
  在 64 位 x86_64 ABI 对齐规则下，由于每个 8 字节变量前都被迫插入 4~7 字节的隐式 Padding，导致结构体膨胀至 88 字节，Padding 空洞浪费高达 **24 字节**！同时跨越了 2 个 Cache Line，极大拉低 L1 缓存利用率。
- **微调优化**：
  严格套用 **`systems-data-design`** 的 **8-4-2-1 降序排列铁律**：
  - 8 字节区（48B）：`request_id`, `applicant_id`, `approver_id`, `created_at_ms`, `expire_at_ms`, `payload_size_bytes`；
  - 4 字节区（12B）：`max_download_limit`, `current_download_count`, `access_passcode_hash`；
  - 2 字节区（2B）：`dlp_violation_score`；
  - 1 字节区（2B）：`security_level`, `status`；
  - **优化成果**：总尺寸收敛为 **64 字节**，Padding 浪费 **0 字节**，完美契合 1 条 CPU Cache Line。

### 步骤四：时序漏洞消除（Typestate 编译期拦截）
- **传统做法隐患**：采用单个类并在内部维护 `status` 字段，当调用 `GenerateToken()` 时通过运行时 `if (status != Approved)` 抛出异常。这种做法极易因外部调用者遗漏校验而在运行时引发未审先发的重大安全事故。
- **微调升级**：
  引入现代 C++ Typestate 模式与幻影类型（Phantom Types）：
  将 `ShareRequest<CurrentState>` 参数化为 `Draft`、`PendingApproval`、`Approved`、`Active` 等类型。
  `Activate()` 函数仅为 `ShareRequest<StateApproved>` 暴露。处于 `Draft` 态的对象在语法上根本不存在生成外发链接的成员方法，将时序安全性固化在编译期。

### 步骤五：物理索引与分页调优（Markus Winand 铁律）
- **慢查询隐患规避**：
  针对日增数十万条日志的 `t_audit_log` 大表：
  - 若使用普通索引 `KEY (created_at, request_id)`，在执行根据外发单号检索历史时，由于等值列未居前，会导致严重的索引扫描；
  - 微调为严格遵守等值在前、排序在后的复合索引：`KEY (request_id, event_type, id DESC)`；
  - 彻底废除 `LIMIT 100000, 20` 语法，推行基于自增物理主键的 `WHERE id < :last_seen_id ORDER BY id DESC LIMIT 20` Keyset 游标分页，保证毫秒级恒定响应。

---

## 3. 自动化检验与评估闭环 (Verification & Closure)

为确保详设不仅停留在文字层面，编写了自动化检验脚本 `demos/verify_architecture_design.py`，完成 7 大专项检验：
1. **ADP DAG 拓扑无环性**：通过 Kahn 算法遍历，闭环数 0，验证通过；
2. **DbC 契约完备性**：`IWatermarkEngine` 等核心接口契约三要素完备；
3. **物理内存 Padding**：实证 0 字节内存浪费，验证通过；
4. **伪共享隔离**：`WorkerAuditMetric` 具备 `alignas(64)` 隔离，验证通过；
5. **Typestate 状态迁移闭环**：幻影类型特化完备，验证通过；
6. **B-Tree 列序与分页**：符合 Markus Winand 最左匹配铁律，验证通过；
7. **OSDI 14 掉电落盘四步法**：完整具备 4 步原子落盘语义，验证通过。

**结论**：阶段二架构与详细设计过程完整、依据充分、可验证、可追踪。
