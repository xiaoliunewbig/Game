/*
 * 文件名: QuickActionButton.qml
 * 说明: 快捷操作按钮组件
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

/**
 * @brief 快捷操作按钮
 * 
 * 用于游戏界面左侧面板的快捷操作按钮，支持：
 * - 图标和文字显示
 * - 悬停效果
 * - 点击反馈
 * - 键盘快捷键提示
 */
Button {
    id: quickActionButton
    
    // 自定义属性
    property string iconSource: ""
    property string keyHint: ""
    property color normalColor: "#4a4a4a"
    property color hoverColor: "#5a5a5a"
    property color pressedColor: "#3a3a3a"
    
    // 按钮样式
    background: Rectangle {
        id: buttonBackground
        color: quickActionButton.pressed ? pressedColor : 
               (quickActionButton.hovered ? hoverColor : normalColor)
        border.color: "#666666"
        border.width: 1
        radius: 6
        
        // 颜色过渡动画
        Behavior on color {
            ColorAnimation {
                duration: 150
                easing.type: Easing.OutQuad
            }
        }
        
        // 悬停时的发光效果
        layer.enabled: quickActionButton.hovered
        layer.effect: DropShadow {
            horizontalOffset: 0
            verticalOffset: 0
            radius: 10
            samples: 21
            color: "#40FFFFFF"
        }
    }
    
    // 按钮内容
    contentItem: Row {
        spacing: 12
        anchors.left: parent.left
        anchors.leftMargin: 15
        anchors.verticalCenter: parent.verticalCenter
        
        // 图标
        Image {
            id: buttonIcon
            source: iconSource
            width: 20
            height: 20
            visible: iconSource !== ""
            anchors.verticalCenter: parent.verticalCenter
            
            // 图标着色
            ColorOverlay {
                anchors.fill: buttonIcon
                source: buttonIcon
                color: "#FFFFFF"
                visible: buttonIcon.visible
            }
        }
        
        // 文字
        Text {
            text: quickActionButton.text
            font.family: "Microsoft YaHei"
            font.pixelSize: 14
            color: "#FFFFFF"
            anchors.verticalCenter: parent.verticalCenter
        }
    }
    
    // 键盘快捷键提示
    Text {
        text: keyHint
        font.pixelSize: 10
        color: "#AAAAAA"
        anchors.right: parent.right
        anchors.rightMargin: 15
        anchors.verticalCenter: parent.verticalCenter
        visible: keyHint !== ""
        
        Rectangle {
            anchors.centerIn: parent
            width: parent.width + 8
            height: parent.height + 4
            color: "#555555"
            radius: 3
            border.color: "#777777"
            border.width: 1
            z: -1
        }
    }
    
    // 鼠标交互
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        
        onClicked: quickActionButton.clicked()
        
        onEntered: {
            scaleAnimation.to = 1.02
            scaleAnimation.start()
        }
        
        onExited: {
            scaleAnimation.to = 1.0
            scaleAnimation.start()
        }
        
        onPressed: {
            pressAnimation.start()
        }
    }
    
    // 缩放动画
    NumberAnimation {
        id: scaleAnimation
        target: quickActionButton
        property: "scale"
        duration: 100
        easing.type: Easing.OutQuad
    }
    
    // 按压动画
    SequentialAnimation {
        id: pressAnimation
        
        NumberAnimation {
            target: quickActionButton
            property: "scale"
            to: 0.98
            duration: 50
            easing.type: Easing.OutQuad
        }
        
        NumberAnimation {
            target: quickActionButton
            property: "scale"
            to: 1.0
            duration: 100
            easing.type: Easing.OutQuad
        }
    }
    
    // 工具提示
    ToolTip {
        id: toolTip
        text: quickActionButton.text + (keyHint !== "" ? ` (${keyHint})` : "")
        visible: parent.hovered
        delay: 1000
        timeout: 3000
        
        background: Rectangle {
            color: "#2a2a2a"
            border.color: "#555555"
            border.width: 1
            radius: 4
        }
        
        contentItem: Text {
            text: toolTip.text
            font.pixelSize: 12
            color: "#FFFFFF"
        }
    }
}
