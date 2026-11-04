#pragma once

#include <cstddef>
#include <iostream>
#include <span>
#include <string_view>

namespace cpp_idioms {

/**
 * @brief CRTP Base Class providing static polymorphism for Stream Writers.
 * 
 * Features:
 * - Zero vtable overhead: no vptr per instance.
 * - Complete compile-time inline dispatch.
 * - Protected non-virtual destructor prevents polymorphic slicing via base pointers.
 */
template <typename Derived>
class DataStreamWriter {
public:
    void write(std::span<const std::byte> bytes) {
        // Enforce static interface requirement at compile-time
        underlying().do_write(bytes);
    }

    void write_string(std::string_view text) {
        auto bytes = std::as_bytes(std::span(text.data(), text.size()));
        write(bytes);
    }

    [[nodiscard]] std::size_t bytes_written() const noexcept {
        return underlying().do_bytes_written();
    }

protected:
    DataStreamWriter() = default;
    ~DataStreamWriter() = default;

private:
    Derived& underlying() noexcept {
        return static_cast<Derived&>(*this);
    }

    const Derived& underlying() const noexcept {
        return static_cast<const Derived&>(*this);
    }
};

/**
 * @brief Stateless compression policy demonstrating Empty Base Optimization (EBO).
 */
struct NoOpCompressionPolicy {
    // Empty class: size is 1 byte standalone, but 0 bytes via [[no_unique_address]]
    void compress() const noexcept {}
};

/**
 * @brief Concrete CRTP Implementation.
 */
class MemoryStreamWriter final : public DataStreamWriter<MemoryStreamWriter> {
public:
    void do_write(std::span<const std::byte> bytes) {
        total_bytes_ += bytes.size();
        policy_.compress(); // Zero-cost empty policy
    }

    [[nodiscard]] std::size_t do_bytes_written() const noexcept {
        return total_bytes_;
    }

private:
    std::size_t total_bytes_{0};
    [[no_unique_address]] NoOpCompressionPolicy policy_{}; // Modern C++20 EBO
};

/**
 * @brief CRTP Mixin for automatically injecting equality and inequality operators.
 */
template <typename Derived>
class ComparableMixin {
    friend bool operator!=(const Derived& lhs, const Derived& rhs) noexcept {
        return !(lhs == rhs);
    }
};

/**
 * @brief Value type utilizing CRTP Mixin.
 */
class Identifier : public ComparableMixin<Identifier> {
public:
    explicit constexpr Identifier(int id) noexcept : id_(id) {}

    friend constexpr bool operator==(const Identifier& lhs, const Identifier& rhs) noexcept {
        return lhs.id_ == rhs.id_;
    }

    [[nodiscard]] constexpr int value() const noexcept { return id_; }

private:
    int id_;
};

} // namespace cpp_idioms
