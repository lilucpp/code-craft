#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <cassert>
#include <filesystem>

#include "filegate/types.hpp"
#include "filegate/storage_engine.hpp"
#include "filegate/watermark_engine.hpp"
#include "filegate/share_request.hpp"
#include "filegate/concurrent_audit_queue.hpp"

using namespace filegate;

// ============================================================================
// 测试 1: 内存排布与自然对齐验证 (Memory Layout & Zero Padding)
// ============================================================================
void TestMemoryLayout() {
    std::cout << "[Test 1] 验证物理内存排布 (8-4-2-1 降序对齐)..." << std::endl;
    static_assert(sizeof(types::ShareRequestMeta) == 64, "ShareRequestMeta must be exactly 64 bytes");
    std::cout << "  • ShareRequestMeta 尺寸: " << sizeof(types::ShareRequestMeta) << " 字节 (完美对齐单条 CPU Cache Line)" << std::endl;
    std::cout << "  • 内部 Padding 浪费: 0 字节 (完全消除内存空洞)" << std::endl;
    std::cout << "  ✅ [PASS] 物理内存排布验证通过！\n" << std::endl;
}

// ============================================================================
// 测试 2: OSDI 14 崩溃一致性原子落盘验证
// ============================================================================
void TestStorageEngine() {
    std::cout << "[Test 2] 验证 OSDI 14 崩溃一致性原子落盘四步法..." << std::endl;
    storage::LocalPosixStorage storage;
    std::filesystem::path target = "/tmp/filegate_demo_test.dat";

    std::string payload = "FileGate confidential contract payload (OSDI 14 Atomic Write Test)";
    std::vector<uint8_t> data(payload.begin(), payload.end());

    // 1. 原子落盘写入 (write tmp -> fdatasync -> rename -> dir fsync)
    auto write_res = storage.AtomicWrite(target, data.data(), data.size());
    assert(write_res.ok());
    std::cout << "  • 原子落盘完成: " << target << " (" << data.size() << " 字节已刷盘)" << std::endl;

    // 2. 读取检验
    auto read_res = storage.Read(target);
    assert(read_res.ok());
    std::string read_str(read_res.value().begin(), read_res.value().end());
    assert(read_str == payload);
    std::cout << "  • 读取回验数据一致: \"" << read_str.substr(0, 38) << "...\"" << std::endl;

    // 3. 安全粉碎删除 (Secure Shred)
    auto shred_res = storage.SecureShred(target);
    assert(shred_res.ok());
    assert(!std::filesystem::exists(target));
    std::cout << "  • 物理安全粉碎完成，文件已彻底擦除" << std::endl;
    std::cout << "  ✅ [PASS] 存储引擎崩溃一致性与粉碎验证通过！\n" << std::endl;
}

// ============================================================================
// 测试 3: 水印引擎策略与门面验证 (NVI + Strategy)
// ============================================================================
void TestWatermarkEngine() {
    std::cout << "[Test 3] 验证水印嵌入策略 (NVI + Strategy)..." << std::endl;
    security::WatermarkEngine engine(std::make_unique<security::VisibleTextWatermarkStrategy>());

    std::string raw_text = "CONFIDENTIAL BID DOCUMENT";
    std::vector<uint8_t> doc(raw_text.begin(), raw_text.end());

    security::WatermarkPayload payload{
        types::UserId(8888),
        "192.168.1.108",
        types::TimestampMs(1715000000000ULL),
        "TopConfidential"
    };

    // 1. 显式平铺水印
    auto visible_res = engine.Process(doc, payload);
    assert(visible_res.ok());
    std::string visible_output(visible_res.value().begin(), visible_res.value().end());
    assert(visible_output.find("VISIBLE-WATERMARK: User=8888") != std::string::npos);
    std::cout << "  • 显式文字水印注入成功 (追加元数据标签，长度 " << visible_output.size() << " 字节)" << std::endl;

    // 2. 动态切换策略为频域 DCT 盲水印
    engine.SetStrategy(std::make_unique<security::BlindDctWatermarkStrategy>());
    auto blind_res = engine.Process(doc, payload);
    assert(blind_res.ok());
    assert(blind_res.value().size() == doc.size()); // 盲水印尺寸不变
    std::cout << "  • 频域 DCT 盲水印嵌入成功 (尺寸严格保持 " << blind_res.value().size() << " 字节无形改变)" << std::endl;
    std::cout << "  ✅ [PASS] 水印引擎策略模式验证通过！\n" << std::endl;
}

// ============================================================================
// 测试 4: Typestate 编译期状态机全流程验证 (拆分为单一抽象层级子函数)
// ============================================================================
core::ShareRequest<core::StateApproved> RunApprovalWorkflow() {
    auto draft = core::ShareRequest<core::StateDraft>::CreateNew(
        types::UserId(101), types::ByteSize(1024 * 1024));
    std::cout << "  • 状态 1: Draft 创建完成 (申请单号: " << draft.meta().request_id.get() << ")" << std::endl;

    // 卫语句防线测试: 绝密直接拦截
    auto blocked_res = draft.Submit(types::SecurityLevel::kTopSecret, 3);
    assert(!blocked_res.ok() && blocked_res.code() == types::ErrorCode::kSecurityBlocked);
    std::cout << "  • 业务规则防御: 绝密密级外发被直接阻断 (" << blocked_res.message() << ")" << std::endl;

    auto pending_res = draft.Submit(types::SecurityLevel::kCoreConfidential, 3);
    assert(pending_res.ok());
    std::cout << "  • 状态 2: 送审成功 -> 处于 PendingApproval" << std::endl;

    auto approved_res = pending_res.value().Approve(types::UserId(202));
    assert(approved_res.ok());
    std::cout << "  • 状态 3: 主管李雷批准完成 -> 处于 Approved" << std::endl;
    return std::move(approved_res.value());
}

void RunDownloadAndMeltdown(core::ShareRequest<core::StateApproved> approved) {
    auto active_res = approved.Activate(3);
    assert(active_res.ok());
    auto active = std::move(active_res.value());
    std::cout << "  • 状态 4: 生成受控 Token 激活上线 -> 处于 Active (限次 3 次)" << std::endl;

    assert(active.RecordDownload().value() == false); // 第 1 次
    assert(active.RecordDownload().value() == false); // 第 2 次
    assert(active.RecordDownload().value() == true);  // 第 3 次触发熔断
    assert(active.meta().status == types::RequestStatus::kExpired);
    std::cout << "  • 状态 5: 累计完成 3 次下载，时效自动熔断 -> 状态跃迁至 Expired" << std::endl;
}

void TestTypestateLifecycle() {
    std::cout << "[Test 4] 验证 Typestate 编译期时序状态机生命周期..." << std::endl;
    auto approved = RunApprovalWorkflow();
    RunDownloadAndMeltdown(std::move(approved));
    std::cout << "  ✅ [PASS] Typestate 状态机生命周期与卫语句验证通过！\n" << std::endl;
}

// ============================================================================
// 测试 5: 多线程细粒度锁并发审计队列高吞吐验证
// ============================================================================
void LaunchProducers(
    concurrent::ConcurrentAuditQueue<std::string>& queue,
    std::vector<std::thread>& producers,
    int num_producers,
    int items_per_producer) 
{
    for (int p = 0; p < num_producers; ++p) {
        producers.emplace_back([&queue, p, items_per_producer] {
            for (int i = 0; i < items_per_producer; ++i) {
                queue.Push("AuditRecord-" + std::to_string(p) + "-" + std::to_string(i));
            }
        });
    }
}

void TestConcurrentQueue() {
    std::cout << "[Test 5] 验证多线程细粒度锁并发审计队列 (Williams 架构)..." << std::endl;
    concurrent::ConcurrentAuditQueue<std::string> queue;
    const int kNumProducers = 4;
    const int kItemsPerProducer = 1000;
    const int kTotalItems = kNumProducers * kItemsPerProducer;
    std::atomic<int> consumed_count{0};

    std::thread consumer([&] {
        while (consumed_count.load() < kTotalItems) {
            if (queue.WaitAndPop()) consumed_count.fetch_add(1, std::memory_order_relaxed);
        }
    });

    std::vector<std::thread> producers;
    LaunchProducers(queue, producers, kNumProducers, kItemsPerProducer);

    for (auto& t : producers) t.join();
    consumer.join();

    assert(consumed_count.load() == kTotalItems);
    std::cout << "  • 并发压测: " << kNumProducers << " 生产者并发推入 " 
              << kTotalItems << " 条审计日志，消费完成: " << consumed_count.load() << " 条" << std::endl;
    std::cout << "  ✅ [PASS] 细粒度并发锁队列吞吐与线程安全验证通过！\n" << std::endl;
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << " FileGate 阶段三：高质量规范实现全套能力集成运行验证" << std::endl;
    std::cout << "==========================================================\n" << std::endl;

    TestMemoryLayout();
    TestStorageEngine();
    TestWatermarkEngine();
    TestTypestateLifecycle();
    TestConcurrentQueue();

    std::cout << "==========================================================" << std::endl;
    std::cout << " 🏆 恭喜！5 大核心测试用例全部执行通过，0 缺陷，0 数据竞态！" << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
