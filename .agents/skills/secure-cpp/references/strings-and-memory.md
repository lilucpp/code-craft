# 字符串安全与动态内存防御

提炼自《Secure Coding in C and C++ (2nd Ed)》第 2 章（Strings）与第 4 章（Dynamic Memory Management）。

---

## 1. 字符串常见致命陷阱与防御

### 1.1 Off-by-One 与缺失 `\0` 终结符（CWE-193 / STR32-C）
`strncpy` 与 `strncat` 是最常制造隐蔽漏洞的“伪安全函数”：
- `strncpy(dst, src, n)` 当 `strlen(src) >= n` 时**不会写入 `\0`**，后续 `strlen(dst)` 或 `printf("%s", dst)` 将越界读取栈/堆内存；
- `strncat(dst, src, n)` 的 `n` 是**最多追加的字符数**（还需额外预留 1 字节 `\0`），传入 `sizeof(dst)` 会直接缓冲区溢出。

```c
// Noncompliant: 经典 strncat 长度误算，满长时溢出 1 字节
char buf[16];
strncpy(buf, input1, sizeof(buf));       // 若 input1 >= 16，buf 无 '\0'！
strncat(buf, input2, sizeof(buf) - 1);   // 仍可能越界

// Compliant (C): 使用 snprintf 并显式检测截断
int n = snprintf(buf, sizeof(buf), "%s%s", input1, input2);
if (n < 0 || (size_t)n >= sizeof(buf)) {
    // 明确处理截断或返回错误，绝不静默继续
}
```

### 1.2 现代 C++ 字符串与视图边界陷阱
- **`std::string` 优势**：自动扩容管理 `\0`，消除了绝大多数 C 式拷贝溢出；但 `str.data()` / `str.c_str()` 传给 C API 时仍需注意内部是否允许嵌入 `\0`（NUL 字节截断攻击）。
- **`std::string_view` 悬垂引用陷阱**：`string_view` 不拥有内存，严禁返回指向临时 `std::string` 或已销毁缓冲区的 `string_view`；且 `string_view` **不保证以 `\0` 结尾**，绝不能直接传给要求 C 字符串的 `printf("%s", sv.data())` 或 `open(sv.data(), ...)`。
- **`std::span<T>` 边界视图**：在接口层优先用 `std::span<const std::byte>` / `std::span<char>` 替代 `(ptr, len)` 裸对，结合 `-D_GLIBCXX_ASSERTIONS` 实现越界快速失败。

---

## 2. 动态内存管理安全（CWE-416 / CWE-415 / CWE-401）

原著第 4 章深度剖析了 `dlmalloc` 与 `RtlHeap` 等堆分配器元数据结构如何被**堆溢出、Double-Free 与 Use-After-Free (UAF)** 转化为任意地址写。

### 2.1 四大堆内存安全铁律

| 漏洞类型 | 触发模式 | 硬性防御准则 |
| :--- | :--- | :--- |
| **Double-Free (CWE-415)** | 多条错误分支重复 `free(p)`，或拷贝构造函数未深拷贝/禁用拷贝导致重复 `delete` | 1. 遵循**单点清理模式**（`goto cleanup;`）或 C++ RAII (`std::unique_ptr`)；<br>2. C 代码 `free(p); p = NULL;`（`free(NULL)` 是安全的无操作）。 |
| **Use-After-Free (CWE-416)** | 释放后未置空，悬垂指针随后被解引用或虚函数调用（直接导致任意代码执行） | 1. 释放后指针立即置 `NULL`/`nullptr`（若仍有别名存在则说明所有权设计错误，必须重构为智能指针或清晰所有权）；<br>2. 禁用返还局部变量地址或已被 `std::vector` 扩容失效的迭代器/指针。 |
| **零字节分配陷阱 (MEM04-C)** | `malloc(0)` 返回值由实现定义（可能为 `NULL` 或不可解引用的非空最小块） | 分配前显式校验 `size > 0 && size <= MAX_ALLOC_LIMIT`；绝不对外部输入 `size == 0` 的分配结果解引用。 |
| **未初始化堆/栈内存 (CWE-908)** | `malloc` 返回旧堆块残留数据，导致指针泄露（绕过 ASLR）或密钥泄露 | 需要零初始化时使用 `calloc(count, size)`（自带乘法溢出检查）或显式初始化结构体；编译期开启 `-ftrivial-auto-var-init=zero`。 |

### 2.2 敏感数据安全擦除（MEM03-C / MSC06-C）
编译器在开启 `-O2` 时会进行**死存储消除（Dead Store Elimination, DSE）**：如果一块内存在 `memset(pwd, 0, len)` 后紧接着被 `free(pwd)` 或离开栈帧，编译器会认为 `memset` 无可见副作用并将其**整行删除**，导致口令/私钥残留在物理内存或 Core Dump 中。

```cpp
// Noncompliant: 编译器优化后会直接删除 memset！
void handle_secret(const char* input, size_t len) {
    char* key = static_cast<char*>(malloc(len));
    // ... 使用 key ...
    memset(key, 0, len); // 极可能被 -O2 优化消除！
    free(key);
}

// Compliant: 使用不被优化消除的安全擦除原语
#include <cstddef>
#if defined(_WIN32)
#  include <windows.h>
#elif defined(__GLIBC__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#  include <strings.h>
#  include <string.h>
#endif

void secure_zero(void* ptr, std::size_t len) {
    if (!ptr || len == 0) return;
#if defined(_WIN32)
    SecureZeroMemory(ptr, len);
#elif defined(__GLIBC__) || defined(__FreeBSD__) || defined(__OpenBSD__)
    explicit_bzero(ptr, len);
#else
    volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
    while (len--) *p++ = 0;
#endif
}
```
