#!/bin/bash
# 自动化需求规格说明书静态质量机检脚本
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
REPO_ROOT="$( cd "$DIR/../.." >/dev/null 2>&1 && pwd )"
LINTER_SCRIPT="$REPO_ROOT/.agents/skills/software-requirements/scripts/lint_requirements.py"
SPEC_FILE="$DIR/filegate_requirements_spec.md"

echo "=========================================================="
echo " [Karl Wiegers] 执行软件需求工程静态坏味道自动化扫描"
echo " 目标文件: $SPEC_FILE"
echo "=========================================================="

python3 "$LINTER_SCRIPT" "$SPEC_FILE"
echo ""
echo "✅ 恭喜！需求规格说明书静态质量检测通过，0 ERROR 严重坏味道违规！"
