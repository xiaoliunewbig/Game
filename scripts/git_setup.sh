#!/bin/bash

# ============================================================================
# 《幻境传说》游戏框架 - Git仓库设置脚本
# ============================================================================
# 文件名: scripts/git_setup.sh
# 说明: 初始化Git仓库并推送到GitHub
# 作者: 彭承康
# 创建时间: 2025-07-20
# ============================================================================

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查是否在项目根目录
if [ ! -f "CMakeLists.txt" ] || [ ! -d "application" ]; then
    print_error "请在项目根目录运行此脚本"
    exit 1
fi

print_info "开始设置《幻境传说》游戏框架的Git仓库..."

# 1. 检查Git是否已安装
if ! command -v git &> /dev/null; then
    print_error "Git未安装，请先安装Git"
    exit 1
fi

# 2. 检查是否已经是Git仓库
if [ ! -d ".git" ]; then
    print_info "初始化Git仓库..."
    git init
    print_success "Git仓库初始化完成"
else
    print_warning "已存在Git仓库，跳过初始化"
fi

# 3. 设置Git配置（如果未设置）
if [ -z "$(git config --global user.name)" ]; then
    print_info "请输入您的Git用户名:"
    read -r git_username
    git config --global user.name "$git_username"
fi

if [ -z "$(git config --global user.email)" ]; then
    print_info "请输入您的Git邮箱:"
    read -r git_email
    git config --global user.email "$git_email"
fi

print_success "Git配置完成"
print_info "用户名: $(git config --global user.name)"
print_info "邮箱: $(git config --global user.email)"

# 4. 检查Git LFS
if command -v git-lfs &> /dev/null; then
    print_info "初始化Git LFS..."
    git lfs install
    print_success "Git LFS初始化完成"
else
    print_warning "Git LFS未安装，大文件将直接存储在Git中"
    print_warning "建议安装Git LFS: https://git-lfs.github.io/"
fi

# 5. 添加所有文件到暂存区
print_info "添加文件到Git暂存区..."
git add .

# 6. 检查暂存区状态
print_info "检查暂存区状态..."
git status --short

# 7. 创建初始提交
if git rev-parse --verify HEAD >/dev/null 2>&1; then
    print_warning "已存在提交历史，创建新的提交..."
    commit_message="feat: 完善游戏框架核心模块和文档注释

- 添加完整的SceneManager、ResourceManager、AudioManager实现
- 完善GameState和InputManager的接口设计
- 添加详细的代码注释和文档说明
- 配置CMake构建系统和CI/CD流水线
- 设置Git LFS管理大文件资源"
else
    print_info "创建初始提交..."
    commit_message="feat: 初始化《幻境传说》游戏框架项目

🎮 项目特性:
- 基于Qt6/C++20的现代游戏框架
- 三层微服务架构设计(Algorithm/Strategy/Application)
- 完整的资源管理和场景系统
- 支持跨平台构建和部署

🏗️ 技术栈:
- Qt 6.3+ (UI框架)
- C++20 (核心逻辑)
- gRPC (服务通信)
- CMake (构建系统)
- Python 3.9+ (开发工具)

📁 项目结构:
- algorithm/: 算法层服务(战斗计算、AI决策)
- strategy/: 策略层服务(游戏规则、状态管理)
- application/: Qt应用层(客户端、UI界面)
- tools/: Python开发工具集
- docs/: 项目文档

✨ 核心功能:
- 场景管理系统(SceneManager)
- 资源管理系统(ResourceManager)
- 音频管理系统(AudioManager)
- 游戏状态管理(GameState)
- 输入处理系统(InputManager)
- 完整的构建和测试流水线"
fi

git commit -m "$commit_message"
print_success "提交创建完成"

# 8. 询问是否推送到GitHub
print_info "是否要推送到GitHub远程仓库? (y/n)"
read -r push_to_github

if [[ $push_to_github =~ ^[Yy]$ ]]; then
    # 检查是否已配置远程仓库
    if git remote get-url origin >/dev/null 2>&1; then
        print_info "检测到已配置的远程仓库:"
        git remote -v
        print_info "是否使用现有远程仓库? (y/n)"
        read -r use_existing
        
        if [[ ! $use_existing =~ ^[Yy]$ ]]; then
            print_info "请输入新的GitHub仓库URL (例: https://github.com/username/Game.git):"
            read -r repo_url
            git remote set-url origin "$repo_url"
        fi
    else
        print_info "请输入GitHub仓库URL (例: https://github.com/xiaoliunewbig/Game.git):"
        read -r repo_url
        git remote add origin "$repo_url"
    fi
    
    # 推送到远程仓库
    print_info "推送到远程仓库..."
    
    # 检查当前分支
    current_branch=$(git branch --show-current)
    print_info "当前分支: $current_branch"
    
    # 推送主分支
    if git push -u origin "$current_branch"; then
        print_success "代码推送成功!"
        print_info "GitHub仓库地址: $(git remote get-url origin)"
    else
        print_error "推送失败，请检查网络连接和仓库权限"
        print_info "您可以稍后手动推送: git push -u origin $current_branch"
    fi
    
    # 推送LFS文件（如果有）
    if command -v git-lfs &> /dev/null; then
        print_info "推送LFS文件..."
        git lfs push origin "$current_branch" || print_warning "LFS文件推送失败，请稍后手动推送"
    fi
else
    print_info "跳过推送到GitHub"
    print_info "您可以稍后手动推送:"
    print_info "  git remote add origin <your-repo-url>"
    print_info "  git push -u origin main"
fi

# 9. 显示后续步骤
print_success "Git仓库设置完成!"
echo
print_info "后续步骤建议:"
echo "1. 在GitHub上启用Actions进行自动化构建"
echo "2. 设置分支保护规则"
echo "3. 配置Issue和PR模板"
echo "4. 添加项目描述和标签"
echo "5. 设置GitHub Pages展示文档"
echo
print_info "开发工作流:"
echo "1. 创建功能分支: git checkout -b feature/new-feature"
echo "2. 开发和测试功能"
echo "3. 提交更改: git commit -m 'feat: add new feature'"
echo "4. 推送分支: git push origin feature/new-feature"
echo "5. 创建Pull Request进行代码审查"
echo
print_success "🎮 《幻境传说》游戏框架已准备就绪！开始您的游戏开发之旅吧！"