#!/usr/bin/env bash
# ==============================================================================
# C++ 规范与静态分析配置一键初始化脚本 (Init C++ Tooling Configurations)
# ==============================================================================
set -euo pipefail

TARGET_DIR="${1:-.}"

echo "=================================================="
echo "🚀 正在初始化 C++ 代码风格与静态检查配置 -> $TARGET_DIR"
echo "=================================================="

# 1. 生成 .clang-format
CLANG_FORMAT_FILE="$TARGET_DIR/.clang-format"
if [ -f "$CLANG_FORMAT_FILE" ]; then
  echo "  ℹ️  .clang-format 已存在，跳过覆盖 (如需强制更新请手动删除)"
else
  cat << 'EOF' > "$CLANG_FORMAT_FILE"
---
# 基于 Google C++ 编码规范
BasedOnStyle: Google

# 1. 禁用指针位置推断，强制严格按照配置执行
DerivePointerAlignment: false

# 2. 指针和引用靠右对齐（例如：char *p; const string &ref;）
PointerAlignment: Right

# 3. 每行最大列宽限制为 120 字符（适合现代宽屏，提升可读性）
ColumnLimit: 120

# 常用配套优化（保持 Google 严谨性）
IndentWidth: 2
TabWidth: 2
UseTab: Never
Standard: Latest
AccessModifierOffset: -1
NamespaceIndentation: None
SortIncludes: CaseSensitive
IncludeBlocks: Regroup
FixNamespaceComments: true
ReflowComments: true
AllowShortFunctionsOnASingleLine: Inline
AllowShortBlocksOnASingleLine: Never
AllowShortCaseLabelsOnASingleLine: false
KeepEmptyLinesAtTheStartOfBlocks: false
EmptyLineBeforeAccessModifier: LogicalBlock
...
EOF
  echo "  ✅ 已创建: $CLANG_FORMAT_FILE (Google风格 / 右对齐 / 120列)"
fi

# 2. 生成 .clang-format-ignore
CLANG_FORMAT_IGNORE="$TARGET_DIR/.clang-format-ignore"
if [ -f "$CLANG_FORMAT_IGNORE" ]; then
  echo "  ℹ️  .clang-format-ignore 已存在，跳过覆盖"
else
  cat << 'EOF' > "$CLANG_FORMAT_IGNORE"
# 自动生成的代码与构建产物
build/**
cmake-build-*/**
bin/**
dist/**
*.pb.cc
*.pb.h

# 外部三方库目录（杜绝格式化第三方源码）
third_party/**
vendor/**
external/**
submodules/**
EOF
  echo "  ✅ 已创建: $CLANG_FORMAT_IGNORE (自动排除三方库与构建目录)"
fi

# 3. 生成 .clang-tidy
CLANG_TIDY_FILE="$TARGET_DIR/.clang-tidy"
if [ -f "$CLANG_TIDY_FILE" ]; then
  echo "  ℹ️  .clang-tidy 已存在，跳过覆盖"
else
  cat << 'EOF' > "$CLANG_TIDY_FILE"
---
# 启用工业级高信噪比 C++17 静态分析规则集
Checks: >
  -*,
  bugprone-*,
  cert-*,
  clang-analyzer-core.*,
  clang-analyzer-cplusplus.*,
  clang-analyzer-deadcode.*,
  clang-analyzer-nullability.*,
  clang-analyzer-security.*,
  -clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling,
  clang-analyzer-unix.*,
  clang-analyzer-optin.cplusplus.UninitializedObject,
  clang-analyzer-optin.performance.Padding,
  concurrency-*,
  cppcoreguidelines-*,
  -cppcoreguidelines-pro-bounds-pointer-arithmetic,
  -cppcoreguidelines-owning-memory,
  -cppcoreguidelines-avoid-magic-numbers,
  modernize-*,
  -modernize-use-trailing-return-type,
  performance-*,
  google-*,
  -google-runtime-references,
  -google-readability-function-size,
  readability-*,
  misc-*

# 将高危隐患与硬工程红线直接视作编译错误阻止合入
WarningsAsErrors: >
  bugprone-*,
  cert-*,
  clang-analyzer-core.*,
  clang-analyzer-cplusplus.*,
  clang-analyzer-security.*,
  clang-analyzer-unix.*,
  concurrency-*,
  cppcoreguidelines-explicit-virtual-functions,
  cppcoreguidelines-pro-type-cstyle-cast,
  cppcoreguidelines-virtual-class-destructor,
  google-explicit-constructor,
  misc-redundant-expression,
  modernize-use-nullptr,
  performance-*,
  readability-function-cognitive-complexity,
  readability-function-size

# 仅检查业务源码头文件，彻底屏蔽第三方库头文件产生的警告
HeaderFilterRegex: '^(src|include|modules|libs|packages|components)/.*'

# 检查项特定配置 (LLVM / clang-tidy CheckOptions 格式)
CheckOptions:
  bugprone-easily-swappable-parameters.MinimumLength: '2'
  performance-move-const-arg.CheckMoveToConstRef: 'true'
  performance-move-const-arg.CheckTriviallyCopyableMove: 'true'
  readability-function-cognitive-complexity.Threshold: '15'
  readability-function-size.LineThreshold: '60'
  readability-function-size.ParameterThreshold: '3'
  readability-function-size.NestingThreshold: '3'
  readability-identifier-length.IgnoredVariableNames: '^(id|fd|rc|ip|db|io|ok|x|y|z)$'
  readability-identifier-length.IgnoredParameterNames: '^(id|fd|rc|ip|db|io|ok|n|x|y|z)$'
  readability-magic-numbers.IgnorePowersOf2IntegerValues: 'true'
  readability-magic-numbers.IgnoredIntegerValues: '-1;0;1;2;'
...
EOF
  echo "  ✅ 已创建: $CLANG_TIDY_FILE (高信噪比强门禁 / 自动屏蔽三方库)"
fi

echo "=================================================="
echo "🎉 项目 C++ 代码质量规范配置初始化完毕！"
echo "=================================================="
