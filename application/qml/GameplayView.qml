/*
 * 文件名: GameplayView.qml
 * 说明: 游戏主界面
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import GameUI 1.0
import "components"

/**
 * @brief 游戏主界面
 * 
 * 游戏进行时的主要界面，包括：
 * - 游戏场景显示
 * - 玩家信息面板
 * - 游戏状态面板
 * - 快捷操作按钮
 * - 技能栏
 */
Rectangle {
    id: gameplayView
    
    // 信号定义
    signal menuRequested()
    signal inventoryRequested()
    signal skillTreeRequested()
    signal questsRequested()
    signal saveRequested()
    
    color: "#1a1a1a"
    
    // 主要布局
    RowLayout {
        anchors.fill: parent
        spacing: 0
        
        // 左侧面板
        Rectangle {
            id: leftPanel
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            color: "#2a2a2a"
            border.color: "#555555"
            border.width: 1
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10
                
                // 玩家信息面板
                PlayerInfoPanel {
                    id: playerInfoPanel
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                }
                
                // 游戏状态面板
                GameStatePanel {
                    id: gameStatePanel
                    Layout.fillWidth: true
                    Layout.preferredHeight: 150
                }
                
                // 快捷操作按钮
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    
                    QuickActionButton {
                        text: "背包"
                        iconSource: "qrc:/resources/icons/inventory.png"
                        Layout.fillWidth: true
                        onClicked: gameplayView.inventoryRequested()
                    }
                    
                    QuickActionButton {
                        text: "任务"
                        iconSource: "qrc:/resources/icons/quests.png"
                        Layout.fillWidth: true
                        onClicked: gameplayView.questsRequested()
                    }
                    
                    QuickActionButton {
                        text: "技能"
                        iconSource: "qrc:/resources/icons/skills.png"
                        Layout.fillWidth: true
                        onClicked: gameplayView.skillTreeRequested()
                    }
                    
                    QuickActionButton {
                        text: "保存"
                        iconSource: "qrc:/resources/icons/save.png"
                        Layout.fillWidth: true
                        onClicked: gameplayView.saveRequested()
                    }
                }
                
                // 填充空间
                Item {
                    Layout.fillHeight: true
                }
            }
        }
        
        // 中央游戏区域
        Rectangle {
            id: gameArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#000000"
            
            // 游戏场景（这里用简单的背景代替）
            Rectangle {
                anchors.fill: parent
                color: "#1a3a1a"
                
                // 模拟游戏场景
                Text {
                    anchors.centerIn: parent
                    text: "游戏场景区域\n（这里将显示实际的游戏内容）"
                    color: "#FFFFFF"
                    font.pixelSize: 24
                    horizontalAlignment: Text.AlignHCenter
                }
                
                // 玩家角色（简单示意）
                Rectangle {
                    id: playerCharacter
                    width: 40
                    height: 40
                    color: "#FFD700"
                    radius: 20
                    x: parent.width / 2 - width / 2
                    y: parent.height / 2 - height / 2
                    
                    Text {
                        anchors.centerIn: parent
                        text: "P"
                        color: "#000000"
                        font.bold: true
                    }
                }
            }
            
            // 游戏UI覆盖层
            GameplayUI {
                id: gameplayUI
                anchors.fill: parent
                
                onMenuRequested: gameplayView.menuRequested()
            }
        }
    }
    
    // 底部技能栏
    Rectangle {
        id: skillBar
        anchors.bottom: parent.bottom
        anchors.left: leftPanel.right
        anchors.right: parent.right
        height: 80
        color: "#2a2a2a"
        border.color: "#555555"
        border.width: 1
        
        RowLayout {
            anchors.centerIn: parent
            spacing: 10
            
            // 技能槽位
            Repeater {
                model: 6
                
                SkillSlot {
                    width: 60
                    height: 60
                    slotIndex: index
                    skillId: index < 3 ? index + 1 : 0 // 前3个槽位有技能
                    keyBinding: index < 3 ? (index + 1).toString() : ""
                }
            }
        }
    }
    
    // 键盘输入处理
    Keys.onPressed: {
        switch (event.key) {
            case Qt.Key_Escape:
                gameplayView.menuRequested()
                event.accepted = true
                break
            case Qt.Key_I:
                gameplayView.inventoryRequested()
                event.accepted = true
                break
            case Qt.Key_K:
                gameplayView.skillTreeRequested()
                event.accepted = true
                break
            case Qt.Key_Q:
                gameplayView.questsRequested()
                event.accepted = true
                break
            case Qt.Key_F5:
                gameplayView.saveRequested()
                event.accepted = true
                break
        }
    }
    
    // 确保可以接收键盘输入
    focus: true
}