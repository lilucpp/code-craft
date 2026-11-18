#pragma once

#include <type_traits>
#include <utility>

namespace systems_craft {

/**
 * @brief C++17 零开销作用域守卫 (ScopeGuard / RAII Cleanup)
 * 确保任何早退、异常分支均能确定性执行回滚或资源释放。
 */
template <typename Fn>
class ScopeGuard {
 public:
  explicit ScopeGuard(Fn &&cleanup) noexcept(std::is_nothrow_move_constructible_v<Fn>)
      : cleanup_(std::move(cleanup)), active_(true) {}

  explicit ScopeGuard(const Fn &cleanup) noexcept(std::is_nothrow_copy_constructible_v<Fn>)
      : cleanup_(cleanup), active_(true) {}

  ScopeGuard(ScopeGuard &&other) noexcept(std::is_nothrow_move_constructible_v<Fn>)
      : cleanup_(std::move(other.cleanup_)), active_(other.active_) {
    other.dismiss();
  }

  ScopeGuard(const ScopeGuard &) = delete;
  ScopeGuard &operator=(const ScopeGuard &) = delete;
  ScopeGuard &operator=(ScopeGuard &&) = delete;

  ~ScopeGuard() noexcept {
    if (active_) {
      cleanup_();
    }
  }

  void dismiss() noexcept { active_ = false; }

 private:
  Fn cleanup_;
  bool active_;
};

template <typename Fn>
[[nodiscard]] ScopeGuard<std::decay_t<Fn>> MakeScopeGuard(Fn &&cleanup) noexcept(
    std::is_nothrow_constructible_v<ScopeGuard<std::decay_t<Fn>>, Fn &&>) {
  return ScopeGuard<std::decay_t<Fn>>(std::forward<Fn>(cleanup));
}

}  // namespace systems_craft
