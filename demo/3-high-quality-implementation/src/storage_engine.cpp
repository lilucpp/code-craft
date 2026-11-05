#include "filegate/storage_engine.hpp"
#include "filegate/scope_guard.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <fstream>

namespace filegate::storage {

// ============================================================================
// NVI 契约守护与前置校验实现
// ============================================================================
types::Result<void> IStorageProvider::AtomicWrite(
    const std::filesystem::path& target_path,
    const uint8_t* data,
    size_t length) 
{
    if (target_path.empty()) {
        return types::Result<void>::Error(types::ErrorCode::kInvalidArgument, "Target path is empty");
    }
    if (data == nullptr || length == 0) {
        return types::Result<void>::Error(types::ErrorCode::kInvalidArgument, "Write buffer is null or empty");
    }
    return DoAtomicWrite(target_path, data, length);
}

types::Result<std::vector<uint8_t>> IStorageProvider::Read(
    const std::filesystem::path& target_path) const 
{
    if (target_path.empty() || !std::filesystem::exists(target_path)) {
        return types::Result<std::vector<uint8_t>>::Error(
            types::ErrorCode::kInvalidArgument, "File does not exist: " + target_path.string());
    }
    return DoRead(target_path);
}

types::Result<void> IStorageProvider::SecureShred(
    const std::filesystem::path& target_path) 
{
    if (target_path.empty() || !std::filesystem::exists(target_path)) {
        return types::Result<void>::Error(types::ErrorCode::kInvalidArgument, "Path invalid or not found");
    }
    return DoSecureShred(target_path);
}

// ============================================================================
// LocalPosixStorage 实现 (OSDI 14 崩溃一致性落盘四步法)
// ============================================================================

types::Result<void> LocalPosixStorage::WriteDataLoop(int fd, const uint8_t* data, size_t length) {
    size_t written = 0;
    while (written < length) {
        ssize_t ret = ::write(fd, data + written, length - written);
        if (ret < 0) {
            if (errno == EINTR) continue;
            return types::Result<void>::Error(types::ErrorCode::kIoError, ::strerror(errno));
        }
        written += static_cast<size_t>(ret);
    }
    return types::Result<void>::Success();
}

types::Result<void> LocalPosixStorage::SyncParentDir(const std::filesystem::path& target_path) {
    auto parent = target_path.parent_path();
    if (parent.empty()) parent = ".";
    
    infra::FileDescriptorGuard dir_guard(::open(parent.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    if (dir_guard.IsValid()) {
        ::fsync(dir_guard.Get());
    }
    return types::Result<void>::Success();
}

types::Result<void> LocalPosixStorage::DoAtomicWrite(
    const std::filesystem::path& target_path,
    const uint8_t* data,
    size_t length) 
{
    // 步骤 1: 创建隐藏临时文件写入
    std::filesystem::path tmp_path = target_path;
    tmp_path.replace_filename("." + target_path.filename().string() + ".tmp");

    infra::FileDescriptorGuard fd_guard(
        ::open(tmp_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0640));
    if (!fd_guard.IsValid()) {
        return types::Result<void>::Error(types::ErrorCode::kIoError, "Failed to open temp file");
    }

    auto write_res = WriteDataLoop(fd_guard.Get(), data, length);
    if (!write_res.ok()) {
        ::unlink(tmp_path.c_str());
        return write_res;
    }

    // 步骤 2: fdatasync 强制将数据与关键元数据冲刷到物理非易失介质
    if (::fdatasync(fd_guard.Get()) != 0) {
        ::unlink(tmp_path.c_str());
        return types::Result<void>::Error(types::ErrorCode::kSyncError, "fdatasync failed");
    }
    fd_guard.Close();

    // 步骤 3: 利用 POSIX 文件系统原子重命名覆盖 (Atomic Rename)
    if (::rename(tmp_path.c_str(), target_path.c_str()) != 0) {
        ::unlink(tmp_path.c_str());
        return types::Result<void>::Error(types::ErrorCode::kRenameError, "Atomic rename failed");
    }

    // 步骤 4: 冲刷父目录 dentry 目录项，确保重命名记录真正落盘持久化
    return SyncParentDir(target_path);
}

types::Result<std::vector<uint8_t>> LocalPosixStorage::DoRead(
    const std::filesystem::path& target_path) const 
{
    std::ifstream file(target_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return types::Result<std::vector<uint8_t>>::Error(types::ErrorCode::kIoError, "Open failed for read");
    }
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return types::Result<std::vector<uint8_t>>::Error(types::ErrorCode::kIoError, "Read content failed");
    }
    return types::Result<std::vector<uint8_t>>::Success(std::move(buffer));
}

types::Result<void> LocalPosixStorage::DoSecureShred(
    const std::filesystem::path& target_path) 
{
    auto size = std::filesystem::file_size(target_path);
    std::vector<uint8_t> zeros(size, 0x00);
    
    // 覆盖零数据再删除
    auto write_res = DoAtomicWrite(target_path, zeros.data(), zeros.size());
    if (!write_res.ok()) {
        return write_res;
    }
    std::error_code ec;
    std::filesystem::remove(target_path, ec);
    if (ec) {
        return types::Result<void>::Error(types::ErrorCode::kIoError, ec.message());
    }
    return types::Result<void>::Success();
}

} // namespace filegate::storage
