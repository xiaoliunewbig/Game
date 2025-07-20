/*
 * 文件名: PlayerInfoPanel.qml
 * 说明: 玩家信息面板组件
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import GameUI 1.0

/**
 * @brief 玩家信息面板
 * 
 * 显示玩家的基本信息：
 * - 头像和名称
 * - 等级和经验
 * - 生命值和魔法值
 * - 基础属性
 */
Rectangle {
    id: playerInfoPanel
    
    color: "#3a3a3a"
    border.color: "#555555"
    border.width: 1
    radius: 8
    
    // 模拟玩家数据（实际应从Player对象获取）
    property string playerName: "测试玩家"
    property int playerLevel: 15
    property int currentExp: 1250
    property int maxExp: 1500
    property int currentHealth: 180
    property int maxHealth: 200
    property int currentMana: 85
    property int maxMana: 120
    property int attack: 45
    property int defense: 32
    property int speed: 28
    property int luck: 15
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 12
        
        // 玩家头像和基本信息
        RowLayout {
            spacing: 12
            
            // 头像
            Rectangle {
                width: 60
                height: 60
                color: "#555555"
                radius: 30
                border.color: "#FFD700"
                border.width: 2
                
                Image {
                    anchors.fill: parent
                    anchors.margins: 3
                    source: "qrc:/resources/images/player_avatar.png"
                    fillMode: Image.PreserveAspectCrop
                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: Rectangle {
                            width: parent.width
                            height: parent.height
                            radius: width / 2
                        }
                    }
                }
            }
            
            // 名称和等级
            ColumnLayout {
                spacing: 4
                
                Text {
                    text: playerName
                    font.family: "Microsoft YaHei"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#FFFFFF"
                }
                
                Text {
                    text: `等级 ${playerLevel}`
                    font.pixelSize: 14
                    color: "#FFD700"
                }
            }
        }
        
        // 经验条
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            
            RowLayout {
                Layout.fillWidth: true
                
                Text {
                    text: "经验"
                    font.pixelSize: 12
                    color: "#CCCCCC"
                }
                
                Text {
                    text: `${currentExp}/${maxExp}`
                    font.pixelSize: 12
                    color: "#CCCCCC"
                    Layout.alignment: Qt.AlignRight
                }
            }
            
            ProgressBar {
                id: expBar
                Layout.fillWidth: true
                from: 0
                to: maxExp
                value: currentExp
                
                background: Rectangle {
                    color: "#2a2a2a"
                    radius: 4
                    border.color: "#555555"
                    border.width: 1
                }
                
                contentItem: Rectangle {
                    color: "#4CAF50"
                    radius: 4
                    
                    Rectangle {
                        anchors.fill: parent
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#66BB6A" }
                            GradientStop { position: 1.0; color: "#388E3C" }
                        }
                        radius: 4
                    }
                }
            }
        }
        
        // 生命值条
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            
            RowLayout {
                Layout.fillWidth: true
                
                Text {
                    text: "生命"
                    font.pixelSize: 12
                    color: "#CCCCCC"
                }
                
                Text {
                    text: `${currentHealth}/${maxHealth}`
                    font.pixelSize: 12
                    color: "#CCCCCC"
                    Layout.alignment: Qt.AlignRight
                }
            }
            
            ProgressBar {
                id: healthBar
                Layout.fillWidth: true
                from: 0
                to: maxHealth
                value: currentHealth
                
                background: Rectangle {
                    color: "#2a2a2a"
                    radius: 4
                    border.color: "#555555"
                    border.width: 1
                }
                
                contentItem: Rectangle {
                    color: currentHealth / maxHealth > 0.3 ? "#F44336" : "#FF5722"
                    radius: 4
                    
                    Rectangle {
                        anchors.fill: parent
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#EF5350" }
                            GradientStop { position: 1.0; color: "#C62828" }
                        }
                        radius: 4
                    }
                }
            }
        }
        
        // 魔法值条
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            
            RowLayout {
                Layout.fillWidth: true
                
                Text {
                    text: "魔法"
                    font.pixelSize: 12
                    color: "#CCCCCC"
                }
                
                Text {
                    text: `${currentMana}/${maxMana}`
                    font.pixelSize: 12
                    color: "#CCCCCC"
                    Layout.alignment: Qt.AlignRight
                }
            }
            
            ProgressBar {
                id: manaBar
                Layout.fillWidth: true
                from: 0
                to: maxMana
                value: currentMana
                
                background: Rectangle {
                    color: "#2a2a2a"
                    radius: 4
                    border.color: "#555555"
                    border.width: 1
                }
                
                contentItem: Rectangle {
                    color: "#2196F3"
                    radius: 4
                    
                    Rectangle {
                        anchors.fill: parent
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#42A5F5" }
                            GradientStop { position: 1.0; color: "#1565C0" }
                        }
                        radius: 4
                    }
                }
            }
        }
        
        // 属性信息
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 15
            rowSpacing: 6
            
            // 攻击力
            Text {
                text: "攻击:"
                font.pixelSize: 12
                color: "#CCCCCC"
            }
            
            Text {
                text: attack.toString()
                font.pixelSize: 12
                color: "#FFFFFF"
                font.bold: true
                Layout.alignment: Qt.AlignRight
            }
            
            // 防御力
            Text {
                text: "防御:"
                font.pixelSize: 12
                color: "#CCCCCC"
            }
            
            Text {
                text: defense.toString()
                font.pixelSize: 12
                color: "#FFFFFF"
                font.bold: true
                Layout.alignment: Qt.AlignRight
            }
            
            // 速度
            Text {
                text: "速度:"
                font.pixelSize: 12
                color: "#CCCCCC"
            }
            
            Text {
                text: speed.toString()
                font.pixelSize: 12
                color: "#FFFFFF"
                font.bold: true
                Layout.alignment: Qt.AlignRight
            }
            
            // 幸运
            Text {
                text: "幸运:"
                font.pixelSize: 12
                color: "#CCCCCC"
            }
            
            Text {
                text: luck.toString()
                font.pixelSize: 12
                color: "#FFFFFF"
                font.bold: true
                Layout.alignment: Qt.AlignRight
            }
        }
    }
    
    // 数值变化动画
    Behavior on currentHealth {
        NumberAnimation {
            duration: 300
            easing.type: Easing.OutQuad
        }
    }
    
    Behavior on currentMana {
        NumberAnimation {
            duration: 300
            easing.type: Easing.OutQuad
        }
    }
    
    Behavior on currentExp {
        NumberAnimation {
            duration: 500
            easing.type: Easing.OutQuad
        }
    }
}
