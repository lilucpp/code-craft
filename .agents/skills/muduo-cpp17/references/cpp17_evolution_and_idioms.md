# C++17 现代化演进与工程习惯

提炼自《Linux 多线程服务端编程》第 10~12 章及 `muduo` 仓库（`master` 与 `origin/cpp17` 分支）演进实践，用于指导现代 C++17 项目落地。

---

## 1. Muduo 传统写法 $\rightarrow$ C++17 标准替代对照表

| 场景 | Muduo 原版做法（C++11/Boost 过渡期） | C++17 推荐写法 | 工程收益 |
| :--- | :--- | :--- | :--- |
| **只读字符串视图** | 手写 `muduo::StringPiece` | `std::string_view`（需向 C API 传 `\0` 结尾串时保留轻量 `StringArg`） | 零自定义轮子，与标准库生态无缝衔接 |
| **动态类型上下文** | `boost::any`（`EventLoop`/`TcpConnection`） | `std::any` | 彻底移除 Boost 依赖 |
| **可能缺失的值** | 魔数或 `Timestamp::invalid()` | `std::optional<T>` | 类型系统显式表达“无值”状态 |
| **异步闭包绑定** | `std::bind` + `_1`/`_2` + 重载成员指针强转 | 初始化捕获 Lambda：`[self = shared_from_this(), buf = std::move(data)]` | 支持只移动捕获、无重载歧义、更易内联 |
| **线程局部存储** | GCC `__thread` | `thread_local` | 标准可移植，支持非 POD 构造/析构 |
| **线程安全单例** | `pthread_once` + 手写 `Singleton<T>` | 函数内局部 `static T instance;`（Magic Statics） | C++11/17 标准保证线程安全，极简无锁 |
| **互斥锁守卫** | `MutexLockGuard`（单锁） | `std::scoped_lock`（支持单锁与多锁死锁防御） | 标准库支持变参原子加锁 |
| **类型/基类检查** | `if (false) { implicit_cast<...>(0); }` | `static_assert(std::is_base_of_v<...>)` / `std::is_convertible_v` | 编译期直接断言，无死代码技巧 |

> **注**：仅当项目在 CI 中明确启用 Clang `-Wthread-safety` 静态并发检查时，才保留对 `std::mutex` 的薄封装（如 `CAPABILITY("mutex")` / `SCOPED_CAPABILITY`）以承载 `GUARDED_BY(mu)` 与 `REQUIRES(mu)` 注解。

---

## 2. 编译防火墙、ABI 与链接期设计

1. **前置声明 + `std::unique_ptr` 防火墙**：
   - 公开头文件中若成员仅为内部实现类（如 `Poller`、`TimerQueue`、`Impl`），只做前向声明并用 `std::unique_ptr<Impl>` 持有；
   - **必须在头文件中声明析构函数 `~Foo();`，并在 `.cc` 中定义**（即使为 `= default`），否则编译器在调用方生成内联析构时会因看不到 `Impl` 完整定义而编译失败。
2. **头文件自给自足**：
   - 每个 `.cc` 文件第一条 `#include` 必须是其对应的 `.h`，用编译器自动验证头文件不依赖外部隐式包含顺序；
   - 公开头文件严禁包含内部实现头文件。
3. **动态库 ABI 意识（若对外分发 `.so`）**：
   - 虚函数按 vtable 偏移绑定，增删或重排虚函数会静默破坏二进制兼容性；对外长期稳定的动态库接口应采用 **不含虚函数的 `pimpl` 具体类** 或 **纯 C ABI + opaque pointer**。
4. **链接期系统调用注入（取代重度 Mock 接口）**：
   - 像 `SocketsOps` 一样把底层系统调用包装在独立 `namespace` 自由函数中，生产链接真实实现，单元测试拆出 `main` 并链接 Mock `.cc`，既无虚函数运行期开销，也不会误拦截第三方库（如 SQLite）的同名系统调用。

---

## 3. 编译期防线与可维护代码格式

### 3.1 推荐编译器门禁
```cmake
-Wall -Wextra -Werror -Wconversion -Wshadow -Wold-style-cast -Woverloaded-virtual
```
- 配合 `[[nodiscard]]` 标记关键返回值（如资源句柄、错误状态、锁守卫），让忽略返回值直接编译失败。

### 3.2 对 `diff`、`git blame`、`grep` 友好的排版原则（第 12 章）
- **`namespace` 不缩进**：让类与函数实现顶格写，确保 `diff -p` 在 hunk 头部准确显示 `ClassName::Method`（与 `.clang-format` 的 `NamespaceIndentation: None` 一致）。
- **多行注释也用 `//`**：在仅 3 行上下文的 `diff` 中清晰可辨，且不怕内部嵌套 `/* */`。
- **一行只定义一个变量**：含局部声明、初始化列表与成员列表，保证 `git blame` 精确到行。
- **禁止 C 风格强转**：强制 `static_cast`/`reinterpret_cast`，确保所有类型转换都能被 `grep` 精确检索。
- **慎用匿名 `namespace` 与运算符重载**：内部符号优先放在具名 `detail` 命名空间以方便 `gdb`/profiler 定位；非数值抽象尽量用具名函数替代运算符重载，提升重构时的全局搜索精度。
