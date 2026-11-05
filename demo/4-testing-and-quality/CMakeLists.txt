cmake_minimum_required(VERSION 3.16)
project(filegate_tests_suite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 基础编译选项
add_compile_options(-Wall -Wextra -Wpedantic -pthread)

# 包含路径: 阶段三源码 include + 测试脚手架 include
set(PHASE3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../3-high-quality-implementation")
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PHASE3_DIR}/include
)

# 阶段三待测源文件
set(CORE_SOURCES
    ${PHASE3_DIR}/src/storage_engine.cpp
    ${PHASE3_DIR}/src/watermark_engine.cpp
)

# 测试源文件
set(TEST_SOURCES
    tests/main.cpp
    tests/test_storage_engine.cpp
    tests/test_watermark_engine.cpp
    tests/test_typestate_lifecycle.cpp
    tests/test_concurrent_audit_queue.cpp
)

# 1. 常规测试目标 (Standard Unit Tests)
add_executable(filegate_tests
    ${CORE_SOURCES}
    ${TEST_SOURCES}
)
target_link_libraries(filegate_tests PRIVATE pthread)

# 2. 内存消毒测试目标 (ASan + UBSan - Sanitizers)
add_executable(filegate_tests_asan
    ${CORE_SOURCES}
    ${TEST_SOURCES}
)
target_compile_options(filegate_tests_asan PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1)
target_link_options(filegate_tests_asan PRIVATE -fsanitize=address,undefined)
target_link_libraries(filegate_tests_asan PRIVATE pthread)

# 3. 代码覆盖率测试目标 (gcov / llvm-cov Coverage)
add_executable(filegate_tests_coverage
    ${CORE_SOURCES}
    ${TEST_SOURCES}
)
target_compile_options(filegate_tests_coverage PRIVATE --coverage -fprofile-arcs -ftest-coverage -g -O0)
target_link_options(filegate_tests_coverage PRIVATE --coverage)
target_link_libraries(filegate_tests_coverage PRIVATE pthread)

# 启用 CTest
enable_testing()
add_test(NAME UnitTests COMMAND filegate_tests)
add_test(NAME SanitizerTests COMMAND filegate_tests_asan)
