#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

namespace systems_craft {

/**
 * @brief C++17 编译期零成本强标量包装器 (Zero-Cost Strong Type)
 * @tparam Tag 幽灵类型标签，用于区分不同的业务标量
 * @tparam T 底层原始类型（如 std::uint64_t, int, double 等）
 */
template <typename Tag, typename T>
class StrongType {
 public:
  using ValueType = T;

  template <typename U = T, typename = std::enable_if_t<std::is_default_constructible_v<U>>>
  constexpr explicit StrongType() noexcept(std::is_nothrow_default_constructible_v<T>) : value_{} {}

  constexpr explicit StrongType(const T &val) noexcept(std::is_nothrow_copy_constructible_v<T>) : value_(val) {}

  constexpr explicit StrongType(T &&val) noexcept(std::is_nothrow_move_constructible_v<T>) : value_(std::move(val)) {}

  [[nodiscard]] constexpr const T &get() const noexcept { return value_; }
  [[nodiscard]] constexpr T &get() noexcept { return value_; }

  [[nodiscard]] friend constexpr bool operator==(const StrongType &lhs, const StrongType &rhs) noexcept {
    return lhs.value_ == rhs.value_;
  }
  [[nodiscard]] friend constexpr bool operator!=(const StrongType &lhs, const StrongType &rhs) noexcept {
    return !(lhs == rhs);
  }
  [[nodiscard]] friend constexpr bool operator<(const StrongType &lhs, const StrongType &rhs) noexcept {
    return lhs.value_ < rhs.value_;
  }
  [[nodiscard]] friend constexpr bool operator<=(const StrongType &lhs, const StrongType &rhs) noexcept {
    return !(rhs < lhs);
  }
  [[nodiscard]] friend constexpr bool operator>(const StrongType &lhs, const StrongType &rhs) noexcept {
    return rhs < lhs;
  }
  [[nodiscard]] friend constexpr bool operator>=(const StrongType &lhs, const StrongType &rhs) noexcept {
    return !(lhs < rhs);
  }

 private:
  T value_;
};

/**
 * @brief 用于 std::unordered_map / std::unordered_set 的强类型哈希仿函数
 * 不向 std 命名空间注入模板特化，完全符合 CERT DCL58-CPP 规范。
 */
struct StrongTypeHash {
  template <typename Tag, typename T>
  std::size_t operator()(const StrongType<Tag, T> &item) const noexcept(noexcept(std::hash<T>{}(item.get()))) {
    return std::hash<T>{}(item.get());
  }
};

}  // namespace systems_craft
