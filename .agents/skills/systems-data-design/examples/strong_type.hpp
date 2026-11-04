#pragma once

#include <utility>
#include <concepts>
#include <functional>

namespace systems_craft {

/**
 * @brief 编译期零成本强标量包装器 (Zero-Cost Strong Type)
 * @tparam Tag 幽灵类型标签，用于区分不同的业务标量
 * @tparam T 底层原始类型（如 uint64_t, int, double 等）
 */
template <typename Tag, typename T>
class StrongType {
public:
    using ValueType = T;

    constexpr explicit StrongType() requires std::default_initializable<T> : value_{} {}
    constexpr explicit StrongType(const T& val) : value_(val) {}
    constexpr explicit StrongType(T&& val) noexcept(std::is_nothrow_move_constructible_v<T>)
        : value_(std::move(val)) {}

    [[nodiscard]] constexpr const T& get() const noexcept { return value_; }
    [[nodiscard]] constexpr T& get() noexcept { return value_; }

    constexpr bool operator==(const StrongType& rhs) const noexcept requires std::equality_comparable<T> {
        return value_ == rhs.value_;
    }

    constexpr auto operator<=>(const StrongType& rhs) const noexcept requires std::three_way_comparable<T> {
        return value_ <=> rhs.value_;
    }

private:
    T value_;
};

} // namespace systems_craft

// 支持标准哈希容器（如 std::unordered_map）
template <typename Tag, typename T>
struct std::hash<systems_craft::StrongType<Tag, T>> {
    size_t operator()(const systems_craft::StrongType<Tag, T>& item) const noexcept {
        return std::hash<T>{}(item.get());
    }
};
