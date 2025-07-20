#!/bin/bash

# 开发环境设置脚本
# 作者: 彭承康

echo "🚀 设置《幻境传说》开发环境..."

# 检查系统类型
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "📋 检测到Linux系统"
    SYSTEM="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "📋 检测到macOS系统"
    SYSTEM="macos"
else
    echo "❌ 不支持的系统类型: $OSTYPE"
    exit 1
fi

# 创建必要的目录
echo "📁 创建项目目录结构..."
mkdir -p {build,logs,temp,configs,resources/{images,sounds,fonts,data}}

# 安装依赖
echo "📦 安装项目依赖..."
if [[ "$SYSTEM" == "linux" ]]; then
    sudo apt update
    sudo apt install -y build-essential cmake git
    sudo apt install -y qt6-base-dev qt6-declarative-dev
    sudo apt install -y libgrpc++-dev libprotobuf-dev protobuf-compiler-grpc
    sudo apt install -y python3-dev python3-pip
elif [[ "$SYSTEM" == "macos" ]]; then
    brew install cmake qt6 grpc protobuf python3
fi

# 安装Python依赖
echo "🐍 安装Python工具依赖..."
pip3 install -r tools/requirements.txt

# 设置环境变量
echo "🔧 设置环境变量..."
export GAME_ROOT=$(pwd)
export GAME_BUILD_DIR="$GAME_ROOT/build"
export GAME_CONFIG_DIR="$GAME_ROOT/configs"

# 创建配置文件
echo "⚙️ 创建默认配置文件..."
cat > configs/development.json << EOF
{
    "game": {
        "title": "幻境传说",
        "version": "0.1.0",
        "debug": true
    },
    "services": {
        "algorithm_port": 50051,
        "strategy_port": 50052,
        "application_port": 8080
    },
    "logging": {
        "level": "DEBUG",
        "file": "logs/game.log"
    }
}
EOF

# 设置Git钩子
echo "🔗 设置Git钩子..."
cp scripts/git-hooks/* .git/hooks/
chmod +x .git/hooks/*

# 编译项目
echo "🔨 编译项目..."
mkdir -p build && cd build
cmake ..
make -j$(nproc)

echo "✅ 开发环境设置完成！"
echo "📝 使用以下命令启动开发："
echo "   cd build && make && ./game_client"