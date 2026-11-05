#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <type_traits>
#include <stdexcept>
#include <optional>

namespace filegate::types {

// ============================================================================
// 1. 零开销强类型模板 (Zero-Cost StrongType - Scott Meyers Item 18)
//    杜绝裸整数滥用 (Primitive Obsession)，防止 UserId 与 ShareRequestId 混淆
// ============================================================================
template <typename T, typename Tag>
class StrongType {
public:
    using ValueType = T;

    constexpr StrongType() noexcept : value_{} {}
    constexpr explicit StrongType(const T& val) : value_(val) {}
    constexpr explicit StrongType(T&& val) noexcept(std::is_nothrow_move_constructible_v<T>) 
        : value_(std::move(val)) {}

    constexpr T& get() noexcept { return value_; }
    constexpr const T& get() const noexcept { return value_; }

    constexpr bool operator==(const StrongType& other) const noexcept { return value_ == other.value_; }
    constexpr bool operator!=(const StrongType& other) const noexcept { return value_ != other.value_; }
    constexpr bool operator<(const StrongType& other) const noexcept { return value_ < other.value_; }

    friend std::ostream& operator<<(std::ostream& os, const StrongType& st) {
        return os << st.value_;
    }

private:
    T value_;
};

// 领域强类型别名
using ShareRequestId = StrongType<uint64_t, struct ShareRequestIdTag>;
using UserId         = StrongType<uint64_t, struct UserIdTag>;
using TimestampMs    = StrongType<uint64_t, struct TimestampMsTag>;
using ByteSize       = StrongType<uint64_t, struct ByteSizeTag>;

// ============================================================================
// 2. 核心领域枚举与错误码
// ============================================================================
enum class SecurityLevel : uint8_t {
    kInternalGeneral = 0,    // 内部普通
    kInternalSensitive = 1,  // 内部敏感
    kCoreConfidential = 2,   // 核心商密
    kTopSecret = 3           // 绝密 (禁止外发)
};

enum class RequestStatus : uint8_t {
    kDraft = 0,
    kPendingApproval = 1,
    kApproved = 2,
    kActive = 3,
    kExpired = 4,
    kRevoked = 5
};

enum class ErrorCode : uint8_t {
    kOk = 0,
    kInvalidArgument = 1,
    kIoError = 2,
    kSyncError = 3,
    kRenameError = 4,
    kInvalidState = 5,
    kSecurityBlocked = 6,
    kQueueShutdown = 7
};

inline std::ostream& operator<<(std::ostream& os, ErrorCode code) {
    return os << static_cast<int>(code);
}

inline std::ostream& operator<<(std::ostream& os, SecurityLevel level) {
    return os << static_cast<int>(level);
}

inline std::ostream& operator<<(std::ostream& os, RequestStatus status) {
    return os << static_cast<int>(status);
}

// ============================================================================
// 3. 强异常安全轻量级结果类型 Result<T> (支持卫语句 Early Return)
// ============================================================================
template <typename T>
class Result {
public:
    static Result<T> Success(T val) { return Result<T>(std::move(val)); }
    static Result<T> Error(ErrorCode code, std::string msg) { 
        return Result<T>(code, std::move(msg)); 
    }

    bool ok() const noexcept { return code_ == ErrorCode::kOk; }
    ErrorCode code() const noexcept { return code_; }
    const std::string& message() const noexcept { return message_; }

    T& value() {
        if (!ok()) throw std::runtime_error("Result has error: " + message_);
        return *value_;
    }

    const T& value() const {
        if (!ok()) throw std::runtime_error("Result has error: " + message_);
        return *value_;
    }

private:
    explicit Result(T val) : code_(ErrorCode::kOk), value_(std::move(val)) {}
    Result(ErrorCode code, std::string msg) : code_(code), message_(std::move(msg)), value_(std::nullopt) {}

    ErrorCode code_;
    std::string message_;
    std::optional<T> value_;
};

// 特化 void 版本
template <>
class Result<void> {
public:
    static Result<void> Success() { return Result<void>(ErrorCode::kOk, ""); }
    static Result<void> Error(ErrorCode code, std::string msg) { 
        return Result<void>(code, std::move(msg)); 
    }

    bool ok() const noexcept { return code_ == ErrorCode::kOk; }
    ErrorCode code() const noexcept { return code_; }
    const std::string& message() const noexcept { return message_; }

private:
    Result(ErrorCode code, std::string msg) : code_(code), message_(std::move(msg)) {}
    ErrorCode code_;
    std::string message_;
};

// ============================================================================
// 4. 物理内存排布 (8-4-2-1 降序对齐无空洞 - systems-data-design)
// ============================================================================
struct alignas(8) ShareRequestMeta {
    // [8 字节区: 48 字节]
    ShareRequestId request_id;         // 8 字节
    UserId applicant_id;               // 8 字节
    UserId approver_id;                // 8 字节
    TimestampMs created_at_ms;         // 8 字节
    TimestampMs expire_at_ms;          // 8 字节
    ByteSize payload_size_bytes;       // 8 字节

    // [4 字节区: 12 字节]
    uint32_t max_download_limit;       // 4 字节
    uint32_t current_download_count;   // 4 字节
    uint32_t access_passcode_hash;     // 4 字节

    // [2 字节区: 2 字节]
    uint16_t dlp_violation_score;      // 2 字节

    // [1 字节区: 2 字节]
    SecurityLevel security_level;      // 1 字节
    RequestStatus status;              // 1 字节
};

// 编译期断言保证尺寸恰为 64 字节，Padding 空洞 0 字节
static_assert(sizeof(ShareRequestMeta) == 64, 
    "ShareRequestMeta must occupy exactly 64 bytes without padding holes");

} // namespace filegate::types
