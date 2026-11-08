#!/usr/bin/env bash
# ==============================================================================
# setup_compile_commands.sh
# 自动化检测并生成/链接 C++ 项目的 compile_commands.json (编译数据库)
# 供 clangd / clang-tidy / Antigravity MCP 语义分析使用
# ==============================================================================

set -euo pipefail

TARGET_DIR="${1:-.}"
cd "$TARGET_DIR"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}[cpp-intel]${NC} 检查 C++ 编译数据库 (compile_commands.json)..."

# 辅助函数：使用 Python 精准计算条目并验证合法性
count_entries() {
    local file="$1"
    if command -v python3 >/dev/null 2>&1; then
        python3 -c "
import json, sys
try:
    with open(sys.argv[1], 'r', encoding='utf-8') as f:
        data = json.load(f)
        print(len(data) if isinstance(data, list) else 0)
except Exception:
    print(0)
" "$file" 2>/dev/null || echo "0"
    else
        grep -c '"file":' "$file" 2>/dev/null || echo "0"
    fi
}

# 1. 检查根目录是否已有有效的 compile_commands.json
if [[ -f "compile_commands.json" && -s "compile_commands.json" ]]; then
    ENTRIES=$(count_entries "compile_commands.json")
    if [[ "$ENTRIES" -gt 0 ]]; then
        echo -e "${GREEN}[cpp-intel]${NC} 根目录已存在有效编译数据库: compile_commands.json (包含 $ENTRIES 个编译单元)"
        exit 0
    else
        echo -e "${YELLOW}[cpp-intel]${NC} 根目录 compile_commands.json 内容为空或格式异常，尝试重新生成..."
    fi
fi

# 2. 检查常见构建子目录中是否存在
SEARCH_DIRS=("build" "build/Debug" "build/Release" "out" "out/Default" "cmake-build-debug" "cmake-build-release")

# 若存在 CMakePresets.json，额外增加对应常见预设路径
if [[ -f "CMakePresets.json" ]]; then
    SEARCH_DIRS+=("build/debug" "build/release" "build/default")
fi

FOUND_PATH=""
for d in "${SEARCH_DIRS[@]}"; do
    if [[ -f "$d/compile_commands.json" && -s "$d/compile_commands.json" ]]; then
        FOUND_PATH="$d/compile_commands.json"
        break
    fi
done

if [[ -n "$FOUND_PATH" ]]; then
    echo -e "${GREEN}[cpp-intel]${NC} 在子目录发现编译数据库: $FOUND_PATH"
    ln -sf "$FOUND_PATH" compile_commands.json
    ENTRIES=$(count_entries "compile_commands.json")
    echo -e "${GREEN}[cpp-intel]${NC} 已在根目录创建软链接: compile_commands.json -> $FOUND_PATH ($ENTRIES 个编译单元)"
    exit 0
fi

# 3. 未找到现有文件，尝试感知构建系统并自动生成
echo -e "${YELLOW}[cpp-intel]${NC} 未检测到现成编译数据库，开始感知项目构建类型..."

if [[ -f "CMakeLists.txt" ]]; then
    echo -e "${BLUE}[cpp-intel]${NC} 识别到 CMake 项目，正在生成编译数据库..."
    mkdir -p build
    
    # 优先使用 ninja (若已安装)
    GENERATOR_FLAG=""
    if command -v ninja >/dev/null 2>&1; then
        GENERATOR_FLAG="-G Ninja"
    fi
    
    cmake -B build $GENERATOR_FLAG -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >/dev/null 2>&1 || {
        echo -e "${YELLOW}[cpp-intel]${NC} Ninja 生成暂未完成，尝试标准 Makefile 默认配置生成..."
        cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON || true
    }

    if [[ -f "build/compile_commands.json" && -s "build/compile_commands.json" ]]; then
        ln -sf build/compile_commands.json compile_commands.json
        ENTRIES=$(count_entries "compile_commands.json")
        echo -e "${GREEN}[cpp-intel]${NC} 成功生成并链接: compile_commands.json (包含 $ENTRIES 个编译单元)"
        exit 0
    fi
fi

if [[ -f "Makefile" || -f "makefile" ]]; then
    echo -e "${BLUE}[cpp-intel]${NC} 识别到 Makefile 项目..."
    if command -v bear >/dev/null 2>&1; then
        echo -e "${YELLOW}[cpp-intel]${NC} 检测到 bear 工具。若需导出编译数据库，可运行: bear -- make -j$(nproc)"
    else
        echo -e "${YELLOW}[cpp-intel]${NC} 提示: 对于 Makefile 项目，建议使用 bear 截获编译命令: bear -- make"
    fi
fi

echo -e "${RED}[cpp-intel] 警告: 未能自动生成 compile_commands.json。${NC}"
echo -e "建议在构建时启用 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON 或通过 bear 导出。"
exit 1
