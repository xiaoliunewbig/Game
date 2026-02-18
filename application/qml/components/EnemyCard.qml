// ============================================================================
// 《幻境传说》游戏框架 - 敌人信息卡片组件
// ============================================================================
// 文件名: EnemyCard.qml
// 说明: 敌人信息卡片，显示敌人名称、等级、血量、元素和状态效果
// 作者: 彭承康
// 创建时间: 2026-02-18
// ============================================================================

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: enemyCard

    width: 220
    height: 120
    radius: 8
    color: mouseArea.containsMouse ? "#3d2c2c" : "#34292e"
    border.color: getElementColor(elementType)
    border.width: 2

    // 公共属性
    property string enemyName: ""
    property int enemyLevel: 1
    property int currentHealth: 100
    property int maxHealth: 100
    property string elementType: ""
    property var statusEffects: []
    property bool isTargeted: false

    readonly property real healthPercent: maxHealth > 0 ? currentHealth / maxHealth : 0

    // 信号
    signal enemyClicked()
    signal enemyTargeted()

    // 主内容
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4

        // 顶部：名称 + 等级 + 元素
        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            // 元素图标
            Text {
                text: getElementIcon(enemyCard.elementType)
                font.pixelSize: 16
                visible: enemyCard.elementType !== ""
            }

            // 敌人名称
            Text {
                text: enemyCard.enemyName
                color: "#ecf0f1"
                font.pixelSize: 13
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            // 等级标签
            Rectangle {
                width: lvlText.width + 10
                height: 18
                radius: 3
                color: "#7f8c8d"

                Text {
                    id: lvlText
                    anchors.centerIn: parent
                    text: "Lv." + enemyCard.enemyLevel
                    color: "white"
                    font.pixelSize: 10
                    font.bold: true
                }
            }
        }

        // HP进度条
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 16
            radius: 4
            color: "#1a1a2e"
            border.color: "#555"
            border.width: 1

            // HP填充
            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: 1
                width: Math.max(0, (parent.width - 2) * enemyCard.healthPercent)
                radius: 3
                color: getHealthColor(enemyCard.healthPercent)

                Behavior on width {
                    NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
                }

                Behavior on color {
                    ColorAnimation { duration: 300 }
                }
            }

            // HP数值文字
            Text {
                anchors.centerIn: parent
                text: enemyCard.currentHealth + " / " + enemyCard.maxHealth
                color: "white"
                font.pixelSize: 10
                font.bold: true
                style: Text.Outline
                styleColor: "#000000"
            }
        }

        // 状态效果行
        Row {
            Layout.fillWidth: true
            Layout.preferredHeight: 20
            spacing: 4
            visible: enemyCard.statusEffects.length > 0

            Repeater {
                model: enemyCard.statusEffects

                Rectangle {
                    width: 20
                    height: 20
                    radius: 3
                    color: getStatusColor(modelData.type)
                    border.color: Qt.lighter(getStatusColor(modelData.type), 1.3)
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: getStatusIcon(modelData.type)
                        font.pixelSize: 12
                    }

                    // 提示
                    ToolTip.visible: statusMouseArea.containsMouse
                    ToolTip.text: modelData.name || modelData.type
                    ToolTip.delay: 300

                    MouseArea {
                        id: statusMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                    }
                }
            }
        }

        // 底部空白占位 (无状态效果时)
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 4
            visible: enemyCard.statusEffects.length === 0
        }
    }

    // 选中指示器
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        border.color: "#e74c3c"
        border.width: 3
        visible: enemyCard.isTargeted

        SequentialAnimation on border.color {
            running: enemyCard.isTargeted
            loops: Animation.Infinite
            ColorAnimation { to: "#ff6b6b"; duration: 800 }
            ColorAnimation { to: "#e74c3c"; duration: 800 }
        }
    }

    // 交互
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            enemyCard.enemyClicked()
        }

        onDoubleClicked: {
            enemyCard.enemyTargeted()
        }

        onEntered: enemyCard.scale = 1.03
        onExited: enemyCard.scale = 1.0
    }

    // 动画
    Behavior on scale { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
    Behavior on color { ColorAnimation { duration: 200 } }
    Behavior on border.color { ColorAnimation { duration: 200 } }

    // 工具函数
    function getHealthColor(percent) {
        if (percent > 0.5) return "#2ecc71"
        if (percent > 0.2) return "#f1c40f"
        return "#e74c3c"
    }

    function getElementColor(element) {
        switch(element) {
            case "fire":    return "#e74c3c"
            case "water":   return "#3498db"
            case "earth":   return "#8B7355"
            case "wind":    return "#2ecc71"
            case "light":   return "#f1c40f"
            case "dark":    return "#9b59b6"
            case "ice":     return "#00bcd4"
            case "thunder": return "#ff9800"
            default:        return "#7f8c8d"
        }
    }

    function getElementIcon(element) {
        switch(element) {
            case "fire":    return "\uD83D\uDD25"
            case "water":   return "\uD83D\uDCA7"
            case "earth":   return "\uD83C\uDF0D"
            case "wind":    return "\uD83C\uDF2C\uFE0F"
            case "light":   return "\u2728"
            case "dark":    return "\uD83C\uDF11"
            case "ice":     return "\u2744\uFE0F"
            case "thunder": return "\u26A1"
            default:        return ""
        }
    }

    function getStatusColor(statusType) {
        switch(statusType) {
            case "burn":    return "#e74c3c"
            case "freeze":  return "#3498db"
            case "poison":  return "#2ecc71"
            case "stun":    return "#f1c40f"
            case "bleed":   return "#c0392b"
            case "slow":    return "#9b59b6"
            case "buff":    return "#27ae60"
            case "debuff":  return "#e67e22"
            default:        return "#7f8c8d"
        }
    }

    function getStatusIcon(statusType) {
        switch(statusType) {
            case "burn":    return "\uD83D\uDD25"
            case "freeze":  return "\u2744\uFE0F"
            case "poison":  return "\u2620\uFE0F"
            case "stun":    return "\u2B50"
            case "bleed":   return "\uD83E\uDE78"
            case "slow":    return "\uD83D\uDC22"
            case "buff":    return "\u2B06\uFE0F"
            case "debuff":  return "\u2B07\uFE0F"
            default:        return "\u2753"
        }
    }

    function setEnemyData(data) {
        if (data) {
            enemyName = data.name || ""
            enemyLevel = data.level || 1
            currentHealth = data.health || 0
            maxHealth = data.maxHealth || 100
            elementType = data.element || ""
            statusEffects = data.statuses || []
        }
    }

    function takeDamage(amount) {
        currentHealth = Math.max(0, currentHealth - amount)
    }

    function heal(amount) {
        currentHealth = Math.min(maxHealth, currentHealth + amount)
    }
}
