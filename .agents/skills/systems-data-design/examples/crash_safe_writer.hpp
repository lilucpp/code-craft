#pragma once

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <new>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace systems_craft {

/**
 * @brief 符合 OSDI 2014 顶会规范的 C++17 崩溃安全原子文件写入器
 *
 * 严格执行四步法：
 * 1. 通过 mkstemp 原子创建临时文件并写入全部字节
 * 2. fdatasync 强制将脏页与文件内容刷入物理存储介质
 * 3. rename 原子替换目标文件
 * 4. 对父目录执行 fsync，确保目录项元数据落盘防丢
 */
class CrashSafeWriter {
 public:
  [[nodiscard]] static std::error_code write_atomically(const std::filesystem::path &target_path, const void *data,
                                                        std::size_t size) noexcept try {
    if (size > 0 && data == nullptr) {
      return std::make_error_code(std::errc::invalid_argument);
    }

    const std::filesystem::path parent_dir =
        target_path.parent_path().empty() ? std::filesystem::path{"."} : target_path.parent_path();
    std::string temp_template = target_path.string() + ".tmp.XXXXXX";
    std::vector<char> temp_buf(temp_template.begin(), temp_template.end());
    temp_buf.push_back('\0');

    const int fd = ::mkstemp(temp_buf.data());
    if (fd < 0) {
      return {errno, std::generic_category()};
    }

    const std::string temp_path(temp_buf.data());
    const std::error_code write_err = write_all_and_sync(fd, static_cast<const std::uint8_t *>(data), size);
    if (write_err) {
      ::close(fd);
      ::unlink(temp_path.c_str());
      return write_err;
    }

    if (::close(fd) < 0) {
      const int err = errno;
      ::unlink(temp_path.c_str());
      return {err, std::generic_category()};
    }

    if (::rename(temp_path.c_str(), target_path.c_str()) < 0) {
      const int err = errno;
      ::unlink(temp_path.c_str());
      return {err, std::generic_category()};
    }

    sync_parent_dir(parent_dir);
    return {};
  } catch (const std::bad_alloc &) {
    return std::make_error_code(std::errc::not_enough_memory);
  }

  [[nodiscard]] static std::error_code write_atomically(const std::filesystem::path &target_path,
                                                        std::string_view text) noexcept {
    return write_atomically(target_path, text.data(), text.size());
  }

 private:
  [[nodiscard]] static std::error_code write_all_and_sync(int fd, const std::uint8_t *data, std::size_t size) noexcept {
    std::size_t written_total = 0;
    while (written_total < size) {
      const ssize_t res = ::write(fd, data + written_total, size - written_total);
      if (res < 0 && errno == EINTR) {
        continue;
      }
      if (res < 0) {
        return {errno, std::generic_category()};
      }
      written_total += static_cast<std::size_t>(res);
    }
    if (::fdatasync(fd) < 0) {
      return {errno, std::generic_category()};
    }
    return {};
  }

  static void sync_parent_dir(const std::filesystem::path &parent_dir) noexcept {
    const int dir_fd = ::open(parent_dir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dir_fd >= 0) {
      ::fsync(dir_fd);
      ::close(dir_fd);
    }
  }
};

}  // namespace systems_craft
