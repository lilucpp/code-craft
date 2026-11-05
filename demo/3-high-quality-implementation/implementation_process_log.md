# FileGate 阶段三：高质量规范实现推导与微调日志 (Implementation Process Log)

> **记录目的**：真实记录从阶段二详细设计（Detailed Design）向阶段三高质量 C++ 代码实现（Implementation）推进过程中的代码架构决策、遇到的技术阻碍与编译期排错细节、代码异味重构过程及实证检验闭环。

---

## 1. 详细设计向 C++ 代码的意图拆解与选型

依据阶段二确立的结构蓝图，在编码阶段确立四大技能落地防线：
1. **`effective-cpp`**：
   - 杜绝裸指针，全量以 `std::unique_ptr` 与自定义 RAII `FileDescriptorGuard` 实施资源管理；
   - 引入零开销 `StrongType<T, Tag>` 封装 `ShareRequestId`、`UserId`、`TimestampMs`、`ByteSize`，消除参数倒置风险；
   - 全面遵守移动语义与 `noexcept` 声明。
2. **`cpp-idioms`**：
   - 在存储抽象与水印策略中推行 **NVI (Non-Virtual Interface)** 惯用模式，公有非虚入口把关前置契约，受保护纯虚函数实现多态；
   - 提取通用的 Scope Guard 模板，消除多重退出分支下的清理遗漏。
3. **`cpp-concurrency`**：
   - 实现 Anthony Williams 经典的细粒度头尾分离锁并发队列，以哨兵 Dummy 节点消除队列为空时的死锁竞态，并在头锁与尾锁间实施 `alignas(64)` 物理伪共享隔离。
4. **`ub-mode` 与编码红线**：
   - 坚持单一抽象层级 (SLAP)；
   - 函数长度严格限制在 $\le 30$ 行；
   - 卫语句优先 (Fail-Fast)。

---

## 2. 编码实现中遭遇的技术阻碍与修复细节 (Exhaustive Debugging Log)

### 阻碍 1: `ShareRequestMeta` 聚合默认构造失败
- **故障现象**：在编译 `ShareRequest<StateDraft>::CreateNew` 时，编译器抛出错误：
  `error: could not convert ‘<brace-enclosed initializer list>()’ to ‘filegate::types::ShareRequestId’`
- **根本原因**：`ShareRequestMeta` 内部包含 `StrongType` 强类型字段，而原本的 `StrongType` 只声明了显式有参构造函数 `explicit StrongType(const T& val)`，缺少默认无参构造函数，导致 `types::ShareRequestMeta meta{}` 进行聚合初始化时无法对 `StrongType` 字段执行零初始化。
- **精准修复**：在 `StrongType` 中补充显式常量无参默认构造：
  `constexpr StrongType() noexcept : value_{} {}`
  成功解决零开销强类型的聚合值初始化。

### 阻碍 2: C++ 模板循环前向引用导致不完整类型 (Incomplete Type)
- **故障现象**：编译器报错：
  `error: invalid use of incomplete type ‘class filegate::core::ShareRequest<filegate::core::StateActive>’`
- **根本原因**：在 `ShareRequest<StateApproved>::Activate` 成员函数的声明与内联定义中，其返回值为 `Result<ShareRequest<StateActive>>`。而在文件编译到该行时，`ShareRequest<StateActive>` 的特化定义位于更靠后的位置，导致 `std::optional<T>` 在探测 `T` 的移动构造特性时因为 `T` 为不完整类而引发硬编译中断。
- **精准修复**：严格执行“先声明所有类特化、再在文件尾部集中内联实现成员函数”的 C++ 头文件规范：
  1. 顶部集中给出 `ShareRequest<StateDraft>`、`StatePendingApproval`、`StateApproved`、`StateActive` 4 个特化声明；
  2. 底部编写 `CreateNew`、`Submit`、`Approve`、`Activate`、`RecordDownload` 等内联函数；
  3. 彻底消除类型不完整依赖。

### 阻碍 3: 自动化体检扫描检出超长函数 (Function Length > 30)
- **故障现象**：自动化体检脚本 `verify_implementation_quality.py` 报告：
  `FAIL: 函数长度与单一抽象层级 (main.cpp: TestTypestateLifecycle=39行, TestConcurrentQueue=38行)`
- **根本原因**：在编写集成测试入口时，直觉性地将创建、送审、签批、激活、多轮下载全部写在同一个大函数中，违反了 Uncle Bob“函数不超过 30 行且维持单一抽象层级”的红线。
- **重构优化**：
  运用 Martin Fowler《重构》的“提炼函数 (Extract Function)”手法：
  1. 将主测试逻辑提炼为 `RunApprovalWorkflow()` (20行) 与 `RunDownloadAndMeltdown()` (15行)；
  2. 将并发压测的生产者线程启动提炼为 `LaunchProducers()` (14行)；
  3. 最终顶层协调函数压缩至仅 10 行；
  4. 重新扫描后 35 个函数全部达成 $\le 30$ 行（100% 达标）。

### 阻碍 4: 正则表达式匹配强类型别名格式偏差
- **故障现象**：体检脚本误报 StrongType 未被采用。
- **根本原因**：在 `types.hpp` 中使用了多空格排版 `using UserId         = StrongType...`，原本字符串直接查找 `using UserId = StrongType` 导致空格不匹配。
- **精准修复**：重写检测逻辑为标准正则 `re.search(r'using\s+UserId\s*=\s*StrongType', content)`，容忍一切美化对齐格式，检测全绿通过。

---

## 3. 最终实证运行与闭环结果

在完成上述重构与精细打磨后，执行全量自动化流水线：
1. **CMake 编译构建**：`-std=c++20 -Wall -Wextra -Wpedantic` 零 Warning、零 Error，生成原生二进制 `filegate_demo`；
2. **二进制执行**：
   - 内存尺寸与自然对齐验证通过 (64 字节，0 Padding)；
   - OSDI 14 掉电保护四步原子落盘与安全粉碎验证通过；
   - 水印策略模式与不可见盲水印微调注入验证通过；
   - Typestate 状态机生命周期与卫语句防御拦截验证通过；
   - 4 生产者高并发推入 4000 条日志多线程细粒度锁队列压测通过（0 丢失，0 数据竞态）；
3. **代码体检脚本**：7/7 项全通，获得 100.0 分（EXCELLENT 卓越等级）。
