# 整型安全与指针欺骗防御

提炼自《Secure Coding in C and C++ (2nd Ed)》第 3 章（Pointer Subterfuge）与第 5 章（Integer Security）。

---

## 1. 整型安全：三大漏洞根源（CWE-190 / CWE-195 / CWE-197）

C/C++ 整数漏洞往往是缓冲区溢出与堆破坏的**前置跳板**：

1. **有符号整数溢出（Signed Integer Overflow, UB）**：在 C/C++ 中是**未定义行为**。编译器可假设有符号溢出永远不会发生，从而直接**优化删除**你在溢出后写的防御检查（如 `if (a + 100 < a)` 会被编译器优化为恒假 `false`！）。
2. **有符号/无符号隐式转换（Sign Conversion, INT02-C）**：当 `int len`（负数，如 `-1`）与 `size_t buf_size` 比较，或直接传给 `memcpy(dst, src, len)` 时，`-1` 被隐式提升为 `SIZE_MAX`，直接绕过上界检查并引发巨量越界拷贝。
3. **整型截断（Truncation）**：将 `size_t` / `uint64_t` 赋值给 `uint32_t` / `uint16_t`（例如计算 `total = header + payload` 后截断成 16 位去 `malloc`，分配极小内存却按原 64 位长度写入）。

### 1.1 黄金铁律：先校验，后运算（Precondition Testing）
**严禁在溢出发生之后再检验结果**，必须在运算前校验，或使用编译器溢出内置原语 / C23 `<stdckdint.h>`：

```cpp
#include <cstdint>
#include <limits>
#include <cstdlib>

// Noncompliant: 1. count*size 可能溢出回绕成极小值; 2. 负 signed len 绕过校验
void* alloc_records_bad(int count, size_t elem_size) {
    if (count > 10000) return nullptr; // 若 count == -1，绕过检查！
    return malloc(count * elem_size);  // 负值转超大 size_t 或乘法溢出
}

// Compliant: 无符号类型 + 前置溢出检查 + 业务上限防护（防 OOM/DoS, CWE-770）
constexpr size_t MAX_ALLOC_BYTES = 256ULL * 1024 * 1024; // 按业务边界设定

void* alloc_records_good(size_t count, size_t elem_size) {
    if (count == 0 || elem_size == 0) return nullptr;
    size_t total_bytes = 0;
#if defined(__GNUC__) || defined(__clang__)
    if (__builtin_mul_overflow(count, elem_size, &total_bytes)) {
        return nullptr; // 检测到乘法溢出
    }
#else
    if (count > std::numeric_limits<size_t>::max() / elem_size) {
        return nullptr;
    }
    total_bytes = count * elem_size;
#endif
    if (total_bytes > MAX_ALLOC_BYTES) return nullptr;
    return calloc(1, total_bytes);
}
```

### 1.2 CERT 防御策略选型（原著 5.6 节）
- **明确无符号 vs 有符号边界**：表示对象大小、数组索引、位掩码一律使用 `size_t` / 无符号固定宽度类型；外部输入的 `int` 必须先验证 `>= 0` 再强转为 `size_t`。
- **禁用依赖回绕的检查**：不要写 `if (a + b < a)`（若 `a, b` 为有符号数是 UB）；改写为 `if (a > MAX - b)`。
- **高安全算术封装**：在涉及大量长度/偏移计算的解析器中，使用 `SafeInt`（微软开源 C++ 类）或自定义 Checked Arithmetic 封装，配合 **UBSan (`-fsanitize=signed-integer-overflow,unsigned-integer-overflow`)** 测试。

---

## 2. 指针欺骗防御（Pointer Subterfuge, 第 3 章）

当攻击者无法直接覆盖栈返回地址（如开启了 Stack Canary）时，会转向覆盖**数据指针、函数指针或 C++ 虚表指针（vptr）**来劫持控制流：

1. **结构体字段布局隔离**：
   - **高危布局**：在同一 `struct` 中将固定数组缓冲区放在函数指针之前（`struct S { char buf[64]; void (*cb)(); };`），一旦 `buf` 溢出即可精准覆写 `cb`。
   - **防御**：将函数指针/状态标志置于缓冲区之前，或直接改用边界安全的 `std::array` / `std::vector` + 索引检查。
2. **C++ 虚表（vtable）安全与 CFI**：
   - 对象内部的 `vptr` 通常位于对象内存最前端，若相邻前一个堆块溢出，可篡改当前对象的 `vptr` 指向伪造虚表；
   - **防御**：消除前序堆块溢出；生产环境对多态重度模块开启 **Clang CFI (`-fsanitize=cfi`)**，在每次虚函数/函数指针间接调用前校验目标地址合法性。
3. **函数指针与索引表边界**：
   - 若通过外部输入的 `cmd_id` 查函数指针表 `handlers[cmd_id]()`，必须执行严格的**无符号范围检查**：`if (cmd_id >= std::size(handlers)) return;`（若 `cmd_id` 为有符号 `int` 且未判 `< 0`，负索引会向前越界读取任意指针并跳转执行！）。
4. **指针加减与对齐**：
   - 严禁对不同对象或数组边界外的指针做减法或大小比较（UB，CWE-469）；指针算术仅限同一数组内 `[begin, end]`（`end` 仅可计算比较，不可解引用）。
