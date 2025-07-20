/*
 * 文件名: ProfessionCard.qml
 * 说明: 职业选择卡片组件
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

/**
 * @brief 职业选择卡片
 * 
 * 用于新游戏时的职业选择界面，显示：
 * - 职业图标
 * - 职业名称
 * - 职业描述
 * - 选中状态
 */
Rectangle {
    id: professionCard
    
    // 自定义属性
    property int profession: 0
    property string professionName: ""
    property string description: ""
    property string iconSource: ""
    property bool selected: false
    
    // 外观属性
    width: 140
    height: 180
    color: selected ? "#4a4a4a" : "#3a3a3a"
    border.color: selected ? "#FFD700" : "#555555"
    border.width: selected ? 3 : 1
    radius: 10
    
    // 选中状态变化动画
    Behavior on color {
        ColorAnimation {
            duration: 200
            easing.type: Easing.OutQuad
        }
    }
    
    Behavior on border.color {
        ColorAnimation {
            duration: 200
            easing.type: Easing.OutQuad
        }
    }
    
    Behavior on border.width {
        NumberAnimation {
            duration: 200
            easing.type: Easing.OutQuad
        }
    }
    
    // 发光效果
    layer.enabled: selected
    layer.effect: DropShadow {
        horizontalOffset: 0
        verticalOffset: 0
        radius: 20
        samples: 41
        color: "#80FFD700"
    }
    
    // 卡片内容
    Column {
        anchors.centerIn: parent
        spacing: 10
        
        // 职业图标
        Rectangle {
            width: 64
            height: 64
            color: "transparent"
            anchors.horizontalCenter: parent.horizontalCenter
            
            Image {
                id: professionIcon
                anchors.fill: parent
                source: iconSource
                fillMode: Image.PreserveAspectFit
            }
            
            // 图标发光效果
            ColorOverlay {
                anchors.fill: professionIcon
                source: professionIcon
                color: selected ? "#FFD700" : "#FFFFFF"
                
                Behavior on color {
                    ColorAnimation {
                        duration: 200
                        easing.type: Easing.OutQuad
                    }
                }
            }
        }
        
        // 职业名称
        Text {
            text: professionName
            font.family: "Microsoft YaHei"
            font.pixelSize: 16
            font.bold: true
            color: selected ? "#FFD700" : "#FFFFFF"
            anchors.horizontalCenter: parent.horizontalCenter
            
            Behavior on color {
                ColorAnimation {
                    duration: 200
                    easing.type: Easing.OutQuad
                }
            }
        }
        
        // 职业描述
        Text {
            text: description
            font.family: "Microsoft YaHei"
            font.pixelSize: 10
            color: "#CCCCCC"
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            width: professionCard.width - 20
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
    
    // 鼠标交互
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        
        onClicked: {
            // 取消其他卡片的选中状态
            let parent = professionCard.parent
            for (let i = 0; i < parent.children.length; i++) {
                let child = parent.children[i]
                if (child.hasOwnProperty("selected") && child !== professionCard) {
                    child.selected = false
                }
            }
            
            // 选中当前卡片
            professionCard.selected = true
            
            // 播放点击动画
            clickAnimation.start()
        }
        
        onEntered: {
            if (!selected) {
                hoverAnimation.to = 1.05
                hoverAnimation.start()
            }
        }
        
        onExited: {
            if (!selected) {
                hoverAnimation.to = 1.0
                hoverAnimation.start()
            }
        }
    }
    
    // 悬停动画
    NumberAnimation {
        id: hoverAnimation
        target: professionCard
        property: "scale"
        duration: 150
        easing.type: Easing.OutQuad
    }
    
    // 点击动画
    SequentialAnimation {
        id: clickAnimation
        
        NumberAnimation {
            target: professionCard
            property: "scale"
            to: 0.95
            duration: 100
            easing.type: Easing.OutQuad
        }
        
        NumberAnimation {
            target: professionCard
            property: "scale"
            to: 1.0
            duration: 100
            easing.type: Easing.OutQuad
        }
    }
}
