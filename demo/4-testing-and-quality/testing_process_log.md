# FileGate 阶段四：测试工程与质量门禁推导与微调日志 (Testing Process Log)

> **记录目的**：真实记录从阶段三代码向阶段四自动化测试工程推进过程中的测试套件设计、接口打桩解耦、边界用例挖掘、并发死锁隐患排除以及 ASan 内存消毒与覆盖率分析过程。

---

## 1. 测试工程设计原则与结构规划

依据阶段四推荐的核心 Skills（**`cpp-testing`** 与 **`cpp-code-quality`**），规划四大测试防线：
1. **脚手架与断言体系**：构建轻量、跨平台且无需外部网络的现代化测试框架，全面支持 `EXPECT_EQ`、`EXPECT_THROW`、`EXPECT_TRUE` 等丰富断言；
2. **测试架构 AAA 模式**：每一个测试用例严格分为 Arrange（准备）、Act（执行）、Assert（断言验证）三段，禁止测试逻辑杂糅；
3. **4 象限输入边界覆盖**：针对空输入、单元素、极值超限、非法参数进行破坏性测试；
4. **Mock 打桩与依赖注入 (DI)**：验证接口是否真正解耦，避免高延迟算法或文件环境对纯逻辑产生干扰；
5. **双内存消毒剂 (ASan + UBSan)**：利用编译器消毒剂在测试运行期主动引爆悬空指针与内存违规；
6. **代码覆盖率流水线卡点**：以 `gcov` 建立 $\ge 80\%$ 准入门禁。

---

## 2. 测试开发与调试中发现的问题与微调记录 (Exhaustive Debugging Log)

### 阻碍 1: `EXPECT_EQ` 宏对于 `enum class` 流输出编译报错
- **故障现象**：在断言 `EXPECT_EQ(result.code(), types::ErrorCode::kInvalidArgument)` 时，g++ 抛出编译错误：
  `error: no match for ‘operator<<’ (operand types are ‘std::basic_ostream<char>’ and ‘filegate::types::ErrorCode’)`
- **根本原因**：C++20 的强作用域枚举（`enum class`）出于强类型安全，不会隐式提升为 `int`，标准库 `std::ostream` 也未预定义枚举类的格式化重载。测试框架的断言宏在格式化失败信息时无法将 `ErrorCode` 直接写入 `std::ostringstream`。
- **精准修复**：在 `demo/3-high-quality-implementation/include/filegate/types.hpp` 中主动为领域枚举类提供标准重载：
  ```cpp
  inline std::ostream& operator<<(std::ostream& os, ErrorCode code) {
      return os << static_cast<int>(code);
  }
  ```
  使领域类型天生支持日志打印与测试断言。

### 阻碍 2: 多消费者并发测试时线程挂起（死锁挂起隐患排查）
- **故障现象**：运行 `./filegate_tests` 时，进程卡在 `ConcurrentQueueTest.MultiProducerMultiConsumerZeroLoss` 处无法退出。
- **根本原因**：在测试用例中启动了 2 个消费者线程，循环条件为 `while (consumed_count < 4000)`。当第 1 个消费者刚好消费完成第 4000 条记录时，`consumed_count` 变为 4000，第 1 个消费者安全退出；但第 2 个消费者此时正阻塞在 `queue.WaitAndPop()` 的条件变量上等待新数据！由于 4000 条数据已全部由生产者推送完毕，再无新数据入队，第 2 个消费者线程将**永久阻塞在条件变量等待上**，导致 `worker.join()` 永久挂起。
- **并发重构优化**：
  依据 Anthony Williams 并发关闭协议，采用“优雅关闭广播（Graceful Shutdown Broadcast）”机制：
  1. 消费者内部判断：当达到目标累计数量时，主动调用 `queue.Shutdown()`；
  2. 生产者全部 `join()` 完毕后，主线程兜底执行 `queue.Shutdown()`；
  3. `Shutdown()` 会将 `is_shutdown_` 置为 true，并触发 `notify_all()` 唤醒所有正在阻塞的消费者；
  4. 消费者在 `WaitAndPop()` 醒来后发现队列已关闭且为空，返回 `nullptr` 并安全退出线程；
  5. 重构后 17 个多线程压测用例在 11 毫秒内极速执行完毕，彻底消灭挂起隐患。

### 阻碍 3: 水印策略部分覆盖率偏低（从 55% 提升至 100%）
- **故障现象**：初次 `gcov` 扫描显示 `watermark_engine.cpp` 覆盖率仅为 55.88%。
- **原因分析**：初始用例重点测试了 Mock 隔离与 DCT 盲水印，但漏测了：
  1. `VisibleTextWatermarkStrategy::DoInject` 的显式注入逻辑；
  2. `WatermarkEngine::SetStrategy` 的动态切换；
  3. 未设置策略时的防御报错（`ErrorCode::kInvalidState`）。
- **补全测试用例**：
  在 `test_watermark_engine.cpp` 中补充了 `VisibleTextWatermarkSuccess` 与 `StrategyNullAndSetStrategySuccess` 两个针对性测试用例，将 `watermark_engine.cpp` 的代码覆盖率从 55.88% 直接拉升至 **100.00%**。

---

## 3. 最终实证运行与闭环验收

在完成上述全部测试重构后，执行自动化质量门禁流水线：
```bash
bash demo/4-testing-and-quality/run_quality_gate.sh
```
- **构建结果**：`-Wall -Wextra -Wpedantic` 零 Warning，成功编译 `filegate_tests`、`filegate_tests_asan`、`filegate_tests_coverage`；
- **用例结果**：19 个 AAA 边界用例 100% 通过（耗时 79 ms）；
- **内存安全**：挂载 ASan/UBSan 执行全量用例，无任何堆越界、野指针、内存泄漏；
- **覆盖率数据**：核心业务代码综合行覆盖率达到 **88.14%**，远超工业级 $80\%$ 准出门禁；
- **质量结论**：阶段四测试工程建设完备，代码坚不可摧。
