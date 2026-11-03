#!/usr/bin/env bash
# ==============================================================================
# C++ 格式化与静态检查调度脚本 (Run C++ Checks: Diff vs All)
# ==============================================================================
set -euo pipefail

MODE="diff"      # diff (增量模式) 或 all (全量模式)
DO_FIX=false     # 是否自动应用修复 (clang-format -i)
TOOL="all"       # all, format, tidy, cppcheck
TARGET_DIR="."

show_help() {
  echo "用法: $0 [选项]"
  echo "选项:"
  echo "  --mode=diff      增量模式（默认）：仅检查 git diff 修改的文件/行（适合老项目）"
  echo "  --mode=all       全量模式：对项目中所有的业务源码执行全面检查"
  echo "  --fix            自动修复（应用 clang-format 排版）"
  echo "  --tool=<name>    指定运行工具：all (默认), format, tidy, cppcheck"
  echo "  --dir=<path>     指定目标项目根目录（默认当前目录）"
  echo "  -h, --help       显示帮助信息"
  exit 0
}

for arg in "$@"; do
  case "$arg" in
    --mode=diff) MODE="diff" ;;
    --mode=all)  MODE="all" ;;
    --fix)       DO_FIX=true ;;
    --tool=*)    TOOL="${arg#*=}" ;;
    --dir=*)     TARGET_DIR="${arg#*=}" ;;
    -h|--help)   show_help ;;
    *) echo "未知参数: $arg"; show_help ;;
  esac
done

cd "$TARGET_DIR"

echo "=================================================="
echo "🛡️  执行 C++ 质量检查 [模式: $MODE | 工具: $TOOL | 自动修复: $DO_FIX]"
echo "=================================================="

# 1. 获取需要检查的源文件列表
get_source_files() {
  if [ "$MODE" = "diff" ]; then
    # 检查是否在 git 仓库中
    if git rev-parse --is-inside-work-tree &>/dev/null; then
      # 获取已修改或已暂存的 C++ 文件，排除 third_party、build 等目录
      git diff --name-only --diff-filter=ACMR HEAD 2>/dev/null | \
        grep -E '\.(cpp|cc|cxx|h|hpp)$' | \
        grep -vE '^(third_party|vendor|external|build|cmake-build)/' || true
    else
      echo "⚠️ 当前目录非 git 仓库，自动退化为全量文件扫描模式"
      find src include -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.cc" \) 2>/dev/null || true
    fi
  else
    find src include -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.cc" \) 2>/dev/null || \
      find . -maxdepth 3 -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.cc" \) | \
      grep -vE '/(third_party|vendor|external|build|\.agents)/' || true
  fi
}

FILES=$(get_source_files)

if [ -z "$FILES" ]; then
  echo "✅ 没有发现需要检查的 C++ 源文件（在当前模式下无变动或已被忽略）。"
  exit 0
fi

echo "📋 待处理的目标文件清单:"
echo "$FILES" | sed 's/^/   - /'
echo "--------------------------------------------------"

# 2. 执行 clang-format 代码格式化
if [ "$TOOL" = "all" ] || [ "$TOOL" = "format" ]; then
  if command -v clang-format &>/dev/null; then
    echo "🎨 [1/3] 执行 clang-format 格式化校验..."
    if [ "$MODE" = "diff" ] && (command -v clang-format-diff &>/dev/null || command -v clang-format-diff.py &>/dev/null); then
      CMD="clang-format-diff"
      command -v clang-format-diff &>/dev/null || CMD="clang-format-diff.py"
      if [ "$DO_FIX" = true ]; then
        echo "   -> 正在使用 $CMD 增量修复修改行..."
        git diff -U0 --no-color HEAD | $CMD -p1 -i
        echo "   ✅ 变动行格式化修复完毕。"
      else
        DIFF_OUT=$(git diff -U0 --no-color HEAD | $CMD -p1 || true)
        if [ -n "$DIFF_OUT" ]; then
          echo "   ⚠️ 检测到代码格式不符合规范的改动行:"
          echo "$DIFF_OUT" | head -n 30
          echo "   💡 提示: 可添加 --fix 参数自动修复这些改动行"
        else
          echo "   ✅ 所有变动行代码格式均符合规范！"
        fi
      fi
    else
      # 全量或无 diff 脚本时的文件级格式化
      for f in $FILES; do
        if [ "$DO_FIX" = true ]; then
          clang-format -i "$f"
        else
          if ! clang-format --dry-run --Werror "$f" 2>/dev/null; then
            echo "   ⚠️ 文件格式违规: $f (运行 --fix 可自动修复)"
          fi
        fi
      done
      [ "$DO_FIX" = true ] && echo "   ✅ 文件格式化修复完毕。"
    fi
  else
    echo "⚠️ 未安装 clang-format，跳过格式化检查。"
  fi
fi

# 3. 执行 clang-tidy 静态语义分析
if [ "$TOOL" = "all" ] || [ "$TOOL" = "tidy" ]; then
  if command -v clang-tidy &>/dev/null; then
    echo "🔍 [2/3] 执行 clang-tidy 静态分析..."
    BUILD_ARG=""
    if [ -f "build/compile_commands.json" ]; then
      BUILD_ARG="-p build"
    elif [ -f "compile_commands.json" ]; then
      BUILD_ARG="-p ."
    else
      echo "   ℹ️ 未找到 compile_commands.json，采用单文件通用参数扫描（建议使用 cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON）"
    fi

    for f in $FILES; do
      echo "   正在分析: $f"
      if [ -n "$BUILD_ARG" ]; then
        clang-tidy $BUILD_ARG "$f" --quiet || true
      else
        clang-tidy "$f" --quiet -- -std=c++17 -Iinclude -Isrc || true
      fi
    done
  else
    echo "⚠️ 未安装 clang-tidy，跳过语义静态检查。"
  fi
fi

# 4. 执行 Cppcheck 轻量排查
if [ "$TOOL" = "all" ] || [ "$TOOL" = "cppcheck" ]; then
  if command -v cppcheck &>/dev/null; then
    echo "⚡ [3/3] 执行 Cppcheck 内存越界与未定义行为排查..."
    cppcheck --enable=warning,performance,portability \
             --inline-suppr \
             --suppress=missingIncludeSystem \
             -i third_party/ -i build/ \
             --quiet \
             $FILES || true
    echo "   ✅ Cppcheck 扫描完成。"
  else
    echo "⚠️ 未安装 cppcheck，跳过快速排查。"
  fi
fi

echo "=================================================="
echo "🎉 C++ 质量检查流程执行完毕！"
echo "=================================================="
