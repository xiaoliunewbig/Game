/*
 * 文件名: MenuButton.qml
 * 说明: 主菜单按钮组件
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

/**
 * @brief 主菜单按钮组件
 * 
 * 提供统一样式的菜单按钮，支持：
 * - 图标和文字显示
 * - 悬停和点击效果
 * - 自定义样式
 */
Button {
    id: menuButton
    
    // 自定义属性
    property string iconSource: ""
    property color normalColor: "#3a3a3a"
    property color hoverColor: "#4a4a4a"
    property color pressedColor: "#2a2a2a"
    property color textColor: "#FFFFFF"
    property color borderColor: "#555555"
    
    // 按钮样式
    background: Rectangle {
        id: buttonBackground
        color: menuButton.pressed ? pressedColor : 
               (menuButton.hovered ? hoverColor : normalColor)
        border.color: borderColor
        border.width: 2
        radius: 8
        
        // 渐变效果
        gradient: Gradient {
            GradientStop { 
                position: 0.0
                color: Qt.lighter(buttonBackground.color, 1.2)
            }
            GradientStop { 
                position: 1.0
                color: Qt.darker(buttonBackground.color, 1.1)
            }
        }
        
        // 动画效果
        Behavior on color {
            ColorAnimation {
                duration: 200
                easing.type: Easing.OutQuad
            }
        }
        
        // 发光效果
        layer.enabled: menuButton.hovered
        layer.effect: DropShadow {
            horizontalOffset: 0
            verticalOffset: 0
            radius: 15
            samples: 31
            color: "#40FFFFFF"
        }
    }
    
    // 按钮内容
    contentItem: Row {
        spacing: 15
        anchors.centerIn: parent
        
        // 图标
        Image {
            id: buttonIcon
            source: iconSource
            width: 24
            height: 24
            visible: iconSource !== ""
            anchors.verticalCenter: parent.verticalCenter
            
            // 图标着色
            ColorOverlay {
                anchors.fill: buttonIcon
                source: buttonIcon
                color: textColor
                visible: buttonIcon.visible
            }
        }
        
        // 文字
        Text {
            text: menuButton.text
            font.family: "Microsoft YaHei"
            font.pixelSize: 18
            font.bold: true
            color: textColor
            anchors.verticalCenter: parent.verticalCenter
        }
    }
    
    // 鼠标悬停效果
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        
        onClicked: menuButton.clicked()
        
        onEntered: {
            scaleAnimation.to = 1.05
            scaleAnimation.start()
        }
        
        onExited: {
            scaleAnimation.to = 1.0
            scaleAnimation.start()
        }
    }
    
    // 缩放动画
    NumberAnimation {
        id: scaleAnimation
        target: menuButton
        property: "scale"
        duration: 150
        easing.type: Easing.OutQuad
    }
    
    // 点击动画
    SequentialAnimation {
        id: clickAnimation
        
        NumberAnimation {
            target: menuButton
            property: "scale"
            to: 0.95
            duration: 100
            easing.type: Easing.OutQuad
        }
        
        NumberAnimation {
            target: menuButton
            property: "scale"
            to: 1.0
            duration: 100
            easing.type: Easing.OutQuad
        }
    }
    
    // 连接点击信号到动画
    onClicked: clickAnimation.start()
}
