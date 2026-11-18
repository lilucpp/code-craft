#pragma once

#include <string>
#include <system_error>
#include <type_traits>
#include <utility>
#include <variant>

namespace systems_craft {

/**
 * @brief C++17 轻量级返回值容器 Result<T, E> (替代 C++23 std::expected)
 * 强制 [[nodiscard]]，消除裸 bool + 出参或静默吞错。
 */
template <typename T, typename E = std::error_code>
class [[nodiscard]] Result {
 public:
  static Result Ok(T val) noexcept(std::is_nothrow_move_constructible_v<T>) {
    return Result(std::in_place_index<0>, std::move(val));
  }

  static Result Err(E err) noexcept(std::is_nothrow_move_constructible_v<E>) {
    return Result(std::in_place_index<1>, std::move(err));
  }

  [[nodiscard]] bool has_value() const noexcept { return storage_.index() == 0; }
  [[nodiscard]] explicit operator bool() const noexcept { return has_value(); }

  [[nodiscard]] const T &value() const & { return std::get<0>(storage_); }
  [[nodiscard]] T &value() & { return std::get<0>(storage_); }
  [[nodiscard]] T &&value() && { return std::get<0>(std::move(storage_)); }

  [[nodiscard]] const E &error() const & { return std::get<1>(storage_); }

 private:
  explicit Result(std::in_place_index_t<0> tag, T &&val) noexcept(std::is_nothrow_move_constructible_v<T>)
      : storage_(tag, std::move(val)) {}
  explicit Result(std::in_place_index_t<1> tag, E &&err) noexcept(std::is_nothrow_move_constructible_v<E>)
      : storage_(tag, std::move(err)) {}

  std::variant<T, E> storage_;
};

// ============================================================================
// C++17 编译期类型状态机 (Typestate Pattern) 示范
// 将运行期 if (state != Ready) 错误拦截提前到编译期，零运行时分支开销。
// ============================================================================
struct Disconnected {};
struct Connected {};
struct Authenticated {};

template <typename StateTag>
class ChannelHandle {
 public:
  ChannelHandle(const ChannelHandle &) = delete;
  ChannelHandle &operator=(const ChannelHandle &) = delete;
  ChannelHandle(ChannelHandle &&) noexcept = default;
  ChannelHandle &operator=(ChannelHandle &&) noexcept = default;
  ~ChannelHandle() = default;

  template <typename S = StateTag, typename = std::enable_if_t<std::is_same_v<S, Disconnected>>>
  static Result<ChannelHandle<Connected>> Connect(std::string endpoint) {
    if (endpoint.empty()) {
      return Result<ChannelHandle<Connected>>::Err(std::make_error_code(std::errc::invalid_argument));
    }
    return Result<ChannelHandle<Connected>>::Ok(ChannelHandle<Connected>(std::move(endpoint)));
  }

  template <typename S = StateTag, typename = std::enable_if_t<std::is_same_v<S, Connected>>>
  Result<ChannelHandle<Authenticated>> Authenticate(const std::string &token) && {
    if (token.empty()) {
      return Result<ChannelHandle<Authenticated>>::Err(std::make_error_code(std::errc::permission_denied));
    }
    return Result<ChannelHandle<Authenticated>>::Ok(ChannelHandle<Authenticated>(std::move(endpoint_)));
  }

  template <typename S = StateTag, typename = std::enable_if_t<std::is_same_v<S, Authenticated>>>
  [[nodiscard]] const std::string &Endpoint() const noexcept {
    return endpoint_;
  }

 private:
  template <typename OtherState>
  friend class ChannelHandle;

  explicit ChannelHandle(std::string endpoint) noexcept : endpoint_(std::move(endpoint)) {}

  std::string endpoint_;
};

}  // namespace systems_craft
