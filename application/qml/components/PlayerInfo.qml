// ============================================================================
// 《幻境传说》游戏框架 - 玩家信息组件
// ============================================================================
// 文件名: PlayerInfo.qml
// 说明: 显示玩家基本信息的UI组件，包括头像、等级、生命值、魔法值等
// 作者: 彭承康
// 创建时间: 2025-07-20
// 版本: v1.0.0
//
// 功能描述:
// - 玩家头像和名称显示
// - 等级和经验值进度条
// - 生命值和魔法值条
// - 状态效果图标显示
// - 实时数据更新和动画效果
//
// 使用场景:
// - 游戏主界面的玩家信息面板
// - 战斗界面的状态显示
// - 角色界面的详细信息
// ============================================================================

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/**
 * @brief 玩家信息面板组件
 * 
 * 显示玩家的基本信息，包括：
 * - 头像和姓名
 * - 等级和经验值
 * - 生命值和魔法值
 * - 属性统计
 */
Rectangle {
    id: root
    
    // 组件属性
    property alias playerName: nameLabel.text
    property alias playerLevel: levelLabel.text
    property alias avatarSource: avatarImage.source
    property int currentHealth: 100
    property int maxHealth: 100
    property int currentMana: 50
    property int maxMana: 50
    property int currentExp: 0
    property int maxExp: 1000
    
    // 外观属性
    width: 280
    height: 200
    color: "#2c3e50"
    radius: 8
    border.color: "#34495e"
    border.width: 2
    
    // 主布局
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10
        
        // 头像和基本信息行
        RowLayout {
            Layout.fillWidth: true
            spacing: 15
            
            // 头像容器
            Rectangle {
                id: avatarContainer
                Layout.preferredWidth: 80
                Layout.preferredHeight: 80
                Layout.alignment: Qt.AlignVCenter
                
                color: "#ecf0f1"
                radius: 40
                border.color: "#bdc3c7"
                border.width: 3
                
                // 头像图片
                Image {
                    id: avatarImage
                    anchors.centerIn: parent
                    width: 70
                    height: 70
                    source: "qrc:/resources/images/player.png"
                    fillMode: Image.PreserveAspectCrop
                    
                    // 圆形裁剪
                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: Rectangle {
                            width: avatarImage.width
                            height: avatarImage.height
                            radius: width / 2
                        }
                    }
                    
                    // 加载失败时显示默认图标
                    onStatusChanged: {
                        if (status === Image.Error) {
                            source = ""
                        }
                    }
                }
                
                // 默认头像文本（当图片加载失败时显示）
                Text {
                    anchors.centerIn: parent
                    text: "👤"
                    font.pixelSize: 40
                    visible: avatarImage.source === ""
                    color: "#7f8c8d"
                }
            }
            
            // 玩家信息列
            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                spacing: 5
                
                // 玩家姓名
                Text {
                    id: nameLabel
                    text: "未知玩家"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#f39c12"
                    Layout.fillWidth: true
                }
                
                // 玩家等级
                Text {
                    id: levelLabel
                    text: "等级 1"
                    font.pixelSize: 14
                    color: "#3498db"
                    Layout.fillWidth: true
                }
            }
        }
        
        // 属性条区域
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8
            
            // 生命值条
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                
                Text {
                    text: "生命值"
                    font.pixelSize: 12
                    color: "#ecf0f1"
                }
                
                ProgressBar {
                    id: healthBar
                    Layout.fillWidth: true
                    Layout.preferredHeight: 20
                    
                    from: 0
                    to: root.maxHealth
                    value: root.currentHealth
                    
                    background: Rectangle {
                        color: "#34495e"
                        radius: 10
                        border.color: "#2c3e50"
                        border.width: 1
                    }
                    
                    contentItem: Rectangle {
                        width: healthBar.visualPosition * parent.width
                        height: parent.height
                        radius: 10
                        color: "#e74c3c"
                        
                        Text {
                            anchors.centerIn: parent
                            text: root.currentHealth + "/" + root.maxHealth
                            font.pixelSize: 10
                            color: "white"
                        }
                    }
                }
            }
            
            // 魔法值条
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                
                Text {
                    text: "魔法值"
                    font.pixelSize: 12
                    color: "#ecf0f1"
                }
                
                ProgressBar {
                    id: manaBar
                    Layout.fillWidth: true
                    Layout.preferredHeight: 20
                    
                    from: 0
                    to: root.maxMana
                    value: root.currentMana
                    
                    background: Rectangle {
                        color: "#34495e"
                        radius: 10
                        border.color: "#2c3e50"
                        border.width: 1
                    }
                    
                    contentItem: Rectangle {
                        width: manaBar.visualPosition * parent.width
                        height: parent.height
                        radius: 10
                        color: "#3498db"
                        
                        Text {
                            anchors.centerIn: parent
                            text: root.currentMana + "/" + root.maxMana
                            font.pixelSize: 10
                            color: "white"
                        }
                    }
                }
            }
            
            // 经验值条
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                
                Text {
                    text: "经验值"
                    font.pixelSize: 12
                    color: "#ecf0f1"
                }
                
                ProgressBar {
                    id: expBar
                    Layout.fillWidth: true
                    Layout.preferredHeight: 20
                    
                    from: 0
                    to: root.maxExp
                    value: root.currentExp
                    
                    background: Rectangle {
                        color: "#34495e"
                        radius: 10
                        border.color: "#2c3e50"
                        border.width: 1
                    }
                    
                    contentItem: Rectangle {
                        width: expBar.visualPosition * parent.width
                        height: parent.height
                        radius: 10
                        color: "#27ae60"
                        
                        Text {
                            anchors.centerIn: parent
                            text: root.currentExp + "/" + root.maxExp
                            font.pixelSize: 10
                            color: "white"
                        }
                    }
                }
            }
        }
    }
    
    // 更新方法
    function updatePlayerInfo(name, level, health, maxHp, mana, maxMp, exp, maxXp) {
        playerName = name
        playerLevel = "等级 " + level
        currentHealth = health
        maxHealth = maxHp
        currentMana = mana
        maxMana = maxMp
        currentExp = exp
        maxExp = maxXp
    }
}
