# CRTP, Metaprogramming, and Empty Base Optimization

This reference covers compile-time polymorphism, mixin composition, memory layout optimizations, and metaprogramming patterns from *More C++ Idioms*.

---

## 1. Curiously Recurring Template Pattern (CRTP)

### Intent & Problem
Runtime polymorphism via virtual functions incurs:
1. Extra pointer storage (`vptr`) per object instance (8 bytes on 64-bit systems).
2. Indirect jump via `vtable` pointer dereference, inhibiting inlining, vectorization, and compiler branch prediction.
3. Heap allocations when storing polymorphic hierarchies without value semantics.

When polymorphic behaviors are strictly known at compile time, CRTP achieves polymorphism with **zero runtime overhead**.

### Core Architecture & Mechanics
A derived class inherits from a template base parameterized by the derived class itself:

```cpp
#include <iostream>
#include <span>

template <typename Derived>
class NumericSeries {
public:
    // Non-virtual static dispatch
    double calculate(double input) {
        // Enforce static contract via static_cast
        return static_cast<Derived*>(this)->do_calculate(input);
    }

    void print_result(double input) {
        std::cout << "Computed value: " << calculate(input) << '\n';
    }

protected:
    NumericSeries() = default;
    ~NumericSeries() = default; // Protected non-virtual prevents slicing via base pointer
};

class FibonacciSeries final : public NumericSeries<FibonacciSeries> {
public:
    double do_calculate(double input) {
        // Fast concrete algorithm, completely inlined
        return input * 1.618;
    }
};

class PowerSeries final : public NumericSeries<PowerSeries> {
public:
    double do_calculate(double input) {
        return input * input;
    }
};
```

### CRTP as a Mixin Class
CRTP can inject functionality (such as comparison operators, serializability, or ref counting) into concrete types without manual boilerplate:

```cpp
template <typename Derived>
class AddEqualityOperators {
    friend bool operator!=(const Derived& lhs, const Derived& rhs) noexcept {
        return !(lhs == rhs);
    }
};

class Point : public AddEqualityOperators<Point> {
public:
    int x{0};
    int y{0};

    friend bool operator==(const Point& lhs, const Point& rhs) noexcept {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
    // operator!= is automatically injected at zero cost!
};
```

*(Note: In modern C++20, `operator<=>` (the spaceship operator) automatically generates `==` and `!=`, but CRTP mixins remain vital for domain behaviors like Cloneable, Visitable, and Observable).*

---

## 2. Barton-Nackman Trick & Hidden Friends

### Intent
Define non-member template operators or utility functions inside a template class definition as `friend` functions.
- The function is injected into the enclosing namespace.
- It is found exclusively via **Argument Dependent Lookup (ADL)**.
- It is instantiated **only** when called for that specific instantiated type, avoiding premature or unwanted overload resolution candidates.

```cpp
template <typename T>
class ValueWrapper {
public:
    explicit ValueWrapper(T val) : value_(val) {}

    // Barton-Nackman "Hidden Friend"
    // Inlined inside class, found ONLY by ADL
    friend ValueWrapper operator+(const ValueWrapper& a, const ValueWrapper& b) {
        return ValueWrapper(a.value_ + b.value_);
    }

private:
    T value_;
};
```

---

## 3. Empty Base Optimization (EBO)

### Intent & Problem
In C++, an empty struct or class (with no non-static data members, no virtual functions) still requires a minimum size of at least **1 byte** to ensure distinct memory addresses for distinct objects.

When an empty policy or allocator is included as a regular data member, alignment padding expands the enclosing class:

```cpp
struct StatelessPolicy {}; // sizeof = 1

struct InefficientContainer {
    int* ptr;               // 8 bytes
    StatelessPolicy policy; // 1 byte + 7 bytes padding!
    // Total size = 16 bytes! 50% wasted space.
};
```

### Classical EBO Solution
Inherit privately from the empty class. The C++ standard allows a base class subobject of empty type to occupy **0 bytes**:

```cpp
class EfficientContainer : private StatelessPolicy {
    int* ptr; // 8 bytes
    // Total size = 8 bytes! Zero overhead for policy.
};
```

### Modern C++20 Alternative: `[[no_unique_address]]`
In C++20 and later, avoid inheritance boilerplate by using the attribute `[[no_unique_address]]`:

```cpp
struct ModernEfficientContainer {
    int* ptr;
    [[no_unique_address]] StatelessPolicy policy; // Takes 0 bytes when empty!
    // Total size = 8 bytes.
};
```

---

## 4. Expression Templates

### Intent & Problem
In scientific computing and vector math, chained operations like `v1 = v2 + v3 + v4` generate temporary objects for each intermediate addition (`v2 + v3`, then `res + v4`), causing multiple redundant heap allocations and cache evictions.

### Solution
Instead of performing calculations immediately, return a lightweight proxy template representing the abstract syntax tree (AST) of the computation. The actual loop evaluates lazily upon assignment to the destination vector:

```cpp
template <typename E>
class VecExpression {
public:
    double operator[](std::size_t i) const {
        return static_cast<const E&>(*this)[i];
    }
    std::size_t size() const {
        return static_cast<const E&>(*this).size();
    }
};

template <typename E1, typename E2>
class VecSum : public VecExpression<VecSum<E1, E2>> {
    const E1& u_;
    const E2& v_;
public:
    VecSum(const E1& u, const E2& v) : u_(u), v_(v) {}
    double operator[](std::size_t i) const { return u_[i] + v_[i]; }
    std::size_t size() const { return u_.size(); }
};

template <typename E1, typename E2>
VecSum<E1, E2> operator+(const VecExpression<E1>& u, const VecExpression<E2>& v) {
    return VecSum<E1, E2>(static_cast<const E1&>(u), static_cast<const E2&>(v));
}
```

When evaluated, a single tightly fused loop runs without any intermediate vector allocations!
