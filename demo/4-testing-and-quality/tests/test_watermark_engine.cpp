#include "test_framework.hpp"
#include "filegate/watermark_engine.hpp"
#include <string>
#include <vector>

using namespace filegate;

// ============================================================================
// 依赖隔离与 Mock 对象 (GMock 风格接口打桩与调用捕获 - cpp-testing)
// ============================================================================
class MockWatermarkStrategy final : public security::IWatermarkStrategy {
public:
    int call_count = 0;
    security::WatermarkPayload last_payload{};
    bool should_succeed = true;

protected:
    types::Result<std::vector<uint8_t>> DoInject(
        const std::vector<uint8_t>& input_data,
        const security::WatermarkPayload& payload) override 
    {
        call_count++;
        last_payload = payload;
        if (!should_succeed) {
            return types::Result<std::vector<uint8_t>>::Error(
                types::ErrorCode::kIoError, "Mock simulated inject failure");
        }
        std::vector<uint8_t> mocked = input_data;
        mocked.push_back(0xFF); // 模拟打桩字节
        return types::Result<std::vector<uint8_t>>::Success(std::move(mocked));
    }
};

// ============================================================================
// 测试套件 2: WatermarkEngineTest
// ============================================================================

TEST(WatermarkEngineTest, MockIsolationVerification) {
    // 1. Arrange: 注入 Mock 策略，隔离物理复杂算法
    auto mock = std::make_unique<MockWatermarkStrategy>();
    MockWatermarkStrategy* mock_ptr = mock.get();
    security::WatermarkEngine engine(std::move(mock));

    std::vector<uint8_t> input = {0x10, 0x20};
    security::WatermarkPayload payload{
        types::UserId(777), "10.0.0.1", types::TimestampMs(123456), "Confidential"
    };

    // 2. Act
    auto res = engine.Process(input, payload);

    // 3. Assert: 验证打桩调用捕获
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(mock_ptr->call_count, 1);
    EXPECT_EQ(mock_ptr->last_payload.applicant_id.get(), 777UL);
    EXPECT_EQ(mock_ptr->last_payload.client_ip, "10.0.0.1");
    EXPECT_EQ(res.value().size(), 3UL); // 原长2 + 1字节打桩
}

TEST(WatermarkEngineTest, BoundaryEmptyInputDataShouldFail) {
    // 1. Arrange
    security::WatermarkEngine engine(std::make_unique<security::VisibleTextWatermarkStrategy>());
    std::vector<uint8_t> empty_input;
    security::WatermarkPayload payload{types::UserId(1), "127.0.0.1", types::TimestampMs(0), "Label"};

    // 2. Act
    auto res = engine.Process(empty_input, payload);

    // 3. Assert: NVI 公有入口应当在调用具体策略前拦截空输入
    EXPECT_FALSE(res.ok());
    EXPECT_EQ(res.code(), types::ErrorCode::kInvalidArgument);
}

TEST(WatermarkEngineTest, BoundaryEmptyClientIpShouldFail) {
    // 1. Arrange
    security::WatermarkEngine engine(std::make_unique<security::VisibleTextWatermarkStrategy>());
    std::vector<uint8_t> input = {0x01};
    security::WatermarkPayload payload{types::UserId(1), "", types::TimestampMs(0), "Label"};

    // 2. Act
    auto res = engine.Process(input, payload);

    // 3. Assert
    EXPECT_FALSE(res.ok());
    EXPECT_EQ(res.code(), types::ErrorCode::kInvalidArgument);
}

TEST(WatermarkEngineTest, BlindDctEmbeddingPreservesLength) {
    // 1. Arrange
    security::WatermarkEngine engine(std::make_unique<security::BlindDctWatermarkStrategy>());
    std::vector<uint8_t> input(256, 0x80);
    security::WatermarkPayload payload{types::UserId(99), "172.16.0.5", types::TimestampMs(999), "Blind"};

    // 2. Act
    auto res = engine.Process(input, payload);

    // 3. Assert
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(res.value().size(), input.size()); // 盲水印不增加文件字节大小
}

TEST(WatermarkEngineTest, VisibleTextWatermarkSuccess) {
    // 1. Arrange
    security::WatermarkEngine engine(std::make_unique<security::VisibleTextWatermarkStrategy>());
    std::string text = "Confidential Contract";
    std::vector<uint8_t> input(text.begin(), text.end());
    security::WatermarkPayload payload{types::UserId(12345), "192.168.0.8", types::TimestampMs(100), "Confidential"};

    // 2. Act
    auto res = engine.Process(input, payload);

    // 3. Assert
    EXPECT_TRUE(res.ok());
    std::string output_str(res.value().begin(), res.value().end());
    EXPECT_TRUE(output_str.find("VISIBLE-WATERMARK: User=12345") != std::string::npos);
}

TEST(WatermarkEngineTest, StrategyNullAndSetStrategySuccess) {
    // 1. Arrange: 初始 strategy 为 nullptr
    security::WatermarkEngine engine(nullptr);
    std::vector<uint8_t> input = {0x01, 0x02};
    security::WatermarkPayload payload{types::UserId(1), "127.0.0.1", types::TimestampMs(0), "Label"};

    // 2. Act: 未设置策略时应报错
    auto fail_res = engine.Process(input, payload);
    EXPECT_FALSE(fail_res.ok());
    EXPECT_EQ(fail_res.code(), types::ErrorCode::kInvalidState);

    // 3. 动态配置策略后再执行
    engine.SetStrategy(std::make_unique<security::VisibleTextWatermarkStrategy>());
    auto success_res = engine.Process(input, payload);
    EXPECT_TRUE(success_res.ok());
}
