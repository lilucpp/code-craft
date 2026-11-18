#!/usr/bin/env bash
# ==============================================================================
# Code-Craft 仓库自身回归测试与验证基础设施 (Self-Test Verification Harness)
# 固化所有已发现边界用例与门禁契约，防止工具脚本与 C++17 基础设施模板退化。
# ==============================================================================
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SCRIPTS_QUALITY="${REPO_ROOT}/.agents/skills/cpp-code-quality/scripts"
SCRIPTS_DESIGN="${REPO_ROOT}/.agents/skills/code-design/scripts"
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

echo "[1/6] 脚本语法、Skill 元数据与全仓 Markdown 链接检查..."
bash -n "${SCRIPTS_QUALITY}/env_check.sh"
bash -n "${SCRIPTS_QUALITY}/init_config.sh"
bash -n "${SCRIPTS_QUALITY}/run_check.sh"
bash -n "${REPO_ROOT}/scripts/self_test.sh"
python3 -m py_compile "${SCRIPTS_QUALITY}/lint_redlines.py" "${SCRIPTS_DESIGN}/check_include_dag.py"

python3 - "${REPO_ROOT}" << 'PY'
import glob, os, re, sys, yaml

root = sys.argv[1]
os.chdir(root)

for skill_md in sorted(glob.glob(".agents/skills/*/SKILL.md")):
    with open(skill_md, "r", encoding="utf-8") as f:
        raw = f.read()
    m = re.match(r"^---\n(.*?)\n---\n", raw, re.DOTALL)
    assert m, f"Missing YAML frontmatter in {skill_md}"
    meta = yaml.safe_load(m.group(1))
    name = meta.get("name", "")
    desc = meta.get("description", "")
    assert re.match(r"^[a-z0-9-]+$", name) and len(name) <= 64, f"Invalid name in {skill_md}: {name}"
    assert len(desc) <= 1024, f"Description > 1024 chars in {skill_md}: {len(desc)}"

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

echo "[2/6] 工具链环境与 init_config.sh 一致性检查..."
"${SCRIPTS_QUALITY}/env_check.sh" >/dev/null
assert_eq $? 0 "env_check.sh 工具链就绪"

INIT_DIR="${WORK_DIR}/init_proj"
mkdir -p "${INIT_DIR}"
"${SCRIPTS_QUALITY}/init_config.sh" "${INIT_DIR}" >/dev/null
diff -u "${REPO_ROOT}/.clang-tidy" "${INIT_DIR}/.clang-tidy"
assert_eq $? 0 "init_config.sh 生成的 .clang-tidy 与根目录一致"
diff -u "${REPO_ROOT}/.clang-format" "${INIT_DIR}/.clang-format"
assert_eq $? 0 "init_config.sh 生成的 .clang-format 与根目录一致"

echo "[3/6] run_check.sh 严格门禁 / --warn-only / 0-commit / Monorepo 子目录检查..."
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

echo "[4/6] lint_redlines.py 红线正反例回归测试..."
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
  (void)p;
}
EOF
set +e
RED_OUT=$(python3 "${SCRIPTS_QUALITY}/lint_redlines.py" --check-ui-i18n "${RED_DIR}/bad.cpp" 2>&1)
RC_RED_BAD=$?
set -e
assert_eq "$RC_RED_BAD" 1 "lint_redlines.py 检出反例阻断"
for expected_rule in "strcpy()" "atoi()" "rand()" "system()" "redline-swallowed-catch" "redline-fragile-cwd-path" "redline-ui-hardcoded-text"; do
  if ! printf '%s' "$RED_OUT" | grep -q "$expected_rule"; then
    echo "❌ [FAIL] lint_redlines.py 未匹配预期规则: ${expected_rule}" >&2
    exit 1
  fi
done
echo "  ✅ [PASS] lint_redlines.py 7 类违例全部精准命中"
PASS_COUNT=$((PASS_COUNT + 1))

echo "[5/6] check_include_dag.py 架构边界与环路回归测试..."
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

echo "[6/6] C++17 基础设施原语 (ASan/UBSan + 四关质量门禁) 回归测试..."
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

echo "=================================================="
echo "🎉 Self-Test 全部通过 (${PASS_COUNT} 项核心断言全绿)！"
echo "=================================================="
