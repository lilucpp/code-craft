# 资源管理、RAII 与智能指针深度指南

> 整合自《Effective C++》(条款 13-17) 与《Effective Modern C++》(条款 18-22)。

---

## 1. 核心所有权哲学：从裸指针到明确语义

在 C++ 中，裸指针（Raw Pointer）最大的弊端在于**所有权语义模糊**：
- 无法从类型判断调用者是否负责销毁（是所有者还是观察者？）；
- 无法得知应使用 `delete`、`delete[]` 还是调用专门的释放函数；
- 发生异常时无法自动执行清理，极易产生内存泄漏。

**RAII（资源获取即初始化）铁律**：资源的生命周期必须绑定在对象的生命周期上，由析构函数自动释放底层资源。

---

## 2. 现代智能指针选择矩阵

| 智能指针类型 | 所有权语义 | 内存开销 | 典型适用场景 | 替代遗留做法 |
| :--- | :--- | :--- | :--- | :--- |
| **`std::unique_ptr<T>`** | **排他独占所有权** | 0 额外开销（默认删除器与裸指针完全同大） | 工厂函数返回值、类内部私有组件、Pimpl 实现 | 废弃并替代 `std::auto_ptr`、裸指针 `new/delete` |
| **`std::shared_ptr<T>`** | **共享所有权** | 2倍裸指针大小（对象指针 + 控制块指针） | 多线程共享缓存、复杂图状/网状拓扑节点 | 手动引用计数基类、全局裸指针注册表 |
| **`std::weak_ptr<T>`** | **弱引用观察者** | 2倍裸指针大小（与对应 `shared_ptr` 一致） | 观察者列表、防 `shared_ptr` 环状死锁、带失效探测的缓存 | 容易悬空的裸指针观察者 |

---

## 3. `make` 系列函数的双刃剑与最佳实践 (条款 21)

C++11 引入 `std::make_shared`，C++14 补齐 `std::make_unique`。

### 3.1 优先使用 `make` 系列的绝对优势
1. **彻底杜绝异常安全隐患**：
   ```cpp
   // ❌ 极度危险：编译器重排参数求值顺序时，若 priority() 抛出异常，new Widget 发生内存泄漏
   processWidget(std::shared_ptr<Widget>(new Widget), priority());

   // ✅ 绝对安全：make_shared 内部无中间裸指针悬空缝隙
   processWidget(std::make_shared<Widget>(), priority());
   ```
2. **性能与内存分配优化**：
   `std::shared_ptr<Widget>(new Widget)` 会触发 **2 次独立的堆内存分配**（一次分配 `Widget`，一次分配控制块 `Control Block`）；
   `std::make_shared<Widget>()` 会执行 **1 次合并内存分配**，将 `Widget` 和控制块打包在连续内存块中，不仅减半内存分配器开销，还极大提升 CPU Cache 命中率。

### 3.2 不适合使用 `make` 系列的 2 种边缘情况
1. **需要自定义删除器（Custom Deleter）时**：`make_shared` / `make_unique` 不支持传入删除器，必须使用带删除器的构造函数；
2. **超大对象且有长生命周期 `weak_ptr` 存在时**：由于 `make_shared` 将对象与控制块合并分配，当所有 `shared_ptr` 析构后，对象的析构函数会被调用，但**其占用的整块内存必须等待所有 `weak_ptr` 也全部析构后才能释放**。如果对象极大而 `weak_ptr` 存活极久，应改用 `new` 独立分配。

---

## 4. Pimpl 惯用法与现代智能指针的深度陷阱 (条款 22)

在大型项目中，Pimpl（Pointer to Implementation）惯用法用于隐藏类内部细节并切断头文件编译依赖（降低编译耗时）。

### ❌ 致命陷阱：未完成类型（Incomplete Type）编译报错
```cpp
// Widget.h
#include <memory>
class Widget {
public:
    Widget();
    // ❌ 隐式生成的析构函数 Widget::~Widget() 会内联在头文件中！
    // 此时编译器尝试在此处销毁 std::unique_ptr<Impl>，调用 sizeof(Impl)
    // 但 Impl 只是前置声明，属于 Incomplete Type，触发 static_assert 失败！
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
```

### ✅ 规范工业级解法：在 `.cpp` 中定义析构函数与移动操作
- **头文件中**：仅前置声明 `struct Impl;`，并显式**声明（不内联定义）**析构函数与移动操作：
  ```cpp
  // Widget.h
  class Widget {
  public:
      Widget();
      ~Widget(); // 仅声明，禁止 = default 放在头文件
      Widget(Widget&&) noexcept;
      Widget& operator=(Widget&&) noexcept;
  private:
      struct Impl;
      std::unique_ptr<Impl> pImpl;
  };
  ```
- **实现文件中**：在定义完整 `struct Impl` 之后，显式定义或 `= default`：
  ```cpp
  // Widget.cpp
  #include "Widget.h"
  struct Widget::Impl { ... }; // 完整类型定义

  Widget::Widget() : pImpl(std::make_unique<Impl>()) {}
  Widget::~Widget() = default; // 此时 Impl 为 Complete Type，安全生成析构代码！
  Widget::Widget(Widget&&) noexcept = default;
  Widget& Widget::operator=(Widget&&) noexcept = default;
  ```
