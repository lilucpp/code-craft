#include "test_framework.hpp"
#include "filegate/storage_engine.hpp"
#include <filesystem>
#include <vector>
#include <string>

using namespace filegate;

// ============================================================================
// 测试套件 1: StorageEngineTest (遵循 AAA 模式与 4 象限边界矩阵)
// ============================================================================

TEST(StorageEngineTest, BoundaryEmptyPathShouldFail) {
    // 1. Arrange (准备)
    storage::LocalPosixStorage storage;
    std::vector<uint8_t> dummy_data = {0x01, 0x02};

    // 2. Act (执行)
    auto result = storage.AtomicWrite("", dummy_data.data(), dummy_data.size());

    // 3. Assert (断言)
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.code(), types::ErrorCode::kInvalidArgument);
}

TEST(StorageEngineTest, BoundaryNullOrZeroLengthDataShouldFail) {
    // 1. Arrange
    storage::LocalPosixStorage storage;
    std::filesystem::path target = "/tmp/fg_null_test.dat";

    // 2. Act
    auto res_null = storage.AtomicWrite(target, nullptr, 10);
    auto res_zero = storage.AtomicWrite(target, reinterpret_cast<const uint8_t*>("a"), 0);

    // 3. Assert
    EXPECT_FALSE(res_null.ok());
    EXPECT_EQ(res_null.code(), types::ErrorCode::kInvalidArgument);
    EXPECT_FALSE(res_zero.ok());
    EXPECT_EQ(res_zero.code(), types::ErrorCode::kInvalidArgument);
}

TEST(StorageEngineTest, SingleBytePayloadWriteAndReadSuccess) {
    // 1. Arrange (单字节边界)
    storage::LocalPosixStorage storage;
    std::filesystem::path target = "/tmp/fg_single_byte.dat";
    std::vector<uint8_t> single_byte = {0x42};

    // 2. Act
    auto write_res = storage.AtomicWrite(target, single_byte.data(), single_byte.size());
    EXPECT_TRUE(write_res.ok());

    auto read_res = storage.Read(target);

    // 3. Assert
    EXPECT_TRUE(read_res.ok());
    EXPECT_EQ(read_res.value().size(), 1UL);
    EXPECT_EQ(read_res.value()[0], 0x42);

    // Cleanup
    storage.SecureShred(target);
    EXPECT_FALSE(std::filesystem::exists(target));
}

TEST(StorageEngineTest, LargeBufferAtomicWriteAndShredSuccess) {
    // 1. Arrange (1MB 极值测试)
    storage::LocalPosixStorage storage;
    std::filesystem::path target = "/tmp/fg_large_1mb.dat";
    const size_t kLargeSize = 1024 * 1024; // 1 MB
    std::vector<uint8_t> large_data(kLargeSize, 0xAA);

    // 2. Act
    auto write_res = storage.AtomicWrite(target, large_data.data(), large_data.size());
    EXPECT_TRUE(write_res.ok());

    auto read_res = storage.Read(target);

    // 3. Assert
    EXPECT_TRUE(read_res.ok());
    EXPECT_EQ(read_res.value().size(), kLargeSize);
    EXPECT_EQ(read_res.value()[0], 0xAA);
    EXPECT_EQ(read_res.value()[kLargeSize - 1], 0xAA);

    // Secure shred
    auto shred_res = storage.SecureShred(target);
    EXPECT_TRUE(shred_res.ok());
    EXPECT_FALSE(std::filesystem::exists(target));
}

TEST(StorageEngineTest, ReadNonExistentPathShouldFail) {
    // 1. Arrange (非法路径测试)
    storage::LocalPosixStorage storage;
    std::filesystem::path fake_path = "/tmp/fg_never_exist_file_9999.dat";

    // 2. Act
    auto read_res = storage.Read(fake_path);

    // 3. Assert
    EXPECT_FALSE(read_res.ok());
    EXPECT_EQ(read_res.code(), types::ErrorCode::kInvalidArgument);
}
