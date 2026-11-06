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
    if git rev-parse --is-inside-work-tree &>/dev/null; then
      git diff --name-only --diff-filter=ACMR HEAD 2>/dev/null | \
        grep -E '\.(cpp|cc|cxx|h|hpp)$' | \
        grep -vE '^(third_party|vendor|external|build|cmake-build|\.agents)/' || true
    else
      echo "⚠️ 当前目录非 git 仓库，自动退化为全量文件扫描模式"
      find src -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.cc" \) 2>/dev/null || true
    fi
  else
    local search_dirs=()
    [ -d "src" ] && search_dirs+=("src")
    [ -d "include" ] && search_dirs+=("include")
    [ -d "tests" ] && search_dirs+=("tests")
    if [ ${#search_dirs[@]} -gt 0 ]; then
      find "${search_dirs[@]}" -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.cc" \) | \
        grep -vE '/(third_party|vendor|external|build|cmake-build|\.agents|\.pb\.)/' | sort -u
    else
      find . -maxdepth 4 -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.cc" \) | \
        grep -vE '/(third_party|vendor|external|build|cmake-build|\.agents|\.pb\.)/' | sort -u
    fi
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
      FORMAT_ERRORS=0
      for f in $FILES; do
        if [ "$DO_FIX" = true ]; then
          clang-format -i "$f"
        else
          if ! clang-format --dry-run --Werror "$f" 2>/dev/null; then
            echo "   ⚠️ 文件格式违规: $f (运行 --fix 可自动修复)"
            FORMAT_ERRORS=$((FORMAT_ERRORS + 1))
          fi
        fi
      done
      if [ "$DO_FIX" = true ]; then
        echo "   ✅ 文件格式化修复完毕。"
      else
        if [ "$FORMAT_ERRORS" -eq 0 ]; then
          echo "   ✅ 所有源文件格式化检查通过！"
        else
          echo "   ⚠️ 共发现 $FORMAT_ERRORS 个文件格式不符合规范（运行 --fix 可自动修复）。"
        fi
      fi
    fi
  else
    echo "❌ [1/3] 未安装 clang-format（当前执行环境无 root 权限，无法自动安装）"
    echo "   👉 请在宿主机终端手动运行: sudo apt-get install -y clang-format"
    if command -v docker &>/dev/null && docker ps &>/dev/null; then
      echo "   🐳 或使用 Docker 免安装格式化: docker run --rm -v \"\$(pwd):/src\" -w /src ghcr.io/muttleyxd/clang-tools:18 clang-format -i \$FILES"
    fi
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
      echo "   ℹ️ 未找到 compile_commands.json，采用通用参数扫描（建议使用 cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON）"
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
    echo "❌ [2/3] 未安装 clang-tidy（当前执行环境无 root 权限，无法自动安装）"
    echo "   👉 请在宿主机终端手动运行: sudo apt-get install -y clang-tidy"
  fi
fi

# 4. 执行 Cppcheck 轻量排查
if [ "$TOOL" = "all" ] || [ "$TOOL" = "cppcheck" ]; then
  if command -v cppcheck &>/dev/null; then
    echo "⚡ [3/3] 执行 Cppcheck 内存越界与未定义行为排查..."
    cppcheck --language=c++ --enable=warning,performance,portability \
             --inline-suppr \
             --suppress=missingIncludeSystem \
             --suppress=unmatchedSuppression \
             --suppress=unknownMacro \
             -Dslots= -Dsignals= \
             -i third_party/ -i build/ -i build-win64/ \
             --quiet \
             $FILES || true
    echo "   ✅ Cppcheck 扫描完成。"
  else
    echo "❌ [3/3] 未安装 cppcheck（当前执行环境无 root 权限，无法自动安装）"
    echo "   👉 请在宿主机终端手动运行: sudo apt-get install -y cppcheck"
  fi
fi

echo "=================================================="
echo "🎉 C++ 质量检查流程执行完毕！"
echo "=================================================="
