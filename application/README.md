# Application 模块 — 幻境传说游戏客户端

基于 Qt 6.3+ 的 2D 动作 RPG 游戏客户端，负责渲染、用户交互和游戏逻辑协调。

## 目录结构

```
application/
├── include/                    # 头文件
│   ├── audio/                  # 音频管理
│   │   └── AudioManager.h      # 背景音乐和音效管理
│   ├── config/                 # 配置管理
│   │   └── ConfigManager.h     # 游戏设置持久化（单例）
│   ├── core/                   # 核心引擎
│   │   ├── EventSystem.h       # 事件发布/订阅系统（单例）
│   │   ├── GameEngine.h        # 主循环和状态机（60FPS）
│   │   ├── ResourceManager.h   # 资源加载与缓存
│   │   └── SceneManager.h      # 场景切换与过渡动画
│   ├── game/                   # 游戏逻辑
│   │   ├── BattleSystem.h      # 回合制战斗系统
│   │   ├── GameState.h         # 全局游戏状态（线程安全）
│   │   ├── InventorySystem.h   # 背包与物品管理
│   │   └── Player.h            # 玩家角色数据
│   ├── graphics/               # 图形渲染
│   │   ├── AnimationManager.h  # 帧动画播放与管理
│   │   └── SpriteRenderer.h    # 精灵加载、缓存和渲染
│   ├── input/                  # 输入处理
│   │   └── InputManager.h      # 键鼠绑定与输入分发
│   ├── network/                # 网络通信
│   │   ├── AlgorithmServiceClient.h  # 算法服务 gRPC 客户端
│   │   ├── NetworkManager.h          # 网络管理器
│   │   └── StrategyServiceClient.h   # 策略服务 gRPC 客户端
│   ├── save/                   # 存档系统
│   │   └── SaveManager.h       # 存档保存/加载/管理
│   ├── ui/                     # 窗口界面
│   │   ├── GameplayWindow.h    # 游戏主界面（Widgets）
│   │   └── InventoryWindow.h   # 背包界面（Widgets）
│   ├── utils/                  # 工具类
│   │   ├── Logger.h            # 日志系统（单例，线程安全）
│   │   └── PerformanceMonitor.h # 性能监控（FPS/内存/渲染）
│   └── GameApplication.h       # 应用入口封装
├── src/                        # 实现文件（与 include/ 结构对应）
├── qml/                        # QML 界面文件
│   ├── main.qml                # QML 根组件
│   ├── scenes/                 # 场景 QML
│   └── components/             # 可复用 QML 组件
│       ├── ActionButton.qml
│       ├── BattleUI.qml
│       ├── EnemyCard.qml
│       ├── InventoryGrid.qml
│       └── ItemCard.qml
├── resources/                  # 静态资源（图片、音效等）
└── CMakeLists.txt              # 构建配置
```

## 核心架构

### GameEngine（游戏引擎）

主循环运行在 60FPS，通过状态机管理游戏流程：

```
Uninitialized → MainMenu → Loading → Playing ⇄ Battle
                                        ↓         ↓
                                      Paused    GameOver
```

引擎负责协调所有子系统的初始化和更新。

### EventSystem（事件系统）

基于发布/订阅模式的解耦通信机制：

```cpp
// 发送事件（推荐使用内联函数代替宏）
sendGameEvent(GameEvent::Type::PlayerAttack);
sendGameEvent(GameEvent::Type::ItemUsed, eventData);

// 异步事件
postGameEvent(GameEvent::Type::SceneChange, sceneData);
```

支持事件过滤、优先级处理和延迟事件。

### 存档系统

存档通过 JSON 序列化存储到用户数据目录：

- 最多 10 个存档槽位
- 自动序列化 Player 和 GameState
- 存档路径：`<AppDataLocation>/saves/save_N.json`

### 输入管理

类型安全的键位绑定系统，支持：

- 键盘/鼠标映射到 Action 枚举
- 按键组合与按键序列检测
- 输入配置的 JSON 导入/导出
- 动作优先级与输入过滤

默认键位：WASD 移动、Space 攻击、Shift 防御、E 交互、I 背包。

## 构建

```bash
# 仅构建游戏客户端
cd build && cmake .. && make game_client -j$(nproc)
```

**依赖**：Qt6（Core, Quick, Widgets, Multimedia, Network）、gRPC、Protobuf

## 子系统说明

| 子系统 | 类 | 说明 |
|--------|-----|------|
| 音频 | `AudioManager` | 背景音乐/音效播放，支持音量控制和预加载 |
| 战斗 | `BattleSystem` | 回合制战斗，AI 决策通过 Algorithm Service |
| 背包 | `InventorySystem` | 物品增删、使用、堆叠、排序，JSON 持久化 |
| 图形 | `SpriteRenderer` + `AnimationManager` | 精灵表加载、帧动画、渲染效果 |
| 性能 | `PerformanceMonitor` | FPS/内存/渲染统计，性能报告导出 |
| 配置 | `ConfigManager` | 画质/音频/控制/游戏设置的持久化管理 |
| 网络 | `NetworkManager` | gRPC 客户端，连接 Algorithm 和 Strategy 服务 |

## 编码规范

遵循项目根目录 `CLAUDE.md` 中的规范：

- C++20 标准
- 成员变量 `m_` 前缀
- 智能指针管理资源，禁止裸指针所有权
- 构造函数标记 `explicit`
- 不使用 `using namespace std`
- Doxygen 风格注释
- 4 空格缩进
