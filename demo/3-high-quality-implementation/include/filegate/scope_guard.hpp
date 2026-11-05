#pragma once

#include <utility>
#include <unistd.h>

namespace filegate::infra {

// ============================================================================
// RAII 资源看门狗与 Scope Guard (Scott Meyers Effective C++ Item 13/14)
// ============================================================================
template <typename F>
class ScopeGuard {
public:
    explicit ScopeGuard(F&& func) noexcept : func_(std::forward<F>(func)), dismissed_(false) {}
    ~ScopeGuard() noexcept {
        if (!dismissed_) {
            func_();
        }
    }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    ScopeGuard(ScopeGuard&& other) noexcept 
        : func_(std::move(other.func_)), dismissed_(other.dismissed_) {
        other.dismissed_ = true;
    }

    void Dismiss() noexcept { dismissed_ = true; }

private:
    F func_;
    bool dismissed_;
};

template <typename F>
ScopeGuard<F> MakeScopeGuard(F&& func) noexcept {
    return ScopeGuard<F>(std::forward<F>(func));
}

// POSIX 文件描述符专属 RAII 看门狗
class FileDescriptorGuard {
public:
    explicit FileDescriptorGuard(int fd = -1) noexcept : fd_(fd) {}
    ~FileDescriptorGuard() noexcept {
        Close();
    }

    FileDescriptorGuard(const FileDescriptorGuard&) = delete;
    FileDescriptorGuard& operator=(const FileDescriptorGuard&) = delete;

    FileDescriptorGuard(FileDescriptorGuard&& other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;
    }

    FileDescriptorGuard& operator=(FileDescriptorGuard&& other) noexcept {
        if (this != &other) {
            Close();
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

    int Get() const noexcept { return fd_; }
    bool IsValid() const noexcept { return fd_ >= 0; }

    int Release() noexcept {
        int temp = fd_;
        fd_ = -1;
        return temp;
    }

    void Close() noexcept {
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
    }

private:
    int fd_;
};

} // namespace filegate::infra
