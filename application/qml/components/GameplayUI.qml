/*
 * 文件名: GameplayUI.qml
 * 说明: 游戏界面覆盖层组件
 * 作者: 彭承康
 * 创建时间: 2026-02-18
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * @brief 游戏界面覆盖层
 * 
 * 显示在游戏场景之上的UI元素：
 * - 菜单按钮
 * - 系统消息
 * - 对话框
 * - 临时提示
 */
Item {
    id: gameplayUI
    
    // 信号定义
    signal menuRequested()
    
    // 右上角菜单按钮
    Button {
        id: menuButton
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 20
        width: 50
        height: 50
        
        background: Rectangle {
            color: parent.pressed ? "#3a3a3a" : 
                   (parent.hovered ? "#5a5a5a" : "#4a4a4a")
            border.color: "#666666"
            border.width: 2
            radius: 25
            opacity: 0.8
            
            Behavior on color {
                ColorAnimation { duration: 150 }
            }
        }
        
        contentItem: Text {
            text: "☰"
            font.pixelSize: 20
            color: "#FFFFFF"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        
        onClicked: gameplayUI.menuRequested()
        
        ToolTip {
            text: "菜单 (ESC)"
            visible: parent.hovered
            delay: 1000
        }
    }
    
    // 系统消息区域
    Column {
        id: messageArea
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 20
        spacing: 10
        width: 300
        
        // 消息项目模型
        Repeater {
            id: messageRepeater
            model: ListModel {
                id: messageModel
            }
            
            delegate: Rectangle {
                width: messageArea.width
                height: messageText.height + 20
                color: getMessageColor(model.type)
                border.color: Qt.darker(color, 1.3)
                border.width: 1
                radius: 6
                opacity: model.opacity || 1.0
                
                Text {
                    id: messageText
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.margins: 10
                    text: model.message
                    font.pixelSize: 12
                    color: "#FFFFFF"
                    wrapMode: Text.WordWrap
                }
                
                // 自动消失动画
                SequentialAnimation {
                    running: model.autoHide
                    
                    PauseAnimation { duration: model.duration || 3000 }
                    
                    NumberAnimation {
                        target: parent
                        property: "opacity"
                        to: 0
                        duration: 500
                        easing.type: Easing.OutQuad
                    }
                    
                    ScriptAction {
                        script: messageModel.remove(index)
                    }
                }
            }
        }
    }
    
    // 对话框区域
    Rectangle {
        id: dialogArea
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 150
        color: "#2a2a2a"
        border.color: "#555555"
        border.width: 2
        opacity: 0.95
        visible: false
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15
            
            // 说话者名称
            Text {
                id: speakerName
                text: "NPC名称"
                font.family: "Microsoft YaHei"
                font.pixelSize: 16
                font.bold: true
                color: "#FFD700"
            }
            
            // 对话内容
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                
                Text {
                    id: dialogText
                    text: "这里是对话内容..."
                    font.family: "Microsoft YaHei"
                    font.pixelSize: 14
                    color: "#FFFFFF"
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
            }
            
            // 对话选项
            Row {
                Layout.alignment: Qt.AlignRight
                spacing: 10
                
                Button {
                    text: "继续"
                    onClicked: hideDialog()
                }
                
                Button {
                    text: "跳过"
                    onClicked: hideDialog()
                }
            }
        }
    }
    
    // 临时提示区域
    Item {
        id: tooltipArea
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        
        // 经验获得提示
        Text {
            id: expGainText
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -50
            text: ""
            font.pixelSize: 18
            font.bold: true
            color: "#4CAF50"
            opacity: 0
            
            // 经验获得动画
            SequentialAnimation {
                id: expGainAnimation
                
                ParallelAnimation {
                    NumberAnimation {
                        target: expGainText
                        property: "opacity"
                        to: 1.0
                        duration: 300
                        easing.type: Easing.OutQuad
                    }
                    
                    NumberAnimation {
                        target: expGainText
                        property: "anchors.verticalCenterOffset"
                        to: -100
                        duration: 1500
                        easing.type: Easing.OutQuad
                    }
                }
                
                NumberAnimation {
                    target: expGainText
                    property: "opacity"
                    to: 0
                    duration: 500
                    easing.type: Easing.OutQuad
                }
            }
        }
        
        // 伤害数字提示
        Text {
            id: damageText
            anchors.centerIn: parent
            text: ""
            font.pixelSize: 24
            font.bold: true
            color: "#F44336"
            opacity: 0
            
            // 伤害数字动画
            SequentialAnimation {
                id: damageAnimation
                
                ParallelAnimation {
                    NumberAnimation {
                        target: damageText
                        property: "opacity"
                        to: 1.0
                        duration: 200
                        easing.type: Easing.OutQuad
                    }
                    
                    NumberAnimation {
                        target: damageText
                        property: "scale"
                        from: 0.5
                        to: 1.2
                        duration: 300
                        easing.type: Easing.OutBack
                    }
                    
                    NumberAnimation {
                        target: damageText
                        property: "anchors.verticalCenterOffset"
                        to: -80
                        duration: 1000
                        easing.type: Easing.OutQuad
                    }
                }
                
                ParallelAnimation {
                    NumberAnimation {
                        target: damageText
                        property: "opacity"
                        to: 0
                        duration: 500
                        easing.type: Easing.OutQuad
                    }
                    
                    NumberAnimation {
                        target: damageText
                        property: "scale"
                        to: 1.0
                        duration: 500
                        easing.type: Easing.OutQuad
                    }
                }
            }
        }
    }
    
    // 获取消息颜色
    function getMessageColor(type) {
        switch (type) {
            case "info": return "#2196F3"
            case "warning": return "#FF9800"
            case "error": return "#F44336"
            case "success": return "#4CAF50"
            case "system": return "#9C27B0"
            default: return "#607D8B"
        }
    }
    
    // 显示系统消息
    function showMessage(message, type, duration, autoHide) {
        messageModel.append({
            message: message,
            type: type || "info",
            duration: duration || 3000,
            autoHide: autoHide !== false,
            opacity: 1.0
        })
    }
    
    // 显示对话框
    function showDialog(speaker, content) {
        speakerName.text = speaker
        dialogText.text = content
        dialogArea.visible = true
    }
    
    // 隐藏对话框
    function hideDialog() {
        dialogArea.visible = false
    }
    
    // 显示经验获得
    function showExpGain(amount) {
        expGainText.text = `+${amount} EXP`
        expGainText.anchors.verticalCenterOffset = -50
        expGainAnimation.start()
    }
    
    // 显示伤害数字
    function showDamage(amount, isCritical) {
        damageText.text = isCritical ? `${amount}!` : amount.toString()
        damageText.color = isCritical ? "#FF5722" : "#F44336"
        damageText.anchors.verticalCenterOffset = 0
        damageText.scale = 0.5
        damageAnimation.start()
    }
}
