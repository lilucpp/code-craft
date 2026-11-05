#pragma once

#include <string>
#include <optional>
#include <sstream>
#include "filegate/types.hpp"

namespace filegate::refactored {

// ============================================================================
// 手法 1: 引入领域值对象与强类型 (Replace Primitive with Object - Fowler 8.1)
// ============================================================================
struct AccessCredentials {
    std::string token;
    std::string passcode;

    bool Matches(const std::string& input_tok, const std::string& input_pwd) const noexcept {
        return !token.empty() && token == input_tok && !passcode.empty() && passcode == input_pwd;
    }
};

// ============================================================================
// 手法 2: 提炼配额管控实体，消灭 Feature Envy (Extract Class - Fowler 7.3)
// ============================================================================
class RequestQuota {
public:
    RequestQuota(uint32_t current, uint32_t max_limit) 
        : current_(current), max_limit_(max_limit) {}

    bool IsExhausted() const noexcept { return current_ >= max_limit_; }
    
    // 状态自封闭自增，消除外部依恋
    bool IncrementAndCheckMeltdown() noexcept {
        if (IsExhausted()) return true;
        current_++;
        return IsExhausted();
    }

    uint32_t current() const noexcept { return current_; }
    uint32_t max_limit() const noexcept { return max_limit_; }

private:
    uint32_t current_;
    uint32_t max_limit_;
};

// ============================================================================
// 手法 3: 提炼格式化器消除发散式变化 (Extract Class - Fowler 7.3 / SRP)
// ============================================================================
class DispatchLogFormatter {
public:
    static std::string FormatSuccess(
        types::ShareRequestId req_id,
        types::UserId applicant_id,
        types::UserId visitor_id,
        const RequestQuota& quota,
        bool is_meltdown) 
    {
        std::ostringstream ss;
        ss << "DISPATCH_OK:Req=" << req_id.get()
           << ";User=" << applicant_id.get()
           << ";Visitor=" << visitor_id.get()
           << ";DownloadCount=" << quota.current() << "/" << quota.max_limit();
        if (is_meltdown) {
            ss << ";MELTDOWN=TRUE";
        }
        return ss.str();
    }
};

// ============================================================================
// 手法 4: 卫语句优先重构服务入口 (Replace Nested Conditional with Guard Clauses)
// ============================================================================
class CleanTokenService {
public:
    types::Result<std::string> ValidateAndProcessDownload(
        types::ShareRequestId req_id,
        types::UserId applicant_id,
        types::UserId visitor_id,
        types::RequestStatus& status,
        const AccessCredentials& creds,
        RequestQuota& quota,
        const std::string& input_token,
        const std::string& input_passcode) 
    {
        // 卫语句 1: 检查激活态
        if (status != types::RequestStatus::kActive) {
            return types::Result<std::string>::Error(
                types::ErrorCode::kInvalidState, "ERROR_REQUEST_NOT_ACTIVE");
        }

        // 卫语句 2: 凭证校验
        if (input_token.empty() || input_token != creds.token) {
            return types::Result<std::string>::Error(
                types::ErrorCode::kInvalidArgument, "ERROR_INVALID_TOKEN");
        }
        if (input_passcode.empty() || input_passcode != creds.passcode) {
            return types::Result<std::string>::Error(
                types::ErrorCode::kInvalidArgument, "ERROR_INVALID_PASSCODE");
        }

        // 卫语句 3: 配额超限检查
        if (quota.IsExhausted()) {
            status = types::RequestStatus::kExpired;
            return types::Result<std::string>::Error(
                types::ErrorCode::kInvalidState, "ERROR_DOWNLOAD_LIMIT_EXCEEDED");
        }

        // 主干流程: 执行自增并生成审计
        bool is_meltdown = quota.IncrementAndCheckMeltdown();
        if (is_meltdown) {
            status = types::RequestStatus::kExpired;
        }

        std::string log = DispatchLogFormatter::FormatSuccess(
            req_id, applicant_id, visitor_id, quota, is_meltdown);
        return types::Result<std::string>::Success(std::move(log));
    }
};

} // namespace filegate::refactored
