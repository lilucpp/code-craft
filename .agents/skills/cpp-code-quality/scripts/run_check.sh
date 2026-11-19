#!/usr/bin/env bash
# ==============================================================================
# C++ 格式化与静态检查调度脚本 (Run C++ Checks: Diff vs All)
# 默认启用严格门禁（发现错误返回非零退出码）；老项目渐进扫描可使用 --warn-only
# ==============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

MODE="diff"        # diff (增量模式) 或 all (全量模式)
DO_FIX=false       # 是否自动应用修复 (clang-format -i)
WARN_ONLY=false    # true: 仅扫描汇报不阻断 (exit 0)；false: 严格门禁 (默认)
CHECK_UI_I18N=false # 是否开启表现层硬编码中文字符串校验
TOOL="all"         # all, format, tidy, cppcheck, redline
CPP_STD="c++17"    # 默认 C++ 标准 (无 compile_commands.json 时生效)
TARGET_DIR="."
CUSTOM_BUILD_DIR=""

show_help() {
  echo "用法: $0 [选项]"
  echo "选项:"
  echo "  --mode=diff        增量模式（默认）：检查未提交改动；若工作区干净则自动对比 HEAD~1..HEAD"
  echo "  --mode=all         全量模式：对项目中所有的业务源码执行全面检查"
  echo "  --fix              自动修复（应用 clang-format 排版）"
  echo "  --warn-only        仅扫描汇报不阻断（返回 exit 0，适合历史老项目摸底）"
  echo "  --check-ui-i18n    开启表现层控件/弹窗硬编码中文字符串拦截"
  echo "  --std=<std>        指定无编译数据库时的 C++ 标准（默认: c++17）"
  echo "  --build-dir=<dir>  指定含 compile_commands.json 的构建目录（默认自动探测 build/build-asan/build-debug）"
  echo "  --tool=<name>      指定运行工具：all (默认), format, tidy, cppcheck, redline"
  echo "  --dir=<path>       指定目标项目根目录（默认当前目录）"
  echo "  -h, --help         显示帮助信息"
  exit 0
}

for arg in "$@"; do
  case "$arg" in
    --mode=diff)     MODE="diff" ;;
    --mode=all)      MODE="all" ;;
    --fix)           DO_FIX=true ;;
    --warn-only)     WARN_ONLY=true ;;
    --check-ui-i18n) CHECK_UI_I18N=true ;;
    --std=*)         CPP_STD="${arg#*=}" ;;
    --build-dir=*)   CUSTOM_BUILD_DIR="${arg#*=}" ;;
    --tool=*)        TOOL="${arg#*=}" ;;
    --dir=*)         TARGET_DIR="${arg#*=}" ;;
    -h|--help)       show_help ;;
    *) echo "未知参数: $arg"; show_help ;;
  esac
done

case "$TOOL" in
  all|format|tidy|cppcheck|redline) ;;
  *) echo "❌ 无效的 --tool 参数: $TOOL (可选: all, format, tidy, cppcheck, redline)" >&2; exit 2 ;;
esac

CALLER_PWD="$PWD"
cd "$TARGET_DIR"

GATE_LABEL="严格门禁(阻塞)"
if [ "$WARN_ONLY" = true ]; then
  GATE_LABEL="仅告警(--warn-only)"
fi

echo "=================================================="
echo "🛡️  执行 C++ 质量检查 [模式: $MODE | 工具: $TOOL | 修复: $DO_FIX | 门禁: $GATE_LABEL | 标准: $CPP_STD]"
echo "=================================================="

EXCLUDE_REGEX='(^|/)(third_party|vendor|external|build|build[_-][^/]+|cmake-build[^/]*|_deps|\.agents|\.git)/|\.pb\.(cc|h)$'

# 1. 获取需要检查的源文件列表
get_untracked_files() {
  git ls-files --others --exclude-standard 2>/dev/null | \
    grep -E '\.(cpp|cc|cxx|h|hpp)$' | \
    grep -vE "$EXCLUDE_REGEX" || true
}

DIFF_BASE_REF="HEAD"
if [ "$MODE" = "diff" ] && git rev-parse --is-inside-work-tree &>/dev/null; then
  if ! git rev-parse --verify HEAD &>/dev/null; then
    DIFF_BASE_REF="--cached"
  else
    DIRTY_CPP=$(
      {
        git diff --relative --name-only --diff-filter=ACMR HEAD 2>/dev/null || true
        get_untracked_files
      } | grep -E '\.(cpp|cc|cxx|h|hpp)$' | grep -vE "$EXCLUDE_REGEX" || true
    )
    if [ -z "$DIRTY_CPP" ] && git rev-parse --verify HEAD~1 &>/dev/null; then
      DIFF_BASE_REF="HEAD~1"
      echo "ℹ️  当前工作区无未提交 C++ 改动，自动回退检查最近一次提交 (HEAD~1..HEAD)"
    fi
  fi
fi

get_tracked_diff_files() {
  git diff --relative --name-only --diff-filter=ACMR "$DIFF_BASE_REF" 2>/dev/null | \
    grep -E '\.(cpp|cc|cxx|h|hpp)$' | \
    grep -vE "$EXCLUDE_REGEX" || true
}

get_source_files() {
  if [ "$MODE" = "diff" ]; then
    if git rev-parse --is-inside-work-tree &>/dev/null; then
      {
        get_tracked_diff_files
        get_untracked_files
      } | sort -u
    else
      echo "⚠️ 当前目录非 git 仓库，自动退化为全量文件扫描模式" >&2
      find . -maxdepth 6 -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.cc" -o -name "*.cxx" \) | \
        sed 's|^\./||' | grep -vE "$EXCLUDE_REGEX" | sort -u || true
    fi
  else
    find . -maxdepth 6 -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.cc" -o -name "*.cxx" \) | \
      sed 's|^\./||' | grep -vE "$EXCLUDE_REGEX" | sort -u || true
  fi
}

mapfile -t FILES < <(get_source_files)

if [ ${#FILES[@]} -eq 0 ]; then
  echo "✅ 没有发现需要检查的 C++ 源文件（在当前模式下无变动或已被忽略）。"
  exit 0
fi

echo "📋 待处理的目标文件清单 (${#FILES[@]} 个):"
printf '   - %s\n' "${FILES[@]}"
echo "--------------------------------------------------"

FORMAT_ERRORS=0
TIDY_ERRORS=0
CPPCHECK_ERRORS=0
REDLINE_ERRORS=0
MISSING_TOOL_ERRORS=0

check_files_format_full() {
  local target_files=("$@")
  for f in "${target_files[@]}"; do
    [ -f "$f" ] || continue
    if [ "$DO_FIX" = true ]; then
      clang-format -i "$f"
    else
      if ! clang-format --dry-run --Werror "$f"; then
        echo "   ❌ 文件格式违规: $f (运行 --fix 可自动修复)"
        FORMAT_ERRORS=$((FORMAT_ERRORS + 1))
      fi
    fi
  done
}

# 2. 执行 clang-format 代码格式化
if [ "$TOOL" = "all" ] || [ "$TOOL" = "format" ]; then
  if command -v clang-format &>/dev/null; then
    echo "🎨 [1/4] 执行 clang-format 格式化校验..."
    if [ "$MODE" = "diff" ] && git rev-parse --is-inside-work-tree &>/dev/null && \
       (command -v clang-format-diff &>/dev/null || command -v clang-format-diff.py &>/dev/null); then
      CMD="clang-format-diff"
      command -v clang-format-diff &>/dev/null || CMD="clang-format-diff.py"

      mapfile -t TRACKED_FILES < <(get_tracked_diff_files)
      mapfile -t UNTRACKED_FILES < <(get_untracked_files)

      if [ ${#TRACKED_FILES[@]} -gt 0 ]; then
        if [ "$DO_FIX" = true ]; then
          echo "   -> 正在使用 $CMD 增量修复已跟踪文件的修改行..."
          git diff --relative -U0 --no-color "$DIFF_BASE_REF" -- "${TRACKED_FILES[@]}" | "$CMD" -p1 -i
        else
          DIFF_OUT=$(git diff --relative -U0 --no-color "$DIFF_BASE_REF" -- "${TRACKED_FILES[@]}" | "$CMD" -p1 || true)
          if [ -n "$DIFF_OUT" ]; then
            echo "   ❌ 检测到代码格式不符合规范的改动行:"
            echo "$DIFF_OUT" | head -n 40
            echo "   💡 提示: 可添加 --fix 参数自动修复这些改动行"
            FORMAT_ERRORS=$((FORMAT_ERRORS + 1))
          fi
        fi
      fi

      if [ ${#UNTRACKED_FILES[@]} -gt 0 ]; then
        echo "   -> 检测到 ${#UNTRACKED_FILES[@]} 个未跟踪的新增文件，执行全文件格式校验..."
        check_files_format_full "${UNTRACKED_FILES[@]}"
      fi

      if [ "$DO_FIX" = true ]; then
        echo "   ✅ 增量及新增文件格式化修复完毕。"
      elif [ "$FORMAT_ERRORS" -eq 0 ]; then
        echo "   ✅ 所有变动行及新增文件代码格式均符合规范！"
      fi
    else
      check_files_format_full "${FILES[@]}"
      if [ "$DO_FIX" = true ]; then
        echo "   ✅ 文件格式化修复完毕。"
      elif [ "$FORMAT_ERRORS" -eq 0 ]; then
        echo "   ✅ 所有源文件格式化检查通过！"
      fi
    fi
  else
    echo "❌ [1/4] 未安装 clang-format（当前执行环境无 root 权限，无法自动安装）"
    echo "   👉 请在宿主机终端手动运行: sudo apt-get install -y clang-format"
    if command -v docker &>/dev/null && docker ps &>/dev/null; then
      echo "   🐳 或使用 Docker 免安装格式化: docker run --rm -v \"\$(pwd):/src\" -w /src ghcr.io/muttleyxd/clang-tools:18 clang-format -i \$FILES"
    fi
    MISSING_TOOL_ERRORS=$((MISSING_TOOL_ERRORS + 1))
  fi
fi

# 3. 执行 clang-tidy 静态语义分析
if [ "$TOOL" = "all" ] || [ "$TOOL" = "tidy" ]; then
  if command -v clang-tidy &>/dev/null; then
    echo "🔍 [2/4] 执行 clang-tidy 静态分析..."
    BUILD_ARGS=()
    if [ -n "$CUSTOM_BUILD_DIR" ]; then
      if [ -f "$CUSTOM_BUILD_DIR/compile_commands.json" ]; then
        BUILD_ARGS=(-p "$CUSTOM_BUILD_DIR")
      elif [ -f "$CALLER_PWD/$CUSTOM_BUILD_DIR/compile_commands.json" ]; then
        BUILD_ARGS=(-p "$CALLER_PWD/$CUSTOM_BUILD_DIR")
      else
        echo "   ⚠️ 指定的 --build-dir ($CUSTOM_BUILD_DIR) 下未找到 compile_commands.json，尝试自动探测..."
      fi
    fi
    if [ ${#BUILD_ARGS[@]} -eq 0 ]; then
      for cand in build build-asan build-debug .; do
        if [ -f "$cand/compile_commands.json" ]; then
          BUILD_ARGS=(-p "$cand")
          break
        fi
      done
    fi
    if [ ${#BUILD_ARGS[@]} -eq 0 ]; then
      echo "   ℹ️ 未找到 compile_commands.json，采用通用参数扫描 (-std=$CPP_STD，建议使用 cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON)"
    fi

    FALLBACK_ARGS=("-std=$CPP_STD")
    for inc_dir in include src modules/*/include modules/*/src libs/*/include libs/*/src packages/*/include packages/*/src components/*/include components/*/src; do
      [ -d "$inc_dir" ] && FALLBACK_ARGS+=("-I$inc_dir")
    done

    for f in "${FILES[@]}"; do
      [ -f "$f" ] || continue
      echo "   正在分析: $f"
      if [ ${#BUILD_ARGS[@]} -gt 0 ]; then
        if ! clang-tidy "${BUILD_ARGS[@]}" "$f" --quiet; then
          echo "   ❌ clang-tidy 发现阻断性错误: $f"
          TIDY_ERRORS=$((TIDY_ERRORS + 1))
        fi
      else
        if ! clang-tidy "$f" --quiet -- "${FALLBACK_ARGS[@]}"; then
          echo "   ❌ clang-tidy 发现阻断性错误: $f"
          TIDY_ERRORS=$((TIDY_ERRORS + 1))
        fi
      fi
    done

    if [ "$TIDY_ERRORS" -eq 0 ]; then
      echo "   ✅ clang-tidy 静态分析检查通过！"
    fi
  else
    echo "❌ [2/4] 未安装 clang-tidy（当前执行环境无 root 权限，无法自动安装）"
    echo "   👉 请在宿主机终端手动运行: sudo apt-get install -y clang-tidy"
    MISSING_TOOL_ERRORS=$((MISSING_TOOL_ERRORS + 1))
  fi
fi

# 4. 执行 Cppcheck 轻量排查
if [ "$TOOL" = "all" ] || [ "$TOOL" = "cppcheck" ]; then
  if command -v cppcheck &>/dev/null; then
    echo "⚡ [3/4] 执行 Cppcheck 内存越界与未定义行为排查..."
    if ! cppcheck --language=c++ --std="$CPP_STD" \
                  --enable=warning,performance,portability \
                  --error-exitcode=1 \
                  --inline-suppr \
                  --suppress=missingIncludeSystem \
                  --suppress=unmatchedSuppression \
                  --suppress=unknownMacro \
                  -Dslots= -Dsignals= \
                  -i third_party/ -i vendor/ -i external/ -i build/ -i build-win64/ \
                  --quiet \
                  "${FILES[@]}"; then
      echo "   ❌ Cppcheck 检出阻断性缺陷！"
      CPPCHECK_ERRORS=$((CPPCHECK_ERRORS + 1))
    else
      echo "   ✅ Cppcheck 扫描通过。"
    fi
  else
    echo "❌ [3/4] 未安装 cppcheck（当前执行环境无 root 权限，无法自动安装）"
    echo "   👉 请在宿主机终端手动运行: sudo apt-get install -y cppcheck"
    MISSING_TOOL_ERRORS=$((MISSING_TOOL_ERRORS + 1))
  fi
fi

# 5. 执行红线反模式静态扫描 (lint_redlines.py)
if [ "$TOOL" = "all" ] || [ "$TOOL" = "redline" ]; then
  if command -v python3 &>/dev/null && [ -f "$SCRIPT_DIR/lint_redlines.py" ]; then
    echo "🚨 [4/4] 执行 C++ 红线反模式静态扫描..."
    REDLINE_ARGS=()
    [ "$CHECK_UI_I18N" = true ] && REDLINE_ARGS+=("--check-ui-i18n")
    if ! python3 "$SCRIPT_DIR/lint_redlines.py" "${REDLINE_ARGS[@]}" "${FILES[@]}"; then
      REDLINE_ERRORS=$((REDLINE_ERRORS + 1))
    fi
  else
    echo "❌ [4/4] 未找到 python3 或 lint_redlines.py，无法执行红线静态扫描"
    MISSING_TOOL_ERRORS=$((MISSING_TOOL_ERRORS + 1))
  fi
fi

TOTAL_ERRORS=$((FORMAT_ERRORS + TIDY_ERRORS + CPPCHECK_ERRORS + REDLINE_ERRORS + MISSING_TOOL_ERRORS))

echo "=================================================="
if [ "$TOTAL_ERRORS" -eq 0 ]; then
  echo "🎉 C++ 质量检查门禁全部通过！"
  echo "=================================================="
  exit 0
fi

echo "📊 检查汇总: format=$FORMAT_ERRORS, tidy=$TIDY_ERRORS, cppcheck=$CPPCHECK_ERRORS, redline=$REDLINE_ERRORS, missing_tools=$MISSING_TOOL_ERRORS"
if [ "$WARN_ONLY" = true ]; then
  echo "⚠️  [--warn-only] 检测到 $TOTAL_ERRORS 项问题，按老项目只扫描不阻断模式放行 (exit 0)。"
  echo "=================================================="
  exit 0
else
  echo "❌ 质量门禁未通过（共 $TOTAL_ERRORS 项阻断问题），禁止交付！"
  echo "   💡 若为历史老项目初次摸底，可显式追加 --warn-only 参数只扫描不阻断。"
  echo "=================================================="
  exit 1
fi
