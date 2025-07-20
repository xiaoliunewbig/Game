/*
 * 文件名: SaveSlotItem.qml
 * 说明: 存档槽位项目组件
 * 作者: 彭承康
 * 创建时间: 2025-07-20
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * @brief 存档槽位项目
 * 
 * 显示单个存档槽位的信息，包括：
 * - 存档是否存在
 * - 玩家信息（名称、等级、职业）
 * - 游戏进度
 * - 保存时间
 */
Rectangle {
    id: saveSlotItem
    
    // 自定义属性
    property int slotIndex: 0
    property bool hasData: false // 模拟数据，实际应从SaveManager获取
    property string playerName: hasData ? "测试玩家" : ""
    property int playerLevel: hasData ? 15 : 0
    property string profession: hasData ? "战士" : ""
    property int gameProgress: hasData ? 35 : 0
    property string saveTime: hasData ? "2025-07-20 14:30" : ""
    
    // 信号
    signal loadRequested(int slotIndex)
    signal deleteRequested(int slotIndex)
    
    // 外观
    height: 80
    color: mouseArea.containsMouse ? "#4a4a4a" : "#3a3a3a"
    border.color: "#555555"
    border.width: 1
    radius: 5
    
    // 颜色动画
    Behavior on color {
        ColorAnimation {
            duration: 200
            easing.type: Easing.OutQuad
        }
    }
    
    // 内容布局
    RowLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15
        
        // 槽位编号
        Rectangle {
            width: 40
            height: 40
            color: "#555555"
            radius: 20
            Layout.alignment: Qt.AlignVCenter
            
            Text {
                anchors.centerIn: parent
                text: (slotIndex + 1).toString()
                font.pixelSize: 16
                font.bold: true
                color: "#FFFFFF"
            }
        }
        
        // 存档信息
        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 5
            
            // 玩家信息行
            RowLayout {
                spacing: 10
                
                Text {
                    text: hasData ? playerName : "空存档"
                    font.pixelSize: 16
                    font.bold: true
                    color: hasData ? "#FFFFFF" : "#888888"
                }
                
                Text {
                    text: hasData ? `Lv.${playerLevel}` : ""
                    font.pixelSize: 14
                    color: "#FFD700"
                    visible: hasData
                }
                
                Text {
                    text: hasData ? profession : ""
                    font.pixelSize: 14
                    color: "#CCCCCC"
                    visible: hasData
                }
            }
            
            // 进度和时间行
            RowLayout {
                spacing: 15
                visible: hasData
                
                Text {
                    text: `进度: ${gameProgress}%`
                    font.pixelSize: 12
                    color: "#CCCCCC"
                }
                
                Text {
                    text: saveTime
                    font.pixelSize: 12
                    color: "#888888"
                }
            }
        }
        
        // 操作按钮
        RowLayout {
            spacing: 10
            Layout.alignment: Qt.AlignVCenter
            
            // 加载按钮
            Button {
                text: hasData ? "加载" : "新建"
                enabled: true
                
                background: Rectangle {
                    color: parent.pressed ? "#2a5a2a" : 
                           (parent.hovered ? "#3a7a3a" : "#2a6a2a")
                    border.color: "#4a8a4a"
                    border.width: 1
                    radius: 4
                    
                    Behavior on color {
                        ColorAnimation { duration: 150 }
                    }
                }
                
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 12
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    saveSlotItem.loadRequested(slotIndex)
                }
            }
            
            // 删除按钮
            Button {
                text: "删除"
                visible: hasData
                
                background: Rectangle {
                    color: parent.pressed ? "#5a2a2a" : 
                           (parent.hovered ? "#7a3a3a" : "#6a2a2a")
                    border.color: "#8a4a4a"
                    border.width: 1
                    radius: 4
                    
                    Behavior on color {
                        ColorAnimation { duration: 150 }
                    }
                }
                
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 12
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    deleteConfirmDialog.open()
                }
            }
        }
    }
    
    // 鼠标交互
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        
        onDoubleClicked: {
            if (hasData) {
                saveSlotItem.loadRequested(slotIndex)
            }
        }
    }
    
    // 删除确认对话框
    Dialog {
        id: deleteConfirmDialog
        title: "确认删除"
        modal: true
        anchors.centerIn: parent
        
        background: Rectangle {
            color: "#2a2a2a"
            border.color: "#555555"
            border.width: 2
            radius: 10
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20
            
            Text {
                text: `确定要删除存档 ${slotIndex + 1} 吗？\n此操作无法撤销。`
                color: "#FFFFFF"
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                Layout.alignment: Qt.AlignHCenter
            }
            
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 20
                
                Button {
                    text: "确定"
                    onClicked: {
                        saveSlotItem.deleteRequested(slotIndex)
                        deleteConfirmDialog.close()
                    }
                }
                
                Button {
                    text: "取消"
                    onClicked: deleteConfirmDialog.close()
                }
            }
        }
    }
    
    // 模拟数据（实际应该从SaveManager获取）
    Component.onCompleted: {
        // 模拟某些槽位有数据
        hasData = (slotIndex % 3 === 0) && (slotIndex < 6)
    }
}
