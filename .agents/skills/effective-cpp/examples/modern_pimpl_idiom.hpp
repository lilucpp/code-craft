#pragma once

#include <memory>
#include <string>

/**
 * @brief 现代 C++ 规范 Pimpl 惯用法示例 (符合 Effective Modern C++ 条款 22)
 *
 * 核心关键：
 * 1. 使用 std::unique_ptr<Impl> 管理具体实现指针；
 * 2. 头文件中前置声明 struct Impl，严禁在头文件中内联定义析构函数与移动操作！
 * 3. 析构函数与移动构造/赋值必须在 .cpp 中定义（此时 Impl 是完整类型）。
 */
class ModernWidget {
public:
    ModernWidget();
    explicit ModernWidget(std::string name);

    // 析构函数：必须在头文件中仅声明，定义留在实现文件！
    ~ModernWidget();

    // 移动操作：由于声明了析构函数，编译器不会自动生成移动操作，必须手动声明
    ModernWidget(ModernWidget&& rhs) noexcept;
    ModernWidget& operator=(ModernWidget&& rhs) noexcept;

    // 拷贝操作：若需要深拷贝，需手动在实现文件中按需实现
    ModernWidget(const ModernWidget& rhs);
    ModernWidget& operator=(const ModernWidget& rhs);

    void doWork();
    [[nodiscard]] const std::string& getName() const noexcept;

private:
    struct Impl;                  // 前置声明 (Incomplete Type)
    std::unique_ptr<Impl> pImpl_; // 独占指针管理实现细节
};
