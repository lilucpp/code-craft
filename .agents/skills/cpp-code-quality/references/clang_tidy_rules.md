# C++ 静态分析高信噪比规则集 (`.clang-tidy`) 详解

静态分析工具如果只输出 warning 而不阻断，AI 与开发者都会产生“告警疲劳（Alert Fatigue）”；反之，若把工程规范下沉为 `WarningsAsErrors`，工具链就能自动拦截 Slop。

本项目精选了一套**高信噪比、直击安全隐患、并发风险与结构腐化**的 C++17 规则组合。

---

## 一、 核心启用的高价值规则族

### 1. `bugprone-*`（程序隐患与参数防呆）
* **`bugprone-use-after-move`**：捕捉对象被 `std::move` 转移所有权后再次被读取/访问的致命 Bug。
* **`bugprone-dangling-handle`**：检测返回临时的 `std::string_view` 等导致的悬空引用。
* **`bugprone-infinite-loop`**：检测循环变量未正确递增导致的死循环。
* **`bugprone-easily-swappable-parameters`**：捕捉相邻同类型参数（如 `int user_id, int order_id`），倒逼落地 `StrongType` 或参数对象。

### 2. `cert-*` 与 `concurrency-*`（安全与并发红线）
* **`cert-env33-c` / `cert-msc50-cpp`**：拦截危险命令执行 `system()` 与不安全伪随机 `rand()`。
* **`cert-err33-c`**：拦截忽略 `snprintf` / `fread` / `fwrite` 等标准函数返回值的隐患。
* **`concurrency-mt-unsafe`**：拦截多线程环境下不安全的 C 库函数（如 `strtok`、`localtime`、`getenv`、`strerror`）。
* **`concurrency-thread-canceltype-asynchronous`**：拦截高危异步线程取消模式。

### 3. `cppcoreguidelines-*` & `readability-*`（结构复杂度与强约束）
* **`cppcoreguidelines-virtual-class-destructor` / `explicit-virtual-functions` / `pro-type-cstyle-cast`**：强制虚析构、`override` 及 C++ 显式转换。
* **`readability-function-size`**：将函数行数（`LineThreshold: 60`）、入参数量（`ParameterThreshold: 3`）和嵌套深度（`NestingThreshold: 3`）机器化拦截，直接阻断大杂烩长函数。
* **`readability-function-cognitive-complexity`**：认知复杂度阈值限制为 `15`。
* **`readability-magic-numbers`**：过滤 `-1;0;1;2` 与 2 的幂次后，督促提取命名常量。

### 4. `performance-*` & `modernize-*`
* **`performance-unnecessary-value-param` / `unnecessary-copy-initialization` / `move-const-arg`**：消除隐式深拷贝与伪移动。
* **`modernize-use-nullptr` / `google-explicit-constructor`**：杜绝 `NULL` 与单参隐式构造。

---

## 二、 刻意调整的降噪规则（防过度骚扰）

* `-modernize-use-trailing-return-type`：避免强制 `auto func() -> int` 后置返回类型影响可读性；
* `-google-runtime-references`：解除“禁止非常量引用形参”的陈旧限制；
* `-google-readability-function-size`：与 `readability-function-size` 重复，关闭别名避免双份告警；
* `-cppcoreguidelines-avoid-magic-numbers`：与 `readability-magic-numbers` 重复，统一由后者配参；
* `readability-identifier-length`：保留 3 字符下限，但放行常见系统简写（`id`、`fd`、`rc`、`ip`、`db`、`io`、`ok`、`x`、`y`、`z`）。

---

## 三、 配置模版与测试目录覆写

根目录配置由 [init_config.sh](../scripts/init_config.sh) 一键生成。若 `tests/` 目录包含大量测试字面量或参数化夹具，可在 `tests/.clang-tidy` 中做局部降噪继承：

```yaml
---
InheritParentConfig: true
Checks: >
  -readability-magic-numbers,
  -readability-function-size,
  -cppcoreguidelines-avoid-c-arrays
...
```
