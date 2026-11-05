#include "test_framework.hpp"
#include "filegate/share_request.hpp"

using namespace filegate;

// ============================================================================
// 测试套件 3: TypestateLifecycleTest (业务规则守卫与状态跃迁全路径)
// ============================================================================

TEST(TypestateLifecycleTest, TopSecretExportMustBeBlocked) {
    // 1. Arrange: 创建草稿
    auto draft = core::ShareRequest<core::StateDraft>::CreateNew(
        types::UserId(100), types::ByteSize(2048));

    // 2. Act: 提交绝密文件
    auto res = draft.Submit(types::SecurityLevel::kTopSecret, 3);

    // 3. Assert: 验证 BR-SEC-001 强制阻断
    EXPECT_FALSE(res.ok());
    EXPECT_EQ(res.code(), types::ErrorCode::kSecurityBlocked);
}

TEST(TypestateLifecycleTest, ExpireDaysBoundaryValidation) {
    // 1. Arrange
    auto draft = core::ShareRequest<core::StateDraft>::CreateNew(
        types::UserId(100), types::ByteSize(2048));

    // 2. Act & 3. Assert: 0天或超过7天均应报错
    auto res_zero = draft.Submit(types::SecurityLevel::kCoreConfidential, 0);
    EXPECT_FALSE(res_zero.ok());
    EXPECT_EQ(res_zero.code(), types::ErrorCode::kInvalidArgument);

    auto res_overflow = draft.Submit(types::SecurityLevel::kCoreConfidential, 8);
    EXPECT_FALSE(res_overflow.ok());
    EXPECT_EQ(res_overflow.code(), types::ErrorCode::kInvalidArgument);

    // 合法7天应成功
    auto res_valid = draft.Submit(types::SecurityLevel::kCoreConfidential, 7);
    EXPECT_TRUE(res_valid.ok());
}

TEST(TypestateLifecycleTest, SelfApprovalMustBeRejected) {
    // 1. Arrange: 发起人为 100
    auto draft = core::ShareRequest<core::StateDraft>::CreateNew(
        types::UserId(100), types::ByteSize(1024));
    auto pending = draft.Submit(types::SecurityLevel::kInternalSensitive, 3).value();

    // 2. Act: 尝试自我审批 (approver == applicant)
    auto self_approve_res = pending.Approve(types::UserId(100));

    // 3. Assert
    EXPECT_FALSE(self_approve_res.ok());
    EXPECT_EQ(self_approve_res.code(), types::ErrorCode::kInvalidArgument);
}

TEST(TypestateLifecycleTest, RejectionReasonTooShortMustFail) {
    // 1. Arrange
    auto draft = core::ShareRequest<core::StateDraft>::CreateNew(
        types::UserId(100), types::ByteSize(1024));
    auto pending = draft.Submit(types::SecurityLevel::kInternalSensitive, 3).value();

    // 2. Act: 驳回理由仅 5 个字符 (<10 字符)
    auto short_reason_res = pending.Reject(types::UserId(200), "Short");

    // 3. Assert: 验证规则强制要求有效驳回说明
    EXPECT_FALSE(short_reason_res.ok());
    EXPECT_EQ(short_reason_res.code(), types::ErrorCode::kInvalidArgument);

    // 满 10 字符应成功
    auto valid_reject = pending.Reject(types::UserId(200), "Exceeded risk boundary policy");
    EXPECT_TRUE(valid_reject.ok());
}

TEST(TypestateLifecycleTest, DownloadCountMeltdownAtExactThreshold) {
    // 1. Arrange: 走通流程至已批准，设定最多下载 2 次
    auto draft = core::ShareRequest<core::StateDraft>::CreateNew(
        types::UserId(100), types::ByteSize(1024));
    auto pending = draft.Submit(types::SecurityLevel::kCoreConfidential, 3).value();
    auto approved = pending.Approve(types::UserId(200)).value();
    auto active = approved.Activate(2).value();

    // 2. Act & 3. Assert
    auto dl1 = active.RecordDownload();
    EXPECT_TRUE(dl1.ok());
    EXPECT_FALSE(dl1.value()); // 未耗尽

    auto dl2 = active.RecordDownload();
    EXPECT_TRUE(dl2.ok());
    EXPECT_TRUE(dl2.value());  // 触发熔断
    EXPECT_EQ(active.meta().status, types::RequestStatus::kExpired);

    // 第3次下载应直接报错拒绝
    auto dl3 = active.RecordDownload();
    EXPECT_FALSE(dl3.ok());
    EXPECT_EQ(dl3.code(), types::ErrorCode::kInvalidState);
}
