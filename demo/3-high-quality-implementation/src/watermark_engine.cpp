#include "filegate/watermark_engine.hpp"
#include <sstream>

namespace filegate::security {

// ============================================================================
// NVI 契约入口
// ============================================================================
types::Result<std::vector<uint8_t>> IWatermarkStrategy::Inject(
    const std::vector<uint8_t>& input_data,
    const WatermarkPayload& payload) 
{
    if (input_data.empty()) {
        return types::Result<std::vector<uint8_t>>::Error(
            types::ErrorCode::kInvalidArgument, "Input data is empty");
    }
    if (payload.client_ip.empty()) {
        return types::Result<std::vector<uint8_t>>::Error(
            types::ErrorCode::kInvalidArgument, "Client IP is empty");
    }
    return DoInject(input_data, payload);
}

// ============================================================================
// 显式水印实现 (追加易读水印信息头与元数据标签)
// ============================================================================
types::Result<std::vector<uint8_t>> VisibleTextWatermarkStrategy::DoInject(
    const std::vector<uint8_t>& input_data,
    const WatermarkPayload& payload) 
{
    std::ostringstream oss;
    oss << "\n[VISIBLE-WATERMARK: User=" << payload.applicant_id.get()
        << " IP=" << payload.client_ip
        << " Time=" << payload.request_timestamp.get()
        << " Label=" << payload.security_label << "]\n";

    std::string tag = oss.str();
    std::vector<uint8_t> output = input_data;
    output.insert(output.end(), tag.begin(), tag.end());
    return types::Result<std::vector<uint8_t>>::Success(std::move(output));
}

// ============================================================================
// 频域 DCT 盲水印实现 (仿真：按位在低频哈希块进行微扰嵌入，不破坏原格式外观)
// ============================================================================
types::Result<std::vector<uint8_t>> BlindDctWatermarkStrategy::DoInject(
    const std::vector<uint8_t>& input_data,
    const WatermarkPayload& payload) 
{
    std::vector<uint8_t> output = input_data;
    uint64_t seed = payload.applicant_id.get() ^ payload.request_timestamp.get();

    // 仿真频域低频系数调制：每隔 64 字节微调最低有效位 (LSB)
    for (size_t i = 0; i < output.size(); i += 64) {
        uint8_t bit = static_cast<uint8_t>((seed >> ((i / 64) % 64)) & 1);
        output[i] = static_cast<uint8_t>((output[i] & ~1) | bit);
    }
    return types::Result<std::vector<uint8_t>>::Success(std::move(output));
}

// ============================================================================
// WatermarkEngine 门面实现
// ============================================================================
WatermarkEngine::WatermarkEngine(std::unique_ptr<IWatermarkStrategy> strategy) noexcept 
    : strategy_(std::move(strategy)) {}

void WatermarkEngine::SetStrategy(std::unique_ptr<IWatermarkStrategy> strategy) noexcept {
    strategy_ = std::move(strategy);
}

types::Result<std::vector<uint8_t>> WatermarkEngine::Process(
    const std::vector<uint8_t>& input_data,
    const WatermarkPayload& payload) const 
{
    if (!strategy_) {
        return types::Result<std::vector<uint8_t>>::Error(
            types::ErrorCode::kInvalidState, "Watermark strategy not set");
    }
    return strategy_->Inject(input_data, payload);
}

} // namespace filegate::security
