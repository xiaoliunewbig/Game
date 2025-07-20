#!/bin/bash

# ============================================================================
# 《幻境传说》游戏框架 - Qt6自动安装脚本
# ============================================================================
# 文件名: scripts/install_qt6.sh
# 说明: 自动安装Qt6及相关依赖
# 作者: 彭承康
# 创建时间: 2025-07-20
# ============================================================================

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

print_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }
print_header() { echo -e "${CYAN}========================================${NC}\n${CYAN}$1${NC}\n${CYAN}========================================${NC}"; }

# 检测系统类型
detect_system() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        OS=$NAME
        VER=$VERSION_ID
    elif type lsb_release >/dev/null 2>&1; then
        OS=$(lsb_release -si)
        VER=$(lsb_release -sr)
    elif [ -f /etc/redhat-release ]; then
        OS="Red Hat Enterprise Linux"
        VER=$(cat /etc/redhat-release | grep -o '[0-9]\+\.[0-9]\+')
    else
        OS=$(uname -s)
        VER=$(uname -r)
    fi
    
    print_info "检测到系统: $OS $VER"
}

# 检查是否为root用户
check_root() {
    if [ "$EUID" -ne 0 ]; then
        print_error "此脚本需要root权限运行"
        print_info "请使用: sudo $0"
        exit 1
    fi
}

# Ubuntu/Debian系统安装Qt6
install_qt6_ubuntu() {
    print_header "Ubuntu/Debian 系统安装 Qt6"
    
    print_info "更新软件包列表..."
    apt update
    
    print_info "安装基础构建工具..."
    apt install -y build-essential cmake git pkg-config
    
    print_info "安装Qt6核心包..."
    apt install -y \
        qt6-base-dev \
        qt6-base-dev-tools \
        qt6-declarative-dev \
        qt6-declarative-dev-tools \
        qt6-tools-dev \
        qt6-tools-dev-tools
    
    print_info "安装Qt6多媒体和网络支持..."
    apt install -y \
        qt6-multimedia-dev \
        libqt6multimedia6 \
        qt6-connectivity-dev
    
    print_info "安装QML模块..."
    apt install -y \
        qml6-module-qtquick \
        qml6-module-qtquick-controls \
        qml6-module-qtquick-layouts \
        qml6-module-qtquick-window \
        qml6-module-qtquick-dialogs \
        qml6-module-qtmultimedia
    
    print_info "安装额外的Qt6组件..."
    apt install -y \
        qt6-svg-dev \
        qt6-image-formats-plugins \
        qt6-wayland-dev || true  # 可选，某些系统可能没有
    
    print_success "Qt6 安装完成！"
}

# CentOS/RHEL/Fedora系统安装Qt6
install_qt6_redhat() {
    print_header "Red Hat 系列系统安装 Qt6"
    
    # 检测包管理器
    if command -v dnf &> /dev/null; then
        PKG_MGR="dnf"
    elif command -v yum &> /dev/null; then
        PKG_MGR="yum"
    else
        print_error "未找到包管理器 (dnf/yum)"
        exit 1
    fi
    
    print_info "使用包管理器: $PKG_MGR"
    
    print_info "安装基础构建工具..."
    $PKG_MGR install -y gcc gcc-c++ cmake git make pkgconfig
    
    print_info "安装Qt6开发包..."
    $PKG_MGR install -y \
        qt6-qtbase-devel \
        qt6-qtdeclarative-devel \
        qt6-qttools-devel \
        qt6-qtmultimedia-devel \
        qt6-qtsvg-devel
    
    print_success "Qt6 安装完成！"
}

# Arch Linux系统安装Qt6
install_qt6_arch() {
    print_header "Arch Linux 系统安装 Qt6"
    
    print_info "更新系统..."
    pacman -Syu --noconfirm
    
    print_info "安装基础构建工具..."
    pacman -S --noconfirm base-devel cmake git
    
    print_info "安装Qt6..."
    pacman -S --noconfirm \
        qt6-base \
        qt6-declarative \
        qt6-tools \
        qt6-multimedia \
        qt6-svg \
        qt6-connectivity
    
    print_success "Qt6 安装完成！"
}

# 验证Qt6安装
verify_qt6_installation() {
    print_header "验证 Qt6 安装"
    
    # 检查qmake6
    if command -v qmake6 &> /dev/null; then
        local qmake_version=$(qmake6 --version | grep "Qt version" | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+')
        print_success "qmake6 版本: $qmake_version"
    else
        print_warning "qmake6 未找到"
    fi
    
    # 检查moc6
    if command -v moc6 &> /dev/null; then
        print_success "moc6: 已安装"
    else
        print_warning "moc6 未找到"
    fi
    
    # 检查Qt6库文件
    local qt6_lib_paths=(
        "/usr/lib/x86_64-linux-gnu/libQt6Core.so"
        "/usr/lib64/libQt6Core.so"
        "/usr/lib/libQt6Core.so"
    )
    
    local found_lib=false
    for lib_path in "${qt6_lib_paths[@]}"; do
        if [ -f "$lib_path" ]; then
            print_success "Qt6 库文件: $lib_path"
            found_lib=true
            break
        fi
    done
    
    if [ "$found_lib" = false ]; then
        print_warning "未找到Qt6库文件"
    fi
    
    # 使用CMake测试Qt6
    print_info "使用CMake测试Qt6..."
    local temp_dir=$(mktemp -d)
    cat > "$temp_dir/CMakeLists.txt" << 'EOF'
cmake_minimum_required(VERSION 3.16)
project(Qt6Test)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Quick)
message(STATUS "Qt6 found: ${Qt6_VERSION}")
EOF
    
    cd "$temp_dir"
    if cmake . &> /dev/null; then
        print_success "CMake 可以找到 Qt6"
    else
        print_error "CMake 无法找到 Qt6"
    fi
    
    cd - > /dev/null
    rm -rf "$temp_dir"
}

# 设置环境变量
setup_environment() {
    print_header "设置环境变量"
    
    # 查找Qt6安装路径
    local qt6_paths=(
        "/usr/lib/x86_64-linux-gnu/cmake/Qt6"
        "/usr/lib64/cmake/Qt6"
        "/usr/lib/cmake/Qt6"
        "/usr/share/cmake/Qt6"
    )
    
    local qt6_dir=""
    for path in "${qt6_paths[@]}"; do
        if [ -d "$path" ]; then
            qt6_dir="$path"
            break
        fi
    done
    
    if [ -n "$qt6_dir" ]; then
        print_success "Qt6 CMake 配置目录: $qt6_dir"
        
        # 创建环境变量设置脚本
        cat > "/etc/profile.d/qt6.sh" << EOF
# Qt6 环境变量
export Qt6_DIR="$qt6_dir"
export CMAKE_PREFIX_PATH="\$CMAKE_PREFIX_PATH:$qt6_dir"
EOF
        
        print_success "已创建环境变量配置: /etc/profile.d/qt6.sh"
        print_info "重新登录或运行 'source /etc/profile.d/qt6.sh' 生效"
    else
        print_warning "未找到Qt6 CMake配置目录"
    fi
}

# 主函数
main() {
    print_header "《幻境传说》游戏框架 - Qt6 自动安装"
    
    # 检查root权限
    check_root
    
    # 检测系统
    detect_system
    
    # 根据系统类型安装Qt6
    case "$OS" in
        *"Ubuntu"*|*"Debian"*)
            install_qt6_ubuntu
            ;;
        *"CentOS"*|*"Red Hat"*|*"Fedora"*)
            install_qt6_redhat
            ;;
        *"Arch"*)
            install_qt6_arch
            ;;
        *)
            print_error "不支持的系统: $OS"
            print_info "请手动安装Qt6或参考官方文档"
            exit 1
            ;;
    esac
    
    echo
    
    # 设置环境变量
    setup_environment
    
    echo
    
    # 验证安装
    verify_qt6_installation
    
    echo
    print_success "🎉 Qt6 安装完成！"
    
    print_info "下一步操作:"
    echo "1. 重新加载环境变量:"
    echo "   source /etc/profile.d/qt6.sh"
    echo
    echo "2. 验证安装:"
    echo "   ./scripts/check_qt6.sh"
    echo
    echo "3. 构建项目:"
    echo "   ./scripts/build.sh"
}

# 运行主函数
main "$@"