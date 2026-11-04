#include "modern_pimpl_idiom.hpp"
#include <iostream>
#include <vector>

// 完整类型定义放在 .cpp 内部，对外部调用者完全隐藏（消除编译依赖）
struct ModernWidget::Impl {
    std::string name;
    std::vector<int> internalData;

    explicit Impl(std::string n) : name(std::move(n)), internalData{1, 2, 3, 4, 5} {}
};

// 构造函数
ModernWidget::ModernWidget() : pImpl_(std::make_unique<Impl>("DefaultWidget")) {}

ModernWidget::ModernWidget(std::string name) : pImpl_(std::make_unique<Impl>(std::move(name))) {}

// 析构函数：此时 Impl 已经具备完整类型定义，std::default_delete<Impl> 可以安全调用 sizeof(Impl)！
ModernWidget::~ModernWidget() = default;

// 移动操作：在具备完整类型的上下文中显式 default
ModernWidget::ModernWidget(ModernWidget&& rhs) noexcept = default;
ModernWidget& ModernWidget::operator=(ModernWidget&& rhs) noexcept = default;

// 拷贝操作：手动实现深拷贝
ModernWidget::ModernWidget(const ModernWidget& rhs) : pImpl_(nullptr) {
    if (rhs.pImpl_) {
        pImpl_ = std::make_unique<Impl>(*rhs.pImpl_);
    }
}

ModernWidget& ModernWidget::operator=(const ModernWidget& rhs) {
    if (this != &rhs) {
        if (rhs.pImpl_) {
            pImpl_ = std::make_unique<Impl>(*rhs.pImpl_);
        } else {
            pImpl_.reset();
        }
    }
    return *this;
}

void ModernWidget::doWork() {
    if (pImpl_) {
        pImpl_->internalData.push_back(42);
    }
}

const std::string& ModernWidget::getName() const noexcept {
    static const std::string empty;
    return pImpl_ ? pImpl_->name : empty;
}
