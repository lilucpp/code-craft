# 盲审 Verifier 与并行 Swarm 调度模板

> 用于打破“同一上下文既写代码又自证通过”的确认偏误（Confirmation Bias）。

---

## 1. 盲审 Verifier 派单模板（`invoke_subagent`）

当完成 **T2 级（跨模块 / 并发 / 持久化 / 大重构）** 任务准备收口时，调用 `invoke_subagent`（推荐 `TypeName: "self"`，只读/验证职责），按以下**盲审模板**派发任务：

### 信息隔离红线
* **必须提供**：需求/契约目标、目标项目根目录、改动文件清单（或 `git diff --stat`）。
* **严禁透传**：“我已修复全部问题”、“所有测试已经通过”、“应该没问题”等作者主观结论，防止诱导子 Agent 偷懒跳过核查。

### 可直接复制的 Verifier Prompt 模板

```text
你是本任务的独立盲审 Verifier（严禁修改业务源码，只负责客观取证与缺陷拦截）。

【待验收目标】
- 工程目录：<TARGET_DIR>
- 契约要求：<1~3 句概括外部行为要求与非功能约束>
- 变更范围：<git diff --name-only 输出的文件列表>

【强制核验步骤】
1. 独立运行总控门禁（严禁擅自加 --warn-only，除非明确标注为历史老项目）：
   bash ./.agents/skills/cpp-verification-playbook/scripts/verify_gate.sh --dir="<TARGET_DIR>" --mode=diff --require-tests
   （若涉及并发/锁修改，请确认 build-tsan 或追加 --tsan-repeat=20）
2. 逐文件执行 `code-review` + `secure-cpp` 深度审计，重点核查：
   - 没有为通过测试而篡改/弱化原有断言（Test Tampering）；
   - 二进制/协议解析是否满足：**校验和覆盖 Header 元数据**、`reserve(count)` 前有剩余字节物理上界校验、无 OOM 放大；
   - 边界条件（空输入/极值/越界/写盘或系统调用权限失败路径）均有真实测试覆盖；
   - 跨模块无新增 `detail/`/`internal/` 穿透，无锁序逆转（AB-BA）或资源泄漏。
3. 严格按实测与审计分级输出结论（**发现任何 Blocker 或 Major 必须判 BLOCKED**）：
   - 明确判定：**PASS（0 Blocker & 0 Major）** 或 **BLOCKED**；
   - 原样附上 `verify_gate.sh` 输出的 **Evidence Bundle** 表格；
   - 若 BLOCKED，逐条列出 `[Blocker/Major/Minor]` 的精确 `file:line` 缺陷与必须补齐的回归测试，打回 Author 修复后重审。
```

---

## 2. 多模块并行 Swarm 编排规范（`swarm` / `/teamwork`）

当需求拆分为 2 个以上解耦的 C++ 模块（如 `module-a` 与 `module-b`）需要并行开发时：

1. **Gate 1 串行先锁契约**：由 Coordinator 先定义并落盘模块间接口头文件（`include/<mod>/`）及 `.arch-boundaries.json`，跑通 `check_include_dag.py` 后再派发并行开发；
2. **隔离工作区写代码**：为每个作者 Worker 分配 `workspace: "share"`（各自独立工作副本），避免同时编译和修改冲突；
3. **只读 Verifier 守合并门**：合入 Coordinator 主工作区后，指派 1 个未参与编码的 `verifier`（`workspace: "inherit"`）运行全量 `verify_gate.sh --mode=all --require-tests` 并输出 Evidence Bundle 后方可向用户交付。
