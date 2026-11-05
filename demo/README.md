# Code-Craft 现代软件工程全生命周期 (SDLC) 实战示例库 (Demo Suites)

本目录汇总了基于 **Code-Craft 核心专业 Skills 矩阵** 演进推导的完整工业级软件工程实战范例。每个子目录均严格对应软件开发生命周期（SDLC）的标准阶段，且每一个阶段均包含：
1. **权威标准规格文档**（按照行业标准编写的正式成果物）；
2. **推导与微调过程日志**（记录从上一阶段向本阶段意图分解、技术权衡与坏味道治理过程）；
3. **自动化评估与机检工具**（可直接运行的 Python / Shell 脚本，提供 100% 实证数据）；
4. **形式化评估报告**（基于业界大师经典理论的量化验收依据）。

---

## SDLC 阶段实战目录索引

```mermaid
flowchart TD
    subgraph D1 ["demo/1-software-requirements/"]
        R1["阶段一：需求工程与业务分析<br>• Karl Wiegers《软件需求》/ ISO 29148 规格说明书<br>• SIBV 痛点挖掘 / In-Out Scope / Gherkin BDD<br>• Table 11-2 自动化坏味道机检 (0 ERROR)"]
    end

    subgraph D2 ["demo/2-architecture-design/"]
        R2["阶段二：系统架构与详细设计<br>• Uncle Bob 组件无环原则 (ADP DAG)<br>• Bertrand Meyer 契约式设计 (DbC)<br>• 8-4-2-1 降序对齐 (0 Padding) / OSDI 14 原子落盘<br>• B-Tree 列序铁律 / AntQt 桌面端 32px 工作台"]
    end

    subgraph D3 ["demo/3-high-quality-implementation/"]
        R3["阶段三：高质量规范实现<br>• Scott Meyers Effective C++ (RAII / 智能指针 / StrongType)<br>• More C++ Idioms (NVI 契约守护 / Type Erasure / ScopeGuard)<br>• Anthony Williams 并发实战 (细粒度分离锁 / alignas 64)<br>• Uncle Bob Clean Code (SLAP / 函数<=30行 / 卫语句优先)"]
    end

    subgraph D4 ["demo/4-testing-and-quality/"]
        R4["阶段四：测试工程与质量门禁<br>• GoogleTest / Catch2 风格 AAA 单元测试 (19个用例全通)<br>• 4 象限输入边界矩阵 (空/单字节/1MB极值/非法越界)<br>• GoogleMock 依赖注入与策略接口打桩隔离<br>• LLVM AddressSanitizer/UBSan 内存消毒 (0 缺陷)<br>• gcov 核心业务代码行覆盖率 88.14% (达成 >=80% 门禁)"]
    end

    subgraph D5 ["demo/5-review-and-refactoring/"]
        R5["阶段五：代码审查诊断与安全重构<br>• Martin Fowler《重构》22 种代码坏味道结构化审查<br>• 68 种经典重构手法落地 (卫语句 / 提炼类 / 强类型)<br>• 嵌套层数 5层 降至 1层 / 圈复杂度下降 67%<br>• 跨版本行为等价守护测试 (Safety-Net 6大场景100%等价)"]
    end

    D1 --> D2
    D2 --> D3
    D3 --> D4
    D4 --> D5
```

| 阶段目录 | 对应 SDLC 阶段 | 调用的核心 Skills | 核心交付成果与检验工具 |
| :--- | :--- | :--- | :--- |
| **[`1-software-requirements/`](./1-software-requirements/)** | **阶段一：需求工程与业务分析** | **`software-requirements`** | • 《FileGate 软件需求规格说明书 (SRS)》<br>• 需求推导与歧义微调复盘日志<br>• `run_lint_check.sh` 需求坏味道机检脚本 |
| **[`2-architecture-design/`](./2-architecture-design/)** | **阶段二：系统架构与详细设计** | **`code-design`**<br>**`systems-data-design`**<br>**`sql-craft`**<br>**`unix-philosophy`**<br>**`antqt`** | • 《FileGate 系统架构与详细设计说明书》<br>• 架构推导、解环与内存优化过程日志<br>• `verify_architecture_design.py` 7项自动化检验工具<br>• 形式化架构评估报告 (100分) |
| **[`3-high-quality-implementation/`](./3-high-quality-implementation/)** | **阶段三：高质量规范实现** | **`effective-cpp`**<br>**`cpp-idioms`**<br>**`cpp-concurrency`**<br>**`ub-mode`** | • C++20 健壮源码 (`include/` + `src/`)<br>• OSDI 14 原子落盘、盲水印、Typestate 与并发锁队列<br>• `filegate_demo` 可执行测试程序 (5大测试全通)<br>• `verify_implementation_quality.py` 自动化质量体检工具 (100分) |
| **[`4-testing-and-quality/`](./4-testing-and-quality/)** | **阶段四：测试工程与质量门禁** | **`cpp-testing`**<br>**`cpp-code-quality`** | • 19 个 AAA 边界单元测试套件 (`tests/`)<br>• `run_quality_gate.sh` 自动化门禁流水线<br>• LLVM ASan/UBSan 内存深度消毒报告 (0缺陷)<br>• `gcov` 核心源码覆盖率 88.14% 报告 |
| **[`5-review-and-refactoring/`](./5-review-and-refactoring/)** | **阶段五：代码审查诊断与安全重构** | **`code-review`**<br>**`refactoring`** | • 《FileGate 遗留模块代码审查与质量诊断报告》<br>• 5 大 Fowler 坏味道诊断与 68 种重构手法演进日志<br>• `verify_refactoring.py` 自动化重构度量与门禁工具<br>• 跨版本行为等价守护测试 (Safety-Net 6大场景100%等价) |
