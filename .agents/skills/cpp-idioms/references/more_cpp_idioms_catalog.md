# More C++ Idioms Catalog & Modern Evolution

This catalog maps idioms from the classic Wikibooks *More C++ Idioms* to modern C++ standards (C++11, C++14, C++17, C++20), categorizing them into:
- **Active & Essential**: Still vital architectural pillars in modern C++.
- **Modernized**: Transformed or simplified by modern C++ syntax/features.
- **Superseded / Built-in**: Directly replaced by standard language keywords or standard library features.

---

## 1. Complete Idioms Matrix

| # | Idiom Name | Status in Modern C++ | Modern Replacement / Best Practice |
|---|------------|----------------------|-----------------------------------|
| 1 | **Address Of** | Superseded | Use `std::addressof()` (avoids overloaded `operator&`). |
| 2 | **Algebraic Hierarchy** | Active | Multi-level arithmetic type hierarchies. |
| 3 | **Attach Execution Path** | Active | Injecting trace/log execution checkpoints. |
| 4 | **Attorney-Client** | **Essential** | Fine-grained `friend` access control without total exposure. |
| 5 | **Barton-Nackman Trick** | Modernized | "Hidden friends" defined inside class templates for ADL-only lookup. |
| 6 | **Base-from-Member** | Obsolete | C++11 non-static member initialization & delegated constructors. |
| 7 | **Boost.Mutant** | Obsolete | Undefined behavior in modern C++; use `std::bit_cast` or `std::variant`. |
| 8 | **Calling Virtuals During Init**| Active | Two-phase init or factory function returning `std::unique_ptr`. |
| 9 | **Capability Query** | Active | Dynamic interface querying via `dynamic_cast` cross-cast. |
| 10 | **Carrier Code** | Active | Transporting exception context across threads (`std::exception_ptr`). |
| 11 | **Checked Delete** | Superseded | `static_assert(sizeof(T) > 0)` or `std::default_delete<T>`. |
| 12 | **Clear-and-minimize** | Superseded | `container.shrink_to_fit()` (replaces `vector<T>().swap(v)`). |
| 13 | **Coercion by Member Template**| Active | Templated constructors enabling smart pointer implicit upcasting. |
| 14 | **Computational Constructor** | Active | Constructor executing heavy preprocessing or caching. |
| 15 | **Concrete Data Type (CDT)** | Active | Pure value-type classes with value semantics (Regular concept). |
| 16 | **Construct On First Use** | **Essential** | Function-local static variables ("Meyers' Singleton", thread-safe in C++11+). |
| 17 | **Construction Tracker** | Modernized | Tracking half-constructed state in constructors with multiple resources. |
| 18 | **Copy-and-Swap** | **Essential** | Strong exception safety and unified pass-by-value `operator=`. |
| 19 | **Copy-on-Write (COW)** | Superseded | COW strings forbidden in C++11 (`std::string` has SSO and move semantics). |
| 20 | **Counted Body** | Superseded | Built-in via `std::shared_ptr<T>`. |
| 21 | **CRTP** | **Essential** | Static polymorphism, zero-cost mixin composition, static interfaces. |
| 22 | **Curiously Recurring Template**| **Essential** | See CRTP. |
| 23 | **Empty Base Optimization** | Modernized | C++20 `[[no_unique_address]]` replaces inheritance gymnastics. |
| 24 | **Enable-if (SFINAE)** | Superseded | C++20 `requires` clauses and Concepts replace SFINAE boilerplate. |
| 25 | **Envelope-Letter** | Active | State pattern / Pimpl variant separating envelope from letter. |
| 26 | **Erasing Type Information** | **Essential** | Type Erasure (`std::function`, `std::any`, `std::move_only_function`). |
| 27 | **Execute-Around Pointer** | Active | Smart proxy intercepting member calls (logging, locking before/after). |
| 28 | **Export Template** | Obsolete | Removed from C++11 standard entirely; use C++20 Modules. |
| 29 | **Expression Templates** | Active | High-performance linear algebra / tensor libraries (Eigen, Blaze). |
| 30 | **Final Class** | Superseded | Replaced by language keyword `final` (`class X final {};`). |
| 31 | **Function Object** | Active | Lambdas (`[...](...) {}`) and custom functors. |
| 32 | **Generic Container Idioms** | Modernized | Standard STL containers, `std::span`, and C++20 Ranges. |
| 33 | **Hierarchy Generation** | Active | Typelist recursion or C++11 fold expressions expanding class hierarchies. |
| 34 | **Inline Guard Macro** | Superseded | `#pragma once` universally supported, C++20 Modules. |
| 35 | **Inner Class** | Active | Encapsulating implementation-detail classes inside outer classes. |
| 36 | **Int-to-type** | Superseded | `std::integral_constant<T, v>` in `<type_traits>`. |
| 37 | **Interface Class** | **Essential** | Pure virtual protocol classes with virtual destructors. |
| 38 | **Iterator Pair** | Modernized | Replaced by C++20 Ranges (`std::ranges::subrange`). |
| 39 | **Making New Friends** | Modernized | Hidden friend idiom via template class definitions. |
| 40 | **Member Detector** | Superseded | Replaced by C++20 Concepts (`requires(T t) { t.func(); }`). |
| 41 | **Move Constructor** | Superseded | Built-in C++11 language feature (`T(T&&) noexcept`). |
| 42 | **Multi-Statement Macro** | Active | `do { ... } while(0)` idiom for safe macro expansion. |
| 43 | **Named Constructor** | Active | Static factory methods (`create()`, `make()`, `from_rgb()`). |
| 44 | **Named External Argument** | Modernized | Designated initializers (`Widget{.width = 10, .height = 20}`). |
| 45 | **Named Loop** | Active | Labelled break/continue simulation via lambdas or structured control flow. |
| 46 | **Named Parameter** | Modernized | Method chaining (`Builder.with_timeout(...).with_retries(...)`). |
| 47 | **Non-copyable Mixin** | Superseded | `= delete` on copy constructor/assignment (`T(const T&) = delete;`). |
| 48 | **Non-member get** | Active | Custom ADL `get<I>(obj)` integration with structured binding. |
| 49 | **Non-throwing swap** | **Essential** | `friend void swap(T& a, T& b) noexcept` supporting Copy-and-Swap. |
| 50 | **Non-Virtual Interface (NVI)**| **Essential** | Template method pattern: public non-virtual + private virtual hooks. |
| 51 | **nullptr** | Superseded | Replaced by language literal `nullptr` (`std::nullptr_t`). |
| 52 | **Object Generator** | Superseded | Auto-deducing factories replaced by C++17 CTAD (`std::pair(1, 2.0)`). |
| 53 | **Object Lifetime Vault** | Active | Managing global/subsystem lifetime teardown order. |
| 54 | **Pimpl (Compilation Firewall)**| **Essential** | Pointer to implementation using `std::unique_ptr<Impl>`. |
| 55 | **Policy-based Design** | Active | Modernized with C++20 Concepts constraining template policies. |
| 56 | **Polymorphic Exception** | Active | Virtual `raise()` or `rethrow()` method in exception hierarchy. |
| 57 | **Polymorphic Hierarchy** | Active | Standard virtual OOP hierarchy with pure interfaces. |
| 58 | **RAII** | **Essential** | Resource Acquisition Is Initialization (smart pointers, guards). |
| 59 | **Reference Counting** | Superseded | Built-in via `std::shared_ptr<T>`. |
| 60 | **Resource Return** | Superseded | Modern move semantics replace raw pointer transfers. |
| 61 | **Return Type Resolver** | Active | Templated conversion operator deducing destination type. |
| 62 | **Safe Bool** | Superseded | `explicit operator bool() const noexcept` (C++11). |
| 63 | **Scope Guard** | **Essential** | Lambda-based cleanup execution on scope exit. |
| 64 | **Short-circuiting Overload** | Obsolete | Dangerous: never overload `operator&&` or `operator||`. |
| 65 | **Simulated Dynamic Binding** | Active | Double dispatch / Visitor pattern. |
| 66 | **Storage Class Tracker** | Active | Determining whether instance is on heap, stack, or static memory. |
| 67 | **Tag Dispatching** | Modernized | Replaced by `if constexpr` and C++20 Concepts. |
| 68 | **Temporary Base Class** | Active | Lifetime extension and temporary manipulation tricks. |
| 69 | **Temporary Proxy** | Active | Returning proxy for chained `operator[]` or lazy assignments. |
| 70 | **Thin Template** | Active | Factoring code into `void*` base class to eliminate binary bloat. |
| 71 | **Traits** | Modernized | `<type_traits>` standard library + C++20 Concepts. |
| 72 | **Type Erasure** | **Essential** | Non-intrusive runtime polymorphism with value semantics. |
| 73 | **Type Safe Enum** | Superseded | Replaced by C++11 scoped enums (`enum class Color : uint8_t`). |
| 74 | **Typelist** | Modernized | C++11 variadic templates replace nested recursive typelists. |
| 75 | **Virtual Constructor** | **Essential** | Virtual `clone()` method for polymorphic deep copying. |
| 76 | **Virtual Friend Function** | Active | Friend function delegating to private virtual member. |

---

## 2. Decision Tree: Which Idiom Should I Choose?

```
Need Polymorphism?
├── Compile-time known types & Zero-overhead required?
│   └── Choose: CRTP (Curiously Recurring Template Pattern)
├── Heterogeneous collections with Value Semantics (No Base Class)?
│   └── Choose: Type Erasure (Concept-Model-ValueWrapper)
└── Traditional OOP with Interface Invariants?
    └── Choose: Non-Virtual Interface (NVI)

Need Encapsulation & Member Control?
├── Fine-grained Friendship without leaking all private data?
│   └── Choose: Attorney-Client Idiom
└── Binary Compatibility & Fast Compilation?
    └── Choose: Pimpl Idiom (Compilation Firewall)

Need Safe Resource Lifecycle?
├── Exception-Safe Copy/Move Assignment?
│   └── Choose: Copy-and-Swap Idiom
├── Zero-cost empty policies/allocators?
│   └── Choose: C++20 [[no_unique_address]] or Empty Base Optimization
├── Elimination of Static Initialization Order Fiasco (SIOF)?
│   └── Choose: Construct On First Use (Meyers' Singleton)
└── Guaranteed scope cleanup/rollback?
    └── Choose: Scope Guard Idiom
```
