#!/usr/bin/env bash
# ==============================================================================
# C++17 端到端单入口总控门禁脚本 (Single-Entry Verification Gate)
# 串联执行: [1] Include DAG 物理边界 -> [2] 四关静态检查 -> [3] CTest -> [4] TSan 并发压测
# 执行结束自动输出结构化 Markdown Evidence Bundle，杜绝漏关与主观臆断。
# ==============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PLAYBOOK_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
SKILLS_ROOT="$(cd "${PLAYBOOK_DIR}/.." && pwd)"

DAG_SCRIPT="${SKILLS_ROOT}/code-design/scripts/check_include_dag.py"
QUALITY_SCRIPT="${SKILLS_ROOT}/cpp-code-quality/scripts/run_check.sh"

TARGET_DIR="."
MODE="diff"
CPP_STD="c++17"
BUILD_DIR=""
TSAN_BUILD_DIR=""
TSAN_REPEAT=1
WARN_ONLY=false
CHECK_UI_I18N=false
SKIP_BUILD=false
REQUIRE_TESTS=false

show_help() {
  cat << 'EOF'
用法: verify_gate.sh [选项]
选项:
  --dir=<path>             目标 C++ 工程根目录（默认: .）
  --mode=diff|all          静态扫描模式（默认: diff）
  --std=<std>              无编译数据库时的 C++ 标准（默认: c++17）
  --build-dir=<path>       CTest 主构建目录（默认自动探测 build / build-asan / build-debug）
  --tsan-build-dir=<path>  TSan 构建目录（默认自动探测 build-tsan）
  --tsan-repeat=<N>        TSan 压测重复次数（默认: 1，并发场景建议 20~50）
  --warn-only              老项目静默放行静态扫描告警（注意：CTest 运行期失败仍阻断）
  --check-ui-i18n          开启表现层硬编码中文字符串拦截
  --skip-build             跳过 cmake --build，直接执行 ctest
  --require-tests          若未检测到可运行 CTest 构建目录则视作失败
  -h, --help               显示帮助信息
EOF
  exit 0
}

for arg in "$@"; do
  case "$arg" in
    --dir=*)            TARGET_DIR="${arg#*=}" ;;
    --mode=diff)        MODE="diff" ;;
    --mode=all)         MODE="all" ;;
    --std=*)            CPP_STD="${arg#*=}" ;;
    --build-dir=*)      BUILD_DIR="${arg#*=}" ;;
    --tsan-build-dir=*) TSAN_BUILD_DIR="${arg#*=}" ;;
    --tsan-repeat=*)    TSAN_REPEAT="${arg#*=}" ;;
    --warn-only)        WARN_ONLY=true ;;
    --check-ui-i18n)    CHECK_UI_I18N=true ;;
    --skip-build)       SKIP_BUILD=true ;;
    --require-tests)    REQUIRE_TESTS=true ;;
    -h|--help)          show_help ;;
    *) echo "未知参数: $arg" >&2; show_help ;;
  esac
done

if ! [[ "$TSAN_REPEAT" =~ ^[1-9][0-9]*$ ]]; then
  echo "❌ 无效的 --tsan-repeat 参数: $TSAN_REPEAT (需为正整数)" >&2
  exit 2
fi

CALLER_PWD="$PWD"
TARGET_DIR="$(cd "$TARGET_DIR" && pwd)"

resolve_dir() {
  local raw="$1"
  if [[ "$raw" = /* ]]; then
    printf '%s' "$raw"
  elif [ -d "${TARGET_DIR}/${raw}" ]; then
    printf '%s' "${TARGET_DIR}/${raw}"
  elif [ -d "${CALLER_PWD}/${raw}" ]; then
    (cd "${CALLER_PWD}/${raw}" && pwd)
  else
    printf '%s' "${TARGET_DIR}/${raw}"
  fi
}

if [ -z "$BUILD_DIR" ]; then
  for cand in "build" "build-asan" "build-debug"; do
    if [ -f "${TARGET_DIR}/${cand}/CTestTestfile.cmake" ] || [ -f "${TARGET_DIR}/${cand}/CMakeCache.txt" ]; then
      BUILD_DIR="${TARGET_DIR}/${cand}"
      break
    fi
  done
else
  BUILD_DIR="$(resolve_dir "$BUILD_DIR")"
  if [ ! -d "$BUILD_DIR" ]; then
    echo "❌ 指定的 --build-dir 不存在: ${BUILD_DIR}" >&2
    exit 2
  fi
fi

if [ -z "$TSAN_BUILD_DIR" ]; then
  if [ -f "${TARGET_DIR}/build-tsan/CTestTestfile.cmake" ] || [ -f "${TARGET_DIR}/build-tsan/CMakeCache.txt" ]; then
    TSAN_BUILD_DIR="${TARGET_DIR}/build-tsan"
  fi
else
  TSAN_BUILD_DIR="$(resolve_dir "$TSAN_BUILD_DIR")"
  if [ ! -d "$TSAN_BUILD_DIR" ]; then
    echo "❌ 指定的 --tsan-build-dir 不存在: ${TSAN_BUILD_DIR}" >&2
    exit 2
  fi
fi

GATE_FAILED=0

DAG_STATUS="SKIPPED"
DAG_DETAIL="脚本缺失"
QUALITY_STATUS="SKIPPED"
QUALITY_DETAIL="脚本缺失"
CTEST_STATUS="SKIPPED"
CTEST_DETAIL="未检测到 CTest 构建目录"
TSAN_STATUS="SKIPPED"
TSAN_DETAIL="未检测到 TSan 构建目录"

echo "=================================================="
echo "🛡️  C++17 总控验证门禁 [目录: ${TARGET_DIR} | 模式: ${MODE} | warn-only: ${WARN_ONLY}]"
echo "=================================================="

# ------------------------------------------------------------------------------
# Gate A: Include DAG 与物理边界检查
# ------------------------------------------------------------------------------
echo "▶ [Gate 1/4] 执行模块边界与 Include DAG 检查..."
DAG_ARGS=(--root "$TARGET_DIR")
[ "$WARN_ONLY" = true ] && DAG_ARGS+=(--warn-only)
T_START=$(date +%s)
set +e
python3 "$DAG_SCRIPT" "${DAG_ARGS[@]}"
DAG_RC=$?
set -e
T_ELAPSED=$(( $(date +%s) - T_START ))
if [ "$DAG_RC" -eq 0 ]; then
  DAG_STATUS=$([ "$WARN_ONLY" = true ] && echo "PASS (warn-only)" || echo "PASS")
  DAG_DETAIL="exit=0 (${T_ELAPSED}s)"
else
  DAG_STATUS="FAIL"
  DAG_DETAIL="exit=${DAG_RC} (${T_ELAPSED}s)"
  GATE_FAILED=$((GATE_FAILED + 1))
fi

# ------------------------------------------------------------------------------
# Gate B: run_check.sh 四关静态检查
# ------------------------------------------------------------------------------
echo "▶ [Gate 2/4] 执行静态质量与红线扫描 (run_check.sh)..."
QUALITY_ARGS=(--mode="$MODE" --std="$CPP_STD" --dir="$TARGET_DIR")
[ -n "$BUILD_DIR" ] && QUALITY_ARGS+=(--build-dir="$BUILD_DIR")
[ "$WARN_ONLY" = true ] && QUALITY_ARGS+=(--warn-only)
[ "$CHECK_UI_I18N" = true ] && QUALITY_ARGS+=(--check-ui-i18n)
T_START=$(date +%s)
set +e
bash "$QUALITY_SCRIPT" "${QUALITY_ARGS[@]}"
QUALITY_RC=$?
set -e
T_ELAPSED=$(( $(date +%s) - T_START ))
if [ "$QUALITY_RC" -eq 0 ]; then
  QUALITY_STATUS=$([ "$WARN_ONLY" = true ] && echo "PASS (warn-only)" || echo "PASS")
  QUALITY_DETAIL="exit=0 (${T_ELAPSED}s)"
else
  QUALITY_STATUS="FAIL"
  QUALITY_DETAIL="exit=${QUALITY_RC} (${T_ELAPSED}s)"
  GATE_FAILED=$((GATE_FAILED + 1))
fi

# ------------------------------------------------------------------------------
# Gate C: CMake Build + 全量 CTest (ASan/UBSan 或默认构建)
# ------------------------------------------------------------------------------
echo "▶ [Gate 3/4] 执行全量 CTest 测试矩阵..."
if [ -n "$BUILD_DIR" ] && [ -d "$BUILD_DIR" ]; then
  T_START=$(date +%s)
  BUILD_RC=0
  if [ "$SKIP_BUILD" = false ] && [ -f "${BUILD_DIR}/CMakeCache.txt" ]; then
    set +e
    cmake --build "$BUILD_DIR" --parallel
    BUILD_RC=$?
    set -e
  fi
  if [ "$BUILD_RC" -ne 0 ]; then
    T_ELAPSED=$(( $(date +%s) - T_START ))
    CTEST_STATUS="FAIL"
    CTEST_DETAIL="cmake --build exit=${BUILD_RC} (${T_ELAPSED}s)"
    GATE_FAILED=$((GATE_FAILED + 1))
  elif [ -f "${BUILD_DIR}/CTestTestfile.cmake" ]; then
    CTEST_ARGS=(--test-dir "$BUILD_DIR" --output-on-failure)
    [ "$REQUIRE_TESTS" = true ] && CTEST_ARGS+=(--no-tests=error)
    set +e
    ASAN_OPTIONS="${ASAN_OPTIONS:-halt_on_error=1:detect_leaks=1}" \
    UBSAN_OPTIONS="${UBSAN_OPTIONS:-halt_on_error=1:print_stacktrace=1}" \
      ctest "${CTEST_ARGS[@]}"
    CTEST_RC=$?
    set -e
    T_ELAPSED=$(( $(date +%s) - T_START ))
    if [ "$CTEST_RC" -eq 0 ]; then
      CTEST_STATUS="PASS"
      CTEST_DETAIL="$(basename "$BUILD_DIR") exit=0 (${T_ELAPSED}s)"
    else
      CTEST_STATUS="FAIL"
      CTEST_DETAIL="$(basename "$BUILD_DIR") exit=${CTEST_RC} (${T_ELAPSED}s)"
      GATE_FAILED=$((GATE_FAILED + 1))
    fi
  else
    CTEST_STATUS="SKIPPED"
    CTEST_DETAIL="无 CTestTestfile.cmake"
    if [ "$REQUIRE_TESTS" = true ]; then
      CTEST_STATUS="FAIL"
      GATE_FAILED=$((GATE_FAILED + 1))
    fi
  fi
elif [ "$REQUIRE_TESTS" = true ]; then
  CTEST_STATUS="FAIL"
  CTEST_DETAIL="缺少测试构建目录 (--require-tests)"
  GATE_FAILED=$((GATE_FAILED + 1))
fi

# ------------------------------------------------------------------------------
# Gate D: 可选 TSan 并发压测
# ------------------------------------------------------------------------------
echo "▶ [Gate 4/4] 检查并发 TSan 压测矩阵..."
if [ -n "$TSAN_BUILD_DIR" ] && [ -d "$TSAN_BUILD_DIR" ]; then
  T_START=$(date +%s)
  TSAN_BUILD_RC=0
  if [ "$SKIP_BUILD" = false ] && [ -f "${TSAN_BUILD_DIR}/CMakeCache.txt" ]; then
    set +e
    cmake --build "$TSAN_BUILD_DIR" --parallel
    TSAN_BUILD_RC=$?
    set -e
  fi
  if [ "$TSAN_BUILD_RC" -ne 0 ]; then
    T_ELAPSED=$(( $(date +%s) - T_START ))
    TSAN_STATUS="FAIL"
    TSAN_DETAIL="cmake --build exit=${TSAN_BUILD_RC} (${T_ELAPSED}s)"
    GATE_FAILED=$((GATE_FAILED + 1))
  elif [ -f "${TSAN_BUILD_DIR}/CTestTestfile.cmake" ]; then
    TSAN_CMD=(ctest --test-dir "$TSAN_BUILD_DIR" --output-on-failure --repeat "until-fail:${TSAN_REPEAT}")
    if [ "$(uname -s)" = "Linux" ] && [ "$(uname -m)" = "x86_64" ] && command -v setarch &>/dev/null; then
      TSAN_CMD=(setarch x86_64 -R "${TSAN_CMD[@]}")
    fi
    set +e
    TSAN_OPTIONS="${TSAN_OPTIONS:-halt_on_error=1:second_deadlock_stack=1}" "${TSAN_CMD[@]}"
    TSAN_RC=$?
    set -e
    T_ELAPSED=$(( $(date +%s) - T_START ))
    if [ "$TSAN_RC" -eq 0 ]; then
      TSAN_STATUS="PASS"
      TSAN_DETAIL="repeat=${TSAN_REPEAT} exit=0 (${T_ELAPSED}s)"
    else
      TSAN_STATUS="FAIL"
      TSAN_DETAIL="repeat=${TSAN_REPEAT} exit=${TSAN_RC} (${T_ELAPSED}s)"
      GATE_FAILED=$((GATE_FAILED + 1))
    fi
  fi
fi

echo ""
echo "### Evidence Bundle (verify_gate.sh)"
echo ""
echo "| 门禁关卡 | 状态 | 实证详情 |"
echo "| :--- | :---: | :--- |"
echo "| **1. 物理边界 & Include DAG** | ${DAG_STATUS} | ${DAG_DETAIL} |"
echo "| **2. 静态规范 & 红线扫描** | ${QUALITY_STATUS} | ${QUALITY_DETAIL} |"
echo "| **3. 全量 CTest (ASan/UBSan)** | ${CTEST_STATUS} | ${CTEST_DETAIL} |"
echo "| **4. TSan 并发压测** | ${TSAN_STATUS} | ${TSAN_DETAIL} |"
echo ""

if [ "$GATE_FAILED" -eq 0 ]; then
  echo "🎉 总控门禁全部通过！"
  exit 0
else
  echo "❌ 总控门禁未通过（${GATE_FAILED} 项失败），禁止交付！"
  exit 1
fi
