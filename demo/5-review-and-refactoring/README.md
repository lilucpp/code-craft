# Review & Refactoring 审查诊断与安全重构范例 (FileGate)

本目录展示了运用 **阶段五审查与重构 Skills**（`code-review`、`refactoring`）输出的代码审查诊断、Martin Fowler 22 种代码坏味道排查、68 种重构手法演进、以及跨版本行为等价守护测试（Safety-Net）工程范例。

---

## 目录文件概览

```text
5-review-and-refactoring/
├── CMakeLists.txt                         # 现代 C++20 构建配置
├── README.md                              # 本说明文件
├── verify_refactoring.py                  # 一键自动化重构门禁与度量检验脚本
├── code_review_report.md                  # 结构化代码审查诊断报告 (5 大 Fowler 坏味道清单)
├── refactoring_process_log.md             # 双帽工作法、小步快跑演进与客观度量记录
├── legacy/                                # 重构前：典型遗留代码坏味道样本
│   └── legacy_token_service.hpp           # 包含深层嵌套金字塔、基本类型偏执、依恋情结等
├── refactored/                            # 重构后：清晰高内聚现代 C++ 实现
│   └── token_service.hpp                  # 应用 Guard Clauses、Extract Class、强类型等
└── tests/                                 # 跨版本行为等价守护测试 (Safety-Net)
    └── test_refactoring_equivalence.cpp   # 6 大核心业务场景重构前后 100% 行为等价断言
```

---

## 本地一键验证命令

在终端中执行：
```bash
cd demo/5-review-and-refactoring
python3 verify_refactoring.py
```

验证脚本将自动完成：
1. **条件分支嵌套深度度量**（5 层金字塔 $\rightarrow$ 1 层扁平卫语句，大幅降低认知负荷）；
2. **函数体长度与职责收敛度量**（消除 Long Method 与 Divergent Change）；
3. **Fowler 坏味道与重构手法映射核验**（5 项坏味道 100% 消除）；
4. **编译并执行行为等价守护测试**（`test_refactoring_equivalence` 6 大业务场景 100% 输出等价保持）。
