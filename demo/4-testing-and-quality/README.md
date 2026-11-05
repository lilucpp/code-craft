# Testing & Quality Gates 自动化测试与质量门禁范例 (FileGate)

本目录展示了运用 **阶段四核心测试与质量门禁 Skills**（`cpp-testing`、`cpp-code-quality`）输出的工业级自动化单元测试体系、Mock 隔离、LLVM 内存消毒与覆盖率门禁范例。

---

## 目录文件概览

```text
4-testing-and-quality/
├── CMakeLists.txt                              # 现代 CMakeLists (CTest、ASan、Coverage 三模式)
├── README.md                                   # 本说明文件
├── run_quality_gate.sh                         # 一键全自动化质量门禁执行流水线
├── testing_evaluation_report.md                # 阶段四测试质量量化评估报告 (覆盖率 88.14%)
├── testing_process_log.md                      # 测试用例推导、并发死锁排障与覆盖率微调日志
├── include/
│   └── test_framework.hpp                      # 现代 GoogleTest/Catch2 风格轻量无网络脚手架
└── tests/
    ├── main.cpp                                # 测试运行入口
    ├── test_storage_engine.cpp                 # 存储引擎 AAA 边界测试 (空/单字节/1MB大文件/非法路径)
    ├── test_watermark_engine.cpp               # 水印引擎 Mock 依赖隔离打桩与显式/盲水印测试
    ├── test_typestate_lifecycle.cpp            # Typestate 状态机全生命周期与时效熔断断言
    └── test_concurrent_audit_queue.cpp         # 细粒度并发锁队列 4000 压测与优雅关闭测试
```

---

## 本地一键质量门禁检验

在终端中执行：
```bash
cd demo/4-testing-and-quality
./run_quality_gate.sh
```
流水线将自动完成：
1. **构建三套目标**：常规测试、ASan 内存消毒测试与 Coverage 覆盖率目标；
2. **执行 19 个 AAA 边界用例**（100% 通过，耗时约 79 ms）；
3. **LLVM AddressSanitizer/UBSan 深度消毒**（0 堆损坏、0 野指针、0 内存泄漏）；
4. **`gcov` 代码覆盖率统计**（核心代码行覆盖率 **88.14%**，达成 $\ge 80\%$ 准入门禁）。
