#!/bin/bash

# 创建Application层目录结构
# 作者: 彭承康

echo "🏗️ 创建Application层目录结构..."

# 创建主要目录
mkdir -p application/{src,include,qml,resources,tests}

# 创建源码目录结构
mkdir -p application/src/{core,ui,network,game,audio,input,graphics}
mkdir -p application/include/{core,ui,network,game,audio,input,graphics}

# 创建QML目录结构
mkdir -p application/qml/{components,views,dialogs}

# 创建资源目录结构
mkdir -p application/resources/{images,sounds,fonts,data}

# 创建测试目录
mkdir -p application/tests/{unit,integration}

echo "✅ Application层目录结构创建完成!"
echo "📁 目录结构:"
tree application/ || ls -la application/