#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "legacy_token_service.hpp"
#include "token_service.hpp"

using namespace filegate;

// ============================================================================
// 行为保持守护测试 (Safety-Net Test - 验证重构前后行为 100% 等价)
// ============================================================================

void TestEquivalenceCase(
    const std::string& case_name,
    int initial_status,
    uint32_t current_dl,
    uint32_t max_dl,
    const std::string& stored_tok,
    const std::string& stored_pwd,
    const std::string& in_tok,
    const std::string& in_pwd) 
{
    std::cout << "[Running Equivalence Case] " << case_name << " ... ";

    // 1. 运行遗留实现
    legacy::LegacyRequestData leg_data{
        1001, 2002, 2, initial_status, max_dl, current_dl, stored_tok, stored_pwd
    };
    legacy::LegacyTokenManager leg_mgr;
    std::string leg_log;
    bool leg_ok = leg_mgr.ValidateAndProcessDownload(
        leg_data, 9999, in_tok, in_pwd, leg_log);

    // 2. 运行重构后实现
    types::RequestStatus ref_status = static_cast<types::RequestStatus>(initial_status);
    refactored::AccessCredentials ref_creds{stored_tok, stored_pwd};
    refactored::RequestQuota ref_quota(current_dl, max_dl);
    refactored::CleanTokenService ref_svc;

    auto ref_res = ref_svc.ValidateAndProcessDownload(
        types::ShareRequestId(1001),
        types::UserId(2002),
        types::UserId(9999),
        ref_status,
        ref_creds,
        ref_quota,
        in_tok,
        in_pwd);

    // 3. 严格断言重构前后行为与输出完全一致
    assert(ref_res.ok() == leg_ok);
    if (leg_ok) {
        assert(ref_res.value() == leg_log);
    } else {
        assert(ref_res.message() == leg_log);
    }
    assert(static_cast<int>(ref_status) == leg_data.status);
    assert(ref_quota.current() == leg_data.current_downloads);

    std::cout << "✅ EQUIVALENT (一致)" << std::endl;
}

int main() {
    std::cout << "\n==========================================================" << std::endl;
    std::cout << " 执行重构安全网测试 (Safety-Net Behavioral Preservation)" << std::endl;
    std::cout << "==========================================================\n" << std::endl;

    // 场景 1: 正常下载
    TestEquivalenceCase("HappyPath Normal Download", 3, 0, 5, "TOK123", "PWD666", "TOK123", "PWD666");

    // 场景 2: 状态非激活 (草稿/待审/过期)
    TestEquivalenceCase("Inactive Request Status", 1, 0, 5, "TOK123", "PWD666", "TOK123", "PWD666");

    // 场景 3: Token 错误
    TestEquivalenceCase("Invalid Token", 3, 0, 5, "TOK123", "PWD666", "WRONG_TOK", "PWD666");

    // 场景 4: 提取码错误
    TestEquivalenceCase("Invalid Passcode", 3, 0, 5, "TOK123", "PWD666", "TOK123", "WRONG_PWD");

    // 场景 5: 已达上限直接拦截
    TestEquivalenceCase("Exceeded Limit Direct Reject", 3, 5, 5, "TOK123", "PWD666", "TOK123", "PWD666");

    // 场景 6: 刚好第 5 次下载触发熔断 (4 -> 5)
    TestEquivalenceCase("Exact Meltdown Transition (4->5)", 3, 4, 5, "TOK123", "PWD666", "TOK123", "PWD666");

    std::cout << "\n==========================================================" << std::endl;
    std::cout << " 🏆 恭喜！6 大业务场景重构前后输出 100% 保持等价，安全无损！" << std::endl;
    std::cout << "==========================================================\n" << std::endl;

    return 0;
}
