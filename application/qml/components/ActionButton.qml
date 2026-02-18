// ============================================================================
// 《幻境传说》游戏框架 - 动作按钮组件
// ============================================================================
// 文件名: ActionButton.qml
// 说明: 带冷却遮罩的动作按钮，用于技能、物品使用等操作
// 作者: 彭承康
// 创建时间: 2026-02-18
// ============================================================================

import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: actionButton

    width: 60
    height: 70
    radius: 8
    color: {
        if (!actionButton.enabled) return "#2c3e50"
        if (mouseArea.pressed) return "#2471a3"
        if (!isReady) return "#7f8c8d"
        if (mouseArea.containsMouse) return "#2980b9"
        return "#3498db"
    }
    border.color: isReady && actionButton.enabled ? "#5dade2" : "#95a5a6"
    border.width: 1
    opacity: actionButton.enabled ? 1.0 : 0.5

    // 公共属性
    property int actionId: 0
    property string actionName: ""
    property string actionIcon: ""
    property int cooldownTotal: 0
    property int cooldownRemaining: 0
    property bool enabled: true
    property string shortcutKey: ""

    readonly property bool isReady: cooldownRemaining <= 0

    // 信号
    signal actionTriggered(int actionId)

    // 图标
    Text {
        id: iconText
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter
        text: actionButton.actionIcon
        font.pixelSize: 24
        color: isReady ? "white" : "#bdc3c7"
    }

    // 动作名称
    Text {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 6
        anchors.horizontalCenter: parent.horizontalCenter
        text: actionButton.actionName
        font.pixelSize: 10
        font.bold: true
        color: isReady ? "white" : "#bdc3c7"
        elide: Text.ElideRight
        width: parent.width - 8
        horizontalAlignment: Text.AlignHCenter
    }

    // 快捷键标签
    Text {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 2
        anchors.rightMargin: 4
        text: actionButton.shortcutKey
        font.pixelSize: 8
        color: "#95a5a6"
        visible: actionButton.shortcutKey !== ""
    }

    // 冷却遮罩
    Rectangle {
        id: cooldownOverlay
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: cooldownTotal > 0 ? (cooldownRemaining / cooldownTotal) * parent.height : 0
        radius: parent.radius
        color: "#000000"
        opacity: 0.6
        visible: !isReady

        Behavior on height {
            NumberAnimation { duration: 100 }
        }
    }

    // 冷却时间文字
    Text {
        anchors.centerIn: parent
        text: Math.ceil(cooldownRemaining / 1000.0).toString() + "s"
        font.pixelSize: 16
        font.bold: true
        color: "#ecf0f1"
        visible: !isReady
        z: 2
    }

    // 冷却计时器
    Timer {
        id: cooldownTimer
        interval: 100
        repeat: true
        running: cooldownRemaining > 0

        onTriggered: {
            cooldownRemaining = Math.max(0, cooldownRemaining - 100)
        }
    }

    // 交互
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            if (isReady && actionButton.enabled) {
                actionButton.actionTriggered(actionButton.actionId)
            }
        }

        onEntered: {
            if (isReady && actionButton.enabled) {
                actionButton.scale = 1.08
            }
        }

        onExited: {
            actionButton.scale = 1.0
        }
    }

    // 动画
    Behavior on scale { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
    Behavior on color { ColorAnimation { duration: 200 } }
    Behavior on opacity { NumberAnimation { duration: 200 } }

    // 启动冷却
    function startCooldown(durationMs) {
        cooldownTotal = durationMs
        cooldownRemaining = durationMs
    }

    // 重置冷却
    function resetCooldown() {
        cooldownTotal = 0
        cooldownRemaining = 0
    }
}
