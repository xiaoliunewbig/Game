#!/bin/bash

# ============================================================================
# 《幻境传说》游戏框架 - CMake构建脚本
# ============================================================================
# 文件名: scripts/build.sh
# 说明: 一键构建整个CMake项目
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

# 检查项目根目录
check_project_root() {
    if [ ! -f "CMakeLists.txt" ]; then
        print_error "请在项目根目录运行此脚本"
        exit 1
    fi
}

# 检查依赖
check_dependencies() {
    print_header "检查构建依赖"
    
    local missing_deps=()
    
    # 检查CMake
    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    else
        local cmake_version=$(cmake --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+')
        print_success "CMake: $cmake_version"
    fi
    
    # 检查编译器
    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        missing_deps+=("g++/clang++")
    else
        if command -v g++ &> /dev/null; then
            local gcc_version=$(g++ --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+')
            print_success "GCC: $gcc_version"
        fi
        if command -v clang++ &> /dev/null; then
            local clang_version=$(clang++ --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+')
            print_success "Clang: $clang_version"
        fi
    fi
    
    # 检查make
    if ! command -v make &> /dev/null; then
        missing_deps+=("make")
    else
        print_success "Make: 已安装"
    fi
    
    if [ ${#missing_deps[@]} -gt 0 ]; then
        print_error "缺少依赖: ${missing_deps[*]}"
        print_info "请先安装缺少的依赖项"
        return 1
    fi
    
    return 0
}

# 配置构建
configure_build() {
    print_header "配置 CMake 构建"
    
    local build_type="${1:-Release}"
    local clean_build="${2:-false}"
    
    # 清理构建目录
    if [ "$clean_build" = "true" ] && [ -d "build" ]; then
        print_info "清理现有构建目录..."
        rm -rf build
    fi
    
    # 创建构建目录
    mkdir -p build
    cd build
    
    print_info "运行 CMake 配置 (构建类型: $build_type)..."
    
    # CMake配置选项
    local cmake_args=(
        "-DCMAKE_BUILD_TYPE=$build_type"
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"  # 生成compile_commands.json
    )
    
    # 如果是Debug模式，启用更多调试信息
    if [ "$build_type" = "Debug" ]; then
        cmake_args+=("-DCMAKE_VERBOSE_MAKEFILE=ON")
    fi
    
    # 运行CMake配置
    if cmake "${cmake_args[@]}" ..; then
        print_success "CMake 配置成功"
        cd ..
        return 0
    else
        print_error "CMake 配置失败"
        cd ..
        return 1
    fi
}

# 构建项目
build_project() {
    print_header "构建项目"
    
    local target="${1:-build_all}"
    local jobs="${2:-$(nproc 2>/dev/null || echo 4)}"
    
    cd build
    
    print_info "开始构建目标: $target (使用 $jobs 个并行任务)..."
    
    # 构建项目
    if make -j"$jobs" "$target"; then
        print_success "构建成功完成"
        
        # 显示构建结果
        print_info "构建产物:"
        find . -name "algorithm_service" -o -name "strategy_service" -o -name "game_client" | while read -r file; do
            if [ -x "$file" ]; then
                print_success "  ✅ $file"
            fi
        done
        
        cd ..
        return 0
    else
        print_error "构建失败"
        cd ..
        return 1
    fi
}

# 运行测试
run_tests() {
    print_header "运行测试"
    
    cd build
    
    if [ -f "Makefile" ] && make -n test &>/dev/null; then
        print_info "运行测试套件..."
        if make test; then
            print_success "所有测试通过"
        else
            print_warning "部分测试失败"
        fi
    else
        print_warning "未找到测试目标"
    fi
    
    cd ..
}

# 显示使用帮助
show_help() {
    echo "用法: $0 [选项]"
    echo
    echo "选项:"
    echo "  -t, --type TYPE     构建类型 (Debug|Release|RelWithDebInfo) [默认: Release]"
    echo "  -j, --jobs JOBS     并行任务数 [默认: CPU核心数]"
    echo "  -c, --clean         清理构建目录"
    echo "  --target TARGET     指定构建目标 [默认: build_all]"
    echo "  --test              构建后运行测试"
    echo "  --check-deps        仅检查依赖项"
    echo "  -h, --help          显示此帮助信息"
    echo
    echo "示例:"
    echo "  $0                          # Release构建"
    echo "  $0 -t Debug -c              # Debug构建，清理重建"
    echo "  $0 --target game_client     # 仅构建游戏客户端"
    echo "  $0 --test                   # 构建并运行测试"
}

# 主函数
main() {
    local build_type="Release"
    local jobs=""
    local clean_build="false"
    local target="build_all"
    local run_test="false"
    local check_deps_only="false"
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -t|--type)
                build_type="$2"
                shift 2
                ;;
            -j|--jobs)
                jobs="$2"
                shift 2
                ;;
            -c|--clean)
                clean_build="true"
                shift
                ;;
            --target)
                target="$2"
                shift 2
                ;;
            --test)
                run_test="true"
                shift
                ;;
            --check-deps)
                check_deps_only="true"
                shift
                ;;
            -h|--help)
                show_help
                exit 0
                ;;
            *)
                print_error "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # 设置默认并行任务数
    if [ -z "$jobs" ]; then
        jobs=$(nproc 2>/dev/null || echo 4)
    fi
    
    print_header "《幻境传说》游戏框架 - CMake 构建脚本"
    
    # 检查项目根目录
    check_project_root
    
    # 检查依赖
    if ! check_dependencies; then
        exit 1
    fi
    
    if [ "$check_deps_only" = "true" ]; then
        print_success "依赖检查完成"
        exit 0
    fi
    
    # 配置构建
    if ! configure_build "$build_type" "$clean_build"; then
        exit 1
    fi
    
    # 构建项目
    if ! build_project "$target" "$jobs"; then
        exit 1
    fi
    
    # 运行测试
    if [ "$run_test" = "true" ]; then
        run_tests
    fi
    
    print_success "🎉 构建流程完成！"
    
    # 显示下一步操作
    echo
    print_info "下一步操作:"
    echo "  cd build"
    echo "  ./algorithm/algorithm_service    # 运行算法服务"
    echo "  ./strategy/strategy_service      # 运行策略服务"
    echo "  ./application/game_client        # 运行游戏客户端"
    echo
    echo "或使用 Docker:"
    echo "  docker-compose up --build"
}

# 运行主函数
main "$@"