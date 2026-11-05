#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <chrono>
#include <sstream>
#include <exception>
#include <cmath>

namespace filegate::testing {

// ============================================================================
// Lightweight Modern C++ Testing Scaffolding (GoogleTest / Catch2 风格轻量脚手架)
// 遵循 AAA 模式、彩色终端输出与边界断言，零三方外部网络依赖
// ============================================================================

struct TestCase {
    std::string suite_name;
    std::string test_name;
    std::function<void()> func;
};

class TestRegistry {
public:
    static TestRegistry& Instance() {
        static TestRegistry registry;
        return registry;
    }

    void Register(std::string suite, std::string name, std::function<void()> f) {
        tests_.push_back({std::move(suite), std::move(name), std::move(f)});
    }

    const std::vector<TestCase>& GetTests() const { return tests_; }

private:
    std::vector<TestCase> tests_;
};

struct TestRegistrar {
    TestRegistrar(const char* suite, const char* name, std::function<void()> f) {
        TestRegistry::Instance().Register(suite, name, std::move(f));
    }
};

#define FG_TEST_CONCAT_INNER(a, b) a##b
#define FG_TEST_CONCAT(a, b) FG_TEST_CONCAT_INNER(a, b)

#define TEST(SuiteName, TestName)                                                 \
    void FG_TEST_CONCAT(Test_##SuiteName##_, TestName)();                          \
    static ::filegate::testing::TestRegistrar FG_TEST_CONCAT(reg_##SuiteName##_, TestName)( \
        #SuiteName, #TestName, FG_TEST_CONCAT(Test_##SuiteName##_, TestName));    \
    void FG_TEST_CONCAT(Test_##SuiteName##_, TestName)()

// 断言宏定义
#define EXPECT_TRUE(cond)                                                          \
    do {                                                                           \
        if (!(cond)) {                                                             \
            throw std::runtime_error("Assertion Failure: " #cond " is FALSE at "   \
                                     __FILE__ ":" + std::to_string(__LINE__));     \
        }                                                                          \
    } while (0)

#define EXPECT_FALSE(cond) EXPECT_TRUE(!(cond))

#define EXPECT_EQ(val1, val2)                                                      \
    do {                                                                           \
        if ((val1) != (val2)) {                                                    \
            std::ostringstream ss;                                                 \
            ss << "Equality Failure: " #val1 " (" << (val1) << ") != " #val2       \
               << " (" << (val2) << ") at " __FILE__ ":" << __LINE__;              \
            throw std::runtime_error(ss.str());                                    \
        }                                                                          \
    } while (0)

#define EXPECT_NE(val1, val2)                                                      \
    do {                                                                           \
        if ((val1) == (val2)) {                                                    \
            std::ostringstream ss;                                                 \
            ss << "Inequality Failure: " #val1 " (" << (val1) << ") == " #val2     \
               << " at " __FILE__ ":" << __LINE__;                                 \
            throw std::runtime_error(ss.str());                                    \
        }                                                                          \
    } while (0)

#define EXPECT_THROW(stmt, ExceptionType)                                          \
    do {                                                                           \
        bool caught = false;                                                       \
        try {                                                                      \
            stmt;                                                                  \
        } catch (const ExceptionType&) {                                           \
            caught = true;                                                         \
        } catch (...) {                                                            \
            throw std::runtime_error("Unexpected exception type thrown at "        \
                                     __FILE__ ":" + std::to_string(__LINE__));     \
        }                                                                          \
        if (!caught) {                                                             \
            throw std::runtime_error("Expected exception " #ExceptionType          \
                                     " not thrown at " __FILE__ ":" +              \
                                     std::to_string(__LINE__));                    \
        }                                                                          \
    } while (0)

// 测试运行器
inline int RunAllTests() {
    const auto& tests = TestRegistry::Instance().GetTests();
    int passed = 0;
    int failed = 0;

    std::cout << "\n==========================================================\n";
    std::cout << " [GoogleTest/Catch2 Scaffolding] 执行单元测试套件\n";
    std::cout << " 待执行测试用例总数: " << tests.size() << " 个\n";
    std::cout << "==========================================================\n\n";

    auto start_all = std::chrono::high_resolution_clock::now();

    for (const auto& t : tests) {
        std::cout << "[ RUN      ] " << t.suite_name << "." << t.test_name << "\n";
        auto t_start = std::chrono::high_resolution_clock::now();
        try {
            t.func();
            auto t_end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();
            std::cout << "[       OK ] " << t.suite_name << "." << t.test_name 
                      << " (" << ms << " us)\n";
            passed++;
        } catch (const std::exception& e) {
            std::cout << "[  FAILED  ] " << t.suite_name << "." << t.test_name 
                      << " -> " << e.what() << "\n";
            failed++;
        }
    }

    auto end_all = std::chrono::high_resolution_clock::now();
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_all - start_all).count();

    std::cout << "\n----------------------------------------------------------\n";
    std::cout << "[==========] 运行完成: " << tests.size() << " 个用例 (" << total_ms << " ms)\n";
    std::cout << "[  PASSED  ] " << passed << " 个用例通过\n";
    if (failed > 0) {
        std::cout << "[  FAILED  ] " << failed << " 个用例失败！\n";
    }
    std::cout << "==========================================================\n\n";

    return failed == 0 ? 0 : 1;
}

} // namespace filegate::testing
