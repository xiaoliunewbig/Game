#!/bin/bash

# ============================================================================
# 《幻境传说》游戏框架 - Qt6环境检查脚本 (CMake版本)
# ============================================================================
# 文件名: scripts/check_qt6.sh
# 说明: 检查CMake项目所需的Qt6环境
# 作者: 彭承康
# 创建时间: 2026-02-18
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

# 检查是否在项目根目录
check_project_root() {
    if [ ! -f "CMakeLists.txt" ] || [ ! -d "application" ]; then
        print_error "请在项目根目录运行此脚本"
        print_info "当前目录: $(pwd)"
        print_info "预期文件: CMakeLists.txt, application/ 目录"
        exit 1
    fi
}

# 检查CMake
check_cmake() {
    print_header "CMake 环境检查"
    
    if ! command -v cmake &> /dev/null; then
        print_error "未安装 CMake"
        print_info "请先安装 CMake 3.16+:"
        echo "  Ubuntu/Debian: sudo apt install cmake"
        echo "  macOS: brew install cmake"
        return 1
    fi
    
    local cmake_version=$(cmake --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+')
    print_success "CMake 版本: $cmake_version"
    
    # 检查版本是否满足要求
    local required_version="3.16.0"
    if [ "$(printf '%s\n' "$required_version" "$cmake_version" | sort -V | head -n1)" = "$required_version" ]; then
        print_success "CMake 版本满足要求 (>= 3.16)"
        return 0
    else
        print_error "CMake 版本过低，需要 >= 3.16，当前: $cmake_version"
        return 1
    fi
}

# 使用CMake检查Qt6
check_qt6_with_cmake() {
    print_header "使用 CMake 检查 Qt6"
    
    # 创建临时测试目录
    local temp_dir=$(mktemp -d)
    local test_result=0
    
    # 创建测试CMakeLists.txt
    cat > "$temp_dir/CMakeLists.txt" << 'EOF'
cmake_minimum_required(VERSION 3.16)
project(Qt6Test)

# 查找Qt6
find_package(Qt6 QUIET COMPONENTS Core Widgets Quick QuickControls2 Multimedia Network)

if(Qt6_FOUND)
    message(STATUS "=== Qt6 检查结果 ===")
    message(STATUS "Qt6_VERSION: ${Qt6_VERSION}")
    message(STATUS "Qt6_DIR: ${Qt6_DIR}")
    
    # 检查各个组件
    if(Qt6Core_FOUND)
        message(STATUS "Qt6Core: FOUND (${Qt6Core_VERSION})")
    else()
        message(STATUS "Qt6Core: NOT FOUND")
    endif()
    
    if(Qt6Widgets_FOUND)
        message(STATUS "Qt6Widgets: FOUND (${Qt6Widgets_VERSION})")
    else()
        message(STATUS "Qt6Widgets: NOT FOUND")
    endif()
    
    if(Qt6Quick_FOUND)
        message(STATUS "Qt6Quick: FOUND (${Qt6Quick_VERSION})")
    else()
        message(STATUS "Qt6Quick: NOT FOUND")
    endif()
    
    if(Qt6QuickControls2_FOUND)
        message(STATUS "Qt6QuickControls2: FOUND (${Qt6QuickControls2_VERSION})")
    else()
        message(STATUS "Qt6QuickControls2: NOT FOUND")
    endif()
    
    if(Qt6Multimedia_FOUND)
        message(STATUS "Qt6Multimedia: FOUND (${Qt6Multimedia_VERSION})")
    else()
        message(STATUS "Qt6Multimedia: NOT FOUND")
    endif()
    
    if(Qt6Network_FOUND)
        message(STATUS "Qt6Network: FOUND (${Qt6Network_VERSION})")
    else()
        message(STATUS "Qt6Network: NOT FOUND")
    endif()
    
    message(STATUS "===================")
else()
    message(FATAL_ERROR "Qt6 not found!")
endif()
EOF
    
    cd "$temp_dir"
    print_info "运行 CMake 配置测试..."
    
    # 运行cmake配置
    local cmake_output
    if cmake_output=$(cmake . 2>&1); then
        print_success "CMake 成功找到 Qt6"
        echo
        
        # 解析输出
        local qt6_version=$(echo "$cmake_output" | grep "Qt6_VERSION:" | sed 's/.*Qt6_VERSION: //')
        local qt6_dir=$(echo "$cmake_output" | grep "Qt6_DIR:" | sed 's/.*Qt6_DIR: //')
        
        if [ -n "$qt6_version" ]; then
            print_success "Qt6 版本: $qt6_version"
        fi
        if [ -n "$qt6_dir" ]; then
            print_success "Qt6 安装路径: $qt6_dir"
        fi
        
        echo
        print_info "Qt6 组件检查结果:"
        
        # 检查各个组件
        local components=("Qt6Core" "Qt6Widgets" "Qt6Quick" "Qt6QuickControls2" "Qt6Multimedia" "Qt6Network")
        local found_components=()
        local missing_components=()
        
        for component in "${components[@]}"; do
            if echo "$cmake_output" | grep -q "$component: FOUND"; then
                local version=$(echo "$cmake_output" | grep "$component: FOUND" | sed "s/.*$component: FOUND (\(.*\))/\1/")
                print_success "  ✅ $component $version"
                found_components+=("$component")
            else
                print_error "  ❌ $component: 未找到"
                missing_components+=("$component")
            fi
        done
        
        echo
        if [ ${#missing_components[@]} -eq 0 ]; then
            print_success "🎉 所有必需的 Qt6 组件都已找到！"
            test_result=0
        else
            print_warning "⚠️  缺少 ${#missing_components[@]} 个组件: ${missing_components[*]}"
            test_result=1
        fi
        
    else
        print_error "CMake 未能找到 Qt6"
        echo
        print_info "CMake 错误输出:"
        echo "$cmake_output" | sed 's/^/  /'
        test_result=1
    fi
    
    # 清理临时目录
    cd - > /dev/null
    rm -rf "$temp_dir"
    
    return $test_result
}

# 测试项目CMake配置
test_project_cmake() {
    print_header "测试项目 CMake 配置"
    
    # 检查是否已有build目录
    if [ -d "build" ]; then
        print_warning "检测到现有 build 目录"
        print_info "是否清理重新配置? (y/N)"
        read -r clean_build
        if [[ $clean_build =~ ^[Yy]$ ]]; then
            print_info "清理 build 目录..."
            rm -rf build
        fi
    fi
    
    # 创建build目录
    mkdir -p build
    cd build
    
    print_info "运行项目 CMake 配置..."
    
    # 运行cmake配置
    if cmake .. 2>&1 | tee cmake_config.log; then
        print_success "✅ 项目 CMake 配置成功！"
        
        # 检查配置日志中的Qt6信息
        if grep -q "Qt6版本:" cmake_config.log; then
            local qt6_version=$(grep "Qt6版本:" cmake_config.log | sed 's/.*Qt6版本: //')
            print_success "项目检测到 Qt6 版本: $qt6_version"
        fi
        
        if grep -q "gRPC版本:" cmake_config.log; then
            local grpc_version=$(grep "gRPC版本:" cmake_config.log | sed 's/.*gRPC版本: //')
            print_success "项目检测到 gRPC 版本: $grpc_version"
        fi
        
        echo
        print_success "🎯 项目已准备好构建！"
        print_info "下一步操作:"
        echo "  cd build"
        echo "  make -j\$(nproc)          # 编译所有目标"
        echo "  make build_all           # 或使用自定义目标"
        
        cd ..
        return 0
    else
        print_error "❌ 项目 CMake 配置失败"
        echo
        print_info "错误日志 (最后20行):"
        tail -n 20 cmake_config.log | sed 's/^/  /'
        
        cd ..
        return 1
    fi
}

# 提供安装建议
provide_installation_guide() {
    print_header "Qt6 安装建议"
    
    local os_type=""
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        os_type="linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        os_type="macos"
    fi
    
    case $os_type in
        "linux")
            print_info "Linux 系统 Qt6 安装 (CMake项目):"
            echo
            echo "1. Ubuntu/Debian 系统:"
            echo "   sudo apt update"
            echo "   sudo apt install -y qt6-base-dev qt6-declarative-dev"
            echo "   sudo apt install -y qt6-multimedia-dev qt6-tools-dev"
            echo "   sudo apt install -y qml6-module-qtquick-controls"
            echo
            echo "2. Fedora/CentOS 系统:"
            echo "   sudo dnf install qt6-qtbase-devel qt6-qtdeclarative-devel"
            echo "   sudo dnf install qt6-qtmultimedia-devel qt6-qttools-devel"
            echo
            echo "3. Arch Linux 系统:"
            echo "   sudo pacman -S qt6-base qt6-declarative qt6-multimedia qt6-tools"
            ;;
        "macos")
            print_info "macOS 系统 Qt6 安装 (CMake项目):"
            echo
            echo "1. 使用 Homebrew (推荐):"
            echo "   brew install qt6"
            echo "   export CMAKE_PREFIX_PATH=\"\$(brew --prefix qt6):\$CMAKE_PREFIX_PATH\""
            echo
            echo "2. 添加到 shell 配置文件:"
            echo "   echo 'export CMAKE_PREFIX_PATH=\"\$(brew --prefix qt6):\$CMAKE_PREFIX_PATH\"' >> ~/.zshrc"
            ;;
        *)
            print_info "通用安装方法:"
            echo "1. 下载 Qt Online Installer: https://www.qt.io/download-qt-installer"
            echo "2. 安装 Qt 6.5+ 版本"
            echo "3. 设置 CMAKE_PREFIX_PATH 环境变量指向Qt安装目录"
            ;;
    esac
    
    echo
    print_info "安装完成后运行:"
    echo "  ./scripts/check_qt6.sh    # 重新检查"
    echo "  ./scripts/install_qt6.sh  # 或使用自动安装脚本"
}

# 主函数
main() {
    print_header "《幻境传说》游戏框架 - Qt6 环境检查 (CMake版)"
    
    # 检查项目根目录
    check_project_root
    
    # 检查CMake
    if ! check_cmake; then
        exit 1
    fi
    
    echo
    
    # 使用CMake检查Qt6
    if check_qt6_with_cmake; then
        echo
        
        # 测试项目配置
        if test_project_cmake; then
            print_success "🎉 环境检查完成，项目可以正常构建！"
        else
            print_error "项目配置失败，请检查依赖项"
            echo
            provide_installation_guide
        fi
    else
        print_error "Qt6 环境不完整"
        echo
        provide_installation_guide
    fi
}

# 运行主函数
main "$@"