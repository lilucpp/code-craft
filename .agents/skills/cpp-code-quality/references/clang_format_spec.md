# C++ 代码格式化规范与 `.clang-format` 详解

本项目 C++ 代码格式化以 **Google C++ Style Guide** 为基准，并融合现代开发习惯应用了三处关键定制：

```yaml
BasedOnStyle: Google
DerivePointerAlignment: false
PointerAlignment: Right
ColumnLimit: 120
```

---

## 一、 三项核心定制意图解析

### 1. `PointerAlignment: Right`（指针与引用靠右对齐）
* **动机**：
  * C++ 中类型声明遵循“声明与使用一致”的历史惯例。
  * `char *p;` 比 `char* p;` 更清晰地表达了指针修饰符归属于变量名本身（特别是在连写 `char *p, ch;` 时避免歧义）。
  * 保持团队内统一的右对齐手感。
* **效果对比**：
  ```cpp
  // ❌ Google 默认（靠左居中或居左）:
  std::string* GetName(const User& user);

  //  本项目标准（强制靠右对齐）:
  std::string *GetName(const User &user);
  const int *const ptr = &val;
  ```

### 2. `DerivePointerAlignment: false`（禁用推断）
* **动机**：
  * `clang-format` 默认会尝试统计文件现存代码的指针位置并自动推测。在多人协作或老项目混合代码中，这会导致推断混乱，产生反复格式化的 Git 脏提交。
  * 强制设为 `false`，彻底消灭推断歧义，以配置为绝对准绳。

### 3. `ColumnLimit: 120`（放宽行宽上限）
* **动机**：
  * Google 原始规范采用 80 字符，源于 20 世纪电传打字机和 4:3 显示器终端；
  * 现代泛型 C++、模板特化、复杂命名空间和 lambda 表达式嵌套较深，80 字符会导致代码被极度碎片化地折行，严重伤害代码可读性；
  * 120 字符是目前现代宽屏显示器与 GitHub/GitLab 网页端 Review 双栏对比的最优平衡点。

---

## 二、 完整推荐配置速查

```yaml
---
BasedOnStyle: Google

# 指针与列宽定制
DerivePointerAlignment: false
PointerAlignment: Right
ColumnLimit: 120

# 基础缩进与对齐
IndentWidth: 2
TabWidth: 2
UseTab: Never
Standard: Latest
AccessModifierOffset: -1
NamespaceIndentation: None

# 头文件与命名空间
SortIncludes: CaseSensitive
IncludeBlocks: Regroup
FixNamespaceComments: true
ReflowComments: true

# 括号与短函数处理
AllowShortFunctionsOnASingleLine: Inline
AllowShortBlocksOnASingleLine: Never
AllowShortCaseLabelsOnASingleLine: false
KeepEmptyLinesAtTheStartOfBlocks: false
EmptyLineBeforeAccessModifier: LogicalKey
...
```
