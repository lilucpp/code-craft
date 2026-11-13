# 格式化输出、文件 I/O 与 TOCTOU 竞争

提炼自《Secure Coding in C and C++ (2nd Ed)》第 6 章（Formatted Output）、第 7 章（Concurrency）与第 8 章（File I/O）。

---

## 1. 格式化字符串漏洞防御（CWE-134 / FIO30-C）

### 1.1 漏洞原理
当外部可控字符串被直接作为 `printf`、`fprintf`、`snprintf`、`syslog` 等可变参数函数的**第一个格式串参数**时：
- 攻击者输入 `%x.%x.%x.%s` 可任意泄漏栈/内存敏感信息（包括 Stack Canary 与 ASLR 基址）；
- 攻击者输入 `%n`（将已输出字节数写入栈上指针指向的地址）可直接实现**任意地址内存写**，劫持程序控制流。

### 1.2 防御铁律
```cpp
// Noncompliant: 外部输入直接充当 format string
void log_msg(const char* user_msg) {
    printf(user_msg);            // 致命！%x 泄密，%n 任意写
    syslog(LOG_INFO, user_msg);  // 同样致命！
}

// Compliant: 格式串永远是编译期常量字面量
void log_msg_safe(const char* user_msg) {
    printf("%s", user_msg);
    syslog(LOG_INFO, "%s", user_msg);
}
```
- **现代 C++ 替代**：优先使用类型安全的 `<iostream>` 或 C++20 `std::format`（编译期校验格式串字面量，且不支持 `%n` 写内存；注意严禁将外部输入传给运行期 `std::vformat`）；
- **编译期硬性拦截**：强制开启 `-Werror=format-security`（编译器直接拒绝非常量格式串调用）。

---

## 2. 文件 I/O 安全与 TOCTOU 竞争消除（CWE-367 / FIO45-C）

### 2.1 TOCTOU（Time of Check, Time of Use）竞争漏洞
“先检查路径属性，再按路径打开文件”是经典的竞态漏洞：攻击者在检查与打开的微秒级窗口内，将目标路径替换为指向 `/etc/shadow` 等敏感文件的**符号链接（Symlink）**或硬链接。

```c
// Noncompliant: 经典 TOCTOU 竞态窗口（access/stat 与 open 之间可被掉包）
if (access(filepath, W_OK) == 0) {
    int fd = open(filepath, O_WRONLY);
    // ...
}

// Compliant (POSIX): 原子打开 + 拒绝符号链接跟随 + 基于 fd 校验
int fd = open(filepath, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
if (fd < 0) { /* 处理已存在、符号链接或权限错误 */ }

struct stat st;
if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) {
    close(fd);
    return -1; // 确保打开的是普通文件而非 FIFO/设备文件
}
```

### 2.2 文件系统安全操作清单
1. **基于目录句柄逐级打开（Eliminate Path Race）**：跨多级目录访问敏感文件时，使用 `openat(dirfd, name, O_NOFOLLOW | ...)` 替代拼接完整路径，彻底规避路径中间目录被替换。
2. **目录穿越防御（Path Traversal, CWE-22）**：
   - 不要仅靠字符串匹配 `strstr(path, "..")` 防御（易被编码、反斜杠或符号链接绕过）；
   - 先用规范化解析（如 `std::filesystem::weakly_canonical` 或 POSIX `realpath` 解析真实路径），再严格校验结果路径是否以允许的**根目录前缀**开头（注意末尾 `/` 边界，防止 `/var/app_secret` 匹配 `/var/app`）。
3. **临时文件安全创建**：严禁自己拼接 `/tmp/app_pid` 并打开；一律使用 `mkstemp("/tmp/app_XXXXXX")`（原子创建、独占、`0600` 权限）。
4. **最小权限原则**：仅在需要时临时提升权限，完成后立即永久或临时降权；新建文件/目录时注意 `umask` 与显式权限位（不创建全局可写 `0666`/`0777` 文件）。

---

## 3. 命令注入与信号处理安全

### 3.1 消除 Shell 命令注入（ENV33-C / CWE-78）
- **禁止调用 `system()` 和 `popen()`**：它们会拉起 `/bin/sh -c`，只要参数中混入 `;`、`|`、`&`、`$(...)`、反引号即触发任意命令执行。
- **合规做法**：使用 `fork()` + `execve()` 或 `posix_spawn()`，将可执行程序路径与每个参数作为独立字符串数组元素传递，彻底绕过 Shell 解析器，并清理不可信环境变量（`PATH`、`LD_PRELOAD`）。

### 3.2 信号处理函数安全（SIG30-C / CWE-479）
- 在 POSIX `signal`/`sigaction` 回调中，**严禁调用非异步信号安全（async-signal-safe）函数**（如 `malloc`、`free`、`printf`、`std::mutex::lock`、C++ 异常抛出），否则一旦主线程正持有堆锁或 IO 锁时触发信号，将导致**死锁或堆结构破坏**。
- **合规做法**：信号处理函数仅设置 `volatile sig_atomic_t`（或 lock-free `std::atomic<bool>`）标志位，或向 `self-pipe` / `eventfd` 写入 1 字节唤醒主循环安全处理。
