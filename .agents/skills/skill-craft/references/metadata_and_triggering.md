# 元数据规范与双语双锚点设计指南

> 依据 Anthropic 官方规范与 Code-Craft 双语工程实践总结。

---

## 1. 元数据核心机制与重要性

在 Agent 系统运行周期中，**元数据（Metadata）是常驻且唯一被预加载的部分**：
1. **预加载开销**：无论工作区有多少个技能，启动时 Agent 仅读取所有技能的 `name` 和 `description`，正文并不消耗初始上下文 Token。
2. **唯一检索索引**：Agent 是完全根据 `description` 中的文本语义与用户 Prompt 的相似度，来决定“是否使用工具读取本技能的 `SKILL.md`”。
3. **生死线**：如果元数据写得模糊、缺失触发关键词或过于冗长，技能将面临**永远无法被唤醒（Under-triggering）**或**处处抢占其他技能（Over-triggering）**的故障。

---

## 2. 字段规范与约束矩阵

| 字段 | 字符限制 | 字符集 | 语法人称 | 核心目标 |
| :--- | :--- | :--- | :--- | :--- |
| **`name`** | $\le 64$ 字符 | `^[a-z0-9-]+$`（小写字母、数字、连字符） | 名词/动名词短语 | 唯一标识符，简明扼要表达领域 |
| **`description`** | $\le 1024$ 字符 | UTF-8 全字符集（建议中英双语） | **严格第三人称**（禁止第一/第二人称） | 解释“做什么”与“何时触发（Use when）” |

---

## 3. 第三人称铁律与常见反模式

Agent 模型在解析工具与技能列表时，会将第一人称解释为 Agent 自身的固有能力混淆，导致决策置信度下降。

### ❌ 错误反模式示例
```yaml
# 错误 1：第一人称
description: I can help you review your C++ code and find memory leaks.

# 错误 2：第二人称命令式
description: You should use this skill whenever you need to optimize database indexes.

# 错误 3：只说了做什么，没说何时用（缺失触发词）
description: This skill contains a collection of Martin Fowler refactoring mechanics.
```

### ✅ 正确模式示例
```yaml
description: >-
  Reviews C++ code for memory leaks, undefined behavior, and modern C++ best practices.
  Use when conducting code reviews, analyzing memory allocation bugs, or auditing pointer safety.
```

---

## 4. 双语双锚点公式（Bilingual Dual-Anchor Formula）

为了同时完美支持多语言环境（英文海外生态、CLI 工具链调用）与本地化中文日常对话，Code-Craft 独创并实践了**双语双锚点描述公式**：

```yaml
description: >-
  [英文能力摘要: 第三人称动词开头，简述核心价值]
  Use when [英文触发场景列表: 涵盖动作动词、英文专业术语、命令词组合].
  [中文能力摘要: 包含中文技术术语与系统领域].
  当用户要求[中文触发场景列表: 包含中文动宾短语、口语化提问变体]时触发。
```

### 生产级示范案例（以 `sql-craft` 为例）：
```yaml
---
name: sql-craft
description: >-
  Designs relational schemas, B-Tree indexes, refactors SQL antipatterns, and optimizes deep pagination based on Markus Winand and Bill Karwin classics.
  Use when designing database schemas, creating composite indexes, optimizing slow queries, keyset pagination, or refactoring EAV and Jaywalking.
  关系型数据库 Schema 设计、B-Tree 物理索引优化、SQL 高性能查询与反模式防御技能。
  当用户要求设计数据库表结构、设计/优化 B-Tree 索引、排查 SQL 慢查询、治理大表深分页、或重构 SQL 反模式时触发。包含“主动建模设计”与“SQL诊断重构”双重引擎。
---
```

---

## 5. 防触发冲突与消歧原则

当项目中存在多个相似技能时（例如 `code-review` 与 `code-design`），必须通过清晰的边界词进行互斥消歧：
1. **输入意图正交化**：
   - `code-design`：聚焦于**“写新代码”、“定义新架构”、“接口设计”**（前向构建）；
   - `code-review`：聚焦于**“看既有代码”、“找坏味道”、“评估质量”**（后向诊断）；
   - `refactoring`：聚焦于**“改动现有实现”、“安全替换行为”、“消除具体坏味道”**（行为重组）。
2. **负向排除提示（若必要）**：
   在技能描述中可适度补充排除提示，例如：“For pure formatting, use cpp-code-quality instead.”。
