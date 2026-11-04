# CMake Scaffolding, Test Toolchains, and Coverage

This reference details production-grade CMake test harness configuration, declarative `FetchContent` dependency management for GoogleTest and Catch2, fmt-style macro test abstractions, LLVM Sanitizers, and code coverage tooling.

---

## 1. Declarative Test Framework Integration (CMake `FetchContent`)

By utilizing CMake's `FetchContent` module, tests build in a self-contained, reproducible manner across all developer workstations and CI runners without requiring pre-installed system packages.

### 1. GoogleTest & GoogleMock Setup
```cmake
cmake_minimum_required(VERSION 3.20)
project(EngineCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Build unit tests" ON)

if(BUILD_TESTING)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0 # Release tag
    )
    # Prevent GoogleTest from installing into system dirs when packaging parent
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    include(GoogleTest)
endif()
```

### 2. Catch2 (v3) Setup
```cmake
if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.2
    )
    FetchContent_MakeAvailable(Catch2)

    include(Catch)
endif()
```

---

## 2. fmt-Style Scalable Test Macro Function

In large projects (like `fmtlib/fmt` and `abseil-cpp`), writing repetitive `add_executable`, `target_link_libraries`, and `add_test` for dozens of test files is error-prone. Introduce a reusable CMake function:

```cmake
# Usage: add_project_test(test_name sources...)
function(add_project_test test_name)
    add_executable(${test_name} ${ARGN})
    
    target_link_libraries(${test_name} PRIVATE
        ${PROJECT_NAME}_lib
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
    )

    # Automatically discover individual test cases for CTest
    gtest_discover_tests(${test_name}
        PROPERTIES
        TIMEOUT 30 # 30-second per-test timeout
    )
endfunction()

# Adding tests is now a single readable line:
add_project_test(test_order_service tests/test_order_service.cpp)
add_project_test(test_memory_pool  tests/test_memory_pool.cpp)
```

---

## 3. Dual Sanitizers Configuration (ASan + UBSan)

AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan) should be enabled by default in all local Debug test runs and CI test steps.

```cmake
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" ON)

if(ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "[Security] AddressSanitizer and UBSan ENABLED")
    add_compile_options(
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -fno-common
        -g
    )
    add_link_options(-fsanitize=address,undefined)
endif()
```

### Critical Compiler Flags Explained
- `-fsanitize=address`: Intercepts `malloc`/`free`, creates redzones around allocations, tracks stack frames, and discovers heap/stack buffer overflows, use-after-free, double-free, and leaks.
- `-fsanitize=undefined`: Catches signed integer overflow, null pointer dereference, misaligned pointer reads, invalid enum casts, and zero-division.
- `-fno-omit-frame-pointer`: Keeps frame pointer in register `%rbp` to guarantee pristine, readable stack traces in crash logs.

---

## 4. Code Coverage Tooling (lcov & llvm-cov)

### 1. GCC / gcov Coverage
```cmake
option(ENABLE_COVERAGE "Enable gcov code coverage instrumentation" OFF)

if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "[Coverage] Gcov coverage instrumentation ENABLED")
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()
```

### 2. Generating Coverage Reports with `lcov`
```bash
# 1. Clean previous counters
lcov --zerocounters --directory .

# 2. Run all tests via CTest
ctest --output-on-failure

# 3. Capture coverage data
lcov --capture --directory . --output-file coverage.raw.info

# 4. Remove external noise (system headers, test code itself, third-party libraries)
lcov --remove coverage.raw.info \
    '/usr/*' \
    '*/third_party/*' \
    '*/tests/*' \
    '*/_deps/*' \
    --output-file coverage.filtered.info

# 5. Generate human-readable HTML report
genhtml coverage.filtered.info --output-directory coverage_html

# 6. Verify coverage threshold gate (e.g. Fail if < 80%)
lcov --summary coverage.filtered.info
```
