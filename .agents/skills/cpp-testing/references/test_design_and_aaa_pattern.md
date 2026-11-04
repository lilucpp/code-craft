# Test Case Design, AAA Pattern, and Boundary Matrices

This reference details the **Arrange-Act-Assert (AAA)** pattern, black-box testing principles, boundary value matrices, parameterized suites (Value & Type), Catch2 BDD style, and C++ Core Guidelines / Abseil exception safety assertions.

---

## 1. The AAA Pattern (Arrange-Act-Assert)

Every clean unit test must be structured into three visually distinguishable phases:

```cpp
#include <gtest/gtest.h>

TEST(ShoppingCartTest, CalculateTotalWithPercentageDiscount) {
    // 1. Arrange: Setup inputs, collaborators, and prerequisites
    ShoppingCart cart;
    cart.add_item("Book", 20.00);
    cart.add_item("Pen", 5.00);
    const double discount_percentage = 0.10; // 10% off

    // 2. Act: Execute the single behavior under test
    const double total = cart.calculate_discounted_total(discount_percentage);

    // 3. Assert: Verify the outcome and invariants
    EXPECT_DOUBLE_EQ(total, 22.50);
}
```

### Key Rules
- **Single Reason to Fail**: Each test should verify one specific behavior or invariant.
- **No Complex Control Flow**: Avoid loops, complex `if-else` branches, or try-catch blocks inside test functions; tests must be linear and self-evident.

---

## 2. Black-Box Testing vs. The "Friend" Anti-Pattern

> [!CAUTION]
> **Never declare unit test classes as `friend` to test private methods.**
> Testing private methods binds test code tightly to volatile internal implementation details. Refactoring internal helper functions will break tests even when the public contract remains 100% backward-compatible.

### Architectural Remedies
1. **Test Public Interfaces**: If a private function is complex enough to demand its own unit tests, it is a symptom of violated Single Responsibility Principle. Extract it into a separate collaborator class with its own clean public interface.
2. **Behavior over State**: Assert on public observable behavior, return values, and emitted side effects, not private member state.

---

## 3. Boundary Value Test Matrix

Systematically design test cases covering the 4 critical boundary zones:

| Boundary Class | Input Scenario | Expected Verification |
|---|---|---|
| **Empty Input** | Empty string `""`, empty `vector{}`, 0 bytes buffer, null `std::optional`. | Container reports size 0; graceful no-op without crashes. |
| **Single Element** | Array of 1 item, single character string, zero values (0, 0.0). | Valid computation without off-by-one errors. |
| **Extreme Limits** | `std::numeric_limits<T>::max()`, `min()`, large payloads. | Arithmetic overflow defense, memory exhaustion defense. |
| **Invalid / Error** | Negative amounts, malformed JSON, out-of-bounds indices. | Throws specific domain exception; preserves invariants. |

---

## 4. Parameterized Tests in GoogleTest

### 1. Value-Parameterized Tests (`TEST_P`)
Avoid duplicating test logic across multiple input vectors:

```cpp
#include <gtest/gtest.h>

struct LeapYearCase {
    int year;
    bool expected_is_leap;
};

class LeapYearTest : public testing::TestWithParam<LeapYearCase> {};

TEST_P(LeapYearTest, ValidatesLeapYearsCorrectly) {
    // Arrange
    const auto& [year, expected] = GetParam();

    // Act
    const bool actual = is_leap_year(year);

    // Assert
    EXPECT_EQ(actual, expected) << "Failed for year: " << year;
}

INSTANTIATE_TEST_SUITE_P(
    CalendarRules,
    LeapYearTest,
    testing::Values(
        LeapYearCase{2000, true},   // Divisible by 400 -> Leap
        LeapYearCase{1900, false},  // Divisible by 100 but not 400 -> Normal
        LeapYearCase{2024, true},   // Divisible by 4 -> Leap
        LeapYearCase{2023, false}   // Normal
    ),
    [](const testing::TestParamInfo<LeapYearCase>& info) {
        return "Year_" + std::to_string(info.param.year);
    }
);
```

### 2. Type-Parameterized Tests (`TYPED_TEST`)
Test generic templates against multiple concrete types (`std::vector`, `std::list`, `std::deque`):

```cpp
template <typename T>
class SequenceContainerTest : public testing::Test {};

using ContainerTypes = testing::Types<std::vector<int>, std::list<int>, std::deque<int>>;
TYPED_TEST_SUITE(SequenceContainerTest, ContainerTypes);

TYPED_TEST(SequenceContainerTest, PushBackIncreasesSize) {
    TypeParam container;
    container.push_back(10);
    EXPECT_EQ(container.size(), 1);
    EXPECT_EQ(container.front(), 10);
}
```

---

## 5. Catch2 BDD Style (Given-When-Then)

Catch2 provides natural-language BDD syntax for domain-driven testing:

```cpp
#include <catch2/catch_test_macros.hpp>

SCENARIO("Vector resizing behavior", "[vector]") {
    GIVEN("An empty integer vector") {
        std::vector<int> v;

        REQUIRE(v.empty());
        REQUIRE(v.size() == 0);

        WHEN("Elements are added") {
            v.push_back(10);
            v.push_back(20);

            THEN("The size increases accordingly") {
                REQUIRE(v.size() == 2);
                REQUIRE(v.front() == 10);
                REQUIRE(v.back() == 20);
            }
        }
    }
}
```

---

## 6. Exception Safety Assertions (Abseil & C++ Core Guidelines)

Based on Dave Abrahams' model and Google Abseil's `exception_safety_testing.h`:

### 1. Basic Guarantee Testing
Verify that when an operation throws, **no resources leak** (ASan verifies heap safety), and all invariants remain intact:
```cpp
EXPECT_THROW({
    service.transfer_funds(invalid_account, 100.0);
}, InvalidAccountException);
```

### 2. Strong Guarantee Testing (Commit-or-Rollback)
Verify that if an operation throws, the object state remains completely unmodified:
```cpp
TEST(DatabaseTransactionTest, RollbackStateOnException) {
    Database db;
    db.insert("balance", 500);
    const auto snapshot_before = db.snapshot();

    EXPECT_THROW({
        db.execute_failing_batch(); // Throws mid-transaction
    }, TransactionFailedException);

    // Strong guarantee: exact pre-operation snapshot restored!
    EXPECT_EQ(db.snapshot(), snapshot_before);
}
```

### 3. Nothrow Guarantee Testing
Critical operations (destructors, deallocation, swap, move constructors) must **never throw** (C++ Core Guidelines E.16):
```cpp
EXPECT_NO_THROW({
    using std::swap;
    swap(entity_a, entity_b);
});
static_assert(std::is_nothrow_move_constructible_v<Entity>);
```
