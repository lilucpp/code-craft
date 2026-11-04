#pragma once

#include <thread>
#include <utility>

/**
 * @brief RAII 线程结合守护者 (符合 Effective Modern C++ 条款 37)
 *
 * 核心法则：
 * std::thread 若在析构时处于可结合 (joinable) 状态，C++ 运行时将直接调用 std::terminate() 杀死进程！
 * 本守护类确保线程在超出作用域或因异常退出时，自动执行 join() 或 detach()，杜绝程序崩溃。
 */
class SafeThreadGuard {
public:
    enum class DtorAction {
        Join,
        Detach
    };

    explicit SafeThreadGuard(std::thread t, DtorAction action = DtorAction::Join) noexcept
        : thread_(std::move(t)), action_(action) {}

    ~SafeThreadGuard() {
        if (thread_.joinable()) {
            if (action_ == DtorAction::Join) {
                thread_.join();
            } else {
                thread_.detach();
            }
        }
    }

    // 禁用拷贝语义（thread 是 move-only 类型）
    SafeThreadGuard(const SafeThreadGuard&) = delete;
    SafeThreadGuard& operator=(const SafeThreadGuard&) = delete;

    // 允许移动语义
    SafeThreadGuard(SafeThreadGuard&& rhs) noexcept
        : thread_(std::move(rhs.thread_)), action_(rhs.action_) {}

    SafeThreadGuard& operator=(SafeThreadGuard&& rhs) noexcept {
        if (this != &rhs) {
            if (thread_.joinable()) {
                if (action_ == DtorAction::Join) {
                    thread_.join();
                } else {
                    thread_.detach();
                }
            }
            thread_ = std::move(rhs.thread_);
            action_ = rhs.action_;
        }
        return *this;
    }

    [[nodiscard]] std::thread& get() noexcept { return thread_; }
    [[nodiscard]] const std::thread& get() const noexcept { return thread_; }

private:
    std::thread thread_;
    DtorAction action_;
};
