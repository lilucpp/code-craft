# C++ 编译数据库 (Compilation Database) 与 Clangd 深度指南

编译数据库 (`compile_commands.json`) 是 LLVM 工具链（Clangd、Clang-Tidy、Clang-Format、Include-What-You-Use 等）获取真实编译上下文的基石规范。

---

## 1. 为什么 C++ 必须依赖编译数据库？

不同于 Python、Go 或 Java 具备严格标准化的包管理与自包含语法体系，C++ 源文件本身并不包含完整的编译上下文：
1. **宏定义决定语法树**：同一个 `.h` 文件在包含不同头文件或 `-D` 预处理宏时，AST 语法树可能完全不同；
2. **包含路径（Header Search Paths）**：系统头文件、三方库路径与内部子模块路径均通过 `-I`、`-isystem` 传递给编译器；
3. **语言标准与特性开关**：`-std=c++20`、`-fno-exceptions`、`-fno-rtti` 等影响符号推导与类型检查。

脱离 `compile_commands.json`，任何 AI Agent 或语言服务器只能退化为“盲猜匹配”，无法穿透宏展开与模板实例化。

---

## 2. 主流构建系统的导出机制

### 2.1 CMake 项目（现代工业标准）
在 CMake 中导出编译数据库最为便捷：
```bash
# 方式 A：命令行一次性指定
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# 方式 B：在根目录 CMakeLists.txt 顶部硬编码约定
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```
> **最佳实践**：在项目根目录创建软链接，避免不同工具扫描子路径不一致：
> ```bash
> ln -sf build/compile_commands.json compile_commands.json
> ```

### 2.2 Makefile 与遗留工程（Bear 截获）
对于使用纯 Makefile、Autotools 或自定义 Shell 脚本编译的项目，推荐使用 `bear` 动态截获编译调用：
```bash
# 清理后使用 bear 包装整个编译过程
make clean
bear -- make -j$(nproc)
```
Bear 会通过 `LD_PRELOAD` 劫持 `execve` 调用，自动记录每一个编译器的具体入参与当前目录。

### 2.3 Ninja 构建与 CMake Presets
若项目采用 `CMakePresets.json`：
```bash
cmake --preset debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```
通常生成物位于 `build/debug/compile_commands.json`。

---

## 3. Clangd 核心运行参数与性能微调

在大型 monorepo（百万行代码级）中，Clangd 的索引开销较高。推荐配合以下参数或配置文件使用：

### 3.1 关键命令行参数
* `--compile-commands-dir=<path>`：显式指定编译数据库所在目录；
* `--background-index`：开启后台索引构建（生成 `.cache/clangd/index/` 缓存，加速后续跨文件符号查询）；
* `-j=4`：限制后台索引构建的线程数，避免满载占用 CPU；
* `--header-insertion=iwyu`：自动补全头文件时遵循 IWYU（Include What You Use）准则；
* `--completion-style=detailed`：提供详细的代码补全信息（包含参数类型与返回签名）。

### 3.2 项目级 `.clangd` 配置文件
可在项目根目录放置 `.clangd` 文件覆盖编译选项：
```yaml
CompileFlags:
  Add: [-Wno-unknown-warning-option]
  Remove: [-Werror*]

Index:
  Background: Build
```
