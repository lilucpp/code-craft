# 规格说明书编写规范与歧义消除防御 (Specification & Ambiguity Defense)

本指南系统提炼自 Karl Wiegers 经典著作《Software Requirements》(第3版) 第 10、11 章与 ISO/IEC/IEEE 29148 国际标准，提供工业级功能需求编写句式、质量特征判据，以及 25+ 种“歧义坏味道”的消除重构方案。

---

## 1. 软件需求规格说明 (SRS) 标准结构

遵循 ISO/IEC/IEEE 29148 体系，SRS 必须保持模块化与原子化组织：

```markdown
1. 引言 (Introduction)
   1.1 目的 (Purpose)
   1.2 文档约定与优先级标记规则 (Document Conventions)
   1.3 预定读者与阅读建议 (Intended Audience)
   1.4 项目范围与产品愿景索引 (Product Scope)
   1.5 参考文献与术语表索引 (References & Glossary)
2. 总体描述 (Overall Description)
   2.1 产品视角与拓扑架构 (Product Perspective)
   2.2 用户类别与特征画像 (User Classes and Characteristics)
   2.3 运行环境与基础设施要求 (Operating Environment)
   2.4 设计与实现约束 (Design and Implementation Constraints)
   2.5 假设与依赖关系 (Assumptions and Dependencies)
3. 系统功能需求 (System Features / Functional Requirements)
   3.x 特性名称 (Feature X)
       3.x.1 描述与商业优先级 (Description and Priority)
       3.x.2 激励/响应序列 (Stimulus/Response Sequences)
       3.x.3 功能需求明细 (Functional Requirements: FR-X.1, FR-X.2...)
4. 数据需求 (Data Requirements)
   4.1 逻辑数据模型与实体关系 (Logical Data Model)
   4.2 数据字典与验证约束 (Data Dictionary)
5. 外部接口需求 (External Interface Requirements)
   5.1 用户界面契约 (User Interfaces)
   5.2 硬件接口规范 (Hardware Interfaces)
   5.3 软件与系统间接口 (Software Interfaces: API/Protocols)
   5.4 通信接口协议 (Communications Interfaces)
6. 非功能需求与质量属性 (Nonfunctional Requirements)
   6.1 性能需求 (Performance)
   6.2 安全性需求 (Security)
   6.3 软件质量属性 (Availability, Robustness, Usability...)
7. 附录：分析模型与业务规则索引 (Appendixes)
```

---

## 2. 卓越需求质量特征判据

### 单条需求陈述的 7 大特征 (Individual Statement Criteria)
1. **完整性 (Complete)**：包含开发者理解和实现所需的所有信息，无未解的“待定 (TBD)”。
2. **正确性 (Correct)**：真实反映客户或系统的实际需要，经 Product Champion 验证签署。
3. **可行性 (Feasible)**：在既定技术栈、预算、合规约束与时间限制内切实可实现。
4. **必要性 (Necessary)**：直接支撑业务需求或特定用户任务，杜绝无关的“镀金 (Gold Plating)”。
5. **已区分优先级 (Prioritized)**：标注有客观、可度量的相对重要程度或交付批次。
6. **无歧义 (Unambiguous)**：所有读者（开发、测试、业务）只能推导出一种唯一的解读。
7. **可验证性 (Verifiable)**：测试人员能够编写出客观、可重现的测试用例来断言其是否达成。

### 需求集合的 4 大特征 (Requirements Set Criteria)
1. **完整性 (Complete)**：无遗漏的关键行为、异常分支与系统状态。
2. **一致性 (Consistent)**：需求条目之间不存在逻辑矛盾、接口协议冲突或数值重叠。
3. **可修改性 (Modifiable)**：每条需求具备全局唯一编号，目录组织结构化，变更易于维护。
4. **可跟踪性 (Traceable)**：能够向前追溯到业务需求/用例，向后追溯到架构设计、代码与测试用例。

---

## 3. 标准需求编写语法规范

### 3.1 主谓宾统一句式
功能需求必须使用主动语态，明确执行主体与触发场景：

$$[\text{可选前置条件}] \quad [\text{可选触发事件}] \quad \textbf{系统应} \quad [\text{具体系统动作}] \quad [\text{作用对象}] \quad [\text{限定参数}]$$

- **标准示例**：“当买家点击‘确认支付’且账户余额充足时，**系统应**向第三方支付网关发送扣款请求，并在 2 秒内展示支付凭据。”

### 3.2 严禁混用情态动词 (Shall vs. Should vs. May)
- **严格统一使用“应 (Shall)”**：表示系统**必须实现的强制功能**；
- **杜绝使用“应该 (Should)”**：“Should”在口语中带有“最好有但可妥协”的歧义，会导致开发人员与业务对交付范围产生纠纷；
- **慎用“可 (May)”**：仅用于描述用户拥有的可选合法权利（如“买家可选择保存支付凭据”），不可用于修饰系统行为。

---

## 4. 25+ 歧义坏味道词汇与重构词典 (Ambiguity Smell Dictionary)

提炼自 Wiegers 经典 Table 11-2，以下词汇严禁直接出现在需求正文中：

| 歧义词 (Smell Word) | 为什么是坏味道 (Smell Rationale) | 精准重构方案与示例 (Pattern & Refactoring) |
| :--- | :--- | :--- |
| **用户友好 (User-friendly / Simple / Easy)** | 主观性极强，无法编写自动化测试断言 | 替换为可量化的可用性指标：“新操作员在完成 30 分钟基础培训后，应能在 2 分钟内独立完成单笔化学品建档。” |
| **快速 / 高效 (Fast / Quick / Rapid / Efficient)** | “快”对每个人标准不同（毫秒 vs 秒 vs 分钟） | 给出具体的响应时长与测试基准：“在网络吞吐 $\ge 10\text{MB/s}$ 条件下，系统检索 10 万条历史记录的首屏渲染耗时应 $\le 800\text{ms}$。” |
| **支持 / 允许 (Support / Enable / Handle)** | 语义过于宽泛（是仅仅存储？还是解析？编辑？导出？） | 拆解为具体功能动词：“系统应提供解析、校验、存储并导出符合 HL7 标准医疗数据包的功能。” |
| **鲁棒 / 健壮 (Robust / Resilient)** | 未说明容忍哪些故障，以及故障时如何降级 | 明确具体异常与恢复行为：“当网络连接意外断开时，系统应将未提交表单暂存本地 SQLite，并在网络重连后自动重试同步。” |
| **无缝 / 透明 (Seamless / Transparent / Graceful)** | 纯修辞营销词汇，无法指导开发实现 | 明确数据与操作过渡标准：“系统完成底层数据库切主期间，前端 HTTP 请求重试队列应保证用户无需重新登录且零数据丢失。” |
| **等等 / 以及其他 (Etc. / And so on / And the like)** | 遗漏了具体边界，读者必须自行猜测完整列表 | 穷举全部枚举值，或引用外部受控列表：“系统支持导入的文件格式为 `.csv`, `.xlsx`, `.json` 三种（完整枚举，不接受其他扩展名）。” |
| **包括但不限于 (Including but not limited to)** | 留下了无法封闭验证的无底洞 | 明确受控闭环范围：“系统在当前版本仅处理表 4-1 中明确列出的 8 种支付通道事件。” |
| **通常 / 一般而言 (Normally / Ideally / In most cases)** | 暗示存在“异常情况”，但没有定义异常场景下的行为 | 补全对立分支：“在营业日内系统实时结算；在非营业日（周末与法定假日），系统应将结算任务顺延至下一个营业日 09:00 执行。” |
| **合理 / 适时 (Reasonable / Appropriate / If possible)** | 判定权力真空，由谁判定？依据什么判定？ | 给出判定依据与阈值：“当上传文件尺寸超过 50MB 时，系统应拦截上传并向用户弹出压缩建议。” |
| **和/或 (and/or) 与斜杠 (A/B)** | 允许 3 种不同组合，读者解读极易出现偏差 | 明确拆分为独立语句或枚举组合：“用户可单独输入订单号检索，或单独输入发票号检索，或同时输入两者进行联合交集检索。” |
| **若干 / 多个 (Several / Multiple / Many / Few)** | 数量边界模糊，无法进行容量规划与压力测试 | 明确最小与最大数值：“系统应允许管理员一次性批量勾选最多 500 个待审核用户。” |
| **最新 / 先进技术 (State-of-the-art / Modern)** | 随时间推移迅速失效，毫无工程约束力 | 锁定明确的技术版本：“系统后端必须兼容 OpenJDK 17 LTS 运行时规范。” |

---

## 5. 常见结构性缺陷防御机制

### 5.1 数值边界防御 (Boundary Value Precision)
- **反模式**：“5 到 10 天的请假需要主管审批；10 天以上的需要总监审批。”（5 天和 10 天到底谁审批？）
- **防御重构**：严格采用包含/排除性语言：
  - “请假天数在 **5 天及以内（$\le 5$）** 的，由直属主管审批；”
  - “请假天数在 **超过 5 天至 10 天以内（$> 5$ 且 $\le 10$）** 的，由部门经理审批；”
  - “请假天数在 **超过 10 天（$> 10$）** 的，由总监审批。”

### 5.2 否定句与双重否定防御 (Negative Requirements Defense)
- **反模式**：“防止未通过身份验证的用户无法访问公共帮助页面。”（双重否定逻辑极易绕晕）
- **防御重构**：一律转换为积极肯定陈述：“系统应允许所有访问者（包括未登录用户）浏览公共帮助页面。”
- **反模式**：“系统不应在没有选择协议时允许注册。”
- **防御重构**：“系统仅在用户勾选‘同意服务协议’后，方可激活‘立即注册’按钮。”

### 5.3 对称性缺失排查 (Symmetry Audit)
- 任何“保存 (Save)”必须配对检查“恢复/继续编辑 (Resume/Retrieve)”；
- 任何“创建/新增 (Create)”必须配对检查“修改 (Update)”与“作废/删除 (Delete)”；
- 任何“冻结/加锁 (Lock)”必须配对检查“解冻/解锁 (Unlock)”与超时自动释放；
- 任何“导入 (Import)”必须配对检查“导出 (Export)”与格式校验失败处理。

### 5.4 组合逻辑决策表 (Decision Table Defense)
当需求涉及两个及以上条件组合时，拒绝纯自然语言陈述，强制绘制真值决策表覆盖全部 $2^n$ 种情况：

| 规则条件 (Conditions) | 组合 1 | 组合 2 | 组合 3 | 组合 4 |
| :--- | :---: | :---: | :---: | :---: |
| 条件 1: 用户为会员身份 (VIP) | Y | Y | N | N |
| 条件 2: 订单金额 $\ge ¥200$ | Y | N | Y | N |
| **系统动作 (Actions)** | | | | |
| 享受免运费特权 | X | X | X | — |
| 加送专属礼品 | X | — | — | — |
| 收取标准运费 (¥15) | — | — | — | X |
