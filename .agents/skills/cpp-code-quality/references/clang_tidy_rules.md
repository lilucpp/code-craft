# C++ 静态分析高信噪比规则集 (`.clang-tidy`) 详解

静态分析工具如果开启所有规则，会产生海量难以修复的无意义告警（如命名微小偏差、非必须的类型推导提示），从而导致开发团队产生“告警疲劳（Alert Fatigue）”。

本项目精选了一套**高信噪比、直击安全隐患与性能致命伤**的规则组合。

---

## 一、 核心启用的高价值规则族

### 1. `bugprone-*`（程序隐患与未定义行为）
* **`bugprone-use-after-move`**：捕捉对象被 `std::move` 转移所有权后再次被读取/访问的致命 Bug。
* **`bugprone-dangling-handle`**：检测返回临时的 `std::string_view` 或 `span` 导致的悬空指针。
* **`bugprone-infinite-loop`**：检测循环变量未正确递增导致的死循环。
* **`bugprone-argument-comment`**：强制长参数列表的调用处使用形参注释。

### 2. `cppcoreguidelines-*`（Bjarne Stroustrup 核心准则）
* **`cppcoreguidelines-virtual-class-destructor`**：含有虚函数的基类，析构函数必须为 `virtual` 或 `protected non-virtual`。
* **`cppcoreguidelines-explicit-virtual-functions`**：子类重写虚函数必须强制标记 `override`，杜绝拼写错误。
* **`cppcoreguidelines-pro-type-cstyle-cast`**：捕捉 `(int)x` 这种危险的 C 风格强转，强制使用 `static_cast`。

### 3. `performance-*`（性能与不必要的拷贝）
* **`performance-unnecessary-copy-initialization`**：捕捉本应声明为 `const auto &ref` 却因疏忽发生深拷贝的大对象。
* **`performance-move-const-arg`**：捕捉在 `const` 对象上调用 `std::move` 的伪移动（因为底层依然会退化为复制）。
* **`performance-for-range-copy`**：遍历容器时，若元素较大提示使用引用。

### 4. `modernize-*`（现代 C++ 规范）
* **`modernize-use-nullptr`**：将 `NULL` 与 `0` 自动替换为类型安全的 `nullptr`。
* **`modernize-use-auto`**：在类型显而易见（如 `new` 或 `make_shared`）时，强制使用 `auto` 提升代码整洁度。
* **`modernize-make-unique`**：推荐 `std::make_unique` 代替裸 `new`。

---

## 二、 刻意禁用的低信噪比规则（防过度骚扰）

* `-bugprone-easily-swappable-parameters`：对类型相同的相邻参数疯狂告警，在很多正常业务函数中产生海量噪音，故禁用；
* `-modernize-use-trailing-return-type`：强制要求全写成 `auto func() -> int` 后置返回类型，破坏传统可读性，故禁用；
* `-google-runtime-references`：Google 早期禁止非常量引用形参（强制用指针），现代 C++ 已废除该陈旧限制，故禁用；
* `-readability-magic-numbers`：对日常数组初始化或常规下标告警过多，故关闭。

---

## 三、 配置模版参考

```yaml
---
Checks: >
  -*,
  bugprone-*,
  -bugprone-easily-swappable-parameters,
  cppcoreguidelines-*,
  -cppcoreguidelines-pro-bounds-pointer-arithmetic,
  -cppcoreguidelines-owning-memory,
  modernize-*,
  -modernize-use-trailing-return-type,
  performance-*,
  google-*,
  -google-runtime-references,
  readability-*,
  -readability-magic-numbers,
  misc-*

WarningsAsErrors: 'bugprone-*,performance-*'
HeaderFilterRegex: '^(src|include)/.*'

CheckOptions:
  google-explicit-constructor: true
  modernize-use-nullptr: true
...
```
