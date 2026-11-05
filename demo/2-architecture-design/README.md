# Architecture & Detailed Design 工业级系统架构与详细设计范例 (FileGate)

本目录展示了运用 **阶段二核心架构 Skills**（`code-design`、`systems-data-design`、`sql-craft`、`unix-philosophy`、`antqt`）输出的标准工程系统架构与详细设计范例。

---

## 目录文件概览

| 文件 | 说明 |
| :--- | :--- |
| **[`filegate_detailed_design.md`](./filegate_detailed_design.md)** | **《FileGate 系统架构与详细设计说明书》**：包拓扑 DAG、DbC 契约、8-4-2-1 降序对齐、Typestate 状态机、B-Tree 索引、OSDI 14 原子落盘与 AntQt 桌面工作台 |
| **[`verify_architecture_design.py`](./verify_architecture_design.py)** | **自动化详设评估与验证工具**：实现对包依赖无环 (ADP)、DbC 完备性、C++ ABI 内存对齐、伪共享隔离、Typestate 状态机、SQL 索引列序与 OSDI 14 落盘四步法的自动化形式化核查 |
| **[`architecture_evaluation_report.md`](./architecture_evaluation_report.md)** | **详设评估量化报告**：基于 Uncle Bob、Bertrand Meyer、Anthony Williams、OSDI 14 与 Markus Winand 的权威标准评审结论（7项全通，100分） |
| **[`design_process_log.md`](./design_process_log.md)** | **架构推导演进日志**：记录从 SRS 意图分解、双向循环依赖解环、结构体内存空洞挤出到编译期时序拦截的完整决策微调过程 |

---

## 本地一键自动化形式化检验

在终端中执行：
```bash
python3 demo/2-architecture-design/verify_architecture_design.py
```
或直接进入该目录执行：
```bash
cd demo/2-architecture-design
python3 verify_architecture_design.py
```
脚本将自动执行 7 项形式化静态核查，输出 100% 通过的实证报告。
