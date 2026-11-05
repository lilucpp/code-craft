#!/usr/bin/env bash
# ==============================================================================
# Script: package_agensi.sh
# Purpose: Package Code-Craft agent skills for Agensi.io distribution
#          - Strips non-essential/copyrighted binary resources (ebooks, etc.)
#          - Packages each skill into an individual standalone zip with SKILL.md
#          - Packages a full bundle zip containing all skills & rules
#          - Generates ready-to-copy listing descriptions for Agensi.io upload
# ==============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
SKILLS_DIR="${REPO_ROOT}/.agents/skills"
RULES_DIR="${REPO_ROOT}/.agents/rules"
OUTPUT_DIR="${REPO_ROOT}/dist/agensi"

# Colors for terminal output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

print_info()  { echo -e "${BLUE}[INFO]${NC} $*"; }
print_ok()    { echo -e "${GREEN}[OK]${NC} $*"; }
print_warn()  { echo -e "${YELLOW}[WARN]${NC} $*"; }
print_err()   { echo -e "${RED}[ERROR]${NC} $*" >&2; }

# Check required commands
if ! command -v zip &>/dev/null; then
    print_err "'zip' command not found. Please install zip (e.g., sudo apt install zip)."
    exit 1
fi

if [[ ! -d "${SKILLS_DIR}" ]]; then
    print_err "Skills directory not found at: ${SKILLS_DIR}"
    exit 1
fi

print_info "============================================================"
print_info "📦 Packaging Code-Craft Agent Skills for Agensi.io"
print_info "============================================================"
print_info "Repository root : ${REPO_ROOT}"
print_info "Skills directory: ${SKILLS_DIR}"
print_info "Output directory: ${OUTPUT_DIR}"

# Ensure clean output directory
rm -rf "${OUTPUT_DIR}"
mkdir -p "${OUTPUT_DIR}"

# 1. Package individual skills
print_info ""
print_info "--- Step 1: Packaging Individual Skills ---"

SKILL_COUNT=0
declare -a PACKAGED_SKILLS=()

# Skills excluded from Agensi packaging (e.g. external upstream imports from anthropics/skills)
EXCLUDED_SKILLS=("frontend-design" "pdf" "docx")

for skill_path in "${SKILLS_DIR}"/*; do
    if [[ -d "${skill_path}" && -f "${skill_path}/SKILL.md" ]]; then
        skill_name="$(basename "${skill_path}")"

        # Skip excluded external skills
        if [[ " ${EXCLUDED_SKILLS[*]} " =~ " ${skill_name} " ]]; then
            print_warn "Skipping external upstream skill: ${CYAN}${skill_name}${NC} (excluded from packaging)"
            continue
        fi

        zip_file="${OUTPUT_DIR}/${skill_name}.zip"

        print_info "Packaging skill: ${CYAN}${skill_name}${NC} -> ${skill_name}.zip"

        # Change into the skill directory to ensure SKILL.md is at the root of the zip archive
        (
            cd "${skill_path}"
            zip -q -r -9 "${zip_file}" . \
                -x "*.DS_Store" \
                -x "*~" \
                -x "*.tmp" \
                -x "*.swp" \
                -x "*.log" \
                -x "*.epub" \
                -x "*.pdf" \
                -x "*.mobi" \
                -x "*.azw3" \
                -x "__pycache__/*"
        )

        file_size="$(du -h "${zip_file}" | cut -f1)"
        file_count="$(unzip -l "${zip_file}" | tail -n 1 | awk '{print $2}')"
        print_ok "Generated ${skill_name}.zip (${file_size}, ${file_count} files)"

        PACKAGED_SKILLS+=("${skill_name}")
        SKILL_COUNT=$((SKILL_COUNT + 1))
    fi
done

# 2. Package Full Bundle
print_info ""
print_info "--- Step 2: Packaging Full Code-Craft Bundle ---"

BUNDLE_ZIP="${OUTPUT_DIR}/code-craft-bundle.zip"
TEMP_BUNDLE_DIR="$(mktemp -d)"

mkdir -p "${TEMP_BUNDLE_DIR}/.agents/skills"
mkdir -p "${TEMP_BUNDLE_DIR}/.agents/rules"

# Copy all skills (excluding ebooks/binaries)
for s in "${PACKAGED_SKILLS[@]}"; do
    cp -r "${SKILLS_DIR}/${s}" "${TEMP_BUNDLE_DIR}/.agents/skills/"
done

# Copy global rules if existing
if [[ -d "${RULES_DIR}" ]]; then
    cp -r "${RULES_DIR}"/* "${TEMP_BUNDLE_DIR}/.agents/rules/"
fi

# Copy main documentation & configs (excluding resources/ebooks)
[[ -f "${REPO_ROOT}/README.md" ]] && cp "${REPO_ROOT}/README.md" "${TEMP_BUNDLE_DIR}/"
[[ -f "${REPO_ROOT}/README_EN.md" ]] && cp "${REPO_ROOT}/README_EN.md" "${TEMP_BUNDLE_DIR}/"
[[ -f "${REPO_ROOT}/.clang-format" ]] && cp "${REPO_ROOT}/.clang-format" "${TEMP_BUNDLE_DIR}/"
[[ -f "${REPO_ROOT}/.clang-tidy" ]] && cp "${REPO_ROOT}/.clang-tidy" "${TEMP_BUNDLE_DIR}/"

(
    cd "${TEMP_BUNDLE_DIR}"
    zip -q -r -9 "${BUNDLE_ZIP}" . \
        -x "*.DS_Store" \
        -x "*~" \
        -x "*.tmp" \
        -x "*.epub" \
        -x "*.pdf"
)
rm -rf "${TEMP_BUNDLE_DIR}"

bundle_size="$(du -h "${BUNDLE_ZIP}" | cut -f1)"
bundle_file_count="$(unzip -l "${BUNDLE_ZIP}" | tail -n 1 | awk '{print $2}')"
print_ok "Generated code-craft-bundle.zip (${bundle_size}, ${bundle_file_count} files)"

# 3. Generate Agensi Listing Copy & Metadata (Bilingual EN / ZH)
print_info ""
print_info "--- Step 3: Generating Agensi.io Listing Copy Reference ---"

LISTING_DOC="${OUTPUT_DIR}/agensi_listing_copy.md"

cat <<'EOF' > "${LISTING_DOC}"
# 📋 Agensi.io Upload Copy & Metadata Guide
# (Agensi.io 上架文案与元数据快速粘贴指南)

> **Instructions**: When uploading each skill zip to https://www.agensi.io/sell, you can copy the corresponding title, tagline, description, tags, and set the price to **Free ($0)**.

---

## 📦 Overview of Generated Packages

- `code-design.zip` - Evolutionary & Object-Oriented Design (Meso Tier)
- `code-review.zip` - 22 Code Smells Diagnosis & Checklist (Micro Tier)
- `cpp-code-quality.zip` - C++ Toolchain, Clang-Format & Clang-Tidy (Tooling Tier)
- `refactoring.zip` - 68 Refactoring Mechanics & Test Safety (Micro Tier)
- `tech-communication.zip` - Technical Writing, MRE & Root Cause Analysis (Communication Tier)
- `unix-philosophy.zip` - Unix TAOUP Architecture & CLI Standards (Macro Tier)
- `code-craft-bundle.zip` - Full Code-Craft 6-in-1 Suite (All Skills + Coding Rules)

---

## 1. code-design

**Title**: Code-Design: Evolutionary & OO Architecture for AI Agents
**Pricing**: Free ($0)
**Tags**: `agent-skills`, `software-architecture`, `evolutionary-design`, `solid-principles`, `antigravity`

### English Description
```markdown
Empower your autonomous AI coding agents with evolutionary design, component architecture, and object-oriented contract discipline.

### Key Capabilities:
- **Package Topology**: Common Closure Principle (CCP), Common Reuse Principle (CRP), Acyclic Dependencies Principle (ADP) with cycle-breaking patterns.
- **Interface & Contract**: Open/Closed Principle (OCP), Interface Segregation Principle (ISP), Design by Contract (DbC: Pre/Post-conditions & Invariants).
- **Evolutionary Design**: YAGNI anti-over-engineering, Composition over Inheritance, Convention over Configuration (CoC).
- **Progressive Disclosure**: Built-in reference guides for component cohesion, coupling metrics, and interface contracts.
```

### 中文描述 (可选/备份)
```text
基于《重构》演进式设计、Uncle Bob 组件原则与面向对象契约原则的模块设计与建模技能。
指导 AI Agent 在包结构拓扑解耦（CCP/CRP/ADP）、接口契约设计（OCP/ISP/DbC）以及避免过度设计（YAGNI/组合优于继承）方面做出资深架构师水准的决策。
```

---

## 2. code-review

**Title**: Code-Review: 22 Code Smells & Quality Diagnosis for AI Agents
**Pricing**: Free ($0)
**Tags**: `agent-skills`, `code-review`, `code-smells`, `refactoring`, `code-quality`

### English Description
```markdown
Transform your AI agent into a senior code reviewer equipped with Martin Fowler's 22 Code Smells catalog and an industrial-grade diagnostic workflow.

### Key Capabilities:
- **22 Code Smells Diagnostic Manual**: Systematic detection of Divergent Change, Shotgun Surgery, Feature Envy, Primitive Obsession, Long Parameter Lists, and more.
- **3-Tier Review Checklist**: Comprehensive verification across package topology, object-oriented contracts, and function-level logic.
- **Actionable Remediation**: Maps every detected code smell directly to canonical refactoring mechanics.
```

### 中文描述
```text
基于马丁·福勒《重构》第3章的 22 种代码坏味道与现代软件工程标准的代码审查(Code Review)技能。
为 AI Agent 提供工业级审查工作流，自动定位代码异味并输出严重度分级与针对性重构建议。
```

---

## 3. cpp-code-quality

**Title**: C++ Code Quality: Clang-Format, Clang-Tidy & Incremental Analysis
**Pricing**: Free ($0)
**Tags**: `agent-skills`, `cpp`, `clang-tidy`, `clang-format`, `static-analysis`

### English Description
```markdown
Comprehensive C++ code quality and formatting toolchain integration designed specifically for AI agents.

### Key Capabilities:
- **Automated Toolchain Detection**: Includes self-check script `env_check.sh` to diagnose installed LLVM/Clang tools.
- **One-Click Config Init**: Generates standard Google C++ style `.clang-format` (120 columns, right pointer alignment), high signal-to-noise `.clang-tidy`, and `.clang-format-ignore`.
- **Incremental & Legacy-Friendly**: Built-in support for checking only modified lines (Boy Scout rule) while strictly ignoring third-party libraries and build artifacts.
```

### 中文描述
```text
现代 C++ 代码规范格式化与静态检查专家技能。
提供环境自检脚本、一键生成高信噪比 .clang-format 与 .clang-tidy、支持老项目增量检查与三方库告警隔离。
```

---

## 4. refactoring

**Title**: Refactoring: 68 Mechanics Catalog & Safe Execution for AI Agents
**Pricing**: Free ($0)
**Tags**: `agent-skills`, `refactoring`, `clean-code`, `martin-fowler`, `test-driven`

### English Description
```markdown
Safe, disciplined code refactoring guide for autonomous AI agents, powered by Martin Fowler's 68 canonical refactoring mechanics.

### Key Capabilities:
- **Safety-First Iron Rules**: Enforces runnable test/demo verification before touching production code; strict "Two Hats" separation (feature development vs. refactoring).
- **68 Refactoring Mechanics Catalog**: Step-by-step execution procedures for composing methods, moving features, organizing data, and simplifying conditional logic.
- **Smell-to-Refactoring Matrix**: Instant lookup mapping specific code anomalies to the safest refactoring techniques.
```

### 中文描述
```text
基于马丁·福勒《重构》68 种经典手法的代码安全重构技能。
坚持测试安全网第一与小步快跑（Baby Steps）原则，指导 AI Agent 在不改变外部行为的前提下深度改善代码架构。
```

---

## 5. tech-communication

**Title**: Tech-Communication: Engineering RFCs, Stack Overflow Asking & RCA
**Pricing**: Free ($0)
**Tags**: `agent-skills`, `technical-writing`, `troubleshooting`, `root-cause-analysis`, `documentation`

### English Description
```markdown
Elevate your AI agent's communication, technical writing, problem-solving, and post-mortem diagnosis.

### Key Capabilities:
- **CoolShell Technical Sharing Model**: Structured knowledge transfer: Problem Model -> Alternatives Comparison -> Internals & Mechanics -> Best Practices.
- **Stack Overflow Asking Standard**: Guides agent to produce Minimal Reproducible Examples (MREs) and precise reproduction logs without guesswork.
- **Industry-Grade README Guidelines**: Clear value propositions, visual architecture diagrams, 3-minute quick starts, and trade-off rationales.
- **Architect-Level RCA (Root Cause Analysis)**: Conclusion-first executive summaries, 5-Whys deduction, and minimal surgical fixes.
```

### 中文描述
```text
融合酷壳（陈皓）技术分享心法与 Stack Overflow 提问规范的技术表达、文档编写、排障提问与答疑技能。
规范 AI Agent 在技术方案（RFC）、项目 README、排障 MRE 提取、社区求助与故障复盘（RCA）中的输出品质。
```

---

## 6. unix-philosophy

**Title**: Unix-Philosophy: System Architecture & CLI Design for AI Agents
**Pricing**: Free ($0)
**Tags**: `agent-skills`, `unix-philosophy`, `system-architecture`, `cli-design`, `kiss`

### English Description
```markdown
System architecture and command-line interface design skills rooted in Eric S. Raymond's classic *The Art of UNIX Programming* (TAOUP).

### Key Capabilities:
- **17 Unix Rules In-Depth**: Modularity, Compactness, Orthogonality, Separation of Policy and Mechanism, Extensibility, and Rule of Silence.
- **10 Classic Interface Patterns**: Filter, Cantrip, Engine-Interface Separation, CLI Server, and Data-Driven Dispatch.
- **CLI & Configuration Standards**: Option dictionaries (`-a` to `-z`), 5-tier configuration precedence, and strict `--` terminator parsing.
```

### 中文描述
```text
沉淀自埃里克·雷蒙德《UNIX编程艺术》与酷壳应对多变需求工程心法的系统架构与接口设计技能。
指导 AI Agent 贯彻“解耦、KISS、拼装”哲学，设计健壮的 CLI 工具、进程间通信契约与配置拓扑。
```

---

## 7. code-craft-bundle (Optional Full Suite)

**Title**: Code-Craft: Full 6-in-1 Software Engineering & Architecture Suite
**Pricing**: Free ($0)
**Tags**: `agent-skills`, `software-engineering`, `architecture`, `clean-code`, `refactoring`, `antigravity`

### English Description
```markdown
The complete Code-Craft engineering suite for autonomous AI coding agents. Includes all 6 specialized domain skills (code-design, code-review, cpp-code-quality, refactoring, tech-communication, unix-philosophy) plus the global baseline coding standards rule.
```
EOF

print_ok "Generated listing descriptions at: ${LISTING_DOC}"

print_info ""
print_info "============================================================"
print_ok "🎉 All packaging tasks completed successfully!"
print_info "============================================================"
print_info "Output directory contents:"
ls -lh "${OUTPUT_DIR}"
