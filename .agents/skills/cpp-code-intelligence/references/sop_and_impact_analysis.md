# C++ 智能体工程 SOP 与影响面分析 (Impact Analysis)

本手册专为使用 Antigravity 进行大型 C++ 项目开发时提供标准化工程作业规范，涵盖新增功能、安全重构与底层优化的闭环流程。

---

## 1. 场景一：新增功能 (Feature Addition)

### 阶段 1：锚定扩展点与契约识别
1. **调用工具**：使用 `workspace_symbol_search` 查找相关领域抽象类或核心服务。
2. **审查契约**：调用 `find_definition` 查看该类的头文件：
   - 是否包含纯虚函数（Interface Contract）？
   - 虚析构函数是否为 `virtual ~ClassName() = default;`？
   - 所有权模型：入参/返回值使用裸指针、引用还是智能指针（`std::unique_ptr` / `std::shared_ptr`）？
   - 线程安全性：接口是否有并发线程约束或互斥锁标记？

### 阶段 2：遵循开闭原则 (OCP) 最小侵入实现
1. **优先组合与派生**：通过继承纯虚接口或使用适配器/装饰器模式新增功能类，严禁直接在庞大的既有旧类中狂加 `if-else` 或无谓的公有成员。
2. **头文件轻量化**：
   - 仅包含必须的头文件，能前置声明（Forward Declaration）的类绝对不 `#include`；
   - 隐藏私有实现细节（推荐 PImpl 惯用法或匿名命名空间）。

### 阶段 3：测试用例自测
1. 编写对应类的 GoogleTest 测试套件（遵循 AAA 模式：Arrange-Act-Assert）；
2. 运行构建并运行 GTest 测试，确保新功能全部通过。

---

## 2. 场景二：代码重构 (Safe Refactoring)

### 阶段 1：全工程影响面分析 (Impact Analysis)
在修改任何公共函数、类名或破坏 ABI/API 的签名之前，**必须**首先调用 `find_references`：
```
目标符号: ServiceCoordinator::DispatchTask(int taskId)
1. 调用 find_references 统计所有被引用的源文件及行号列表
2. 评估调用范围:
   - 局部重构: 仅在当前源文件或内部子模块被调用 -> 安全快速重构
   - 广泛影响: 被数十个跨目录模块调用 -> 制定渐进重构过渡策略 (如 [[deprecated]] 标记过渡)
```

### 阶段 2：小步安全演进 (Martin Fowler Mechanics)
1. **以多态取代条件表达式 (Replace Conditional with Polymorphism)**：
   消除散落在各处的 `switch (type)` 逻辑，将分支下沉到策略派生类中；
2. **以查询取代临时变量 (Replace Temp with Query)**：
   消除长函数内污染上下文的高耦合临时局部变量；
3. **提炼函数 (Extract Function)**：
   确保单个函数满足单一职责原则 (SRP) 与单一抽象层级 (SLAP)，长度控制在 30 行以内。

### 阶段 3：零编译破坏与零行为衰退回归
1. 调用 `get_diagnostics` 确保全工程没有由于签名变动导致的编译报错或类型隐式转换告警；
2. 运行回归测试，验证重构后旧功能行为严格保持 100% 保真。

---

## 3. 场景三：性能与底层优化 (Performance Optimization)

### 阶段 1：数据驱动与基准测试先行
* 严格遵循 `RULE[user_global]` 严谨性原则，**严禁凭空推测瓶颈**；
* 编写可复现的最小可运行 Benchmark 代码，记录优化前 baseline 数据（纳秒/微秒级延迟或吞吐量）。

### 阶段 2：硬件与体系结构级优化排查
1. **Cache Line 伪共享 (False Sharing)**：
   并发多线程频繁写入的相邻变量，显式对齐至 Cache Line：
   ```cpp
   alignas(64) std::atomic<uint64_t> write_counter_{0};
   ```
2. **结构体内存重排 (Memory Padding)**：
   按大小降序重排数据成员（`uint64_t` $\to$ 指针 $\to$ `int32_t` $\to$ `bool`），消除隐式内存间隙；
3. **消除虚函数开销**：
   在热路径（Hot Path）中，将基于 vtable 的动态多态替换为 CRTP 静态多态，支持内联优化；
4. **移动语义与消除隐式拷贝**：
   重载大对象的右值移动构造函数，显式标记 `noexcept`，以便 `std::vector` 扩容时自动触发移动而非拷贝。

### 阶段 3：优化数据实测对比
* 重新运行 Benchmark，生成“优化前 vs 优化后”实测数据对比表（延时下降比例、吞吐提升倍数、内存峰值变化）。
