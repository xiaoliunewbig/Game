// ============================================================================
// 《幻境传说》游戏框架 - 战斗界面视图
// ============================================================================
// 文件名: BattleView.qml
// 说明: 游戏战斗场景的主界面，包含战斗UI、敌人显示、技能操作等
// 作者: 彭承康
// 创建时间: 2026-02-18
// 版本: v1.0.0
//
// 功能描述:
// - 战斗场景渲染和管理
// - 玩家与敌人的交互界面
// - 技能释放和战斗动画显示
// - 战斗状态信息展示
// - 战斗结果处理
//
// 依赖组件:
// - BattleUI: 战斗操作界面
// - EnemyCard: 敌人信息卡片
// - ActionButton: 战斗动作按钮
// ============================================================================

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * @brief 战斗界面视图组件
 * @details 提供完整的战斗场景界面，包括敌人显示、技能操作、状态监控等功能
 * 
 * 主要功能:
 * - 战斗场景背景渲染
 * - 敌人和玩家角色显示
 * - 战斗操作按钮布局
 * - 实时战斗状态更新
 * - 战斗动画效果播放
 * 
 * 使用示例:
 * @code
 * BattleView {
 *     anchors.fill: parent
 *     onBattleFinished: {
 *         // 处理战斗结束逻辑
 *     }
 * }
 * @endcode
 */
Item {
    id: battleView
    
    // ------------------------------------------------------------------------
    // 公共属性定义
    // ------------------------------------------------------------------------
    
    /// 战斗是否进行中
    property bool battleInProgress: false
    
    /// 当前回合数
    property int currentTurn: 1
    
    /// 玩家生命值
    property int playerHealth: 100
    
    /// 敌人生命值  
    property int enemyHealth: 100
    
    // ------------------------------------------------------------------------
    // 信号定义
    // ------------------------------------------------------------------------
    
    /// 战斗开始信号
    signal battleStarted()
    
    /// 战斗结束信号
    /// @param victory 是否胜利
    signal battleFinished(bool victory)
    
    /// 技能释放信号
    /// @param skillId 技能ID
    signal skillCasted(int skillId)
    
    // ------------------------------------------------------------------------
    // 主界面布局
    // ------------------------------------------------------------------------
    
    Rectangle {
        id: backgroundRect
        anchors.fill: parent
        color: "#2c3e50"  // 深蓝灰色背景
        
        // 背景渐变效果
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#2c3e50" }
            GradientStop { position: 1.0; color: "#34495e" }
        }
        
        // ------------------------------------------------------------------------
        // 战斗状态显示区域
        // ------------------------------------------------------------------------
        
        Rectangle {
            id: statusBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 60
            color: "#34495e"
            border.color: "#7f8c8d"
            border.width: 1
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                
                // 回合信息
                Text {
                    text: "回合: " + battleView.currentTurn
                    color: "#ecf0f1"
                    font.pixelSize: 16
                    font.bold: true
                }
                
                Item { Layout.fillWidth: true } // 弹性空间
                
                // 玩家血量
                Text {
                    text: "生命: " + battleView.playerHealth + "/100"
                    color: battleView.playerHealth > 30 ? "#2ecc71" : "#e74c3c"
                    font.pixelSize: 16
                    font.bold: true
                }
            }
        }
        
        // ------------------------------------------------------------------------
        // 中央战斗区域
        // ------------------------------------------------------------------------
        
        Item {
            id: battleArea
            anchors.top: statusBar.bottom
            anchors.bottom: controlArea.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 20
            
            // 占位文本 - 后续替换为实际战斗场景
            Text {
                anchors.centerIn: parent
                text: "战斗场景区域\n(开发中)"
                color: "#bdc3c7"
                font.pixelSize: 24
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                
                // 闪烁动画效果
                SequentialAnimation on opacity {
                    running: !battleView.battleInProgress
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.3; duration: 1000 }
                    NumberAnimation { to: 1.0; duration: 1000 }
                }
            }
        }
        
        // ------------------------------------------------------------------------
        // 底部控制区域
        // ------------------------------------------------------------------------
        
        Rectangle {
            id: controlArea
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 120
            color: "#34495e"
            border.color: "#7f8c8d"
            border.width: 1
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10
                
                // 攻击按钮
                Button {
                    text: "攻击"
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 40
                    enabled: battleView.battleInProgress
                    
                    background: Rectangle {
                        color: parent.pressed ? "#c0392b" : "#e74c3c"
                        radius: 6
                        border.color: "#a93226"
                        border.width: 1
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        console.log("执行攻击动作")
                        battleView.skillCasted(1) // 基础攻击技能ID为1
                    }
                }
                
                // 技能按钮
                Button {
                    text: "技能"
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 40
                    enabled: battleView.battleInProgress
                    
                    background: Rectangle {
                        color: parent.pressed ? "#8e44ad" : "#9b59b6"
                        radius: 6
                        border.color: "#7d3c98"
                        border.width: 1
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        console.log("打开技能菜单")
                        battleView.skillCasted(2) // 特殊技能ID为2
                    }
                }
                
                // 防御按钮
                Button {
                    text: "防御"
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 40
                    enabled: battleView.battleInProgress
                    
                    background: Rectangle {
                        color: parent.pressed ? "#2980b9" : "#3498db"
                        radius: 6
                        border.color: "#2471a3"
                        border.width: 1
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        console.log("执行防御动作")
                        battleView.skillCasted(3) // 防御技能ID为3
                    }
                }
                
                Item { Layout.fillWidth: true } // 弹性空间
                
                // 逃跑按钮
                Button {
                    text: "逃跑"
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 40
                    enabled: battleView.battleInProgress
                    
                    background: Rectangle {
                        color: parent.pressed ? "#f39c12" : "#f1c40f"
                        radius: 6
                        border.color: "#d68910"
                        border.width: 1
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        color: "#2c3e50"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        console.log("尝试逃跑")
                        battleView.battleFinished(false) // 逃跑算作失败
                    }
                }
            }
        }
    }
    
    // ------------------------------------------------------------------------
    // 组件初始化
    // ------------------------------------------------------------------------
    
    Component.onCompleted: {
        console.log("BattleView 组件初始化完成")
        // 这里可以添加初始化逻辑
    }
    
    // ------------------------------------------------------------------------
    // 私有函数
    // ------------------------------------------------------------------------
    
    /**
     * @brief 开始战斗
     * @details 初始化战斗状态，重置各项数值
     */
    function startBattle() {
        battleInProgress = true
        currentTurn = 1
        playerHealth = 100
        enemyHealth = 100
        battleStarted()
        console.log("战斗开始！")
    }
    
    /**
     * @brief 结束战斗
     * @param victory 是否胜利
     * @details 清理战斗状态，触发结束信号
     */
    function endBattle(victory) {
        battleInProgress = false
        battleFinished(victory)
        console.log("战斗结束，结果:", victory ? "胜利" : "失败")
    }
}
