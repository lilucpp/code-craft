#pragma once

#include "filegate/types.hpp"
#include <chrono>

namespace filegate::core {

// ============================================================================
// Typestate 幻影类型标记 (Phantom Types - systems-data-design)
// ============================================================================
struct StateDraft {};
struct StatePendingApproval {};
struct StateApproved {};
struct StateActive {};
struct StateExpired {};

template <typename State>
class ShareRequest;

// ============================================================================
// 类声明 (Forward Declarations of Specializations)
// ============================================================================
template <>
class ShareRequest<StateDraft> {
public:
    static ShareRequest<StateDraft> CreateNew(types::UserId applicant, types::ByteSize size);
    types::Result<ShareRequest<StatePendingApproval>> Submit(types::SecurityLevel level, uint32_t expire_days);
    const types::ShareRequestMeta& meta() const noexcept { return meta_; }

private:
    template <typename S> friend class ShareRequest;
    explicit ShareRequest(types::ShareRequestMeta meta) : meta_(meta) {}
    types::ShareRequestMeta meta_;
};

template <>
class ShareRequest<StatePendingApproval> {
public:
    types::Result<ShareRequest<StateApproved>> Approve(types::UserId approver);
    types::Result<void> Reject(types::UserId approver, const std::string& reason);
    const types::ShareRequestMeta& meta() const noexcept { return meta_; }

private:
    template <typename S> friend class ShareRequest;
    explicit ShareRequest(types::ShareRequestMeta meta) : meta_(meta) {}
    types::ShareRequestMeta meta_;
};

template <>
class ShareRequest<StateApproved> {
public:
    types::Result<ShareRequest<StateActive>> Activate(uint32_t max_downloads);
    const types::ShareRequestMeta& meta() const noexcept { return meta_; }

private:
    template <typename S> friend class ShareRequest;
    explicit ShareRequest(types::ShareRequestMeta meta) : meta_(meta) {}
    types::ShareRequestMeta meta_;
};

template <>
class ShareRequest<StateActive> {
public:
    types::Result<bool> RecordDownload();
    const types::ShareRequestMeta& meta() const noexcept { return meta_; }

private:
    template <typename S> friend class ShareRequest;
    explicit ShareRequest(types::ShareRequestMeta meta) : meta_(meta) {}
    types::ShareRequestMeta meta_;
};

// ============================================================================
// 成员函数内联实现 (Inlined after all classes are complete types)
// ============================================================================

// --- 状态 1: Draft ---
inline ShareRequest<StateDraft> ShareRequest<StateDraft>::CreateNew(types::UserId applicant, types::ByteSize size) {
    types::ShareRequestMeta meta{};
    meta.request_id = types::ShareRequestId(1001);
    meta.applicant_id = applicant;
    meta.payload_size_bytes = size;
    meta.created_at_ms = types::TimestampMs(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    meta.status = types::RequestStatus::kDraft;
    return ShareRequest<StateDraft>(meta);
}

inline types::Result<ShareRequest<StatePendingApproval>> ShareRequest<StateDraft>::Submit(
    types::SecurityLevel level, uint32_t expire_days) 
{
    if (level == types::SecurityLevel::kTopSecret) {
        return types::Result<ShareRequest<StatePendingApproval>>::Error(
            types::ErrorCode::kSecurityBlocked, "TopSecret files are strictly forbidden from export");
    }
    if (expire_days == 0 || expire_days > 7) {
        return types::Result<ShareRequest<StatePendingApproval>>::Error(
            types::ErrorCode::kInvalidArgument, "Expire days must be in range [1, 7]");
    }

    meta_.security_level = level;
    meta_.status = types::RequestStatus::kPendingApproval;
    meta_.expire_at_ms = types::TimestampMs(meta_.created_at_ms.get() + expire_days * 86400000ULL);
    return types::Result<ShareRequest<StatePendingApproval>>::Success(
        ShareRequest<StatePendingApproval>(meta_));
}

// --- 状态 2: PendingApproval ---
inline types::Result<ShareRequest<StateApproved>> ShareRequest<StatePendingApproval>::Approve(types::UserId approver) {
    if (approver.get() == 0 || approver == meta_.applicant_id) {
        return types::Result<ShareRequest<StateApproved>>::Error(
            types::ErrorCode::kInvalidArgument, "Approver cannot be empty or self-approving");
    }
    meta_.approver_id = approver;
    meta_.status = types::RequestStatus::kApproved;
    return types::Result<ShareRequest<StateApproved>>::Success(
        ShareRequest<StateApproved>(meta_));
}

inline types::Result<void> ShareRequest<StatePendingApproval>::Reject(types::UserId approver, const std::string& reason) {
    if (reason.length() < 10) {
        return types::Result<void>::Error(
            types::ErrorCode::kInvalidArgument, "Rejection reason must be at least 10 chars");
    }
    meta_.approver_id = approver;
    meta_.status = types::RequestStatus::kRevoked;
    return types::Result<void>::Success();
}

// --- 状态 3: Approved ---
inline types::Result<ShareRequest<StateActive>> ShareRequest<StateApproved>::Activate(uint32_t max_downloads) {
    if (max_downloads == 0 || max_downloads > 20) {
        return types::Result<ShareRequest<StateActive>>::Error(
            types::ErrorCode::kInvalidArgument, "Max downloads must be in range [1, 20]");
    }
    meta_.max_download_limit = max_downloads;
    meta_.current_download_count = 0;
    meta_.status = types::RequestStatus::kActive;
    return types::Result<ShareRequest<StateActive>>::Success(
        ShareRequest<StateActive>(meta_));
}

// --- 状态 4: Active ---
inline types::Result<bool> ShareRequest<StateActive>::RecordDownload() {
    if (meta_.current_download_count >= meta_.max_download_limit) {
        meta_.status = types::RequestStatus::kExpired;
        return types::Result<bool>::Error(
            types::ErrorCode::kInvalidState, "Download limit reached; token expired");
    }
    meta_.current_download_count++;
    bool is_now_exhausted = (meta_.current_download_count >= meta_.max_download_limit);
    if (is_now_exhausted) {
        meta_.status = types::RequestStatus::kExpired;
    }
    return types::Result<bool>::Success(is_now_exhausted);
}

} // namespace filegate::core
