#!/usr/bin/env bash
# ==============================================================================
# Code-Craft 仓库自身回归测试与验证基础设施 (Self-Test Verification Harness)
# 固化所有已发现边界用例与门禁契约，防止工具脚本与 C++17 基础设施模板退化。
# ==============================================================================
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SCRIPTS_QUALITY="${REPO_ROOT}/.agents/skills/cpp-code-quality/scripts"
SCRIPTS_DESIGN="${REPO_ROOT}/.agents/skills/code-design/scripts"
SCRIPTS_PLAYBOOK="${REPO_ROOT}/.agents/skills/cpp-verification-playbook/scripts"
EXAMPLES_SYS="${REPO_ROOT}/.agents/skills/systems-data-design/examples"

WORK_DIR="$(mktemp -d)"
trap 'rm -rf "$WORK_DIR"' EXIT

PASS_COUNT=0
assert_eq() {
  local actual="$1"
  local expected="$2"
  local label="$3"
  if [ "$actual" -ne "$expected" ]; then
    echo "❌ [FAIL] ${label}: expected exit=${expected}, got=${actual}" >&2
    exit 1
  fi
  echo "  ✅ [PASS] ${label} (exit=${actual})"
  PASS_COUNT=$((PASS_COUNT + 1))
}

echo "=================================================="
echo "🧪 Code-Craft Self-Test Verification Harness"
echo "=================================================="

echo "[1/7] 脚本语法、Skill 元数据与全仓 Markdown 链接检查..."
bash -n "${SCRIPTS_QUALITY}/env_check.sh"
bash -n "${SCRIPTS_QUALITY}/init_config.sh"
bash -n "${SCRIPTS_QUALITY}/run_check.sh"
bash -n "${SCRIPTS_PLAYBOOK}/verify_gate.sh"
bash -n "${REPO_ROOT}/scripts/self_test.sh"
python3 -m py_compile "${SCRIPTS_QUALITY}/lint_redlines.py" "${SCRIPTS_DESIGN}/check_include_dag.py"

python3 - "${REPO_ROOT}" << 'PY'
import glob, os, re, sys

root = sys.argv[1]
os.chdir(root)

def parse_frontmatter(text):
    try:
        import yaml
        return yaml.safe_load(text)
    except ImportError:
        name_m = re.search(r"^name:\s*(\S+)", text, re.MULTILINE)
        desc_m = re.search(r"^description:\s*>-\n((?:  .*\n?)+)", text, re.MULTILINE)
        if not desc_m:
            desc_m = re.search(r"^description:\s*(.+)", text, re.MULTILINE)
        return {
            "name": name_m.group(1).strip() if name_m else "",
            "description": re.sub(r"\s+", " ", desc_m.group(1)).strip() if desc_m else "",
        }

for skill_md in sorted(glob.glob(".agents/skills/*/SKILL.md")):
    with open(skill_md, "r", encoding="utf-8") as f:
        raw = f.read()
    m = re.match(r"^---\n(.*?)\n---\n", raw, re.DOTALL)
    assert m, f"Missing YAML frontmatter in {skill_md}"
    meta = parse_frontmatter(m.group(1))
    name = meta.get("name", "")
    desc = meta.get("description", "")
    assert re.match(r"^[a-z0-9-]+$", name) and len(name) <= 64, f"Invalid name in {skill_md}: {name}"
    assert len(desc) <= 1024, f"Description > 1024 chars in {skill_md}: {len(desc)}"

# Enforce Complexity Freeze on lint_redlines.py (<= 5 rule classes, <= 380 LOC)
redline_path = ".agents/skills/cpp-code-quality/scripts/lint_redlines.py"
with open(redline_path, "r", encoding="utf-8") as f:
    redline_src = f.read()
redline_loc = len(redline_src.splitlines())
redline_rules = set(re.findall(r'rule_id="(redline-[a-z0-9-]+)"', redline_src))
if len(redline_rules) > 5 or redline_loc > 380:
    print(
        f"❌ [FAIL] lint_redlines.py 触发复杂度冻结红线 (当前: {len(redline_rules)} 类规则 / {redline_loc} 行; 上限: 5 类规则 / 380 行)！\n"
        "   禁止用正则模拟 AST 语义分析：语法/类型/控制流问题请配置 .clang-tidy；协议/业务不变式请写入 coding-standards.md 事前心智与 Gate 3 盲审清单。",
        file=sys.stderr,
    )
    sys.exit(1)

md_files = sorted(glob.glob(".agents/skills/*/SKILL.md") + glob.glob(".agents/rules/*.md") + ["README.md", "README_EN.md"])
for md in md_files:
    base = os.path.dirname(md) or "."
    with open(md, "r", encoding="utf-8") as f:
        text = f.read()
    no_code = re.sub(r"```.*?```", "", text, flags=re.DOTALL)
    for lm in re.finditer(r"\]\(([^)]+)\)", no_code):
        target = lm.group(1).strip().split("#")[0]
        if not target or target.startswith(("http://", "https://", "mailto:")):
            continue
        resolved = os.path.normpath(os.path.join(base, target))
        assert os.path.exists(resolved), f"Broken link in {md}: {target} -> {resolved}"
PY
echo "  ✅ [PASS] 脚本语法、Skill Frontmatter 与全仓 Markdown 链接校验通过"
PASS_COUNT=$((PASS_COUNT + 1))

echo "[2/7] 工具链环境与 init_config.sh 一致性检查..."
"${SCRIPTS_QUALITY}/env_check.sh" >/dev/null
assert_eq $? 0 "env_check.sh 工具链就绪"

INIT_DIR="${WORK_DIR}/init_proj"
mkdir -p "${INIT_DIR}"
"${SCRIPTS_QUALITY}/init_config.sh" "${INIT_DIR}" >/dev/null
diff -u "${REPO_ROOT}/.clang-tidy" "${INIT_DIR}/.clang-tidy"
assert_eq $? 0 "init_config.sh 生成的 .clang-tidy 与根目录一致"
diff -u "${REPO_ROOT}/.clang-format" "${INIT_DIR}/.clang-format"
assert_eq $? 0 "init_config.sh 生成的 .clang-format 与根目录一致"

echo "[3/7] run_check.sh 严格门禁 / --warn-only / 0-commit / Monorepo 子目录检查..."
ZERO_DIR="${WORK_DIR}/zero_repo"
mkdir -p "${ZERO_DIR}/src"
cp "${REPO_ROOT}/.clang-format" "${REPO_ROOT}/.clang-tidy" "${ZERO_DIR}/"
(
  cd "${ZERO_DIR}"
  git init -q
  cat << 'EOF' > src/ui_ok.cpp
struct Label {
  static void setText(const char *text) { (void)text; }
};
static const char *Tr(const char *msg) { return msg; }
void SetupUi() { Label::setText(Tr("确定")); }
EOF
  clang-format -i src/ui_ok.cpp
  git add src/ui_ok.cpp
)
"${SCRIPTS_QUALITY}/run_check.sh" --mode=diff --check-ui-i18n --dir="${ZERO_DIR}" >/dev/null
assert_eq $? 0 "0-commit git add 干净文件 + Tr(...) 通过"

cat << 'EOF' > "${ZERO_DIR}/src/bad.cpp"
#include <cstdlib>
int BadFn(int p1, int p2, int p3, int p4){int arr[2]={0,1};if(p1==p1){return std::system("ls");}return arr[2]+p2+p3+p4;}
int NullDeref(const int *ptr, bool cond){if(cond && ptr == nullptr){return *ptr;}return 0;}
EOF
set +e
"${SCRIPTS_QUALITY}/run_check.sh" --mode=diff --dir="${ZERO_DIR}" >/dev/null 2>&1
RC_STRICT=$?
set -e
assert_eq "$RC_STRICT" 1 "严格模式阻断格式/tidy/cppcheck/redline 违规"

"${SCRIPTS_QUALITY}/run_check.sh" --mode=diff --warn-only --dir="${ZERO_DIR}" >/dev/null 2>&1
assert_eq $? 0 "--warn-only 老项目放行"

MONO_DIR="${WORK_DIR}/monorepo"
mkdir -p "${MONO_DIR}/subproj/modules/auth/include/auth" "${MONO_DIR}/subproj/modules/auth/src"
cp "${REPO_ROOT}/.clang-format" "${REPO_ROOT}/.clang-tidy" "${MONO_DIR}/subproj/"
(
  cd "${MONO_DIR}"
  git init -q && git config user.email "t@e.com" && git config user.name "T"
  cat << 'EOF' > subproj/modules/auth/include/auth/token.hpp
#pragma once
#include <string_view>

int ValidateToken(std::string_view token);
EOF
  cat << 'EOF' > subproj/modules/auth/src/token.cpp
#include "auth/token.hpp"

#include <string_view>

int ValidateToken(std::string_view token) {
  const char *sql = R"SQL(SELECT * FROM t WHERE c = "system('ls'); strcpy(a, b);")SQL";
  return (token.empty() || sql[0] == '\0') ? 0 : 1;
}
EOF
  clang-format -i subproj/modules/auth/include/auth/token.hpp subproj/modules/auth/src/token.cpp
  git add . && git commit -q -m "init"
  cat << 'EOF' > subproj/modules/auth/src/token.cpp
#include "auth/token.hpp"

#include <string_view>

int ValidateToken(std::string_view token){
  const char* sql=R"SQL(SELECT * FROM t WHERE c = "system('ls'); strcpy(a, b);")SQL";
  return (token.empty()||sql[0]=='\0')?0:1;
}
EOF
)
"${SCRIPTS_QUALITY}/run_check.sh" --mode=diff --fix --dir="${MONO_DIR}/subproj" >/dev/null
assert_eq $? 0 "Monorepo 子目录 --mode=diff --fix + modules/*/include 通过"
"${SCRIPTS_QUALITY}/run_check.sh" --mode=all --dir="${MONO_DIR}/subproj" >/dev/null
assert_eq $? 0 "Monorepo 子目录 --mode=all 通过"

# Verify post-commit HEAD~1..HEAD fallback in --mode=diff
(
  cd "${MONO_DIR}"
  cat << 'EOF' > subproj/modules/auth/src/token.cpp
#include "auth/token.hpp"
#include <cstdlib>
int ValidateToken(std::string_view token) {
  return token.empty() ? std::system("ls") : 0;
}
EOF
  clang-format -i subproj/modules/auth/src/token.cpp
  git add . && git commit -q -m "bad commit"
)
set +e
"${SCRIPTS_QUALITY}/run_check.sh" --mode=diff --dir="${MONO_DIR}/subproj" >/dev/null 2>&1
RC_POST_COMMIT=$?
set -e
assert_eq "$RC_POST_COMMIT" 1 "--mode=diff 在已 commit 后自动回退对比 HEAD~1..HEAD 并拦截违规"

echo "[4/7] lint_redlines.py 红线正反例回归测试..."
RED_DIR="${WORK_DIR}/redline"
mkdir -p "${RED_DIR}"
cat << 'EOF' > "${RED_DIR}/clean.cpp"
#include <string>
struct CustomRng {
  int rand() const { return 1; }
  void fork() const {}
};
int Clean(const CustomRng &rng) {
  // comment strcpy() system() ./build/
  const std::string doc = "strcpy() atoi()";
  const char *raw = R"SQL(system("ls"); strcpy(a, b);)SQL";
  rng.fork();
  const CustomRng *ptr = &rng;
  const int spaced_arrow = ptr -> rand();
  // NOLINTNEXTLINE(redline): boundary test
  const int n = atoi("7");
  const char *p = "../build/tool";  // NOLINT(redline): fixture
  lbl->setText(Tr("确定"));
  std::error_code fs_ec;
  if (!std::filesystem::exists(p, fs_ec)) {
    if (fs_ec) return -1;
  }
  const std::size_t offset = 1;
  const std::size_t len = 2;
  if (len > doc.size() - offset) return -1;
  return spaced_arrow + n + static_cast<int>(doc.size()) + (raw[0] != '\0') + (p[0] != '\0');
}
EOF
python3 "${SCRIPTS_QUALITY}/lint_redlines.py" --check-ui-i18n "${RED_DIR}/clean.cpp" >/dev/null
assert_eq $? 0 "lint_redlines.py 正例与豁免零误报"

cat << 'EOF' > "${RED_DIR}/bad.cpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
void Bad(const char *s, int factor) {
  char buf[8]{};
  std::strcpy(buf, s);
  const int x = atoi("1");
  std::cout << factor * rand();
  do rand(); while (false);
  ::system("true");
  try {
    if (x > 0) throw std::exception();
  } catch (...) {
  }
  const char *p = "../build/bin";
  lbl->setText("硬编码文案");
  std::error_code fs_ec;
  if (!std::filesystem::exists(p, fs_ec)) {
    return;
  }
  (void)p;
}
EOF
set +e
RED_OUT=$(python3 "${SCRIPTS_QUALITY}/lint_redlines.py" --check-ui-i18n "${RED_DIR}/bad.cpp" 2>&1)
RC_RED_BAD=$?
set -e
assert_eq "$RC_RED_BAD" 1 "lint_redlines.py 检出反例阻断"
for expected_rule in "strcpy()" "atoi()" "rand()" "system()" "redline-swallowed-catch" "redline-fragile-cwd-path" "redline-ui-hardcoded-text" "redline-ignored-fs-error-code"; do
  if ! printf '%s' "$RED_OUT" | grep -q "$expected_rule"; then
    echo "❌ [FAIL] lint_redlines.py 未匹配预期规则: ${expected_rule}" >&2
    exit 1
  fi
done
echo "  ✅ [PASS] lint_redlines.py 5 类冻结词法规则 (8 处违例) 全部精准命中"
PASS_COUNT=$((PASS_COUNT + 1))

echo "[5/7] check_include_dag.py 架构边界与环路回归测试..."
DAG_DIR="${WORK_DIR}/dag"
mkdir -p "${DAG_DIR}"/{include/{core,domain,infra/detail},src/{domain,infra,app}}
cat << 'EOF' > "${DAG_DIR}/.arch-boundaries.json"
{
  "modules": {
    "core": { "allow": [] },
    "domain": { "allow": ["core"] },
    "infra": { "allow": ["core", "domain"] },
    "app": { "allow": ["core", "domain", "infra"] }
  }
}
EOF
echo '#pragma once' > "${DAG_DIR}/include/core/types.hpp"
echo -e '#pragma once\n#include "core/types.hpp"' > "${DAG_DIR}/include/domain/order.hpp"
echo '#pragma once' > "${DAG_DIR}/include/infra/detail/priv.hpp"
echo -e '#pragma once\n#include "domain/order.hpp"' > "${DAG_DIR}/include/infra/repo.hpp"
echo -e '#include "infra/repo.hpp"\n#include "infra/detail/priv.hpp"' > "${DAG_DIR}/src/infra/repo.cpp"
echo -e '#include "domain/order.hpp"\n#include "infra/repo.hpp"' > "${DAG_DIR}/src/app/main.cpp"
cat << 'EOF' >> "${DAG_DIR}/src/app/main.cpp"
const char *kTemplate = R"CPP(
#include "infra/detail/priv.hpp"
)CPP";
EOF

python3 "${SCRIPTS_DESIGN}/check_include_dag.py" --root "${DAG_DIR}" >/dev/null
assert_eq $? 0 "check_include_dag.py 合法分层 DAG 通过"

echo -e '#pragma once\n#include "domain/order.hpp"' > "${DAG_DIR}/include/core/types.hpp"
echo '#include "../../include/core/types.hpp"' > "${DAG_DIR}/src/domain/order.cpp"
echo '#include "infra/detail/priv.hpp"' >> "${DAG_DIR}/src/app/main.cpp"
set +e
DAG_OUT=$(python3 "${SCRIPTS_DESIGN}/check_include_dag.py" --root "${DAG_DIR}" 2>&1)
RC_DAG_BAD=$?
set -e
assert_eq "$RC_DAG_BAD" 1 "check_include_dag.py 阻断环路与越权"
for expected_arch in "arch-cyclic-dependency" "arch-private-header-leak" "arch-relative-parent-include" "arch-layer-violation"; do
  if ! printf '%s' "$DAG_OUT" | grep -q "$expected_arch"; then
    echo "❌ [FAIL] check_include_dag.py 未匹配预期规则: ${expected_arch}" >&2
    exit 1
  fi
done
echo "  ✅ [PASS] check_include_dag.py 4 类边界违例全部精准命中"
PASS_COUNT=$((PASS_COUNT + 1))

# Verify single-library flat src/ layout (include/mylib/detail/x.hpp + src/detail/local.hpp + src/x.cpp)
SINGLE_LIB_DIR="${WORK_DIR}/single_lib"
mkdir -p "${SINGLE_LIB_DIR}/include/mylib/detail" "${SINGLE_LIB_DIR}/src/detail"
echo '#pragma once' > "${SINGLE_LIB_DIR}/include/mylib/detail/x_detail.hpp"
echo '#pragma once' > "${SINGLE_LIB_DIR}/src/detail/local_detail.hpp"
echo '#pragma once' > "${SINGLE_LIB_DIR}/include/mylib/x.hpp"
printf '#include "mylib/x.hpp"\n#include "mylib/detail/x_detail.hpp"\n#include "detail/local_detail.hpp"\n' > "${SINGLE_LIB_DIR}/src/x.cpp"
python3 "${SCRIPTS_DESIGN}/check_include_dag.py" --root "${SINGLE_LIB_DIR}" >/dev/null
assert_eq $? 0 "check_include_dag.py 单库扁平 src/ 引用自身 detail/ 零误报"

echo "[6/7] C++17 基础设施原语 (ASan/UBSan + 四关质量门禁) 回归测试..."
CPP_DIR="${WORK_DIR}/cpp17_core"
mkdir -p "${CPP_DIR}"/{include/core,src,real_data}
ln -s "${CPP_DIR}/real_data" "${CPP_DIR}/linked_data"
cp "${REPO_ROOT}/.clang-format" "${REPO_ROOT}/.clang-tidy" "${CPP_DIR}/"
cp "${EXAMPLES_SYS}"/*.hpp "${CPP_DIR}/include/core/"
cat << 'EOF' > "${CPP_DIR}/src/verify_core.cpp"
#include "core/crash_safe_writer.hpp"
#include "core/result_and_typestate.hpp"
#include "core/scope_guard.hpp"
#include "core/strong_type.hpp"

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <unordered_map>
#include <utility>

namespace {

struct OrderIdTag {};
using OrderId = systems_craft::StrongType<OrderIdTag, std::uint64_t>;
static_assert(sizeof(OrderId) == sizeof(std::uint64_t), "StrongType must be zero-overhead");

}  // namespace

int main(int argc, char *argv[]) {
  assert(argc >= 2);
  const std::filesystem::path target_dir(argv[1]);

  const OrderId oid(42U);
  std::unordered_map<OrderId, std::string, systems_craft::StrongTypeHash> orders;
  orders.emplace(oid, "created");
  assert(orders.at(oid) == "created");

  const auto ok_same = systems_craft::Result<std::string, std::string>::Ok("hello");
  const auto err_same = systems_craft::Result<std::string, std::string>::Err("boom");
  assert(ok_same.has_value() && ok_same.value() == "hello");
  assert(!err_same.has_value() && err_same.error() == "boom");

  bool cleaned = false;
  {
    auto guard = systems_craft::MakeScopeGuard([&cleaned]() noexcept { cleaned = true; });
    (void)guard;
  }
  assert(cleaned);

  auto conn = systems_craft::ChannelHandle<systems_craft::Disconnected>::Connect("tcp://localhost:9000");
  assert(conn.has_value());
  auto authed = std::move(conn.value()).Authenticate("tok");
  assert(authed.has_value() && authed.value().Endpoint() == "tcp://localhost:9000");

  const std::filesystem::path out_file = target_dir / "config.txt";
  const std::error_code err = systems_craft::CrashSafeWriter::write_atomically(out_file, "ok-payload");
  assert(!err);
  std::ifstream ifs(out_file);
  std::string content;
  std::getline(ifs, content);
  assert(content == "ok-payload");
  return 0;
}
EOF
clang-format -i "${CPP_DIR}/src/verify_core.cpp"
g++ -std=c++17 -Wall -Wextra -Werror -Wpedantic -Wconversion -Wshadow -fsanitize=address,undefined -g \
  -I"${CPP_DIR}/include" "${CPP_DIR}/src/verify_core.cpp" -o "${CPP_DIR}/verify_bin"
"${CPP_DIR}/verify_bin" "${CPP_DIR}/linked_data"
assert_eq $? 0 "C++17 原语 ASan/UBSan 编译与运行通过"
rm -f "${CPP_DIR}/verify_bin"

"${SCRIPTS_QUALITY}/run_check.sh" --mode=all --dir="${CPP_DIR}" >/dev/null
assert_eq $? 0 "C++17 原语通过 run_check.sh 四关严格门禁"

echo "[7/7] verify_gate.sh 单入口总控门禁 (CMake + ASan/UBSan + TSan + Evidence Bundle)..."
GATE_DIR="${WORK_DIR}/gate_proj"
mkdir -p "${GATE_DIR}/src"
"${SCRIPTS_QUALITY}/init_config.sh" "${GATE_DIR}" >/dev/null
cat << 'EOF' > "${GATE_DIR}/CMakeLists.txt"
cmake_minimum_required(VERSION 3.16)
project(GateSmoke LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(ENABLE_TSAN "Enable TSan" OFF)
if(ENABLE_TSAN)
  add_compile_options(-fsanitize=thread -g)
  add_link_options(-fsanitize=thread)
else()
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
  add_link_options(-fsanitize=address,undefined)
endif()
enable_testing()
add_executable(smoke_test src/smoke_test.cpp)
add_test(NAME smoke_test COMMAND smoke_test)
EOF
cat << 'EOF' > "${GATE_DIR}/src/smoke_test.cpp"
#include <atomic>
#include <thread>

int main() {
  std::atomic<int> count{0};
  std::thread t1([&count]() { count.fetch_add(1, std::memory_order_relaxed); });
  std::thread t2([&count]() { count.fetch_add(1, std::memory_order_relaxed); });
  t1.join();
  t2.join();
  return count.load() == 2 ? 0 : 1;
}
EOF
clang-format -i "${GATE_DIR}/src/smoke_test.cpp"
cmake -S "${GATE_DIR}" -B "${GATE_DIR}/build" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >/dev/null 2>&1
cmake -S "${GATE_DIR}" -B "${GATE_DIR}/build-tsan" -DENABLE_TSAN=ON >/dev/null 2>&1

GATE_OUT=$("${SCRIPTS_PLAYBOOK}/verify_gate.sh" --dir="${GATE_DIR}" --mode=all --require-tests --tsan-repeat=3 2>&1)
assert_eq $? 0 "verify_gate.sh 串联 DAG + 四关静态 + CTest(ASan) + TSan(repeat=3) 通过"
for expected_eb in "Evidence Bundle (verify_gate.sh)" "物理边界 & Include DAG" "静态规范 & 红线扫描" "全量 CTest (ASan/UBSan)" "TSan 并发压测"; do
  if ! printf '%s' "$GATE_OUT" | grep -qF "$expected_eb"; then
    echo "❌ [FAIL] verify_gate.sh 未输出预期 Evidence Bundle 字段: ${expected_eb}" >&2
    exit 1
  fi
done
echo "  ✅ [PASS] verify_gate.sh Evidence Bundle 表格输出完整"
PASS_COUNT=$((PASS_COUNT + 1))

cat << 'EOF' > "${GATE_DIR}/src/smoke_test.cpp"
int main() { return 1; }
EOF
clang-format -i "${GATE_DIR}/src/smoke_test.cpp"
set +e
"${SCRIPTS_PLAYBOOK}/verify_gate.sh" --dir="${GATE_DIR}" --mode=all --warn-only >/dev/null 2>&1
RC_FAIL_TEST=$?
"${SCRIPTS_PLAYBOOK}/verify_gate.sh" --dir="${GATE_DIR}" --build-dir="nonexistent_build" >/dev/null 2>&1
RC_BAD_DIR=$?
set -e
assert_eq "$RC_FAIL_TEST" 1 "verify_gate.sh 在 --warn-only 下仍阻断失败的 CTest"
assert_eq "$RC_BAD_DIR" 2 "verify_gate.sh 拒绝不存在的显式 --build-dir"

echo "=================================================="
echo "🎉 Self-Test 全部通过 (${PASS_COUNT} 项核心断言全绿)！"
echo "=================================================="
