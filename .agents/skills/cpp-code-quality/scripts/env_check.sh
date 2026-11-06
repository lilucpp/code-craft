#!/usr/bin/env bash
# ==============================================================================
# C++ 代码质量工具环境检查脚本 (C++ Tooling Pre-flight Check)
# ==============================================================================
set -euo pipefail

echo "=================================================="
echo "🔍 C++ 质量与静态分析工具链环境自检"
echo "=================================================="

MISSING_TOOLS=()
DETECTED_TOOLS=()

check_tool() {
  local tool_name="$1"
  local cmd_name="$2"
  local description="$3"

  if command -v "$cmd_name" &>/dev/null; then
    local version
    version=$("$cmd_name" --version 2>&1 | head -n 1 || echo "installed")
    echo "  [OK] $tool_name: 已就绪 ($version)"
    DETECTED_TOOLS+=("$tool_name")
  else
    echo "  ❌ [MISSING] $tool_name ($description): 未找到"
    MISSING_TOOLS+=("$tool_name")
  fi
}

# 1. 核心格式化工具
check_tool "clang-format" "clang-format" "C++ 代码自动格式化"

# 2. 增量格式化工具
if command -v clang-format-diff &>/dev/null; then
  check_tool "clang-format-diff" "clang-format-diff" "增量行级代码格式化"
elif command -v clang-format-diff.py &>/dev/null; then
  check_tool "clang-format-diff" "clang-format-diff.py" "增量行级代码格式化"
else
  echo "  ⚠️ [OPTIONAL] clang-format-diff: 未找到（可通过 LLVM 附带脚本或 pip 安装）"
fi

# 3. 核心静态检查工具
check_tool "clang-tidy" "clang-tidy" "现代 C++ 语法树级语义静态检查"

# 4. 辅助轻量静态检查工具
check_tool "cppcheck" "cppcheck" "快速未定义行为与内存越界检测"

# 5. 构建系统（用于导出 compile_commands.json）
check_tool "cmake" "cmake" "生成编译数据库供 clang-tidy 使用"

echo "--------------------------------------------------"

# 检测权限状态
CAN_SUDO=false
if [ "$(id -u)" -eq 0 ] || sudo -n true 2>/dev/null; then
  CAN_SUDO=true
fi

# 检测 Docker 状态
DOCKER_READY=false
if command -v docker &>/dev/null && docker ps &>/dev/null; then
  DOCKER_READY=true
fi

if [ ${#MISSING_TOOLS[@]} -eq 0 ]; then
  echo "🎉 恭喜！所有 C++ 代码质量核心工具均已就绪，可直接执行格式化与静态检查。"
  exit 0
else
  echo "⚠️ 检测到部分核心工具缺失: ${MISSING_TOOLS[*]}"
  echo ""
  if [ "$CAN_SUDO" = false ]; then
    echo "🔒 [权限告知] 当前执行环境为普通用户（无 root / 无免密 sudo 特权）。"
    echo "   AI 助手严禁擅自执行交互式提权命令。请由宿主机管理员手动在终端执行以下安装命令："
    echo ""
  else
    echo "👉 推荐安装命令："
    echo ""
  fi

  echo "• Ubuntu / Debian:"
  echo "  sudo apt-get update && sudo apt-get install -y clang-format clang-tidy cppcheck cmake"
  echo ""
  echo "• RHEL / CentOS / Rocky / Fedora:"
  echo "  sudo dnf install -y clang-tools-extra cppcheck cmake"
  echo ""
  echo "• Arch Linux:"
  echo "  sudo pacman -S --noconfirm clang cppcheck cmake"
  echo ""
  echo "• macOS (Homebrew):"
  echo "  brew install clang-format llvm cppcheck cmake"

  if [ "$DOCKER_READY" = true ]; then
    echo ""
    echo "--------------------------------------------------"
    echo "🐳 [DOCKER 容器替代方案] 本机 Docker 守护进程已就绪！"
    echo "   若不想在宿主机安装额外依赖，也可通过 Docker 容器免安装运行格式化或检查，例如："
    echo "   docker run --rm -v \"\$(pwd):/src\" -w /src ghcr.io/muttleyxd/clang-tools:18 clang-format -i \$(git ls-files '*.cpp' '*.h')"
  fi

  echo "=================================================="
  exit 1
fi
