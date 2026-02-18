# Application 资源文件清单

本文档列出 `application/resources/` 下所有需要的资源文件，以及每个文件的用途、建议尺寸和格式要求。

> 目录结构：
> ```
> application/resources/
> ├── configs/
> ├── fonts/
> ├── icons/
> ├── images/
> │   └── ui/
> └── sounds/
> ```

---

## 一、图片 (`images/`)

| 文件 | 格式 | 建议尺寸 | 用途 | 引用位置 |
|------|------|----------|------|----------|
| `player.png` | PNG | 64×64 | 玩家角色头像（PlayerInfo 面板显示） | `components/PlayerInfo.qml:88` |
| `player_avatar.png` | PNG | 64×64 | 玩家头像（GameplayView 信息面板） | `components/PlayerInfoPanel.qml:65` |
| `enemies.png` | PNG | 64×64 | 敌人角色图片 | CMakeLists 注册，预留 |
| `items.png` | PNG | 32×32 | 物品图集 | CMakeLists 注册，预留 |
| `default_item.png` | PNG | 32×32 | 物品默认/缺省图标（物品图片加载失败时的 fallback） | `components/ItemSlot.qml:171` |
| `main_menu_bg.jpg` | JPEG | 1280×720 | 主菜单全屏背景图 | `MainMenu.qml:39`, `components/MainMenu.qml:45` |
| `app_icon.png` | PNG | 64×64 | 应用窗口图标 | CMakeLists 注册 |

### UI 子目录 (`images/ui/`)

| 文件 | 格式 | 建议尺寸 | 用途 | 引用位置 |
|------|------|----------|------|----------|
| `button_normal.png` | PNG | 200×50 | 通用按钮默认态纹理 | `ResourceManager.cpp:120` 预加载 |
| `button_hover.png` | PNG | 200×50 | 通用按钮悬停态纹理 | `ResourceManager.cpp:121` 预加载 |
| `button_pressed.png` | PNG | 200×50 | 通用按钮按下态纹理 | `ResourceManager.cpp:122` 预加载 |
| `background.png` | PNG | 1280×720 | 通用 UI 背景 | `ResourceManager.cpp:123` 预加载 |

---

## 二、图标 (`icons/`)

所有图标均为 **PNG** 格式，建议尺寸 **32×32**。

### 主菜单图标

| 文件 | 用途 | 引用位置 |
|------|------|----------|
| `new_game.png` | "新游戏"按钮图标 | `MainMenu.qml:93`, `components/MainMenu.qml:109` |
| `load_game.png` | "加载游戏"按钮图标 | `MainMenu.qml:106`, `components/MainMenu.qml:122` |
| `settings.png` | "设置"按钮图标 | `MainMenu.qml:119`, `components/MainMenu.qml:135` |
| `exit.png` | "退出"按钮图标 | `MainMenu.qml:132`, `components/MainMenu.qml:148` |

### 职业图标

| 文件 | 用途 | 引用位置 |
|------|------|----------|
| `warrior.png` | 战士职业图标 | `MainMenu.qml:219`, `components/MainMenu.qml:229` |
| `mage.png` | 法师职业图标 | `MainMenu.qml:228`, `components/MainMenu.qml:241` |
| `archer.png` | 弓箭手职业图标 | `MainMenu.qml:236`, `components/MainMenu.qml:253` |

### 游戏界面快捷按钮图标

| 文件 | 用途 | 引用位置 |
|------|------|----------|
| `inventory.png` | 背包按钮图标 | `GameplayView.qml:76` |
| `quests.png` | 任务按钮图标 | `GameplayView.qml:83` |
| `skills.png` | 技能按钮图标 | `GameplayView.qml:90` |
| `save.png` | 存档按钮图标 | `GameplayView.qml:97` |

### 游戏状态面板图标

| 文件 | 用途 | 引用位置 |
|------|------|----------|
| `chapter.png` | 章节信息图标 | `components/GameStatePanel.qml:64` |
| `location.png` | 位置信息图标 | `components/GameStatePanel.qml:89` |
| `time.png` | 时间信息图标 | `components/GameStatePanel.qml:114` |
| `quest.png` | 当前任务图标 | `components/GameStatePanel.qml:139` |

### 技能图标

| 文件 | 用途 | 引用位置 |
|------|------|----------|
| `skill_attack.png` | 攻击技能图标 | `components/SkillSlot.qml:239` |
| `skill_heal.png` | 治疗技能图标 | `components/SkillSlot.qml:240` |
| `skill_fireball.png` | 火球技能图标 | `components/SkillSlot.qml:241` |
| `skill_default.png` | 默认/未知技能图标 | `components/SkillSlot.qml:242` |
| `skill_1.png` | 技能栏位 1 | CMakeLists 注册，预留 |
| `skill_2.png` | 技能栏位 2 | CMakeLists 注册，预留 |
| `skill_3.png` | 技能栏位 3 | CMakeLists 注册，预留 |

### 物品图标

| 文件 | 用途 | 引用位置 |
|------|------|----------|
| `health_potion.png` | 生命药水图标 | `InventorySystem.cpp:602` |
| `mana_potion.png` | 魔法药水图标 | `InventorySystem.cpp:612` |
| `iron_sword.png` | 铁剑图标 | `InventorySystem.cpp:622` |

---

## 三、字体 (`fonts/`)

| 文件 | 格式 | 用途 | 引用位置 |
|------|------|------|----------|
| `game_font.ttf` | TTF | 游戏主字体（启动时全局加载） | `main.cpp:83`, `ResourceManager.cpp:134` |
| `ui_font.ttf` | TTF | UI 界面字体 | `ResourceManager.cpp:135` |

要求：需为合法的 TrueType 字体文件，建议使用支持中文的开源字体（如思源黑体 Noto Sans SC）。

---

## 四、音效 (`sounds/`)

| 文件 | 格式 | 用途 | 引用位置 |
|------|------|------|----------|
| `bgm.mp3` | MP3 | 背景音乐 | CMakeLists 注册 |
| `effects.wav` | WAV | 通用音效 | CMakeLists 注册 |

---

## 五、配置文件 (`configs/`)

这两个文件已有有效内容，一般无需替换。

| 文件 | 格式 | 用途 |
|------|------|------|
| `game_config.json` | JSON | 游戏全局配置（窗口大小、画质、音量等） |
| `ui_config.json` | JSON | UI 配置（主题、字号、语言等） |

---

## 快速检查清单

共 **42** 个资源文件：

```
resources/
├── configs/
│   ├── game_config.json
│   └── ui_config.json
├── fonts/
│   ├── game_font.ttf
│   └── ui_font.ttf
├── icons/
│   ├── archer.png
│   ├── chapter.png
│   ├── exit.png
│   ├── health_potion.png
│   ├── inventory.png
│   ├── iron_sword.png
│   ├── load_game.png
│   ├── location.png
│   ├── mage.png
│   ├── mana_potion.png
│   ├── new_game.png
│   ├── quest.png
│   ├── quests.png
│   ├── save.png
│   ├── settings.png
│   ├── skill_1.png
│   ├── skill_2.png
│   ├── skill_3.png
│   ├── skill_attack.png
│   ├── skill_default.png
│   ├── skill_fireball.png
│   ├── skill_heal.png
│   ├── skills.png
│   ├── time.png
│   └── warrior.png
├── images/
│   ├── app_icon.png
│   ├── default_item.png
│   ├── enemies.png
│   ├── items.png
│   ├── main_menu_bg.jpg
│   ├── player.png
│   ├── player_avatar.png
│   └── ui/
│       ├── background.png
│       ├── button_hover.png
│       ├── button_normal.png
│       └── button_pressed.png
└── sounds/
    ├── bgm.mp3
    └── effects.wav
```
