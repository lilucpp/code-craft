# 禁用函数红线与安全加固速查表

本手册整合 **Microsoft SDL Banned Functions**、**SEI CERT C/C++**、**《Secure Coding in C and C++》** 与 **OpenSSF Compiler Hardening Guide**，提供编码阶段与代码审计阶段的“一秒查表”依据。

---

## 1. 高危/禁用 C API 与安全替代对照表

| 类别 | 禁用/高危函数 | 核心漏洞机理 (CWE / CERT) | C 安全替代方案 | 现代 C++ 安全替代方案 |
| :--- | :--- | :--- | :--- | :--- |
| **输入读取** | `gets()` | 无法指定缓冲区上限，必发栈溢出 (CWE-242 / MSC24-C) | `fgets(buf, sizeof(buf), stdin)`（检查 `\n` 防截断）或 POSIX `getline()` | `std::getline(std::cin, str)` |
| **字符串拷贝** | `strcpy()`, `wcscpy()` | 不检查目标长度，越界写 (CWE-120 / STR31-C) | `snprintf(dst, sizeof(dst), "%s", src)`（检查返回值 `< sizeof(dst)`）或 `strlcpy` | `std::string` 赋值、`std::string_view` 只读引用 |
| **字符串拼接** | `strcat()`, `wcscat()` | 不检查剩余容量，越界写 (CWE-120 / STR31-C) | `snprintf` 统一格式化，或显式计算剩余空间后 `memcpy` | `std::string::append()` / `+=` |
| **易错截断函数** | `strncpy()`, `strncat()` | 长度满时不自动写 `\0`；`strncat` 的 `n` 易被误传为总容量而非剩余量 (STR32-C) | 先校验 `src_len < dst_cap`，再用 `memcpy(dst, src, src_len); dst[src_len] = '\0';` | `std::string` / `std::string_view::substr()` |
| **格式化写入** | `sprintf()`, `vsprintf()` | 无输出长度上限，必发溢出 (CWE-120) | `snprintf(dst, sizeof(dst), fmt, ...)`（校验 `ret >= 0 && (size_t)ret < sizeof(dst)`） | `std::format` (C++20) / `fmt::format` |
| **格式化读取** | `scanf("%s")`, `sscanf()`, `fscanf()` | `%s` 无宽度限制等价于 `gets` (CWE-120) | 指定宽度如 `scanf("%63s", buf)`，或 `fgets` + `strtol`/`strtod` 解析 | `std::getline` + `std::from_chars` (C++17) |
| **进程执行** | `system()`, `popen()` | 经 `/bin/sh -c` 解析，极易触发命令注入 (CWE-78 / ENV33-C) | `execve()` / `posix_spawn()`（参数数组直传，不经过 Shell） | 同左，配合 RAII 封装子进程句柄 |
| **临时文件** | `mktemp()`, `tmpnam()`, `tempnam()` | 文件名生成与打开非原子，存在 TOCTOU 劫持 (CWE-377 / FIO21-C) | `mkstemp(tmpl)`（原子创建且权限 `0600`）或 `O_TMPFILE` | 同左，使用 `mkstemp` 返还的 `fd` |
| **线程不安全** | `strtok()`, `asctime()`, `ctime()`, `gmtime()`, `localtime()` | 使用全局静态缓冲区，多线程数据竞态 (CON33-C) | `strtok_r()`, `gmtime_r()`, `localtime_r()` | `std::string_view::find` 手动切分；`<chrono>` |
| **随机数** | `rand()`, `srand()` | 周期短、可预测，严禁用于密钥/Token/Salt (CWE-338 / MSC30-C) | Linux `getrandom()` / `/dev/urandom`；Windows `BCryptGenRandom` | 加密场景用 OS CSPRNG / OpenSSL；非加密模拟用 `std::mt19937` + `std::random_device` |
| **敏感内存擦除** | `memset(secret, 0, n)` | 若缓冲区随后被释放，编译器 DSE 优化会直接删掉 `memset` (MSC06-C) | `explicit_bzero(secret, n)` / `memset_s()` / `SecureZeroMemory()` | 封装自定义 Allocator 或析构调用 `explicit_bzero` |
| **栈动态分配** | `alloca()`, `_alloca()` | 分配过大时直接越过栈护栏页（Stack Guard Page）导致栈碰撞 (CWE-770) | 改用堆分配 `malloc` + 上限校验，或固定上限小数组 | `std::vector<T>` / 小对象栈缓冲策略 |

---

## 2. 受限但合法的底层函数操作规范

以下函数虽未被全面禁用，但必须在调用点紧邻处满足硬性前置条件：

1. **`memcpy` / `memmove`**：
   - 必须校验 `dst != nullptr && src != nullptr`（即使 `n == 0` 在 C 中传空指针也是未定义行为 UB）；
   - 必须校验 `n <= dst_remaining_bytes` 且 `n <= src_remaining_bytes`；
   - 内存区域可能重叠时禁止用 `memcpy`，必须用 `memmove`（或 C++20 `std::copy`）。
2. **`atoi` / `atol` / `atof`**：
   - 遇到非法输入或溢出时行为未定义且无法报错（`MSC24-C` / `ERR34-C`），**生产代码禁止使用**；
   - C 改用 `strtol`/`strtoul`（调用前清 `errno = 0`，检查 `endptr != str && *endptr == '\0'` 且 `errno != ERANGE`）；C++17 优先用 `std::from_chars`。
3. **`realloc(ptr, new_size)`**：
   - 严禁直接写 `ptr = realloc(ptr, new_size);`（失败返回 `NULL` 会覆盖原指针造成内存泄漏）；
   - 必须用临时指针承接：`void* tmp = realloc(ptr, new_size); if (!tmp) { /* 保留/释放原 ptr */ }`。

---

## 3. OpenSSF 编译器与标准库安全加固基线

生产构建（Release / CI）推荐默认开启以下纵深防御参数：

### GCC / Clang 编译与链接加固
```makefile
# 1. 告警即错误（格式串、有符号转换、未初始化）
-Wall -Wextra -Werror=format-security -Werror=implicit-function-declaration -Wconversion

# 2. 栈保护与栈碰撞防御
-fstack-protector-strong -fstack-clash-protection

# 3. 局部变量零初始化（消除未初始化栈内存信息泄露/野指针，Clang 12+ / GCC 12+）
-ftrivial-auto-var-init=zero

# 4. glibc 缓冲区边界编译期/运行期增强检查（需配合 -O2；=3 要求 glibc 2.34+，旧版用 =2）
-O2 -D_FORTIFY_SOURCE=3

# 5. 二进制链接加固（ASLR + GOT 只读 + 不可执行栈）
-fPIE -pie -Wl,-z,relro,-z,now -Wl,-z,noexecstack

# 6. 控制流完整性（Clang 专用 CFI 防御虚表/函数指针劫持；GCC x86_64 对应 -fcf-protection）
-fsanitize=cfi -flto -fvisibility=hidden
```

### C++ 标准库运行期硬化（Hardened Stdlib）
- **libstdc++ (GCC)**：`-D_GLIBCXX_ASSERTIONS`（为 `std::vector::operator[]`、`std::span`、`std::string_view` 开启廉价运行期越界断言，越界立即 `abort`）。
- **libc++ (Clang 18+)**：`-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE`（全面激活标准库边界与前置条件检查）。

### 测试与 CI 动态审计门禁
```bash
# 内存破坏与未定义行为联合检测（开发/CI 必跑）
-fsanitize=address,undefined -fno-omit-frame-pointer -g
```
