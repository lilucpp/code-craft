#!/usr/bin/env bash
# ==============================================================================
# inspect_project.sh
# 快速剖析 C++ 项目整体架构、源文件分布、编译数据库就绪情况与质量规范文件
# ==============================================================================

set -euo pipefail

TARGET_DIR="${1:-.}"
cd "$TARGET_DIR"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${BLUE}======================================================${NC}"
echo -e "${CYAN}      C++ Project Code Intelligence Quick Inspection${NC}"
echo -e "${BLUE}======================================================${NC}"
echo -e "工作目录: $(pwd)"
echo ""

# 1. 编译数据库状态
echo -e "${CYAN}[1] 编译数据库 (Compilation Database)${NC}"
if [[ -f "compile_commands.json" && -s "compile_commands.json" ]]; then
    REAL_TARGET=""
    if [[ -L "compile_commands.json" ]]; then
        REAL_TARGET=" -> $(readlink -f compile_commands.json)"
    fi
    ENTRIES=$(python3 -c "
import json
try:
    with open('compile_commands.json', 'r', encoding='utf-8') as f:
        print(len(json.load(f)))
except Exception:
    print('未知')
" 2>/dev/null || echo "未知")
    echo -e "  状态: ${GREEN}就绪${NC} (compile_commands.json${REAL_TARGET}, 包含约 $ENTRIES 个编译单元)"
else
    echo -e "  状态: ${YELLOW}未发现 compile_commands.json${NC} (clangd 无法获取跨头文件宏展开与精确符号)"
    echo -e "  建议: 运行 ./scripts/setup_compile_commands.sh 自动生成"
fi
echo ""

# 2. 规范与配置文件
echo -e "${CYAN}[2] 代码规范与静态检查配置${NC}"
for conf in ".clang-format" ".clang-tidy" ".clangd" "CMakeLists.txt" "CMakePresets.json"; do
    if [[ -f "$conf" ]]; then
        echo -e "  - $conf: ${GREEN}存在${NC}"
    else
        echo -e "  - $conf: ${YELLOW}缺失${NC}"
    fi
done
echo ""

# 3. 源文件与头文件统计
echo -e "${CYAN}[3] C/C++ 源文件与头文件资产分布${NC}"
HEADER_COUNT=$(find . -maxdepth 4 \( -name "*.h" -o -name "*.hpp" -o -name "*.hxx" \) -not -path "*/.*" -not -path "*/build/*" 2>/dev/null | wc -l || echo "0")
SOURCE_COUNT=$(find . -maxdepth 4 \( -name "*.c" -o -name "*.cpp" -o -name "*.cxx" -o -name "*.cc" \) -not -path "*/.*" -not -path "*/build/*" 2>/dev/null | wc -l || echo "0")

echo -e "  - 头文件 (.h/.hpp): $HEADER_COUNT 个"
echo -e "  - 源文件 (.cpp/.cc): $SOURCE_COUNT 个"
echo ""

# 4. 主要目录架构与顶级模块
echo -e "${CYAN}[4] 顶层模块与目录拓扑${NC}"
find . -maxdepth 2 -type d -not -path "*/.*" -not -path "*/build*" | sort | while read -r dir; do
    if [[ "$dir" != "." ]]; then
        FILES_IN_DIR=$(find "$dir" -maxdepth 1 -type f 2>/dev/null | wc -l || echo "0")
        echo -e "  - $dir (直属文件: $FILES_IN_DIR)"
    fi
done

echo ""
echo -e "${BLUE}======================================================${NC}"
