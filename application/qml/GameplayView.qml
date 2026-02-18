/*
 * 文件名: GameplayView.qml
 * 说明: 游戏主界面
 * 作者: 彭承康
 * 创建时间: 2026-02-18
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
            clip: true

            // 游戏场景
            Rectangle {
                id: gameScene
                anchors.fill: parent
                color: "#2d5a1e"

                // 地面网格图案
                Repeater {
                    model: Math.ceil(gameScene.width / 64) * Math.ceil(gameScene.height / 64)

                    Rectangle {
                        property int col: index % Math.ceil(gameScene.width / 64)
                        property int row: Math.floor(index / Math.ceil(gameScene.width / 64))

                        x: col * 64
                        y: row * 64
                        width: 64
                        height: 64
                        color: (col + row) % 2 === 0 ? "#2d5a1e" : "#346b23"
                        border.color: "#255018"
                        border.width: 1
                    }
                }

                // 环境装饰：树木
                Repeater {
                    model: [
                        {tx: 100, ty: 80},
                        {tx: 300, ty: 150},
                        {tx: 500, ty: 60},
                        {tx: 150, ty: 350},
                        {tx: 600, ty: 300},
                        {tx: 450, ty: 200},
                        {tx: 700, ty: 100},
                        {tx: 250, ty: 450}
                    ]

                    Item {
                        x: modelData.tx
                        y: modelData.ty
                        width: 48
                        height: 64

                        // 树冠
                        Rectangle {
                            anchors.horizontalCenter: parent.horizontalCenter
                            y: 0
                            width: 40
                            height: 40
                            radius: 20
                            color: "#1a7a1a"
                            border.color: "#0f5c0f"
                            border.width: 1
                        }

                        // 树干
                        Rectangle {
                            anchors.horizontalCenter: parent.horizontalCenter
                            y: 32
                            width: 12
                            height: 32
                            color: "#6b3a1f"
                            border.color: "#4a2810"
                            border.width: 1
                        }
                    }
                }

                // 环境装饰：石头
                Repeater {
                    model: [
                        {rx: 200, ry: 250, rw: 30, rh: 22},
                        {rx: 550, ry: 180, rw: 36, rh: 26},
                        {rx: 400, ry: 400, rw: 28, rh: 20},
                        {rx: 680, ry: 350, rw: 32, rh: 24}
                    ]

                    Rectangle {
                        x: modelData.rx
                        y: modelData.ry
                        width: modelData.rw
                        height: modelData.rh
                        radius: height / 2
                        color: "#7a7a7a"
                        border.color: "#5a5a5a"
                        border.width: 1
                    }
                }

                // 小路
                Rectangle {
                    x: gameScene.width / 2 - 30
                    y: 0
                    width: 60
                    height: gameScene.height
                    color: "#8b7355"
                    opacity: 0.5
                }

                // 玩家角色
                Item {
                    id: playerCharacter
                    width: 40
                    height: 48
                    x: gameScene.width / 2 - width / 2
                    y: gameScene.height / 2 - height / 2

                    // 角色阴影
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.bottom
                        width: 32
                        height: 10
                        radius: 5
                        color: "#00000040"
                    }

                    // 角色身体
                    Rectangle {
                        id: playerBody
                        anchors.horizontalCenter: parent.horizontalCenter
                        y: 8
                        width: 28
                        height: 32
                        radius: 4
                        color: "#3498db"
                        border.color: "#2980b9"
                        border.width: 2
                    }

                    // 角色头部
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        y: 0
                        width: 20
                        height: 20
                        radius: 10
                        color: "#f1c27d"
                        border.color: "#e0a95c"
                        border.width: 1
                    }

                    // 方向指示器（小三角）
                    Text {
                        id: directionIndicator
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.top
                        anchors.bottomMargin: 2
                        text: "▼"
                        color: "#FFD700"
                        font.pixelSize: 12
                        rotation: 180
                    }

                    // 移动动画
                    Behavior on x {
                        NumberAnimation { duration: 50 }
                    }
                    Behavior on y {
                        NumberAnimation { duration: 50 }
                    }
                }

                // 坐标显示
                Text {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 8
                    text: "位置: (" + Math.round(playerCharacter.x) + ", " + Math.round(playerCharacter.y) + ")"
                    color: "#FFFFFF"
                    font.pixelSize: 12
                    opacity: 0.7
                }

                // 操作提示
                Text {
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottomMargin: 90
                    text: "WASD / 方向键移动角色"
                    color: "#FFFFFF"
                    font.pixelSize: 14
                    opacity: 0.5
                }

                // 键盘移动处理
                focus: true
                Keys.onPressed: function(event) {
                    var speed = 8
                    switch (event.key) {
                        case Qt.Key_W:
                        case Qt.Key_Up:
                            playerCharacter.y = Math.max(0, playerCharacter.y - speed)
                            directionIndicator.rotation = 180
                            event.accepted = true
                            break
                        case Qt.Key_S:
                        case Qt.Key_Down:
                            playerCharacter.y = Math.min(gameScene.height - playerCharacter.height, playerCharacter.y + speed)
                            directionIndicator.rotation = 0
                            event.accepted = true
                            break
                        case Qt.Key_A:
                        case Qt.Key_Left:
                            playerCharacter.x = Math.max(0, playerCharacter.x - speed)
                            directionIndicator.rotation = 90
                            event.accepted = true
                            break
                        case Qt.Key_D:
                        case Qt.Key_Right:
                            playerCharacter.x = Math.min(gameScene.width - playerCharacter.width, playerCharacter.x + speed)
                            directionIndicator.rotation = 270
                            event.accepted = true
                            break
                    }
                }

                // 点击场景获取焦点
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        gameScene.forceActiveFocus()
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
        anchors.left: parent.left
        anchors.leftMargin: 300
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