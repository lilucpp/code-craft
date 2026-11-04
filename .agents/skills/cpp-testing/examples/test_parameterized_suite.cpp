#include <gtest/gtest.h>

#include <deque>
#include <list>
#include <string>
#include <vector>

// =========================================================================
// 1. Value-Parameterized Test Suite (TEST_P)
// =========================================================================

struct TaxCalculationScenario {
    double gross_income;
    double tax_rate;
    double expected_tax;
};

class TaxCalculatorTest : public ::testing::TestWithParam<TaxCalculationScenario> {};

TEST_P(TaxCalculatorTest, CalculatesCorrectTaxForGivenIncomeBracket) {
    // 1. Arrange
    const auto& [income, rate, expected_tax] = GetParam();

    // 2. Act
    const double calculated = income * rate;

    // 3. Assert
    EXPECT_DOUBLE_EQ(calculated, expected_tax);
}

INSTANTIATE_TEST_SUITE_P(
    IncomeTaxScenarios,
    TaxCalculatorTest,
    ::testing::Values(
        TaxCalculationScenario{10000.0, 0.05, 500.0},
        TaxCalculationScenario{50000.0, 0.15, 7500.0},
        TaxCalculationScenario{100000.0, 0.25, 25000.0},
        TaxCalculationScenario{0.0, 0.0, 0.0}
    ),
    [](const ::testing::TestParamInfo<TaxCalculationScenario>& info) {
        return "Income_" + std::to_string(static_cast<long>(info.param.gross_income));
    }
);

// =========================================================================
// 2. Type-Parameterized Test Suite (TYPED_TEST)
// =========================================================================

template <typename T>
class SequenceContainerContractTest : public ::testing::Test {
protected:
    T container_;
};

using SupportedContainers = ::testing::Types<
    std::vector<int>,
    std::list<int>,
    std::deque<int>
>;

TYPED_TEST_SUITE(SequenceContainerContractTest, SupportedContainers);

TYPED_TEST(SequenceContainerContractTest, PushBackIncreasesSizeAndMaintainsFIFOOrder) {
    // 1. Arrange
    auto& container = this->container_;
    EXPECT_TRUE(container.empty());

    // 2. Act
    container.push_back(100);
    container.push_back(200);

    // 3. Assert
    EXPECT_EQ(container.size(), 2);
    EXPECT_EQ(container.front(), 100);
    EXPECT_EQ(container.back(), 200);
}

TYPED_TEST(SequenceContainerContractTest, ClearEmptiesTheContainer) {
    auto& container = this->container_;
    container.push_back(1);
    container.push_back(2);
    EXPECT_FALSE(container.empty());

    container.clear();

    EXPECT_TRUE(container.empty());
    EXPECT_EQ(container.size(), 0);
}
