#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <iostream>

namespace filegate::legacy {

// ============================================================================
// 典型的遗留坏味道代码样本 (Legacy Code Smell Target)
// 包含典型的 Martin Fowler 代码坏味道:
// 1. Long Method (过长函数 > 80行)
// 2. Deep Nested Conditionals (4层嵌套金字塔)
// 3. Primitive Obsession (裸 uint64_t / 裸 string 充斥)
// 4. Feature Envy (过度依恋并直接操作外部元数据细节)
// 5. Divergent Change (同时承载鉴权、限次计算与通知格式化等多重变动原因)
// ============================================================================

struct LegacyRequestData {
    uint64_t req_id;
    uint64_t user_id;
    int sec_level; // 0=普通, 1=敏感, 2=商密, 3=绝密
    int status;    // 0=草稿, 1=待审, 2=批准, 3=激活, 4=过期
    uint32_t max_downloads;
    uint32_t current_downloads;
    std::string token;
    std::string passcode;
};

class LegacyTokenManager {
public:
    // 坏味道 1: 臃肿过长函数 (Long Method) + 坏味道 2: 嵌套条件金字塔 (Deep Nesting)
    bool ValidateAndProcessDownload(
        LegacyRequestData& req,
        uint64_t visitor_id,
        const std::string& input_token,
        const std::string& input_passcode,
        std::string& out_dispatch_log) 
    {
        // 第 1 层嵌套
        if (req.status == 3) { // 激活态
            // 第 2 层嵌套
            if (req.token == input_token && !input_token.empty()) {
                // 第 3 层嵌套 (基本类型校验)
                if (req.passcode == input_passcode && !input_passcode.empty()) {
                    // 第 4 层嵌套: 坏味道 4: Feature Envy - 直接介入外部计数与熔断判定
                    if (req.current_downloads < req.max_downloads) {
                        req.current_downloads++;
                        
                        // 坏味道 5: Divergent Change - 混合排版通知与日志格式化
                        out_dispatch_log = "DISPATCH_OK:Req=" + std::to_string(req.req_id) +
                                           ";User=" + std::to_string(req.user_id) +
                                           ";Visitor=" + std::to_string(visitor_id) +
                                           ";DownloadCount=" + std::to_string(req.current_downloads) +
                                           "/" + std::to_string(req.max_downloads);

                        if (req.current_downloads >= req.max_downloads) {
                            req.status = 4; // 状态变为过期
                            out_dispatch_log += ";MELTDOWN=TRUE";
                        }
                        return true;
                    } else {
                        req.status = 4;
                        out_dispatch_log = "ERROR_DOWNLOAD_LIMIT_EXCEEDED";
                        return false;
                    }
                } else {
                    out_dispatch_log = "ERROR_INVALID_PASSCODE";
                    return false;
                }
            } else {
                out_dispatch_log = "ERROR_INVALID_TOKEN";
                return false;
            }
        } else {
            out_dispatch_log = "ERROR_REQUEST_NOT_ACTIVE";
            return false;
        }
    }
};

} // namespace filegate::legacy
