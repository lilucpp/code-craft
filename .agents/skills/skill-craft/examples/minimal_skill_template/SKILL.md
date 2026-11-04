---
name: your-skill-name
description: >-
  [Third-person summary of what this skill does: 1-2 concise sentences].
  Use when [English triggering keywords and scenarios, e.g. when building X, debugging Y, or configuring Z].
  [中文能力核心概括：1-2 句简洁定义，说明核心专业领域与产出物]。
  当用户要求[中文触发关键词列表：如 创建X、排查Y、配置Z]时触发。
---

# [技能中文标题]

简要说明本技能解决的核心问题与理论依据。

---

## 1. 核心工作流与决策树

```mermaid
flowchart TD
    Req[用户输入诉求] --> Decision{判断任务场景}
    Decision -->|场景 A: 创建新方案| BranchA[执行路径 A]
    Decision -->|场景 B: 诊断与优化| BranchB[执行路径 B]
```

---

## 2. 核心执行指南

### 场景 1：[场景名称]
当用户要求 [具体场景描述] 时：
1. 步骤说明与命令：
   ```bash
   ./scripts/run.sh --flag
   ```
2. 注意事项与边界约束。

---

## 3. 核心参考手册

- [专题手册 A](./references/topic_a.md)：针对特定技术的详细规范。
- [专题手册 B](./references/topic_b.md)：针对疑难问题的排查清单。
