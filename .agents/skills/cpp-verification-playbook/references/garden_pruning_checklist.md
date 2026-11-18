# 软件花园除草与防杂草清单 (Garden Pruning Checklist)

> “代码库不是一锤子买卖的建筑，而是需要持续修剪的花园；放任一处 Slop 杂草，AI 就会把整座花园种满杂草。”

---

## 1. 定期除草四步扫描命令

```bash
# 1. 全量静态体检（老项目初次摸底先不阻断）
./.agents/skills/cpp-code-quality/scripts/run_check.sh --mode=all --warn-only

# 2. 检测未被调用的死函数（结合编译数据库或业务目录）
cppcheck --language=c++ --std=c++17 --enable=unusedFunction --quiet \
  -i third_party/ -i build/ -i tests/ src/ include/

# 3. 检查全仓模块依赖拓扑与私有头泄露
python3 ./.agents/skills/code-design/scripts/check_include_dag.py --root . --dump-graph --warn-only

# 4. 独立红线扫雷（禁用 API、空 catch、脆弱路径）
./.agents/skills/cpp-code-quality/scripts/run_check.sh --mode=all --tool=redline --warn-only
```

---

## 2. 五类高危“AI 杂草”清理优先级

| 优先级 | 杂草类型 | 典型症状 | 清理动作与固化手段 |
| :---: | :--- | :--- | :--- |
| **P0** | **平行轮子并存** | 引入了 `reproc++`/`utfcpp` 后，角落里仍残留裸 `fork`/`popen` 或手写 UTF-8 解码 | 全局替换收敛至统一库；在 `lint_redlines.py` 封杀旧 API |
| **P0** | **跨层偷引与环路** | `domain` 直接 `#include "infra/detail/..."` | 用接口反转解环；在 CMake 隔离 `PRIVATE` 路径并开 `check_include_dag.py` |
| **P1** | **Happy-Path 自嗨测试** | 单测只测正常输入且全靠 Mock 自证，边界/异常分支 0 覆盖 | 补齐 4 类边界矩阵、RapidCheck 属性测试及 Diff Coverage 门禁 |
| **P1** | **僵尸函数与冗余 Include** | AI 重构后留下的未调用辅助函数、无用 `#include` | 删除死代码；清理 `misc-include-cleaner` 告警缩短编译时间 |
| **P2** | **膨胀的函数与裸标量** | 超过 60 行、4 个以上同类型 `int` 形参的面条函数 | 提炼纯函数 / 封装参数对象 / 替换为 `StrongType` 并靠 `.clang-tidy` 卡死 |

---

## 3. Verifier 独立验收快速检查表

当作为独立 Verifier 检查另一名 Agent 提交的 C++ 变更时，逐项核验：
1. [ ] `run_check.sh --mode=diff` 退出码是否为 `0`（确认未使用 `--warn-only` 蒙混过关）？
2. [ ] `check_include_dag.py` 是否无新增环路或跨层穿透？
3. [ ] 是否跑了**全量** `ctest`（非仅跑单测文件名过滤），且挂载了 **ASan + UBSan**？
4. [ ] 若变更涉及锁、原子变量、线程池，是否额外跑过 **TSan** 重复压测？
5. [ ] 若变更涉及文件写入、协议解析，是否有故障注入或异常输入边界测试？
6. [ ] 输出的 **Evidence Bundle** 中的命令、数字、退出码是否均来自真实工具运行输出？
