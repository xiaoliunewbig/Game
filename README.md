# 《幻境传说》游戏框架

一款基于 Qt/C++ 技术栈的 2D 动作角色扮演游戏，采用三层微服务架构设计。

## 项目概述

### 核心特性
- **丰富的剧情系统**：6个主线章节，50+支线任务，多重结局
- **策略战斗系统**：连击、元素克制、状态效果
- **角色成长系统**：3种职业（战士/法师/盗贼）、技能树、装备强化
- **动态世界系统**：NPC关系、世界状态变化

### 技术栈
| 技术 | 版本 | 用途 |
|------|------|------|
| Qt 6.3+ | 最新稳定版 | UI框架（QML + Widgets） |
| C++20 | 现代标准 | 核心逻辑 |
| gRPC | 1.45+ | 服务间通信 |
| Protobuf | 3.x | 消息序列化 |
| CMake | 3.16+ | 构建系统 |
| PostgreSQL (libpqxx) | - | 策略层数据存储 |

## 架构设计

### 三层架构
```
+------------------------------+
|  Algorithm Service (C++)     |  <- 战斗计算、AI决策 (端口: 50051)
+------------------------------+
|  Strategy Service (C++)      |  <- 游戏规则、世界状态 (端口: 50052)
+------------------------------+
|  Qt Application (C++)        |  <- 游戏客户端、UI界面 (端口: 8080)
+------------------------------+
```

### 服务职责
- **Algorithm Service** (端口: 50051)
  - 战斗伤害计算（DamageCalculator）
  - AI行为决策（AIDecisionEngine）
  - 技能树管理（SkillTreeManager）
  - 数据校验模块（InputValidator）

- **Strategy Service** (端口: 50052)
  - 游戏规则管理（GameRuleManager）
  - 世界状态维护（WorldStateEngine）
  - 事件调度系统（EventScheduler）
  - 性能监控（PerformanceMonitor）

- **Qt Application** (端口: 8080)
  - QML用户界面渲染
  - 游戏引擎核心（GameEngine）
  - 场景管理（SceneManager）
  - 输入处理（InputManager）
  - 音频管理（AudioManager）
  - 资源管理（ResourceManager）
  - 网络通信（NetworkManager + gRPC客户端）
  - 存档系统（SaveManager）

### gRPC 接口定义

**AlgorithmService.proto** — 4个RPC：
- `CalculateDamage` — 战斗伤害计算
- `GetSkillTree` — 获取技能树数据
- `AIActionDecision` — AI行为决策
- `ValidateInput` — 输入数据校验

**StrategyService.proto** — 4个RPC：
- `GetGameRules` — 获取游戏规则
- `UpdateWorldState` — 更新世界状态
- `TriggerEvent` — 触发事件
- `QueryGameState` — 查询游戏状态

## 快速开始

### 环境要求
- **操作系统**: Windows 10+ / Ubuntu 20.04+ / macOS 11+
- **编译器**: MSVC 2019+ / GCC 10+ / Clang 12+
- **CMake**: 3.16+
- **Qt**: 6.3+

### 安装依赖

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install -y build-essential cmake git
sudo apt install -y qt6-base-dev qt6-declarative-dev qt6-multimedia-dev
sudo apt install -y libgrpc++-dev libprotobuf-dev protobuf-compiler-grpc
sudo apt install -y libpqxx-dev
```

#### macOS
```bash
brew install cmake qt6 grpc protobuf libpqxx
```

### 编译与运行（Linux / macOS）

```bash
# 使用 CMake Presets（推荐）
cmake --preset linux-debug
cmake --build --preset linux-debug

# 或手动编译
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel
```

```bash
# 运行（需要3个终端）
# 终端1: Algorithm Service
./build/bin/algorithm_server

# 终端2: Strategy Service
./build/bin/strategy_server

# 终端3: 游戏客户端
./build/bin/game_client
```

---

## Windows 开发指南（详细）

本节提供在 Windows 上从零搭建开发环境、编译和运行项目的完整步骤。

### 1. 安装前置工具

#### 1.1 Visual Studio 2022

安装 Visual Studio 2022（Community 版免费），安装时勾选以下组件：
- **使用 C++ 的桌面开发** 工作负载（包含 MSVC 编译器和 Windows SDK）
- 单个组件中确认包含 **CMake tools for Windows**

安装完成后，确认 MSVC 编译器可用：
```powershell
# 在 "Developer Command Prompt for VS 2022" 或 "Developer PowerShell" 中执行
cl
# 应输出 Microsoft (R) C/C++ Optimizing Compiler 版本信息
```

#### 1.2 CMake 和 Ninja

Visual Studio 2022 自带 CMake 和 Ninja，但也可独立安装最新版：
```powershell
# 使用 winget 安装（Windows 10 1709+ 自带）
winget install Kitware.CMake
winget install Ninja-build.Ninja

# 验证
cmake --version    # 需要 3.16+
ninja --version
```

#### 1.3 Qt 6

从 [Qt 官方在线安装器](https://www.qt.io/download-qt-installer) 下载并安装，选择：
- **Qt 6.5+**（或 6.7 / 6.8 等较新的 LTS 版本）
- 编译器套件选择 **MSVC 2022 64-bit**
- 组件勾选：Qt Core, Qt Quick, Qt Widgets, Qt Multimedia, Qt Network

默认安装路径为 `C:\Qt\6.x.x\msvc2022_64`。CMakeLists.txt 已配置自动检测以下路径：
```
C:/Qt/6.8.0/msvc2022_64
C:/Qt/6.7.0/msvc2022_64
C:/Qt/6.6.0/msvc2022_64
C:/Qt/6.5.3/msvc2019_64
C:/Qt/6.5.0/msvc2022_64
```

如果安装在其他路径，需要在 CMake 配置时手动指定：
```powershell
-DQt6_DIR=D:/Qt/6.8.0/msvc2022_64/lib/cmake/Qt6
```

#### 1.4 vcpkg（管理 gRPC / Protobuf / libpqxx）

```powershell
# 克隆 vcpkg（建议放在固定位置，如 D:\vcpkg）
git clone https://github.com/microsoft/vcpkg.git D:\vcpkg\vcpkg
cd D:\vcpkg\vcpkg
.\bootstrap-vcpkg.bat

# 安装项目所需依赖（默认 x64-windows triplet）
.\vcpkg install grpc:x64-windows protobuf:x64-windows libpqxx:x64-windows

# 设置环境变量（推荐写入系统环境变量，一劳永逸）
# PowerShell 临时设置：
$env:CMAKE_TOOLCHAIN_FILE = "D:\vcpkg\vcpkg\scripts\buildsystems\vcpkg.cmake"
$env:VCPKG_ROOT = "D:\vcpkg\vcpkg"

# 或在系统环境变量中永久设置：
# CMAKE_TOOLCHAIN_FILE = D:\vcpkg\vcpkg\scripts\buildsystems\vcpkg.cmake
# VCPKG_ROOT = D:\vcpkg\vcpkg
```

> **注意**: vcpkg 安装 gRPC 和依赖项体积较大，首次构建需要较长时间并占用约 5-10GB 磁盘空间。

### 2. 编译项目

以下所有命令在 **Developer PowerShell for VS 2022** 中执行（确保 MSVC 编译器在 PATH 中）。

#### 方式一：CMake Presets（推荐）

```powershell
cd D:\Game\Game

# Debug 构建
cmake --preset windows-debug -DCMAKE_TOOLCHAIN_FILE=D:/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build --preset windows-debug

# Release 构建
cmake --preset windows-release -DCMAKE_TOOLCHAIN_FILE=D:/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build --preset windows-release
```

> 如果已设置 `CMAKE_TOOLCHAIN_FILE` 环境变量，可省略 `-DCMAKE_TOOLCHAIN_FILE=...` 参数。

Presets 使用 Ninja 生成器，构建输出在 `build/windows-debug/` 或 `build/windows-release/` 目录。

#### 方式二：手动 CMake

```powershell
cd D:\Game\Game
mkdir build
cd build

# 配置（使用 Ninja 生成器 + vcpkg）
cmake -G Ninja ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_TOOLCHAIN_FILE=D:/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
    ..

# 编译所有目标
cmake --build . --parallel

# 或只编译特定目标
cmake --build . --target game_client
cmake --build . --target algorithm_server
cmake --build . --target strategy_server
```

#### 方式三：Visual Studio 直接打开

Visual Studio 2022 原生支持 CMake 项目：
1. 打开 VS2022 → **文件** → **打开** → **CMake...**
2. 选择项目根目录的 `CMakeLists.txt`
3. VS 会自动读取 `CMakePresets.json` 并配置
4. 在工具栏选择配置（windows-debug / windows-release）
5. **生成** → **全部生成**

### 3. 运行项目

构建完成后，可执行文件位于 `build/bin/`（手动构建）或 `build/windows-debug/bin/`（Preset 构建）。

#### 3.1 启动服务

需要打开 **3 个终端窗口**（PowerShell 或 CMD），按顺序启动：

```powershell
# 终端1: 启动 Algorithm Service（端口 50051）
.\build\bin\algorithm_server.exe

# 终端2: 启动 Strategy Service（端口 50052）
.\build\bin\strategy_server.exe

# 终端3: 启动游戏客户端
.\build\bin\game_client.exe
```

> 如果使用 CMake Presets 构建，路径改为 `.\build\windows-debug\bin\*.exe`。

#### 3.2 Qt DLL 问题处理

运行 `game_client.exe` 时如果提示找不到 Qt DLL，有以下解决方案：

**方案 A：使用 windeployqt（推荐）**
```powershell
# 找到 windeployqt 工具（在 Qt 安装目录的 bin 下）
C:\Qt\6.8.0\msvc2022_64\bin\windeployqt.exe .\build\bin\game_client.exe
```
这会自动将所有需要的 Qt DLL 和插件复制到 `game_client.exe` 所在目录。

**方案 B：添加 Qt bin 目录到 PATH**
```powershell
$env:PATH = "C:\Qt\6.8.0\msvc2022_64\bin;" + $env:PATH
.\build\bin\game_client.exe
```

#### 3.3 PostgreSQL 连接配置

Strategy Service 需要连接 PostgreSQL 数据库。确保：
1. 安装并运行 PostgreSQL（可通过 [PostgreSQL 官网](https://www.postgresql.org/download/windows/) 或 `winget install PostgreSQL.PostgreSQL` 安装）
2. 创建对应的数据库和用户
3. 配置 Strategy Service 的数据库连接参数

### 4. VS Code 开发环境

项目已预配置 VS Code：

1. 安装 VS Code 扩展：
   - **C/C++** (ms-vscode.cpptools)
   - **CMake Tools** (ms-vscode.cmake-tools)
   - **Qt for Visual Studio Code** (theqtcompany.qt) — 可选，提供 QML 语法高亮

2. 用 VS Code 打开项目根目录，CMake 会自动配置

3. 在 VS Code 底部状态栏选择：
   - **Kit**: Visual Studio Community 2022 Release - amd64
   - **Build variant**: Debug / Release
   - **Build target**: build_all / game_client 等

4. 按 `F7` 构建，`Ctrl+F5` 运行

### 5. 常见问题排查

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| `cl` 不是可识别的命令 | 未使用 Developer 终端 | 从开始菜单打开 "Developer PowerShell for VS 2022" |
| CMake 找不到 Qt6 | Qt 安装路径不在默认搜索列表 | 添加 `-DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/msvc2022_64` |
| CMake 找不到 gRPC/Protobuf | 未指定 vcpkg toolchain | 确认 `-DCMAKE_TOOLCHAIN_FILE=.../vcpkg.cmake` 已设置 |
| 找不到 `*.dll` | 运行时缺少动态库 | 使用 `windeployqt` 或将 DLL 目录加入 PATH |
| `ninja: error: loading 'build.ninja'` | 构建目录损坏 | 删除 `build/` 目录重新配置 |
| 中文乱码 | 终端编码问题 | 在 PowerShell 中执行 `chcp 65001` 切换为 UTF-8 |
| `LNK2019` 链接错误 | 依赖库未正确链接 | 检查 vcpkg 安装是否为 `x64-windows` triplet |
| `C1083: 无法打开头文件` | vcpkg 依赖未安装 | 运行 `vcpkg install grpc:x64-windows protobuf:x64-windows libpqxx:x64-windows` |

## 项目结构

```
Game/
├── algorithm/              # 算法层服务
│   ├── include/           # 头文件
│   │   ├── AlgorithmService.h
│   │   ├── DamageCalculator.h
│   │   ├── AIDecisionEngine.h
│   │   ├── SkillTreeManager.h
│   │   └── InputValidator.h
│   ├── src/              # 源代码
│   └── CMakeLists.txt
├── strategy/              # 策略层服务
│   ├── include/
│   │   ├── Algorithm_interact/    # 算法交互模块
│   │   ├── common/                # 公共类型
│   │   ├── log/                   # 日志模块
│   │   └── monitor/               # 性能监控
│   ├── src/
│   └── CMakeLists.txt
├── application/           # Qt应用层
│   ├── include/
│   │   ├── core/         # 核心模块 (GameEngine, SceneManager等)
│   │   ├── network/      # 网络模块 (gRPC客户端)
│   │   ├── audio/        # 音频模块
│   │   ├── input/        # 输入模块
│   │   ├── graphics/     # 图形模块
│   │   ├── game/         # 游戏逻辑 (Player, BattleSystem等)
│   │   ├── save/         # 存档模块
│   │   ├── config/       # 配置模块
│   │   ├── utils/        # 工具模块 (Logger, PerformanceMonitor)
│   │   └── ui/           # UI窗口
│   ├── src/
│   ├── qml/              # QML界面文件
│   │   ├── main.qml
│   │   ├── MainMenu.qml
│   │   ├── GameplayView.qml
│   │   ├── BattleView.qml
│   │   ├── InventoryView.qml
│   │   ├── SettingsView.qml
│   │   └── components/   # QML组件
│   ├── resources/        # 资源文件
│   └── CMakeLists.txt
├── proto/                 # gRPC协议定义
│   ├── AlgorithmService.proto
│   └── StrategyService.proto
├── docs/                  # 项目文档
│   ├── 开发文档.md
│   ├── 架构设计文档.md
│   └── 代码规范.md
├── tests/                 # 测试
├── scripts/              # 构建脚本
├── .vscode/              # VS Code配置
│   ├── settings.json
│   └── c_cpp_properties.json
├── CMakeLists.txt        # 根构建配置
├── CMakePresets.json      # CMake预设（Windows/Linux）
├── CLAUDE.md             # Claude Code项目指引
└── README.md
```

## 跨平台支持

项目支持 Windows 和 Linux 双平台开发与构建：

| 特性 | Windows | Linux |
|------|---------|-------|
| 编译器 | MSVC (Visual Studio) | GCC 10+ |
| 生成器 | Ninja | Unix Makefiles |
| 依赖管理 | vcpkg (find_package) | pkg-config / find_package |
| Qt发现 | CMake自动检测 | CMake自动检测 |
| gRPC/Protobuf | vcpkg安装 | 系统包 |
| PostgreSQL | vcpkg (libpqxx) | apt (libpqxx-dev) |

### VS Code 开发

项目已配置好 VS Code 的 CMake Tools 和 C/C++ 扩展：
- 打开项目后 CMake 会自动配置
- IntelliSense 通过 `compile_commands.json` 提供精确补全
- 支持 Win32 和 Linux 两套 IntelliSense 配置

## 模块实现进度

### Algorithm Service — 约95%完成
- [x] DamageCalculator（伤害计算引擎）
- [x] AIDecisionEngine（AI决策引擎）
- [x] SkillTreeManager（技能树管理器）
- [x] InputValidator（输入校验器）
- [x] AlgorithmService（gRPC服务入口）

### Strategy Service — 约95%完成
- [x] GameRuleManager（游戏规则管理）
- [x] WorldStateEngine（世界状态引擎）
- [x] EventScheduler（事件调度器）
- [x] PerformanceMonitor（性能监控，支持Windows/Linux）
- [x] StrategyLogger（日志系统，支持Windows/Linux）

### Qt Application — 约85%完成
- [x] GameEngine（游戏引擎核心）
- [x] GameApplication（应用生命周期管理）
- [x] SceneManager（场景管理与转场）
- [x] ResourceManager（资源加载与缓存）
- [x] EventSystem（事件总线）
- [x] NetworkManager（HTTP/JSON通信）
- [x] AlgorithmServiceClient（算法服务客户端）
- [x] StrategyServiceClient（策略服务客户端）
- [x] AudioManager（Qt Multimedia音频管理）
- [x] InputManager（键盘/鼠标输入映射）
- [x] SpriteRenderer（精灵渲染器）
- [x] AnimationManager（动画管理器）
- [x] SaveManager（JSON格式存档系统）
- [x] ConfigManager（QSettings配置管理）
- [x] GameConfig（游戏配置）
- [x] Logger（文件+控制台日志系统）
- [x] PerformanceMonitor（FPS/内存/渲染性能监控）
- [x] Player（玩家角色系统）
- [x] BattleSystem（战斗系统）
- [x] InventorySystem（背包系统）
- [x] GameState（游戏状态管理）
- [x] GameplayWindow（游戏玩法界面）
- [x] InventoryWindow（背包界面）
- [x] QML前端（主菜单、战斗、背包、设置等视图）
- [ ] 完整的QML与C++绑定调试
- [ ] 多人联机功能

### Proto 定义 — 100%完成
- [x] AlgorithmService.proto（4个RPC + 消息定义）
- [x] StrategyService.proto（4个RPC + 消息定义）

## 编译（快速参考）

```bash
# Windows (在 Developer PowerShell for VS 2022 中)
cmake --preset windows-debug -DCMAKE_TOOLCHAIN_FILE=D:/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build --preset windows-debug

# Linux
cmake --preset linux-debug
cmake --build --preset linux-debug
```

详细步骤参见上方 [Windows 开发指南](#windows-开发指南详细) 或 [编译与运行（Linux / macOS）](#编译与运行linux--macos) 章节。

## 测试

```bash
# 在 build 目录下运行所有测试
cd build && ctest --verbose

# Windows (Preset 构建)
cd build/windows-debug && ctest --verbose
```

## 代码规范

- 遵循项目[代码规范](docs/代码规范.md)
- C++20标准，禁止 `using namespace std`
- 智能指针管理内存，成员变量使用 `m_` 前缀（应用层）或 `_` 后缀（算法/策略层）
- 4空格缩进，Doxygen风格注释
- 中文注释和日志

## 版本历史

### v0.2.0 (当前)
- 构建系统 Windows/MSVC 适配（CMakePresets, find_package + vcpkg）
- 补全 AlgorithmService.proto 和 StrategyService.proto
- 完善应用层所有核心模块实现
- PerformanceMonitor 跨平台支持（Windows API + Linux /proc）
- 修复 GameApplication.cpp 语法错误
- VS Code 跨平台配置

### v0.1.0
- 三层架构基础框架
- Algorithm Service 核心功能
- Strategy Service 基础实现
- Qt Application UI 开发

### 计划功能
- [ ] 完整战斗系统联调
- [ ] 剧情系统实现
- [ ] 多人联机支持
- [ ] Docker容器化部署
- [ ] 移动端适配

## 许可证

本项目采用 [MIT License](LICENSE) 开源协议。

## 开发团队

- **项目负责人**: chengkangpeng
- **架构设计**: chengkangpeng
- **核心开发**: chengkangpeng

## 联系方式

- **项目主页**: https://github.com/xiaoliunewbig/Game
- **问题反馈**: https://github.com/xiaoliunewbig/Game/issues
- **开发文档**: [docs/](docs/)
