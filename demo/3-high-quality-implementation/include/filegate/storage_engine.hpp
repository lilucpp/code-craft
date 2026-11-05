#pragma once

#include <filesystem>
#include <vector>
#include <cstdint>
#include "filegate/types.hpp"

namespace filegate::storage {

// ============================================================================
// IStorageProvider - 存储抽象接口 (应用 NVI 非虚接口模式 - cpp-idioms)
// ============================================================================
class IStorageProvider {
public:
    virtual ~IStorageProvider() = default;

    // NVI 契约守护入口 (公有非虚函数：负责前置断言、参数校验与卫语句拦截)
    types::Result<void> AtomicWrite(
        const std::filesystem::path& target_path,
        const uint8_t* data,
        size_t length);

    types::Result<std::vector<uint8_t>> Read(
        const std::filesystem::path& target_path) const;

    types::Result<void> SecureShred(
        const std::filesystem::path& target_path);

protected:
    // 纯虚派生钩子 (由底层驱动具体实现)
    virtual types::Result<void> DoAtomicWrite(
        const std::filesystem::path& target_path,
        const uint8_t* data,
        size_t length) = 0;

    virtual types::Result<std::vector<uint8_t>> DoRead(
        const std::filesystem::path& target_path) const = 0;

    virtual types::Result<void> DoSecureShred(
        const std::filesystem::path& target_path) = 0;
};

// ============================================================================
// LocalPosixStorage - 本地文件系统实现 (遵循 OSDI 14 崩溃一致性原子落盘四步法)
// ============================================================================
class LocalPosixStorage final : public IStorageProvider {
protected:
    types::Result<void> DoAtomicWrite(
        const std::filesystem::path& target_path,
        const uint8_t* data,
        size_t length) override;

    types::Result<std::vector<uint8_t>> DoRead(
        const std::filesystem::path& target_path) const override;

    types::Result<void> DoSecureShred(
        const std::filesystem::path& target_path) override;

private:
    static types::Result<void> WriteDataLoop(int fd, const uint8_t* data, size_t length);
    static types::Result<void> SyncParentDir(const std::filesystem::path& target_path);
};

} // namespace filegate::storage
