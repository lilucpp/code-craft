# 高品质工程与开源 README 编写指南 (README Guidelines)

> 融合酷壳技术认知模型与顶级开源项目（如 Redis, Vue, FastAPI）的标准实践。

README 是一个项目的门面。一个卓越的 README 能够让读者在 **30 秒内理解项目的核心价值**，并在 **3 分钟内完成首次上手运行**。

---

## 一、 工业级 README 黄金骨架 (Standard Template)

```markdown
# 项目名称 (Project Name)

> 一句话价值主张 (Value Proposition)：清晰、准确地描述这个项目“为什么存在”、“解决什么核心痛点”。

[English](./README_EN.md) | 简体中文

---

## 1. 为什么需要这个项目？(Why & Problem Model)
- 描述它所针对的真实场景与现有方案的痛点；
- 说明本项目的核心设计理念（例如：轻量、零依赖、高性能、易扩展）。

---

## 2. 架构图解与核心特性 (Architecture & Features)
- **架构拓扑**：使用 Mermaid 或 ASCII 流程图直观展示核心组件关系；
- **特性清单**：提炼 3~5 个核心亮点（避免罗列细枝末节）。

```mermaid
flowchart LR
    Client --> API Gateway
    API Gateway --> Core Engine
    Core Engine --> Storage
```

---

## 3. 快速上手 (Quick Start)
必须提供可以在空白环境下“复制、粘贴、立即运行成功”的最小完整示例：

### 前置条件 (Prerequisites)
- 如：Python 3.10+ / Go 1.21+ / Docker 24.0+

### 安装与运行 (Installation & Usage)
```bash
# 1. 克隆代码
git clone https://github.com/org/repo.git
cd repo

# 2. 启动最小示例
python main.py
```

### 最小代码用例 (Minimal Code Snippet)
```python
from mylib import Client

# 3 行代码展示核心能力
client = Client()
result = client.process(data="hello")
print(result)
```

---

## 4. 关键技术选型与权衡 (Design Trade-offs)
- 说明为什么采用某种技术/架构路线，而不是其他主流替代方案；
- 坦承当前方案的适用边界与不适用场景（Pitfalls / Trade-offs）。

---

## 5. 配置说明 (Configuration)
遵循“惯例优于配置 (CoC)”原则，只列出关键配置项及其合理默认值：

| 配置键 (Key) | 环境变量 (Env) | 默认值 (Default) | 说明 (Description) |
| :--- | :--- | :--- | :--- |
| `port` | `APP_PORT` | `8080` | HTTP 服务监听端口 |
| `log_level` | `LOG_LEVEL` | `info` | 日志输出级别 (`debug`, `info`, `warn`) |

---

## 6. 参与贡献与致谢 (Contributing & License)
- 简短说明如何提交 PR / Issue；
- 声明开源许可证（MIT / Apache 2.0 / BSD）。
```

---

## 二、 避免常见的 README 坏味道 (Anti-Patterns)

1. **只有安装命令，没有代码示例**：让用户猜怎么使用 API；
2. **示例代码缺失上下文**：贴出的代码缺少导入、缺少入参构造，复制下来根本跑不通；
3. **大段纯文字无图解**：缺乏架构流程图，导致理解成本翻倍；
4. **报喜不报忧**：不说明系统局限性和性能边界，误导使用者；
5. **版本失效**：文档中的参数名早已在最新代码中被废弃。
