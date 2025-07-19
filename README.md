# 《幻境传说》游戏框架

一款基于 Qt/C++/Python 技术栈的 2D 动作角色扮演游戏，采用三层微服务架构设计。

## 🎮 项目概述

### 核心特性
- **丰富的剧情系统**：6个主线章节，50+支线任务，多重结局
- **策略战斗系统**：连击、元素克制、状态效果
- **角色成长系统**：3种职业、技能树、装备强化
- **动态世界系统**：NPC关系、世界状态变化

### 技术栈
| 技术 | 版本 | 用途 |
|------|------|------|
| Qt 6.3+ | 最新稳定版 | UI框架 |
| C++20 | 现代标准 | 核心逻辑 |
| Python 3.9+ | 稳定版 | 工具脚本 |
| gRPC | 1.45+ | 服务间通信 |
| CMake | 3.16+ | 构建系统 |
| Docker | 最新版 | 容器化部署 |

## 🏗️ 架构设计

### 三层架构
```
┌──────────────────────────────┐
│     Python Tools Layer       │  ← 开发工具、关卡编辑器
├──────────────────────────────┤
│  Algorithm Service (C++)     │  ← 战斗计算、AI决策
├──────────────────────────────┤
│  Strategy Service (C++)      │  ← 游戏规则、世界状态
├──────────────────────────────┤
│  Qt Application (C++)        │  ← 游戏客户端、UI界面
└──────────────────────────────┘
```

### 服务职责
- **Algorithm Service** (端口: 50051)
  - 战斗伤害计算
  - AI行为决策
  - 数据校验模块

- **Strategy Service** (端口: 50052)
  - 游戏规则管理
  - 世界状态维护
  - 事件调度系统

- **Qt Application** (端口: 8080)
  - 用户界面渲染
  - 输入处理
  - 资源管理

## 🚀 快速开始

### 环境要求
- **操作系统**: Ubuntu 20.04+ / Windows 10+ / macOS 11+
- **编译器**: GCC 10+ / Clang 12+ / MSVC 2019+
- **CMake**: 3.16+
- **Qt**: 6.3+
- **Python**: 3.9+

### 安装依赖

#### Ubuntu/Debian
```bash
# 安装基础依赖
sudo apt update
sudo apt install -y build-essential cmake git

# 安装Qt6
sudo apt install -y qt6-base-dev qt6-declarative-dev

# 安装gRPC
sudo apt install -y libgrpc++-dev libprotobuf-dev protobuf-compiler-grpc

# 安装Python依赖
sudo apt install -y python3-dev python3-pip
pip3 install -r tools/requirements.txt
```

#### macOS
```bash
# 使用Homebrew安装
brew install cmake qt6 grpc protobuf python3
pip3 install -r tools/requirements.txt
```

### 编译项目

#### 1. 克隆仓库
```bash
git clone https://github.com/xiaoliunewbig/Game.git 
cd Game
```

#### 2. 编译Algorithm Service
```bash
cd algorithm
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### 3. 编译Strategy Service
```bash
cd ../../strategy
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### 4. 编译Qt Application
```bash
cd ../../application
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### 5. 一键编译（推荐）
```bash
# 在项目根目录执行
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 运行游戏

#### 方式一：手动启动服务
```bash
# 终端1: 启动Algorithm Service
cd algorithm/build
./algorithm_service

# 终端2: 启动Strategy Service
cd strategy/build
./strategy_service

# 终端3: 启动游戏客户端
cd application/build
./game_client
```

#### 方式二：Docker容器化部署
```bash
# 构建并启动所有服务
docker-compose up --build

# 后台运行
docker-compose up -d
```

## 📁 项目结构

```
Game/
├── algorithm/              # 算法层服务
│   ├── include/           # 头文件
│   ├── src/              # 源代码
│   ├── test/             # 单元测试
│   └── CMakeLists.txt    # 构建配置
├── strategy/              # 策略层服务
│   ├── include/
│   ├── src/
│   ├── test/
│   └── CMakeLists.txt
├── application/           # Qt应用层
│   ├── src/
│   ├── qml/              # QML界面文件
│   ├── resources/        # 资源文件
│   └── CMakeLists.txt
├── tools/                 # Python开发工具
│   ├── level_editor/     # 关卡编辑器
│   ├── data_validator/   # 数据校验工具
│   └── requirements.txt  # Python依赖
├── proto/                 # gRPC协议定义
│   ├── algorithm.proto
│   └── strategy.proto
├── docs/                  # 项目文档
│   ├── 开发文档.md
│   ├── 架构设计文档.md
│   └── 代码规范.md
├── tests/                 # 集成测试
├── scripts/              # 构建脚本
├── configs/              # 配置文件
├── docker-compose.yml    # Docker编排
└── README.md            # 项目说明
```

## 🧪 测试

### 运行单元测试
```bash
# 在build目录下
make test

# 或者使用ctest
ctest --verbose
```

### 运行特定测试
```bash
# Algorithm Service测试
cd algorithm/build
./test_algorithm

# Strategy Service测试
cd strategy/build
./test_strategy
```

## 🔧 开发工具

### Python工具集
```bash
cd tools

# 启动关卡编辑器
python3 level_editor/main.py

# 数据校验工具
python3 data_validator/validate.py --config ../configs/game_data.json

# 运行测试套件
python3 -m pytest tests/
```

### 代码格式化
```bash
# C++代码格式化
find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Python代码格式化
black tools/
```

## 📊 性能监控

### 服务健康检查
```bash
# 检查Algorithm Service
curl http://localhost:50051/health

# 检查Strategy Service
curl http://localhost:50052/health
```

### 性能基准测试
```bash
cd algorithm/build
./benchmark_algorithm

cd strategy/build
./benchmark_strategy
```

## 🐳 Docker部署

### 构建镜像
```bash
# 构建所有服务镜像
docker-compose build

# 构建单个服务
docker build -t game/algorithm ./algorithm
docker build -t game/strategy ./strategy
docker build -t game/application ./application
```

### 生产环境部署
```bash
# 使用生产配置
docker-compose -f docker-compose.prod.yml up -d

# 查看服务状态
docker-compose ps

# 查看日志
docker-compose logs -f
```

## 🤝 贡献指南

### 开发流程
1. Fork项目到个人仓库
2. 创建功能分支: `git checkout -b feature/new-feature`
3. 提交更改: `git commit -am 'Add new feature'`
4. 推送分支: `git push origin feature/new-feature`
5. 创建Pull Request

### 代码规范
- 遵循项目[代码规范](docs/代码规范.md)
- 所有提交需通过CI/CD检查
- 新功能需要添加相应测试
- 重要变更需更新文档

### 提交信息格式
```
[模块] 简要描述

详细描述（可选）

- 变更点1
- 变更点2
```

## 📝 版本历史

### v0.1.0 (开发中)
- ✅ 三层架构基础框架
- ✅ Algorithm Service核心功能
- ✅ Strategy Service基础实现
- 🚧 Qt Application UI开发
- 🚧 Python工具集完善

### 计划功能
- [ ] 完整战斗系统
- [ ] 剧情系统实现
- [ ] 角色成长系统
- [ ] 多人联机支持
- [ ] 移动端适配

## 📄 许可证

本项目采用 [MIT License](LICENSE) 开源协议。

## 👥 开发团队

- **项目负责人**: chengkangpeng
- **架构设计**: chengkangpeng
- **核心开发**: chengkangpeng

## 📞 联系方式

- **项目主页**: https://github.com/your-username/fantasy-legend-game
- **问题反馈**: https://github.com/your-username/fantasy-legend-game/issues
- **开发文档**: [docs/](docs/)

## 🙏 致谢

感谢以下开源项目的支持：
- [Qt Framework](https://www.qt.io/)
- [gRPC](https://grpc.io/)
- [Protocol Buffers](https://developers.google.com/protocol-buffers)
- [CMake](https://cmake.org/)

---

**⭐ 如果这个项目对你有帮助，请给个Star支持一下！**