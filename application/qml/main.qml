/*
 * 文件名: main.qml
 * 说明: 游戏应用程序的主QML界面文件。
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 *
 * 本文件定义了游戏的主窗口和界面结构，包括场景切换、
 * 主菜单、游戏界面等核心UI组件。作为QML界面的入口点，
 * 协调各个界面组件的显示和交互。
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import GameUI 1.0

/**
 * @brief 应用程序主窗口
 * 
 * 游戏的主要显示窗口，包含：
 * - 窗口基本配置（大小、标题、可见性）
 * - 场景切换管理（StackView）
 * - 游戏引擎接口绑定
 * - 全局事件处理和错误处理
 */
ApplicationWindow {
    id: window
    width: 1280                    // 窗口宽度：1280像素
    height: 720                    // 窗口高度：720像素（16:9比例）
    visible: true                  // 窗口启动时可见
    title: "幻境传说 v1.0"         // 窗口标题

    /**
     * @brief 游戏引擎访问属性
     * 
     * 提供对游戏引擎的便捷访问，用于QML组件调用游戏功能。
     */
    property alias gameEngine: gameEngineLoader.item

    /**
     * @brief 游戏引擎加载器
     * 
     * 动态加载游戏引擎组件，实现延迟初始化。
     * 避免在QML解析时立即创建重量级对象。
     */
    Loader {
        id: gameEngineLoader
        source: "qrc:/GameEngine"   // 游戏引擎QML组件路径
    }

    /**
     * @brief 主要内容区域
     * 
     * 使用StackView管理不同场景的切换，包括：
     * - 主菜单场景
     * - 游戏进行场景
     * - 战斗场景
     * - 设置界面等
     */
    StackView {
        id: stackView
        anchors.fill: parent        // 填充整个窗口
        initialItem: mainMenuComponent  // 初始显示主菜单

        /**
         * @brief 主菜单组件
         * 
         * 游戏的主菜单界面，提供：
         * - 新游戏选项
         * - 加载游戏选项
         * - 设置和退出功能
         */
        Component {
            id: mainMenuComponent
            MainMenu {
                /**
                 * @brief 新游戏点击处理
                 * 
                 * 当玩家选择开始新游戏时的处理逻辑。
                 * 
                 * @param playerName 玩家输入的角色名称
                 * @param profession 玩家选择的职业
                 */
                onNewGameClicked: {
                    // 切换到游戏界面
                    stackView.push(gameplayComponent)
                    // 通知游戏引擎开始新游戏
                    gameEngine.startNewGame(playerName, profession)
                }
                
                /**
                 * @brief 加载游戏点击处理
                 * 
                 * 当玩家选择加载存档时的处理逻辑。
                 * 
                 * @param saveSlot 选择的存档槽位
                 */
                onLoadGameClicked: {
                    // 切换到游戏界面
                    stackView.push(gameplayComponent)
                    // 通知游戏引擎加载游戏
                    gameEngine.loadGame(saveSlot)
                }
            }
        }

        /**
         * @brief 游戏界面组件
         * 
         * 主要的游戏进行界面，包括：
         * - 游戏世界显示
         * - 玩家信息界面
         * - 操作控制界面
         */
        Component {
            id: gameplayComponent
            GameplayView {
                // 绑定游戏状态数据
                gameState: gameEngine.gameState
                
                /**
                 * @brief 战斗请求处理
                 * 
                 * 当玩家触发战斗时的处理。
                 * 
                 * @param enemyId 敌人ID
                 */
                onBattleRequested: gameEngine.enterBattle(enemyId)
                
                /**
                 * @brief 技能使用处理
                 * 
                 * 当玩家使用技能时的处理。
                 * 
                 * @param skillId 技能ID
                 * @param targetId 目标ID
                 */
                onSkillUsed: gameEngine.useSkill(skillId, targetId)
                
                /**
                 * @brief 物品使用处理
                 * 
                 * 当玩家使用物品时的处理。
                 * 
                 * @param itemId 物品ID
                 */
                onItemUsed: gameEngine.useItem(itemId)
                
                /**
                 * @brief 玩家移动处理
                 * 
                 * 当玩家移动时的处理。
                 * 
                 * @param x 目标X坐标
                 * @param y 目标Y坐标
                 */
                onPlayerMoved: gameEngine.movePlayer(x, y)
            }
        }
    }

    /**
     * @brief 游戏引擎信号连接
     * 
     * 连接游戏引擎的各种信号，处理游戏事件。
     */
    Connections {
        target: gameEngine
        
        /**
         * @brief 游戏开始处理
         * 
         * 当游戏成功开始时的处理逻辑。
         */
        function onGameStarted() {
            console.log("游戏开始")
            // 可以在这里添加游戏开始的特效或音效
        }
        
        /**
         * @brief 战斗开始处理
         * 
         * 当进入战斗状态时的处理逻辑。
         * 
         * @param battleData 战斗相关数据
         */
        function onBattleStarted(battleData) {
            // 显示战斗界面
            stackView.push("qrc:/BattleView.qml", {battleData: battleData})
        }
        
        /**
         * @brief 错误处理
         * 
         * 当游戏发生错误时的处理逻辑。
         * 
         * @param error 错误信息
         */
        function onErrorOccurred(error) {
            // 设置错误信息并显示错误对话框
            errorDialog.text = error
            errorDialog.open()
        }
    }

    /**
     * @brief 错误对话框
     * 
     * 用于显示游戏运行过程中的错误信息。
     * 提供用户友好的错误提示界面。
     */
    Dialog {
        id: errorDialog
        title: "错误"                // 对话框标题
        
        /**
         * @brief 错误文本属性
         * 
         * 用于设置和获取错误信息文本。
         */
        property alias text: errorText.text
        
        /**
         * @brief 错误信息显示文本
         * 
         * 显示具体的错误信息内容。
         */
        Text {
            id: errorText
            wrapMode: Text.WordWrap    // 文本自动换行
            width: parent.width        // 宽度适应父容器
        }
        
        // 对话框标准按钮（确定按钮）
        standardButtons: Dialog.Ok
    }
}
