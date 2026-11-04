#pragma once

#include <string>
#include <string_view>
#include <span>
#include <filesystem>
#include <system_error>
#include <fcntl.h>
#include <unistd.h>

namespace systems_craft {

/**
 * @brief 符合 OSDI 2014 顶会规范的崩溃安全原子文件写入器
 * 
 * 严格执行四步法：
 * 1. 写入临时文件 (<target>.tmp.<pid>)
 * 2. fdatasync 强制将脏页与文件内容刷入物理存储介质
 * 3. rename 原子替换旧文件
 * 4. 关键！对父目录执行 fsync，确保目录项元数据落盘防丢
 */
class CrashSafeWriter {
public:
    static std::error_code write_atomically(const std::filesystem::path& target_path,
                                           std::span<const uint8_t> data) noexcept {
        const auto parent_dir = target_path.parent_path().empty() ? "." : target_path.parent_path();
        const auto temp_path = target_path.string() + ".tmp." + std::to_string(getpid());

        // 步骤 1: 创建并写入临时文件
        int fd = ::open(temp_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
        if (fd < 0) {
            return std::error_code(errno, std::generic_category());
        }

        size_t written_total = 0;
        while (written_total < data.size()) {
            ssize_t res = ::write(fd, data.data() + written_total, data.size() - written_total);
            if (res < 0) {
                if (errno == EINTR) continue;
                int err = errno;
                ::close(fd);
                ::unlink(temp_path.c_str());
                return std::error_code(err, std::generic_category());
            }
            written_total += static_cast<size_t>(res);
        }

        // 步骤 2: 强制文件数据与必要元数据刷盘
        if (::fdatasync(fd) < 0) {
            int err = errno;
            ::close(fd);
            ::unlink(temp_path.c_str());
            return std::error_code(err, std::generic_category());
        }

        if (::close(fd) < 0) {
            int err = errno;
            ::unlink(temp_path.c_str());
            return std::error_code(err, std::generic_category());
        }

        // 步骤 3: POSIX 目录项原子重命名替换
        if (::rename(temp_path.c_str(), target_path.c_str()) < 0) {
            int err = errno;
            ::unlink(temp_path.c_str());
            return std::error_code(err, std::generic_category());
        }

        // 步骤 4: 刷新包含该文件的父目录元数据（防 ext4/btrfs 延迟分配导致孤儿或零字节文件）
        int dir_fd = ::open(parent_dir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
        if (dir_fd >= 0) {
            ::fsync(dir_fd);
            ::close(dir_fd);
        }

        return {};
    }
};

} // namespace systems_craft
