#pragma once

#include <string>
#include <vector>
#include <memory>
#include "filegate/types.hpp"

namespace filegate::security {

struct WatermarkPayload {
    types::UserId applicant_id;
    std::string client_ip;
    types::TimestampMs request_timestamp;
    std::string security_label;
};

// ============================================================================
// IWatermarkStrategy - 策略模式抽象基类 (应用 NVI 模式 - cpp-idioms)
// ============================================================================
class IWatermarkStrategy {
public:
    virtual ~IWatermarkStrategy() = default;

    // NVI 契约入口
    types::Result<std::vector<uint8_t>> Inject(
        const std::vector<uint8_t>& input_data,
        const WatermarkPayload& payload);

protected:
    virtual types::Result<std::vector<uint8_t>> DoInject(
        const std::vector<uint8_t>& input_data,
        const WatermarkPayload& payload) = 0;
};

// 策略 1: 显式半透明文字平铺水印
class VisibleTextWatermarkStrategy final : public IWatermarkStrategy {
protected:
    types::Result<std::vector<uint8_t>> DoInject(
        const std::vector<uint8_t>& input_data,
        const WatermarkPayload& payload) override;
};

// 策略 2: 频域 DCT 盲水印 (不可见频域嵌入仿真)
class BlindDctWatermarkStrategy final : public IWatermarkStrategy {
protected:
    types::Result<std::vector<uint8_t>> DoInject(
        const std::vector<uint8_t>& input_data,
        const WatermarkPayload& payload) override;
};

// ============================================================================
// WatermarkEngine - 对外统一门面服务 (Type Erasure / Smart Pointer 管理)
// ============================================================================
class WatermarkEngine {
public:
    explicit WatermarkEngine(std::unique_ptr<IWatermarkStrategy> strategy) noexcept;
    ~WatermarkEngine() = default;

    // 禁用拷贝，支持移动 (Scott Meyers Rule of 5)
    WatermarkEngine(const WatermarkEngine&) = delete;
    WatermarkEngine& operator=(const WatermarkEngine&) = delete;
    WatermarkEngine(WatermarkEngine&&) noexcept = default;
    WatermarkEngine& operator=(WatermarkEngine&&) noexcept = default;

    void SetStrategy(std::unique_ptr<IWatermarkStrategy> strategy) noexcept;

    types::Result<std::vector<uint8_t>> Process(
        const std::vector<uint8_t>& input_data,
        const WatermarkPayload& payload) const;

private:
    std::unique_ptr<IWatermarkStrategy> strategy_;
};

} // namespace filegate::security
